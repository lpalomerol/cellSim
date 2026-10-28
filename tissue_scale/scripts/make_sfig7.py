#!/usr/bin/env python3
"""
Generate Supplementary Figure S7 (formerly Figure S5 in initial workflow)
for the BRCA1-ONSET tissue-scale model: Homogeneous Tissue-Restraint Fitting Trade-off.

Outputs:
- Supplementary Figure S7: Composite 2-panel figure:
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
    "grid.alpha": 0.4,
    "lines.linewidth": 1.5,
    "lines.markersize": 5,
    "pdf.fonttype": 42,
    "ps.fonttype": 42,
})

CLINICAL_AGES = np.array([30, 40, 50, 60, 70, 80])


def plot_figure_s7(
    hom_df: pd.DataFrame,
    search_df: pd.DataFrame,
    clinical_df: pd.DataFrame,
    output_dir: Path,
) -> tuple[Path, Path]:
    """Generate Supplementary Figure S7 (PDF + PNG)."""
    fig, axes = plt.subplots(1, 2, figsize=(11.0, 4.8))

    # --- Panel a: Homogeneous penetrance ---
    ax_a = axes[0]
    obs_pen = clinical_df.loc[CLINICAL_AGES, "penetrance"].to_numpy(float)
    ci_low = clinical_df.loc[CLINICAL_AGES, "ci_low"].to_numpy(float)
    ci_high = clinical_df.loc[CLINICAL_AGES, "ci_high"].to_numpy(float)
    sd = (ci_high - ci_low) / 3.92

    hom_pen = np.array([(hom_df["detect_age"].between(0, age)).mean() * 100.0 for age in CLINICAL_AGES])
    wsse_hom = float(np.sum(((hom_pen - obs_pen) / sd) ** 2))

    ax_a.fill_between(CLINICAL_AGES, ci_low, ci_high, color="#2c3e50", alpha=0.15, label="Observed 95% CI (Kuchenbaecker 2017)")
    ax_a.plot(CLINICAL_AGES, obs_pen, "s-", color="#2c3e50", lw=2, label="Observed clinical penetrance")
    ax_a.plot(CLINICAL_AGES, hom_pen, "o--", color="#e74c3c", lw=2, label="Homogeneous tissue-scale model")

    ax_a.set_xlabel("Age (years)")
    ax_a.set_ylabel("Cumulative penetrance (%)")
    ax_a.set_xlim(25, 82)
    ax_a.set_ylim(0, 100)
    ax_a.grid(True, linestyle="--")
    ax_a.legend(frameon=True, facecolor="white", edgecolor="none", loc="upper left")
    ax_a.text(0.95, 0.08, f"Weighted SSE = {wsse_hom:.2f}", transform=ax_a.transAxes, ha="right",
              fontsize=9, bbox=dict(boxstyle="round,pad=0.3", facecolor="#f8f9fa", edgecolor="#bdc3c7"))
    ax_a.set_title("a  Homogeneous Model Penetrance Fit", fontweight="bold", loc="left")

    # --- Panel b: Trade-off early/mid vs late ---
    ax_b = axes[1]
    sc = ax_b.scatter(
        search_df["sse_early_mid"],
        search_df["sse_late"],
        c=search_df["wsse"],
        cmap="viridis_r",
        s=32,
        alpha=0.75,
        edgecolors="none"
    )

    exact_early = float(np.sum((hom_pen[:3] - obs_pen[:3]) ** 2))
    exact_late = float(np.sum((hom_pen[3:] - obs_pen[3:]) ** 2))

    ax_b.scatter([exact_early], [exact_late], marker="X", color="#e74c3c", s=110, label="Selected homogeneous parameter set", zorder=5)

    ax_b.set_xlabel("Early/mid-life discrepancy\n(ages 30–50; unweighted SSE)")
    ax_b.set_ylabel("Late-life discrepancy\n(ages 60–80; unweighted SSE)")
    ax_b.grid(True, linestyle="--")
    ax_b.legend(frameon=True, facecolor="white", edgecolor="none", loc="upper right")

    cb = fig.colorbar(sc, ax=ax_b, pad=0.03)
    cb.set_label("Overall weighted SSE")

    ax_b.set_title("b  Fitting Trade-off (Ages 30–50 vs 60–80)", fontweight="bold", loc="left")

    fig.tight_layout()

    pdf_path = output_dir / "FigureS7.pdf"
    png_path = output_dir / "FigureS7.png"
    fig.savefig(pdf_path, dpi=300)
    fig.savefig(png_path, dpi=300)
    plt.close(fig)

    return pdf_path, png_path


def main():
    base_dir = Path(__file__).resolve().parent.parent
    parser = argparse.ArgumentParser(description="Generate Supplementary Figure S7.")
    parser.add_argument("--homogeneous", default=str(base_dir / "data" / "homogeneous_validation_3000.csv"))
    parser.add_argument("--search", default=str(base_dir / "data" / "homogeneous_search_source.csv"))
    parser.add_argument("--clinical", default=str(base_dir / "data" / "clinical_penetrance.csv"))
    parser.add_argument("--output-dir", default=str(base_dir / "results"))

    args = parser.parse_args()
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    hom_df = pd.read_csv(args.homogeneous)
    search_df = pd.read_csv(args.search)
    clinical_df = pd.read_csv(args.clinical).set_index("age")

    pdf_path, png_path = plot_figure_s7(hom_df, search_df, clinical_df, out_dir)
    print(f"[SUCCESS] Supplementary Figure S7 generated:")
    print(f"  -> Vector: {pdf_path}")
    print(f"  -> Raster: {png_path} (300 DPI)")


if __name__ == "__main__":
    main()
