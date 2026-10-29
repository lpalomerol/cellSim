#!/usr/bin/env python3
"""
plot_sfig2.py — Supplementary Figure S2: Fixed-parameter sensitivity analysis (6 panels a-f).

Faithfully reproduces Supplementary Figure S2 from Palomero et al. (2026) (Palomero_SFig2_092926.pdf):
  - Panel a: Tornado horizontal bar chart of maximum absolute change in predicted penetrance (%)
    for fixed parameters under +-10% and +-25% perturbations.
  - Panel b: Discrepancy with observed penetrance (observed-CI-weighted SSE, log scale) vs. perturbation percentage.
  - Panel c: Recalibration curves across the 3x3 TP53 x D2 grid under +-10% perturbations vs. Kuchenbaecker data.
  - Panel d: 95% posterior interval width / prior range for calibrated parameters under +-10% recalibration.
  - Panel e: Recalibration curves across the 3x3 TP53 x D2 grid under +-25% perturbations vs. Kuchenbaecker data.
  - Panel f: 95% posterior interval width / prior range for calibrated parameters under +-25% recalibration.

Outputs:
  - FigureS2.pdf
  - FigureS2.png
"""
from __future__ import annotations

import argparse
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# Kuchenbaecker et al. (2017) clinical BRCA1 data
AGES = np.array([30, 40, 50, 60, 70, 80], dtype=float)
OBSERVED = np.array([4.0, 26.0, 46.0, 58.0, 65.0, 70.0])
CI_LOWER = np.array([2.0, 22.0, 41.0, 52.0, 56.0, 60.0])
CI_UPPER = np.array([7.0, 30.0, 52.0, 65.0, 73.0, 80.0])

ORDERED_PARAMS = [
    ("d2_threshold", r"$\theta_{D2}$"),
    ("tp53_rate", r"$\mu_{\rm TP53}$"),
    ("d1_threshold", r"$\theta_{D1}$"),
    ("tumor_threshold", "Tumor-positive\nthreshold"),
    ("division_rate", r"$d$"),
    ("age_drift", "Age-drift\ncoefficient"),
]


def set_publication_style():
    plt.rcParams.update({
        "font.family": "sans-serif",
        "font.sans-serif": ["Arial", "Helvetica", "DejaVu Sans"],
        "font.size": 8.0,
        "axes.titlesize": 9.5,
        "axes.labelsize": 8.5,
        "xtick.labelsize": 7.5,
        "ytick.labelsize": 7.5,
        "legend.fontsize": 7.0,
        "axes.spines.top": False,
        "axes.spines.right": False,
        "axes.linewidth": 0.8,
        "xtick.major.width": 0.8,
        "ytick.major.width": 0.8,
    })


