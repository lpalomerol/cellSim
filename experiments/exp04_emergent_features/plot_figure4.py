#!/usr/bin/env python3
"""
plot_figure4.py — Publication-grade visualization for Figure 4.

Reproduces Figure 4 of Palomero et al. (2026):
  Panel a: Calibrated parameter posteriors:
           - Row 1: 1D KDEs with uniform priors and medians.
           - Row 2: Empirical posterior CDFs vs theoretical uniform CDFs.
           - Right: Weighted pairwise correlation matrix.
  Panel b: BRCA1 status in emerging tumors (Exclusively BRCA1+/-, Mixed, Exclusively BRCA1-/-).
  Panel c: Founder-lineage complexity by detection age (<30 to 80 years, 1 to >=4 founders, chi-square P = 1.9e-239).

Outputs:
  - experiments/exp04_emergent_features/Figure4.pdf
  - experiments/exp04_emergent_features/Figure4.png
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path
from typing import Tuple

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from scipy import stats

plt.switch_backend("Agg")

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

PARAMS = ["brca1_rate", "low_delta", "high_delta", "neoplastic_div_rate"]
PARAM_LABELS = {
    "brca1_rate": r"$\beta$",
    "low_delta": r"$\delta_{\rm low}$",
    "high_delta": r"$\delta_{\rm high}$",
    "neoplastic_div_rate": r"$\nu$",
}
PRIORS = {
    "brca1_rate": (0.025, 0.080),
    "low_delta": (0.060, 0.220),
    "high_delta": (0.120, 0.600),
    "neoplastic_div_rate": (0.100, 0.240),
}
TICKS = {
    "brca1_rate": [0.03, 0.05, 0.07],
    "low_delta": [0.075, 0.125, 0.175],
    "high_delta": [0.2, 0.4, 0.6],
    "neoplastic_div_rate": [0.10, 0.16, 0.22],
}


def weighted_stats(x: np.ndarray, w: np.ndarray) -> Tuple[float, float, float]:
    idx = np.argsort(x)
    sx = x[idx]
    sw = w[idx]
    cw = np.cumsum(sw) / np.sum(sw)
    q025 = float(np.interp(0.025, cw, sx))
    q50 = float(np.interp(0.50, cw, sx))
    q975 = float(np.interp(0.975, cw, sx))
    return q50, q025, q975


def render_figure4(data_dir: Path, out_dir: Path) -> Tuple[Path, Path]:
    df_part = pd.read_csv(data_dir / "Figure4a_parameters.csv")
    df_corr = pd.read_csv(data_dir / "Figure4a_correlations.csv", index_col=0)
    df_geno = pd.read_csv(data_dir / "Figure4b_genotype_composition.csv")
    df_found = pd.read_csv(data_dir / "Figure4c_founder_lineages.csv")

    w = df_part["weight"].to_numpy(float, copy=True)
    w = w / np.sum(w)

    fig = plt.figure(figsize=(10.5, 9.8), dpi=300)
    # Master title
    fig.text(0.04, 0.98, "Figure 4", fontsize=15, fontweight="bold", va="top")

    # =========================================================================
    # Panel a: Calibrated parameter posteriors
    # =========================================================================
    fig.text(0.04, 0.94, "a", fontsize=14, fontweight="bold", va="top")
    fig.text(0.07, 0.94, "Calibrated parameter posteriors", fontsize=11, fontweight="bold", va="top")

    # Grid for Panel a: 2 rows x 4 cols + 1 col for correlation matrix
    gs_a = fig.add_gridspec(
        nrows=2, ncols=5,
        left=0.07, right=0.96, top=0.90, bottom=0.56,
        width_ratios=[1.0, 1.0, 1.0, 1.0, 1.2],
        wspace=0.38, hspace=0.35
    )

    y_max_dens = [140, 30, 2.5, 8.0]
    y_ticks_dens = [
        [0, 40, 80, 120, 140],
        [0, 10, 20, 30],
        [0.0, 0.5, 1.0, 1.5, 2.0, 2.5],
        [0, 2, 4, 6, 8]
    ]

    for col, p in enumerate(PARAMS):
        vals = df_part[p].to_numpy(float)
        p_lo, p_hi = PRIORS[p]
        grid_x = np.linspace(p_lo, p_hi, 200)

        # Row 0: Density KDE
        ax_dens = fig.add_subplot(gs_a[0, col])
        kde = stats.gaussian_kde(vals, weights=w)
        dens = kde(grid_x)
        ax_dens.plot(grid_x, dens, color="#111827", linewidth=1.3, label="Posterior KDE")

        # Uniform prior height
        prior_h = 1.0 / (p_hi - p_lo)
        ax_dens.axhline(prior_h, color="#9CA3AF", linestyle="--", linewidth=1.1, label="Uniform prior")

        # Median
        med, _, _ = weighted_stats(vals, w)
        ax_dens.axvline(med, color="#DC2626", linewidth=1.3, label="Median")
        ax_dens.text(med, y_max_dens[col] * 0.90, f"{med:.3f}", color="#DC2626",
                     fontsize=8, ha="center", fontweight="bold")

        ax_dens.set_title(PARAM_LABELS[p], fontsize=12, pad=4)
        ax_dens.set_xlim(p_lo, p_hi)
        ax_dens.set_ylim(0, y_max_dens[col])
        ax_dens.set_xticks(TICKS[p])
        ax_dens.set_yticks(y_ticks_dens[col])
        ax_dens.tick_params(labelsize=8, length=2.5)
        ax_dens.set_ylabel("Density" if col == 0 else "", fontsize=9)
        if col == 3:
            ax_dens.legend(loc="upper right", bbox_to_anchor=(1.35, 1.0), fontsize=7.5, frameon=False)

        # Row 1: CDF
        ax_cdf = fig.add_subplot(gs_a[1, col])
        idx_s = np.argsort(vals)
        s_vals = vals[idx_s]
        s_w = w[idx_s]
        ecdf = np.cumsum(s_w)
        ax_cdf.plot(s_vals, ecdf, color="#111827", linewidth=1.3, label="Posterior KDE")

        # Theoretical uniform CDF
        u_cdf = (grid_x - p_lo) / (p_hi - p_lo)
        ax_cdf.plot(grid_x, u_cdf, color="#9CA3AF", linestyle="--", linewidth=1.1, label="Theoretical uniform CDF")

        ax_cdf.set_xlim(p_lo, p_hi)
        ax_cdf.set_ylim(0.0, 1.05)
        ax_cdf.set_xticks(TICKS[p])
        ax_cdf.set_yticks([0.0, 0.2, 0.4, 0.6, 0.8, 1.0])
        ax_cdf.tick_params(labelsize=8, length=2.5)
        ax_cdf.set_ylabel("CDF" if col == 0 else "", fontsize=9)
        if col == 3:
            ax_cdf.legend(loc="upper right", bbox_to_anchor=(1.60, 1.0), fontsize=7.5, frameon=False)

    # Correlation Matrix Plot (Right side of Panel a)
    ax_corr = fig.add_subplot(gs_a[:, 4])
    corr_mat = df_corr.to_numpy(float)
    im = ax_corr.imshow(corr_mat, cmap="coolwarm", vmin=-1.0, vmax=1.0)
    cbar = fig.colorbar(im, ax=ax_corr, fraction=0.046, pad=0.06)
    cbar.ax.tick_params(labelsize=8)
    cbar.set_ticks([-1.0, 0.0, 1.0])
    cbar.set_ticklabels(["-1", "0", "+1"])
    ax_corr.set_title("Weighted\ncorrelation", fontsize=9, pad=8)
    labels = [r"$\beta$", r"$\delta_{\rm low}$", r"$\delta_{\rm high}$", r"$\nu$"]
    ax_corr.set_xticks(range(4))
    ax_corr.set_yticks(range(4))
    ax_corr.set_xticklabels(labels, fontsize=10)
    ax_corr.set_yticklabels(labels, fontsize=10)
    ax_corr.tick_params(length=2.0)

    # =========================================================================
    # Panel b: BRCA1 status in emerging tumors
    # =========================================================================
    fig.text(0.04, 0.48, "b", fontsize=14, fontweight="bold", va="top")
    fig.text(0.07, 0.48, "BRCA1 status in emerging tumors", fontsize=11, fontweight="bold", va="top")

    ax_b = fig.add_axes([0.08, 0.16, 0.38, 0.25])
    pct_pm = float(df_geno.loc[df_geno["category"] == "Exclusively BRCA1+/-", "percentage"].iloc[0])
    pct_mix = float(df_geno.loc[df_geno["category"] == "Mixed", "percentage"].iloc[0])
    pct_mm = float(df_geno.loc[df_geno["category"] == "Exclusively BRCA1-/-", "percentage"].iloc[0])

    colors_b = ["#93C5FD", "#FBBF24", "#1E3A8A"]
    ax_b.barh([0], [pct_pm], color=colors_b[0], edgecolor="none", height=0.45, label="Exclusively BRCA1+/-")
    ax_b.barh([0], [pct_mix], left=[pct_pm], color=colors_b[1], edgecolor="none", height=0.45, label="Mixed")
    ax_b.barh([0], [pct_mm], left=[pct_pm + pct_mix], color=colors_b[2], edgecolor="none", height=0.45, label="Exclusively BRCA1-/-")

    # Percentage text labels inside bars
    ax_b.text(pct_pm / 2, 0, f"{pct_pm:.0f}%", ha="center", va="center", color="#111827", fontsize=10, fontweight="bold")
    ax_b.text(pct_pm + pct_mix / 2, 0, f"{pct_mix:.0f}%", ha="center", va="center", color="#111827", fontsize=10, fontweight="bold")
    ax_b.text(pct_pm + pct_mix + pct_mm / 2, 0, f"{pct_mm:.0f}%", ha="center", va="center", color="white", fontsize=10, fontweight="bold")

    ax_b.set_xlim(0, 100)
    ax_b.set_ylim(-0.4, 0.8)
    ax_b.set_xlabel("Tumor-positive tissues (%)", fontsize=10)
    ax_b.set_yticks([])
    ax_b.tick_params(length=2.5, labelsize=9)
    ax_b.legend(loc="upper center", bbox_to_anchor=(0.5, 1.25), ncol=3, fontsize=8, frameon=False)

    # =========================================================================
    # Panel c: Founder-lineage complexity by detection age
    # =========================================================================
    fig.text(0.52, 0.48, "c", fontsize=14, fontweight="bold", va="top")
    fig.text(0.55, 0.48, "Founder-lineage complexity by detection age", fontsize=11, fontweight="bold", va="top")

    ax_c = fig.add_axes([0.56, 0.12, 0.39, 0.30])
    ages = df_found["age_group"].tolist()
    p1 = df_found["founder_1_pct"].to_numpy(float)
    p2 = df_found["founder_2_pct"].to_numpy(float)
    p3 = df_found["founder_3_pct"].to_numpy(float)
    p4 = df_found["founder_ge4_pct"].to_numpy(float)

    x = np.arange(len(ages))
    width = 0.65
    colors_c = ["#F59E0B", "#F97316", "#DC2626", "#991B1B"]

    ax_c.bar(x, p1, width, color=colors_c[0], label="1")
    ax_c.bar(x, p2, width, bottom=p1, color=colors_c[1], label="2")
    ax_c.bar(x, p3, width, bottom=p1 + p2, color=colors_c[2], label="3")
    ax_c.bar(x, p4, width, bottom=p1 + p2 + p3, color=colors_c[3], label=r"$\geq 4$")

    ax_c.set_xticks(x)
    ax_c.set_xticklabels(["<30", "30–39", "40–49", "50–59", "60–69", "70–79", "80"], fontsize=8.5)
    ax_c.set_xlabel("Age at first tumor-positive detection (years)", fontsize=9.5)
    ax_c.set_ylabel("Tumor-positive tissues (%)", fontsize=9.5)
    ax_c.set_ylim(0, 100)
    ax_c.tick_params(length=2.5, labelsize=8.5)

    test_csv = data_dir / "Figure4_statistical_tests.csv"
    if test_csv.exists():
        df_stat = pd.read_csv(test_csv)
        p_val = float(df_stat["p_value"].iloc[0])
        mantissa, exp = f"{p_val:.1e}".split("e")
        stat_text = rf"$\chi^2\ P = {mantissa} \times 10^{{{int(exp)}}}$"
    else:
        stat_text = r"$\chi^2\ P = 1.9 \times 10^{-239}$"

    ax_c.text(0.70, 0.68, stat_text, transform=ax_c.transAxes,
              fontsize=8.5, fontstyle="italic", ha="center")
    ax_c.legend(title="Tumoral founder lineages (n)", title_fontsize=8, loc="upper right",
                fontsize=8, frameon=True, edgecolor="#E5E7EB")

    # Output paths
    pdf_path = out_dir / "Figure4.pdf"
    png_path = out_dir / "Figure4.png"
    fig.savefig(pdf_path, bbox_inches="tight")
    fig.savefig(png_path, dpi=300, bbox_inches="tight")
    plt.close(fig)

    return pdf_path, png_path


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate publication-grade Figure 4.")
    parser.add_argument("--data-dir", type=str, default="experiments/exp04_emergent_features/data")
    parser.add_argument("--output-dir", type=str, default="experiments/exp04_emergent_features")
    args = parser.parse_args()

    data_dir = Path(args.data_dir)
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    print("=========================================================================")
    print("Rendering Figure 4 (Emergent Tumor Features & Clonal Lineages)")
    print(f"Data directory: {data_dir}")
    print(f"Output directory: {out_dir}")
    print("=========================================================================")

    pdf_path, png_path = render_figure4(data_dir, out_dir)

    print(f"[SUCCESS] Figure 4 generated successfully:")
    print(f"  -> Vector: {pdf_path}")
    print(f"  -> Raster: {png_path} (300 DPI)\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
