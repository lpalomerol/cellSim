#!/usr/bin/env python3
"""
generate_liepe_posterior_matrix.py — Generate Pairwise Joint Posterior Matrix (Liepe et al. 2014 Nature Protocols Fig 6a style).

Generates:
1. fig_posterior_matrix_5x5.png / .pdf (Key primary parameters + TP53)
2. fig_posterior_matrix_9x9.png / .pdf (Full 9-parameter space for Supplementary)
"""

from __future__ import annotations

from pathlib import Path
import numpy as np
import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from scipy.stats import gaussian_kde

SCRIPT_DIR = Path(__file__).resolve().parent
PAPER_DIR = SCRIPT_DIR.parent
DATA_DIR = PAPER_DIR / "data"
OUT_DIR = PAPER_DIR / "figures"
OUT_DIR.mkdir(parents=True, exist_ok=True)

# Parameter labels with math formatting
PARAM_LABELS_SHORT = {
    "brca1_rate": r"$\beta_{BRCA1}$",
    "low_delta": r"$\delta_{\rm BRCA+}$",
    "high_delta": r"$\delta_{\rm BRCA-}$",
    "neoplastic_div_rate": r"$d_{\rm neo}$",
    "tp53_rate": r"$\mu_{TP53}$",
    "d1_threshold": r"$\theta_{D\_intr}$",
    "d2_threshold": r"$\theta_{D\_inmune}$",
    "division_rate": r"$d_{\rm basal}$",
    "tumor_threshold": r"$\phi_{\rm tumor}$",
}

PARAM_LABELS_FULL = {
    "brca1_rate": r"$\beta_{BRCA1}$ (2º hit rate)",
    "low_delta": r"$\delta_{\rm BRCA+}$ ($BRCA1^{+/-}$ instab.)",
    "high_delta": r"$\delta_{\rm BRCA-}$ ($BRCA1^{-/-}$ instab.)",
    "neoplastic_div_rate": r"$d_{\rm neo}$ (Neoplastic div.)",
    "tp53_rate": r"$\mu_{TP53}$ (TP53 rate)",
    "d1_threshold": r"$\theta_{D\_intr}$ (Primer thr.)",
    "d2_threshold": r"$\theta_{D\_inmune}$ (Immune thr.)",
    "division_rate": r"$d_{\rm basal}$ (Basal div.)",
    "tumor_threshold": r"$\phi_{\rm tumor}$ (Onset frac.)",
}