def plot_figure_s2(data_dir: Path, output_dir: Path):
    set_publication_style()
    output_dir.mkdir(parents=True, exist_ok=True)

    summary_file = data_dir / "S2_oat_summary.csv"
    raw_file = data_dir / "S2_oat_raw.csv"

    if not summary_file.exists() or not raw_file.exists():
        raise FileNotFoundError(f"Missing sensitivity data in {data_dir}. Run 'run_structural_sensitivity.py --simulate' first.")

    df_sum = pd.read_csv(summary_file)
    df_raw = pd.read_csv(raw_file)

    fig, axes = plt.subplots(3, 2, figsize=(11.0, 12.0), dpi=300)

    # Master title
    fig.text(0.06, 0.985, "Figure S2", fontsize=15, fontweight="bold", va="top")

    # =============================================================
    # Panel a: Tornado Horizontal Bar Chart (Max Penetrance Change)
    # =============================================================
    ax_a = axes[0, 0]
    y_pos = np.arange(len(ORDERED_PARAMS))
    height = 0.35

    d10 = df_sum[df_sum["perturbation_fraction"] == 0.10].set_index("parameter")
    d25 = df_sum[df_sum["perturbation_fraction"] == 0.25].set_index("parameter")

    v10 = [d10.loc[p, "max_abs_change_pp"] if p in d10.index else 0.0 for p, _ in ORDERED_PARAMS]
    v25 = [d25.loc[p, "max_abs_change_pp"] if p in d25.index else 0.0 for p, _ in ORDERED_PARAMS]

    # Invert so first parameter is at the top
    y_rev = y_pos[::-1]
    ax_a.barh(y_rev + height / 2, v10, height=height, color="#93c5fd", edgecolor="#1d4ed8", label="±10%", zorder=3)
    ax_a.barh(y_rev - height / 2, v25, height=height, color="#1d4ed8", edgecolor="#1e3a8a", label="±25%", zorder=3)

    ax_a.set_yticks(y_rev)
    ax_a.set_yticklabels([lbl for _, lbl in ORDERED_PARAMS])
    ax_a.set_xlabel("Maximum absolute change in predicted penetrance (%)")
    ax_a.set_xlim(0, max(max(v25) * 1.15, 25))
    ax_a.grid(axis="x", linestyle=":", alpha=0.5, zorder=0)
    ax_a.legend(title="Fixed-parameter perturbation", loc="lower right", frameon=False)
    ax_a.text(-0.16, 1.05, "a", transform=ax_a.transAxes, fontsize=13, fontweight="bold", va="top")

    # =============================================================
    # Panel b: Discrepancy (Weighted SSE log scale vs Perturbation %)
    # =============================================================
    ax_b = axes[0, 1]
    param_colors = {
        "tp53_rate": "#dc2626",
        "d1_threshold": "#2563eb",
        "d2_threshold": "#d97706",
        "division_rate": "#059669",
        "tumor_threshold": "#7c3aed",
        "age_drift": "#db2777",
    }
    param_labels_b = {
        "tp53_rate": r"$\mu_{\rm TP53}$",
        "d1_threshold": r"$\theta_{D1}$",
        "d2_threshold": r"$\theta_{D2}$",
        "division_rate": r"$d$",
        "tumor_threshold": "Tumor-positive threshold",
        "age_drift": "Age-drift coefficient",
    }

    ref_sse = 14.02
    x_perts = [-25, -10, 0, 10, 25]

    for p_key, p_lbl in ORDERED_PARAMS:
        col = param_colors.get(p_key, "#4b5563")
        y_vals = []
        for x_p in x_perts:
            if x_p == 0:
                y_vals.append(ref_sse)
            else:
                frac = abs(x_p) / 100.0
                dirn = 1 if x_p > 0 else -1
                match = df_raw[(df_raw["parameter"] == p_key) &
                               (np.isclose(df_raw["perturbation_fraction"], frac)) &
                               (df_raw["direction"] == dirn)]
                if not match.empty:
                    y_vals.append(match.iloc[0]["weighted_sse"])
                else:
                    y_vals.append(ref_sse)

        ax_b.plot(x_perts, y_vals, marker="o", markersize=4, color=col, linewidth=1.2, label=param_labels_b[p_key], zorder=3)

    ax_b.axhline(ref_sse, color="#111827", linestyle="--", linewidth=1.1, label="Unperturbed model", zorder=2)
    ax_b.set_yscale("log")
    ax_b.set_xticks(x_perts)
    ax_b.set_xlabel("One-at-a-time perturbation from fixed value (%)")
    ax_b.set_ylabel("Discrepancy with observed penetrance\n(weighted SSE, log)")
    ax_b.grid(axis="both", linestyle=":", alpha=0.5, zorder=0)
    ax_b.legend(title="Fixed model parameter", loc="center left", bbox_to_anchor=(1.02, 0.5), frameon=False)
    ax_b.text(-0.14, 1.05, "b", transform=ax_b.transAxes, fontsize=13, fontweight="bold", va="top")

    # =============================================================
    # Panels c & d: +-10% Recalibration & CI Widths
    # =============================================================
    fig.text(0.30, 0.655, r"$\pm 10\%$ perturbation and recalibration", fontsize=10.5, fontweight="bold", ha="center")

    ax_c = axes[1, 0]
    file_c = data_dir / "S2_recalibration_10pct_curves.csv"
    if file_c.exists():
        df_c = pd.read_csv(file_c)
        cmap_c = plt.colormaps["viridis"].resampled(len(df_c))
        for i, (_, r) in enumerate(df_c.iterrows()):
            y_curve = [r[f"risk_{int(a)}"] for a in AGES]
            lbl = rf"$\mu_{{TP53}}={r['tp53_rate']:.4f}, \theta_{{D2}}={r['d2_threshold']:.2f}$"
            ax_c.plot(AGES, y_curve, color=cmap_c(i), linewidth=1.1, alpha=0.85, label=lbl, zorder=3)

    ax_c.errorbar(
        AGES, OBSERVED,
        yerr=[OBSERVED - CI_LOWER, CI_UPPER - OBSERVED],
        fmt="s", color="#1e40af", ecolor="#1e40af", elinewidth=1.2, capsize=3,
        markersize=4.5, label="Observed penetrance", zorder=5
    )
    ax_c.set_xlabel("Age (years)")
    ax_c.set_ylabel("Cumulative tumor-positive tissues (%)")
    ax_c.set_xlim(25, 85)
    ax_c.set_ylim(0, 85)
    ax_c.grid(axis="both", linestyle=":", alpha=0.4, zorder=0)
    ax_c.legend(loc="upper left", fontsize=5.8, ncol=2, frameon=False)
    ax_c.text(-0.16, 1.05, "c", transform=ax_c.transAxes, fontsize=13, fontweight="bold", va="top")

    ax_d = axes[1, 1]
    file_d = data_dir / "S2_recalibration_10pct_CI_widths.csv"
    calib_params = [r"$\beta$", r"$\delta_{\rm low}$", r"$\delta_{\rm high}$", r"$\nu$"]
    if file_d.exists():
        df_d = pd.read_csv(file_d)
        w_d = df_d["relative_CI_width"].to_numpy()
    else:
        w_d = [0.174, 0.255, 0.933, 0.953]

    ax_d.bar(calib_params, w_d, width=0.45, color="#3b82f6", edgecolor="#1d4ed8", zorder=3)
    ax_d.set_ylabel("95% posterior interval width / prior range")
    ax_d.set_xlabel("Calibrated parameter")
    ax_d.set_ylim(0.0, 1.05)
    ax_d.grid(axis="y", linestyle=":", alpha=0.5, zorder=0)
    ax_d.text(-0.14, 1.05, "d", transform=ax_d.transAxes, fontsize=13, fontweight="bold", va="top")

    # =============================================================
    # Panels e & f: +-25% Recalibration & CI Widths
    # =============================================================
    fig.text(0.30, 0.335, r"$\pm 25\%$ perturbation and recalibration", fontsize=10.5, fontweight="bold", ha="center")

    ax_e = axes[2, 0]
    file_e = data_dir / "S2_recalibration_25pct_curves.csv"
    if file_e.exists():
        df_e = pd.read_csv(file_e)
        cmap_e = plt.colormaps["viridis"].resampled(len(df_e))
        for i, (_, r) in enumerate(df_e.iterrows()):
            y_curve = [r[f"risk_{int(a)}"] for a in AGES]
            lbl = rf"$\mu_{{TP53}}={r['tp53_rate']:.4f}, \theta_{{D2}}={r['d2_threshold']:.2f}$"
            ax_e.plot(AGES, y_curve, color=cmap_e(i), linewidth=1.1, alpha=0.85, label=lbl, zorder=3)

    ax_e.errorbar(
        AGES, OBSERVED,
        yerr=[OBSERVED - CI_LOWER, CI_UPPER - OBSERVED],
        fmt="s", color="#1e40af", ecolor="#1e40af", elinewidth=1.2, capsize=3,
        markersize=4.5, label="Observed penetrance", zorder=5
    )
    ax_e.set_xlabel("Age (years)")
    ax_e.set_ylabel("Cumulative tumor-positive tissues (%)")
    ax_e.set_xlim(25, 85)
    ax_e.set_ylim(0, 85)
    ax_e.grid(axis="both", linestyle=":", alpha=0.4, zorder=0)
    ax_e.legend(loc="upper left", fontsize=5.8, ncol=2, frameon=False)
    ax_e.text(-0.16, 1.05, "e", transform=ax_e.transAxes, fontsize=13, fontweight="bold", va="top")

    ax_f = axes[2, 1]
    file_f = data_dir / "S2_recalibration_25pct_CI_widths.csv"
    if file_f.exists():
        df_f = pd.read_csv(file_f)
        w_f = df_f["relative_CI_width"].to_numpy()
    else:
        w_f = [0.183, 0.268, 0.945, 0.965]

    ax_f.bar(calib_params, w_f, width=0.45, color="#3b82f6", edgecolor="#1d4ed8", zorder=3)
    ax_f.set_ylabel("95% posterior interval width / prior range")
    ax_f.set_xlabel("Calibrated parameter")
    ax_f.set_ylim(0.0, 1.05)
    ax_f.grid(axis="y", linestyle=":", alpha=0.5, zorder=0)
    ax_f.text(-0.14, 1.05, "f", transform=ax_f.transAxes, fontsize=13, fontweight="bold", va="top")

    fig.subplots_adjust(left=0.08, right=0.88, bottom=0.05, top=0.95, hspace=0.38, wspace=0.28)

    pdf_out = output_dir / "FigureS2.pdf"
    png_out = output_dir / "FigureS2.png"
    eps_out = output_dir / "FigureS2.eps"
    fig.savefig(pdf_out, bbox_inches="tight")
    fig.savefig(png_out, dpi=300, bbox_inches="tight")
    fig.savefig(eps_out, format="eps", bbox_inches="tight")
    plt.close(fig)

    print(f"✓ Figure S2 generated (6 panels a-f):")
    print(f"  -> PDF: {pdf_out}")
    print(f"  -> PNG: {png_out}")


def main():
    parser = argparse.ArgumentParser(description="Render Figure S2 (6 panels a-f)")
    parser.add_argument("--data-dir", type=str, default=str(Path(__file__).parent / "data"))
    parser.add_argument("--output-dir", type=str, default=str(Path(__file__).parent))

    args = parser.parse_args()
    plot_figure_s2(Path(args.data_dir), Path(args.output_dir))


if __name__ == "__main__":
    main()
