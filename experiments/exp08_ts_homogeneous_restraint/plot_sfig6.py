#!/usr/bin/env python3
"""
Generate Supplementary Figure S6 for the BRCA1-ONSET tissue-scale model:
Homogeneous Tissue-Restraint Fitting Trade-off.
Matches review manuscript Palomero_SFig6_100126.pdf.

Outputs:
- Supplementary Figure S6: Composite 2-panel figure:
  Panel a: Cumulative penetrance under homogeneous tissue restraint vs observed.
  Panel b: Trade-off between early/mid-life discrepancy (ages 30–50) vs late-life (ages 60–80).
"""

from __future__ import annotations

import argparse
from pathlib import Path
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# Publication styling
plt.rcParams.update({
    "font.family": "sans-serif",
    "font.sans-serif": ["DejaVu Sans", "Helvetica", "Arial"],
    "font.size": 9,
    "axes.labelsize": 10,
    "axes.titlesize": 10,
    "xtick.labelsize": 8.5,
    "ytick.labelsize": 8.5,
    "legend.fontsize": 8.5,
    "figure.titlesize": 11,
    "axes.linewidth": 0.8,
    "grid.linewidth": 0.5,
    "grid.alpha": 0.35,
    "lines.linewidth": 1.6,
    "lines.markersize": 5,
    "pdf.fonttype": 42,
    "ps.fonttype": 42,
})

CLINICAL_AGES = np.array([30, 40, 50, 60, 70, 80])


def plot_figure_s6(
    hom_df: pd.DataFrame,
    search_df: pd.DataFrame,
    clinical_df: pd.DataFrame,
    output_dir: Path,
) -> tuple[Path, Path]:
    """Generate Supplementary Figure S6 (PDF + PNG)."""
    fig, axes = plt.subplots(1, 2, figsize=(11.5, 5.0), gridspec_kw={"wspace": 0.28})

    # --- Panel a: Homogeneous penetrance ---
    ax_a = axes[0]
    obs_pen = clinical_df.loc[CLINICAL_AGES, "penetrance"].to_numpy(float)
    ci_low = clinical_df.loc[CLINICAL_AGES, "ci_low"].to_numpy(float)
    ci_high = clinical_df.loc[CLINICAL_AGES, "ci_high"].to_numpy(float)
    sd = (ci_high - ci_low) / 3.92

    hom_pen = np.array([(hom_df["detect_age"].between(0, age)).mean() * 100.0 for age in CLINICAL_AGES])
    wsse_hom = float(np.sum(((hom_pen - obs_pen) / sd) ** 2))

    ax_a.fill_between(CLINICAL_AGES, ci_low, ci_high, color="#2c3e50", alpha=0.15, label="Observed penetrance 95% CI")
    ax_a.plot(CLINICAL_AGES, obs_pen, "s-", color="#2c3e50", lw=2, label="Observed penetrance value")
    ax_a.plot(CLINICAL_AGES, hom_pen, "o--", color="#e74c3c", lw=2, label="Homogeneous tissue-level restraint")

    ax_a.set_xlabel("Age (years)")
    ax_a.set_ylabel("Cumulative tumor-positive tissues (%)")
    ax_a.set_xlim(25, 85)
    ax_a.set_ylim(0, 105)
    ax_a.grid(True, linestyle="--")
    ax_a.legend(frameon=True, facecolor="white", edgecolor="none", loc="upper left")
    ax_a.text(0.95, 0.08, f"Weighted SSE = {wsse_hom:.2f}", transform=ax_a.transAxes, ha="right",
              fontsize=9, bbox=dict(boxstyle="round,pad=0.3", facecolor="#f8f9fa", edgecolor="#bdc3c7", lw=0.8))
    ax_a.set_title("Tissue-scale modeling", loc="center", fontsize=9.5, color="#34495e")
    ax_a.text(-0.12, 1.05, "a", transform=ax_a.transAxes, fontsize=13, fontweight="bold", va="top")

    # --- Panel b: Trade-off early/mid vs late ---
    ax_b = axes[1]
    sc = ax_b.scatter(
        search_df["sse_early_mid"],
        search_df["sse_late"],
        c=search_df["wsse"],
        cmap="viridis_r",
        s=36,
        alpha=0.8,
        edgecolors="none",
    )

    exact_early = float(np.sum((hom_pen[:3] - obs_pen[:3]) ** 2))
    exact_late = float(np.sum((hom_pen[3:] - obs_pen[3:]) ** 2))

    ax_b.scatter([exact_early], [exact_late], marker="X", color="#e74c3c", s=120, label="Selected homogeneous parameter set", zorder=5)

    ax_b.set_xlabel("Ages 30-50 discrepancy (SSE)")
    ax_b.set_ylabel("Ages 60-80 discrepancy (SSE)")
    ax_b.grid(True, linestyle="--")
    ax_b.legend(frameon=True, facecolor="white", edgecolor="none", loc="upper right")
    ax_b.set_title("Validation of homogeneous\ntissue-level restraint", loc="center", fontsize=9.5, color="#34495e")
    ax_b.text(-0.12, 1.05, "b", transform=ax_b.transAxes, fontsize=13, fontweight="bold", va="top")

    cb = fig.colorbar(sc, ax=ax_b, pad=0.03)
    cb.set_label("Overall wSSE")

    output_dir.mkdir(parents=True, exist_ok=True)
    pdf_path = output_dir / "FigureS6.pdf"
    png_path = output_dir / "FigureS6.png"
    eps_path = output_dir / "FigureS6.eps"

    fig.savefig(pdf_path, dpi=300, bbox_inches="tight")
    fig.savefig(png_path, dpi=300, bbox_inches="tight")
    fig.savefig(eps_path, format="eps", bbox_inches="tight")
    plt.close(fig)

    print(f"  [OK] Saved Figure S6 -> {pdf_path}")
    print(f"  [OK] Saved Figure S6 -> {png_path}")
    print(f"  [OK] Saved Figure S6 -> {eps_path}")
    return pdf_path, png_path


def main():
    parser = argparse.ArgumentParser(description="Plot Supplementary Figure S6.")
    parser.add_argument("--data-dir", default=str(Path(__file__).parent / "data"))
    parser.add_argument("--outdir", default=str(Path(__file__).parent))
    args = parser.parse_args()

    data_dir = Path(args.data_dir)
    outdir = Path(args.outdir)

    hom_df = pd.read_csv(data_dir / "homogeneous_validation_3000.csv")
    search_df = pd.read_csv(data_dir / "homogeneous_search_source.csv")
    clin_df = pd.read_csv(data_dir / "clinical_penetrance.csv").set_index("age")

    plot_figure_s6(hom_df, search_df, clin_df, outdir)


if __name__ == "__main__":
    main()
