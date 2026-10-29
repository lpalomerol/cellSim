#!/usr/bin/env python3
"""
plot_figure3.py — Publication-grade visualization for Figure 3.

Reproduces Figure 3 of Palomero et al. (2026):
  Panel a: ABC-SMC calibration performance:
           - Left: ABC-SMC tolerance threshold (epsilon), median weighted SSE,
                   and interquartile range (IQR) across retained parameter combinations.
           - Right: Proposal acceptance rate and effective sample size (ESS)
                    expressed as a percentage of the 250 retained combinations.
  Panel b: Posterior predictive cumulative penetrance versus Kuchenbaecker et al. (2017)
           prospective clinical cohort with 95% credible intervals.

Outputs:
--------
  - experiments/exp03_abc_smc_calibration/Figure3.pdf
  - experiments/exp03_abc_smc_calibration/Figure3.png
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

# Clinical target penetrance from Kuchenbaecker et al. (2017)
AGES = np.array([30, 40, 50, 60, 70, 80], dtype=int)
OBS = np.array([4.0, 26.0, 46.0, 58.0, 65.0, 70.0])
CI_LO = np.array([2.0, 22.0, 41.0, 52.0, 56.0, 60.0])
CI_HI = np.array([7.0, 30.0, 52.0, 65.0, 73.0, 80.0])


def render_figure3(data_dir: Path, out_dir: Path) -> Tuple[Path, Path]:
    """Generates the publication-grade Figure 3 matching manuscript PDF."""
    diag_csv = data_dir / "smc_diagnostics_20gen.csv"
    ppc_csv = data_dir / "posterior_predictive_summary.csv"

    if not diag_csv.exists() or not ppc_csv.exists():
        raise FileNotFoundError(f"Missing required calibration files in {data_dir}")

    df_diag = pd.read_csv(diag_csv)
    df_ppc = pd.read_csv(ppc_csv)

    fig = plt.figure(figsize=(10.5, 9.5), dpi=300)
    gs = fig.add_gridspec(2, 2, height_ratios=[1.0, 1.05], hspace=0.38, wspace=0.30)

    # Master title
    fig.text(0.04, 0.98, "Figure 3", fontsize=15, fontweight="bold", va="top")

    # =========================================================================
    # Panel a (Left): Weighted SSE Convergence (Tolerance, Median, IQR)
    # =========================================================================
    ax_a1 = fig.add_subplot(gs[0, 0])
    ax_a1.text(-0.20, 1.14, "a", transform=ax_a1.transAxes, fontsize=15, fontweight="bold", va="top")
    fig.text(0.50, 0.94, "ABC-SMC calibration performance", fontsize=11, ha="center", va="top")

    gens = df_diag["generation"].to_numpy()
    med_sse = df_diag["weighted_sse_median"].to_numpy(float)
    q25_sse = df_diag["weighted_sse_q25"].to_numpy(float)
    q75_sse = df_diag["weighted_sse_q75"].to_numpy(float)

    # Epsilon starts at generation 1
    mask_eps = df_diag["epsilon"].notna()
    eps_gens = gens[mask_eps]
    eps_vals = df_diag.loc[mask_eps, "epsilon"].to_numpy(float)

    # IQR shaded band
    ax_a1.fill_between(
        gens, q25_sse, q75_sse, color="#D1D5DB", alpha=0.5, label="Individual parameter combination IQR"
    )

    # Tolerance threshold line
    ax_a1.plot(
        eps_gens, eps_vals, color="#111827", marker="s", markersize=4.5,
        linewidth=1.2, label="ABC-SMC tolerance"
    )

    # Median weighted SSE line
    ax_a1.plot(
        gens, med_sse, color="#6B7280", marker="o", markersize=4.5,
        linewidth=1.2, label="Median weighted SSE"
    )

    ax_a1.set_xlabel("ABC-SMC generation", fontsize=10)
    ax_a1.set_ylabel("Weighted SSE", fontsize=10)
    ax_a1.set_xlim(-0.5, 19.5)
    ax_a1.set_ylim(0, 600)
    ax_a1.set_xticks([0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 19])
    ax_a1.set_yticks([0, 100, 200, 300, 400, 500, 600])
    ax_a1.tick_params(direction="out", length=3.0, labelsize=9)
    ax_a1.grid(True, linestyle=":", alpha=0.45, color="#E5E7EB")
    ax_a1.legend(loc="upper right", frameon=True, fontsize=8.5, edgecolor="#E5E7EB", framealpha=0.9)

    # =========================================================================
    # Panel a (Right): Acceptance Rate & Effective Sample Size (ESS)
    # =========================================================================
    ax_a2 = fig.add_subplot(gs[0, 1])

    ess_pct = df_diag["ess_ratio"].to_numpy(float) * 100.0
    acc_pct = df_diag["acceptance_rate"].to_numpy(float) * 100.0

    ax_a2.plot(
        gens, ess_pct, color="#111827", marker="^", markersize=4.5,
        linewidth=1.2, label="Effective sample size / combinations"
    )

    mask_acc = df_diag["acceptance_rate"].notna()
    ax_a2.plot(
        gens[mask_acc], acc_pct[mask_acc], color="#111827", marker="d", markersize=4.5,
        linewidth=1.2, label="Acceptance rate"
    )

    ax_a2.set_xlabel("ABC-SMC generation", fontsize=10)
    ax_a2.set_ylabel("Percent (%)", fontsize=10)
    ax_a2.set_xlim(-0.5, 19.5)
    ax_a2.set_ylim(0, 100)
    ax_a2.set_xticks([0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 19])
    ax_a2.set_yticks([0, 20, 40, 60, 80, 100])
    ax_a2.tick_params(direction="out", length=3.0, labelsize=9)
    ax_a2.grid(True, linestyle=":", alpha=0.45, color="#E5E7EB")
    ax_a2.legend(loc="center right", bbox_to_anchor=(0.95, 0.65), frameon=True, fontsize=8.5, edgecolor="#E5E7EB", framealpha=0.9)

    # =========================================================================
    # Panel b: Posterior Predictive Penetrance vs Kuchenbaecker 2017
    # =========================================================================
    ax_b = fig.add_subplot(gs[1, 0])
    ax_b.text(-0.20, 1.12, "b", transform=ax_b.transAxes, fontsize=15, fontweight="bold", va="top")
    ax_b.set_title("Posterior predictive penetrance", fontsize=11, pad=10)

    ppc_ages = df_ppc["Age"].to_numpy(int)
    ppc_med = df_ppc["Weighted median (%)"].to_numpy(float)
    ppc_q025 = df_ppc["2.5% (%)"].to_numpy(float)
    ppc_q975 = df_ppc["97.5% (%)"].to_numpy(float)

    # 95% Credible Interval shaded band
    ax_b.fill_between(
        ppc_ages, ppc_q025, ppc_q975, color="#FECACA", alpha=0.6, label="Posterior predictive 95% interval"
    )

    # Observed clinical cohort (Kuchenbaecker 2017)
    yerr_lo = OBS - CI_LO
    yerr_hi = CI_HI - OBS
    ax_b.errorbar(
        AGES, OBS, yerr=[yerr_lo, yerr_hi], fmt="s", color="#1D4ED8",
        ecolor="#1D4ED8", elinewidth=1.2, capsize=3, capthick=1.2, markersize=4.5,
        label="Observed penetrance (95% CI)"
    )

    # Posterior predictive median
    ax_b.plot(
        ppc_ages, ppc_med, color="#DC2626", marker="o", markersize=4.5,
        linewidth=1.4, label="Posterior predictive median"
    )

    ax_b.set_xlabel("Age (years)", fontsize=10)
    ax_b.set_ylabel("Cumulative penetrance /\ntumor-positive tissues (%)", fontsize=10)
    ax_b.set_xlim(28, 82)
    ax_b.set_ylim(0, 85)
    ax_b.set_xticks(AGES)
    ax_b.set_yticks([0, 20, 40, 60, 80])
    ax_b.tick_params(direction="out", length=3.0, labelsize=9)
    ax_b.grid(True, linestyle=":", alpha=0.45, color="#E5E7EB")
    ax_b.legend(loc="upper left", frameon=True, fontsize=8.5, edgecolor="#E5E7EB", framealpha=0.9)

    # Save output files
    pdf_path = out_dir / "Figure3.pdf"
    png_path = out_dir / "Figure3.png"
    eps_path = out_dir / "Figure3.eps"
    fig.savefig(pdf_path, bbox_inches="tight")
    fig.savefig(png_path, dpi=300, bbox_inches="tight")
    fig.savefig(eps_path, format="eps", bbox_inches="tight")
    plt.close(fig)

    return pdf_path, png_path


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate publication-grade Figure 3.")
    parser.add_argument(
        "--data-dir",
        type=str,
        default=str(Path(__file__).parent / "data"),
        help="Path to precomputed data directory",
    )
    parser.add_argument(
        "--output-dir",
        type=str,
        default=str(Path(__file__).parent),
        help="Output directory",
    )

    args = parser.parse_args()
    data_dir = Path(args.data_dir)
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    print("=========================================================================")
    print("Rendering Figure 3 (ABC-SMC Performance & Posterior Predictive)")
    print(f"Data directory: {data_dir}")
    print(f"Output directory: {out_dir}")
    print("=========================================================================")

    pdf_path, png_path = render_figure3(data_dir, out_dir)

    print(f"[SUCCESS] Figure 3 generated successfully:")
    print(f"  -> Vector: {pdf_path}")
    print(f"  -> Raster: {png_path} (300 DPI)\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
