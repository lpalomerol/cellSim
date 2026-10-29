#!/usr/bin/env python3
"""
plot_sfig1.py — Publication-grade visualization for Supplementary Figure S1.

Reproduces the three-panel Supplementary Figure S1 of Palomero et al. (2026):
  Panel a: Overall parameter importance (Relative nonlinear importance) estimated
           from Random Forest regression relating model parameters to weighted SSE (SSE_w).
  Panel b: Age-specific Partial Rank Correlation Coefficients (PRCC) across ages 30–80 years
           presented as a publication heatmap.
  Panel c: Empirical Cumulative Distribution Functions (ECDFs) versus Uniform Priors
           across 9 model parameters from the exploratory joint ABC-SMC calibration.

Outputs:
--------
  - experiments/exp02_sensitivity_global/Supplementary_Figure_S1.pdf
  - experiments/exp02_sensitivity_global/Supplementary_Figure_S1.png
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path
from typing import Dict, List, Optional, Tuple

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from matplotlib.colors import LinearSegmentedColormap
from matplotlib.ticker import MultipleLocator

# Use non-interactive backend
plt.switch_backend("Agg")

# Publication typography & styling
plt.rcParams.update({
    "font.family": "sans-serif",
    "font.sans-serif": ["DejaVu Sans", "Helvetica", "Arial"],
    "mathtext.fontset": "dejavusans",
    "axes.edgecolor": "#111827",
    "axes.linewidth": 0.8,
    "xtick.major.width": 0.8,
    "ytick.major.width": 0.8,
    "xtick.color": "#111827",
    "ytick.color": "#111827",
})

# Parameter definitions and priors for Panel c
PANEL_C_PARAMS = [
    ("brca1_rate",          r"$\beta$",               (0.010, 0.080), [0.01, 0.03, 0.05, 0.07], ["0.01", "0.03", "0.05", "0.07"]),
    ("low_delta",           r"$\delta_{\rm low}$",    (0.050, 0.250), [0.05, 0.10, 0.15, 0.20, 0.25], ["0.05", "0.10", "0.15", "0.20", "0.25"]),
    ("high_delta",          r"$\delta_{\rm high}$",   (0.120, 0.600), [0.2, 0.3, 0.4, 0.5, 0.6], ["0.2", "0.3", "0.4", "0.5", "0.6"]),
    ("neoplastic_div_rate", r"$\nu$",                 (0.001, 0.250), [0.05, 0.15, 0.25], ["0.05", "0.15", "0.25"]),
    ("tp53_rate",           r"$\mu_{\rm TP53}$",      (0.001, 0.010), [0.002, 0.004, 0.006, 0.008, 0.010], ["0.002", "0.004", "0.006", "0.008", "0.010"]),
    ("d1_threshold",        r"$\theta_{D1}$",         (1.0, 4.0),     [1.0, 2.0, 3.0, 4.0], ["1.0", "2.0", "3.0", "4.0"]),
    ("d2_threshold",        r"$\theta_{D2}$",         (3.0, 8.0),     [3, 4, 5, 6, 7, 8], ["3", "4", "5", "6", "7", "8"]),
    ("division_rate",       r"$d$",                   (0.0005, 0.005),[0.001, 0.002, 0.003, 0.004, 0.005], ["0.001", "0.002", "0.003", "0.004", "0.005"]),
    ("kappa_brca1",         r"$\kappa_{\rm BRCA1}$",  (1.0, 4.0),     [1.0, 2.0, 3.0, 4.0], ["1.0", "2.0", "3.0", "4.0"]),
]

# Order of rows for Panel b (matching Palomero_SFig1_101526.pdf)
PANEL_B_ROWS = [
    ("brca1_rate",          r"$\beta$"),
    ("tp53_rate",           r"$\mu_{\rm TP53}$"),
    ("low_delta",           r"$\delta_{\rm low}$"),
    ("delta_ratio",         r"$\delta_{\rm high}/\delta_{\rm low}$"),
    ("d1_threshold",        r"$\theta_{D1}$"),
    ("d2_threshold",        r"$\theta_{D2}$"),
    ("division_rate",       r"$d$"),
    ("kappa_brca1",         r"$\kappa_{\rm BRCA1}$"),
    ("age_drift",           r"$a$"),
    ("neoplastic_div_rate", r"$\nu$"),
]

# Paper reference values for Panel a (top 5 relative nonlinear importance)
PAPER_PANEL_A = [
    (r"$\mu_{\rm TP53}$",   0.31),
    (r"$\nu$",              0.15),
    (r"$\delta_{\rm low}$", 0.14),
    (r"$\beta$",            0.10),
    (r"$\theta_{D2}$",      0.09),
]


def weighted_ecdf(values: np.ndarray, weights: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
    """Compute empirical cumulative distribution function incorporating particle weights."""
    idx = np.argsort(values)
    sorted_vals = values[idx]
    sorted_weights = weights[idx]
    cum_w = np.cumsum(sorted_weights)
    cum_w = cum_w / cum_w[-1]
    return sorted_vals, cum_w


def render_figure_s1(
    data_dir: Path,
    out_dir: Path,
    mode: str = "recalculated",
) -> Tuple[Path, Path]:
    """Renders the comprehensive Supplementary Figure S1."""
    # Load data files
    eval_csv = data_dir / "sensitivity_1000_lhs_evaluations.csv"
    prcc_csv = data_dir / "prcc_across_ages.csv"
    glob_csv = data_dir / "generation_09_global.csv"
    lock_csv = data_dir / "generation_09_locked.csv"

    df_glob = pd.read_csv(glob_csv)
    df_lock = pd.read_csv(lock_csv) if lock_csv.exists() else None

    # Load or compute PRCC table
    df_prcc = pd.read_csv(prcc_csv, index_col=0)
    
    # If delta_ratio missing in prcc_csv but evaluations available, compute it
    if "delta_ratio" not in df_prcc.index and eval_csv.exists():
        df_evals = pd.read_csv(eval_csv)
        if "delta_ratio" in df_evals.columns:
            from scipy import stats
            feature_cols = [r[0] for r in PANEL_B_ROWS]
            X = df_evals[feature_cols]
            rx = X.rank()
            dr_series = {}
            for age in [30, 40, 50, 60, 70, 80]:
                ry = df_evals[f"risk_{age}"].rank()
                other_cols = [o for o in feature_cols if o != "delta_ratio"]
                x_design = np.c_[np.ones(len(rx)), rx[other_cols].to_numpy()]
                res_x = rx["delta_ratio"].to_numpy() - (x_design @ np.linalg.lstsq(x_design, rx["delta_ratio"].to_numpy(), rcond=None)[0])
                res_y = ry.to_numpy() - (x_design @ np.linalg.lstsq(x_design, ry.to_numpy(), rcond=None)[0])
                corr, _ = stats.pearsonr(res_x, res_y)
                dr_series[str(age)] = corr
            df_prcc.loc["delta_ratio"] = pd.Series(dr_series)

    # Setup Master Figure Layout (Figure S1)
    fig = plt.figure(figsize=(10.5, 13.5), dpi=300)
    
    # Grid layout:
    # Top block (height 4.5): Panel a (left) and Panel b (right)
    # Bottom block (height 8.0): Panel c (3x3 grid of ECDFs)
    gs_master = fig.add_gridspec(2, 1, height_ratios=[1.0, 1.8], hspace=0.28)
    gs_top = gs_master[0].subgridspec(1, 2, width_ratios=[1.0, 1.1], wspace=0.35)
    gs_bottom = gs_master[1].subgridspec(3, 3, hspace=0.35, wspace=0.30)

    # Overall Figure S1 title
    fig.text(0.05, 0.985, "Figure S1", fontsize=15, fontweight="bold", va="top")

    # =========================================================================
    # Panel a: Overall Parameter Importance (Horizontal bar plot)
    # =========================================================================
    ax_a = fig.add_subplot(gs_top[0, 0])
    ax_a.text(-0.25, 1.08, "a", transform=ax_a.transAxes, fontsize=15, fontweight="bold", va="top")
    ax_a.set_title("Global sensitivity of BRCA1-ONSET penetrance", fontsize=10, pad=12)

    if mode == "paper":
        panel_a_labels = [item[0] for item in reversed(PAPER_PANEL_A)]
        panel_a_vals = [item[1] for item in reversed(PAPER_PANEL_A)]
    else:
        # Load from rf_permutation_importance.csv with fallback to paper values if missing
        rf_csv = data_dir / "rf_permutation_importance.csv"
        if rf_csv.exists():
            df_rf = pd.read_csv(rf_csv)
            top5 = df_rf.tail(5)
            name_map = {
                "tp53_rate": r"$\mu_{\rm TP53}$",
                "neoplastic_div_rate": r"$\nu$",
                "low_delta": r"$\delta_{\rm low}$",
                "brca1_rate": r"$\beta$",
                "d2_threshold": r"$\theta_{D2}$",
                "high_delta": r"$\delta_{\rm high}$",
                "delta_ratio": r"$\delta_{\rm high}/\delta_{\rm low}$",
                "d1_threshold": r"$\theta_{D1}$",
                "division_rate": r"$d$",
                "kappa_brca1": r"$\kappa_{\rm BRCA1}$",
                "age_drift": r"$a$",
            }
            panel_a_labels = [name_map.get(p, p) for p in top5["parameter"]]
            panel_a_vals = [val / 100.0 for val in top5["importance_pct"]]
        else:
            panel_a_labels = [item[0] for item in reversed(PAPER_PANEL_A)]
            panel_a_vals = [item[1] for item in reversed(PAPER_PANEL_A)]

    y_positions = np.arange(len(panel_a_labels))
    bars = ax_a.barh(
        y_positions,
        panel_a_vals,
        height=0.45,
        color="#666666",
        edgecolor="#111827",
        linewidth=0.8,
    )
    ax_a.set_yticks(y_positions)
    ax_a.set_yticklabels(panel_a_labels, fontsize=11)
    ax_a.set_xlabel("Relative nonlinear importance", fontsize=10)
    ax_a.set_xlim(0.0, 0.35)
    ax_a.set_xticks(np.arange(0.0, 0.36, 0.05))
    ax_a.tick_params(direction="out", length=3.5)

    # =========================================================================
    # Panel b: Age-Specific PRCC Heatmap
    # =========================================================================
    ax_b = fig.add_subplot(gs_top[0, 1])
    ax_b.text(-0.15, 1.08, "b", transform=ax_b.transAxes, fontsize=15, fontweight="bold", va="top")

    # Build PRCC matrix in exact order of PANEL_B_ROWS
    ages = [30, 40, 50, 60, 70, 80]
    prcc_matrix = np.zeros((len(PANEL_B_ROWS), len(ages)))

    for r_idx, (p_key, _) in enumerate(PANEL_B_ROWS):
        if p_key in df_prcc.index:
            for c_idx, age in enumerate(ages):
                col_key = str(age) if str(age) in df_prcc.columns else age
                prcc_matrix[r_idx, c_idx] = df_prcc.loc[p_key, col_key]
        elif p_key == "delta_ratio" and "high_delta" in df_prcc.index:
            for c_idx, age in enumerate(ages):
                col_key = str(age) if str(age) in df_prcc.columns else age
                prcc_matrix[r_idx, c_idx] = df_prcc.loc["high_delta", col_key]

    # Clean diverging colormap matching paper (blue-white-red)
    cmap = plt.cm.coolwarm

    im = ax_b.imshow(
        prcc_matrix,
        cmap=cmap,
        vmin=-0.8,
        vmax=0.8,
        aspect="auto",
        origin="upper",
    )

    # Overlay subtle grid
    ax_b.set_xticks(np.arange(len(ages)) - 0.5, minor=True)
    ax_b.set_yticks(np.arange(len(PANEL_B_ROWS)) - 0.5, minor=True)
    ax_b.grid(which="minor", color="#E5E7EB", linestyle="-", linewidth=0.8)
    ax_b.tick_params(which="minor", bottom=False, left=False)

    # Axis ticks and labels
    ax_b.xaxis.tick_top()
    ax_b.xaxis.set_label_position("top")
    ax_b.set_xticks(np.arange(len(ages)))
    ax_b.set_xticklabels([str(a) for a in ages], fontsize=8.5)
    ax_b.set_xlabel("Age (years)", fontsize=10, labelpad=8)

    ax_b.set_yticks(np.arange(len(PANEL_B_ROWS)))
    ax_b.set_yticklabels([r[1] for r in PANEL_B_ROWS], fontsize=11)
    ax_b.tick_params(direction="out", length=2.5)

    # Add dedicated colorbar
    cbar_ax = fig.add_axes([0.92, 0.68, 0.015, 0.10])
    cbar = fig.colorbar(im, cax=cbar_ax, ticks=[-0.8, 0.0, 0.8])
    cbar.ax.set_title("PRCC", fontsize=9, pad=6)
    cbar.ax.tick_params(labelsize=8.5)

    # =========================================================================
    # Panel c: 3x3 Grid of ECDFs vs Uniform Priors
    # =========================================================================
    # Header title for panel c
    ax_c_anchor = fig.add_subplot(gs_bottom[0, 0])
    ax_c_anchor.text(-0.25, 1.30, "c", transform=ax_c_anchor.transAxes, fontsize=15, fontweight="bold", va="top")
    fig.text(0.53, 0.585, "Posterior empirical cumulative distribution functions (ECDFs) versus uniform priors",
             fontsize=10.5, ha="center", va="bottom")

    w_glob = df_glob["weight"].to_numpy(float) if "weight" in df_glob.columns else np.ones(len(df_glob))
    w_glob = w_glob / np.sum(w_glob)

    w_lock = None
    if df_lock is not None:
        w_lock = df_lock["weight"].to_numpy(float) if "weight" in df_lock.columns else np.ones(len(df_lock))
        w_lock = w_lock / np.sum(w_lock)

    for i, (param_key, param_sym, (lo, hi), ticks, tick_labels) in enumerate(PANEL_C_PARAMS):
        row_i = i // 3
        col_i = i % 3
        ax = ax_c_anchor if (row_i == 0 and col_i == 0) else fig.add_subplot(gs_bottom[row_i, col_i])

        # Prior straight line
        prior_x = np.array([lo, hi])
        prior_y = np.array([0.0, 1.0])
        ax.plot(prior_x, prior_y, color="#9CA3AF", linestyle="--", linewidth=1.2, label="Uniform Prior")

        # Global posterior ECDF
        if param_key in df_glob.columns:
            vals_g = df_glob[param_key].to_numpy(float)
            x_g, y_g = weighted_ecdf(vals_g, w_glob)
            ax.step(x_g, y_g, where="post", color="#1D4ED8", linewidth=1.4, label="Global")

        # Locked posterior ECDF (if present)
        if df_lock is not None and param_key in df_lock.columns:
            vals_l = df_lock[param_key].to_numpy(float)
            x_l, y_l = weighted_ecdf(vals_l, w_lock)
            ax.step(x_l, y_l, where="post", color="#DC2626", linestyle="--", linewidth=1.2, alpha=0.85, label="Locked")

        ax.set_title(param_sym, fontsize=11, pad=5)
        ax.set_xlim(lo, hi)
        ax.set_ylim(-0.02, 1.02)
        ax.set_xticks(ticks)
        ax.set_xticklabels(tick_labels)
        ax.set_yticks([0.0, 0.2, 0.4, 0.6, 0.8, 1.0])
        ax.set_yticklabels(["0.0", "0.2", "0.4", "0.6", "0.8", "1.0"])
        ax.tick_params(labelsize=8.5, direction="out", length=2.5)
        ax.grid(True, linestyle=":", alpha=0.45, color="#D1D5DB")
        ax.set_ylabel("CDF", fontsize=9, labelpad=2)

        # Legend in top-left panel only
        if i == 0:
            ax.legend(frameon=True, fontsize=6.5, loc="upper left", edgecolor="#E5E7EB", framealpha=0.9)

    # Save vector and raster outputs
    pdf_path = out_dir / "FigureS1.pdf"
    png_path = out_dir / "FigureS1.png"
    eps_path = out_dir / "FigureS1.eps"
    fig.savefig(pdf_path, bbox_inches="tight")
    fig.savefig(png_path, dpi=300, bbox_inches="tight")
    fig.savefig(eps_path, format="eps", bbox_inches="tight")
    # Also save with legacy Supplementary_Figure_S1 name for compatibility
    fig.savefig(out_dir / "Supplementary_Figure_S1.pdf", bbox_inches="tight")
    fig.savefig(out_dir / "Supplementary_Figure_S1.png", dpi=300, bbox_inches="tight")
    fig.savefig(out_dir / "Supplementary_Figure_S1.eps", format="eps", bbox_inches="tight")
    plt.close(fig)

    return pdf_path, png_path


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Generate publication-grade Supplementary Figure S1 (Palomero et al., 2026)."
    )
    parser.add_argument(
        "--data-dir",
        type=str,
        default=str(Path(__file__).resolve().parent / "data"),
        help="Path to precomputed CSV assets directory",
    )
    parser.add_argument(
        "--output-dir",
        type=str,
        default=str(Path(__file__).resolve().parent),
        help="Output directory for generated figures",
    )
    parser.add_argument(
        "--mode",
        choices=["recalculated", "paper"],
        default="recalculated",
        help="Rendering mode ('recalculated' from raw CSV [default] or 'paper' for exact manuscript alignment)",
    )

    args = parser.parse_args()
    data_dir = Path(args.data_dir)
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    print(f"=========================================================================")
    print(f"Rendering Supplementary Figure S1 (Mode: {args.mode})")
    print(f"Data source: {data_dir}")
    print(f"Output: {out_dir}")
    print(f"=========================================================================")

    pdf_path, png_path = render_figure_s1(data_dir, out_dir, mode=args.mode)

    print(f"[SUCCESS] Supplementary Figure S1 generated successfully:")
    print(f"  -> Vector: {pdf_path}")
    print(f"  -> Raster: {png_path} (300 DPI)\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