def plot_liepe_corner_matrix(
    df: pd.DataFrame,
    params: list[str],
    out_prefix: str,
    title: str = "",
    figsize: tuple[float, float] = (10, 10),
    fontsize: int = 10,
    n_bins: int = 50,
):
    n = len(params)
    fig, axes = plt.subplots(n, n, figsize=figsize, dpi=300)
    plt.subplots_adjust(wspace=0.18, hspace=0.18, left=0.08, right=0.96, bottom=0.08, top=0.94)

    weights = df["weight"].values if "weight" in df.columns else None
    if weights is not None:
        weights = weights / weights.sum()

    # Colormap matching Liepe et al. (yellow to deep orange/brown)
    cmap = plt.cm.YlOrBr

    for i in range(n):
        for j in range(n):
            ax = axes[i, j]
            p_y = params[i]
            p_x = params[j]

            if i == j:
                # Diagonal: 1D Marginal Posterior Density
                data = df[p_x].values
                kde = gaussian_kde(data, weights=weights)
                x_grid = np.linspace(data.min() - 0.05 * np.ptp(data), data.max() + 0.05 * np.ptp(data), 200)
                density = kde(x_grid)

                # Style: Red/brown line with light fill (Liepe style)
                ax.plot(x_grid, density, color="#A82810", lw=1.8)
                ax.fill_between(x_grid, 0, density, color="#E87838", alpha=0.25)
                ax.set_ylim(bottom=0)
                ax.set_title(PARAM_LABELS_SHORT.get(p_x, p_x), fontsize=fontsize + 2, fontweight="bold", pad=4)
                ax.grid(False)
                ax.set_facecolor("#FDFAF3")

                # Diagonal border styling (black box like Liepe)
                for spine in ax.spines.values():
                    spine.set_edgecolor("#222222")
                    spine.set_linewidth(1.2)
                
                # Format ticks
                ax.tick_params(axis='both', which='major', labelsize=fontsize - 2, direction='out', length=3)
                if i < n - 1:
                    ax.set_xticklabels([])
            else:
                # Off-diagonal: 2D Pairwise Joint Posterior Distribution (Liepe style)
                x_data = df[p_x].values
                y_data = df[p_y].values

                # Perform 2D KDE
                try:
                    xy = np.vstack([x_data, y_data])
                    kde2d = gaussian_kde(xy, weights=weights)
                    
                    # Create grid
                    xmin, xmax = x_data.min(), x_data.max()
                    ymin, ymax = y_data.min(), y_data.max()
                    x_pad = 0.08 * (xmax - xmin) if xmax > xmin else 0.01
                    y_pad = 0.08 * (ymax - ymin) if ymax > ymin else 0.01
                    
                    X, Y = np.mgrid[xmin - x_pad:xmax + x_pad:complex(0, n_bins), ymin - y_pad:ymax + y_pad:complex(0, n_bins)]
                    positions = np.vstack([X.ravel(), Y.ravel()])
                    Z = np.reshape(kde2d(positions).T, X.shape)

                    # Filled contours with Liepe colormap
                    # Normalize Z to [0, 1] for contour levels
                    Z_norm = Z / Z.max() if Z.max() > 0 else Z
                    levels = np.linspace(0.05, 1.0, 12)
                    
                    # Background color matching Liepe paper (very pale warm beige)
                    ax.set_facecolor("#FAF6EB")
                    ax.contourf(X, Y, Z_norm, levels=levels, cmap=cmap, extend="min", alpha=0.92)
                    # Subtle outline contours for highest density peaks
                    ax.contour(X, Y, Z_norm, levels=[0.3, 0.6, 0.85], colors=["#8C2D04", "#67000D", "#49000B"], linewidths=0.7, alpha=0.7)
                except Exception as e:
                    # Fallback to 2d hist if KDE fails
                    ax.hist2d(x_data, y_data, bins=25, cmap=cmap)

                ax.grid(False)
                # Border styling
                for spine in ax.spines.values():
                    spine.set_edgecolor("#CCCCCC")
                    spine.set_linewidth(0.8)

                ax.tick_params(axis='both', which='major', labelsize=fontsize - 2, direction='out', length=3)
                
                # Show tick labels only on exterior
                if i < n - 1:
                    ax.set_xticklabels([])
                if j > 0:
                    ax.set_yticklabels([])

            # Exterior labels
            if j == 0 and i > 0 and i != j:
                ax.set_ylabel(PARAM_LABELS_SHORT.get(p_y, p_y), fontsize=fontsize + 1, fontweight="bold")
            if i == n - 1:
                ax.set_xlabel(PARAM_LABELS_SHORT.get(p_x, p_x), fontsize=fontsize + 1, fontweight="bold")

    if title:
        fig.suptitle(title, fontsize=fontsize + 4, fontweight="bold", y=0.98)

    pdf_path = OUT_DIR / f"{out_prefix}.pdf"
    png_path = OUT_DIR / f"{out_prefix}.png"
    plt.savefig(pdf_path, dpi=300, bbox_inches="tight")
    plt.savefig(png_path, dpi=300, bbox_inches="tight")
    plt.close()
    print(f"Saved: {png_path} and {pdf_path}")


def main():
    levels = [
        ("level_1", "Nivel 1 (±10% - Rango Fisiológico Estricto)", "fig_supp_s1_posterior_matrix_9x9_level1", "fig6a_liepe_posterior_matrix_5x5"),
        ("level_2", "Nivel 2 (±25% - Variabilidad Moderada)", "fig_supp_s2_posterior_matrix_9x9_level2", None),
        ("level_3", "Nivel 3 (±50% - Prueba de Estrés de Frontera)", "fig_supp_s3_posterior_matrix_9x9_level3", None),
    ]

    params_5x5 = [
        "brca1_rate",
        "low_delta",
        "high_delta",
        "neoplastic_div_rate",
        "tp53_rate",
    ]

    params_9x9 = [
        "brca1_rate",
        "low_delta",
        "high_delta",
        "neoplastic_div_rate",
        "tp53_rate",
        "d1_threshold",
        "d2_threshold",
        "division_rate",
        "tumor_threshold",
    ]

    for lvl_dir, lvl_title, supp_prefix_9x9, prefix_5x5 in levels:
        csv_path = DATA_DIR / lvl_dir / "gen_07.csv"
        if not csv_path.exists():
            print(f"Skipping {lvl_dir}: {csv_path} not found.")
            continue
        
        df = pd.read_csv(csv_path)
        print(f"\nProcessing {lvl_dir} ({len(df)} particles)...")

        # 5x5 Matrix (Primary Fig 1 for Level 1 only)
        if prefix_5x5:
            print(f"  Generating 5x5 Matrix: {prefix_5x5}...")
            plot_liepe_corner_matrix(
                df=df,
                params=params_5x5,
                out_prefix=prefix_5x5,
                title=f"CellSim Posterior Distributions (5x5): {lvl_title}",
                figsize=(11, 11),
                fontsize=10,
                n_bins=60,
            )

        # 9x9 Matrix (Supplementary Fig S1 / S2 / S3)
        print(f"  Generating 9x9 Supplementary Matrix: {supp_prefix_9x9}...")
        plot_liepe_corner_matrix(
            df=df,
            params=params_9x9,
            out_prefix=supp_prefix_9x9,
            title=f"Full 9-Dimensional Parameter Posterior Space: {lvl_title}",
            figsize=(18, 18),
            fontsize=8,
            n_bins=40,
        )

if __name__ == "__main__":
    main()

