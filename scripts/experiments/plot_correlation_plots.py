#!/usr/bin/env python3
"""
plot_correlation_plots.py — Publication-grade pairwise correlation plots (corrplot)
styled after R's corrplot / seaborn heatmap style for both scenarios:
  1. Global Model (9 free parameters including nu)
  2. Locked Model (8 free parameters, nu = d)

Features:
  - Elegant R corrplot style:
    * Lower triangle: colored ellipses / circles or heatmap tiles proportional to correlation strength
    * Upper triangle: correlation coefficient text colored and sized by significance/strength
    * Diagonal: parameter names or compact continuous density KDE
  - Standalone pure R-style heatmap matrix (with diverging colormap RdBu_r / coolwarm, hierarchical clustering order, colorbar, and clear labels)
  - Comprehensive pair-grid with smooth scatter + KDE contours
"""

from __future__ import annotations

import argparse
from pathlib import Path
from typing import Dict, List, Tuple

import matplotlib.pyplot as plt
from matplotlib.patches import Circle, Ellipse, Rectangle
import numpy as np
import pandas as pd
from scipy import stats

plt.switch_backend("Agg")

LABELS_GLOBAL = {
    "brca1_rate": r"$\beta$",
    "low_delta": r"$\delta_{\rm low}$",
    "high_delta": r"$\delta_{\rm high}$",
    "neoplastic_div_rate": r"$\nu$",
    "tp53_rate": r"$\mu_{\rm TP53}$",
    "d1_threshold": r"$\theta_{D1}$",
    "d2_threshold": r"$\theta_{D2}$",
    "division_rate": r"$d$",
    "kappa_brca1": r"$\kappa_{\rm BRCA1}$",
}

LABELS_LOCKED = {
    "brca1_rate": r"$\beta$",
    "low_delta": r"$\delta_{\rm low}$",
    "high_delta": r"$\delta_{\rm high}$",
    "tp53_rate": r"$\mu_{\rm TP53}$",
    "d1_threshold": r"$\theta_{D1}$",
    "d2_threshold": r"$\theta_{D2}$",
    "division_rate": r"$d$",
    "kappa_brca1": r"$\kappa_{\rm BRCA1}$",
}


def weighted_corr(x: np.ndarray, y: np.ndarray, w: np.ndarray) -> float:
    """Compute weighted Pearson correlation coefficient."""
    w_norm = w / np.sum(w)
    mx = np.sum(w_norm * x)
    my = np.sum(w_norm * y)
    cov = np.sum(w_norm * (x - mx) * (y - my))
    vx = np.sum(w_norm * (x - mx) ** 2)
    vy = np.sum(w_norm * (y - my) ** 2)
    if vx <= 0 or vy <= 0:
        return 0.0
    return float(cov / np.sqrt(vx * vy))


def render_r_style_corrplot(
    df: pd.DataFrame,
    param_dict: Dict[str, str],
    title: str,
    output_prefix: Path,
):
    """
    R corrplot style:
    - Diagonal: Parameter labels
    - Upper triangle: Circles with radius proportional to |r| and colored by r (RdYlBu_r / coolwarm)
    - Lower triangle: Numerical correlation values r with color coding
    """
    params = list(param_dict.keys())
    labels = [param_dict[p] for p in params]
    n = len(params)
    w = np.array(df["weight"].to_numpy(float), copy=True)
    w = w / np.sum(w)

    # Compute correlation matrix
    corr_mat = np.zeros((n, n))
    for i, p1 in enumerate(params):
        for j, p2 in enumerate(params):
            if i == j:
                corr_mat[i, j] = 1.0
            else:
                x1 = df[p1].to_numpy(float)
                x2 = df[p2].to_numpy(float)
                corr_mat[i, j] = weighted_corr(x1, x2, w)

    # Save correlation matrix CSV
    corr_df = pd.DataFrame(corr_mat, index=params, columns=params)
    corr_df.to_csv(output_prefix.parent / f"{output_prefix.name}_correlation_matrix.csv")

    # Plot 1: Classic R corrplot (Circle / Mixed mode)
    fig, ax = plt.subplots(figsize=(10.5, 9.5), dpi=300)
    cmap = plt.cm.coolwarm

    ax.set_xlim(-0.5, n - 0.5)
    ax.set_ylim(-0.5, n - 0.5)
    ax.invert_yaxis()

    # Draw grid background
    for i in range(n + 1):
        ax.axhline(i - 0.5, color="#E5E7EB", lw=1.0)
        ax.axvline(i - 0.5, color="#E5E7EB", lw=1.0)

    for i in range(n):
        for j in range(n):
            r = corr_mat[i, j]
            color = cmap(0.5 + 0.5 * r)

            if i == j:
                # Diagonal: Parameter Label with subtle colored box
                rect = Rectangle((j - 0.48, i - 0.48), 0.96, 0.96, facecolor="#F3F4F6", edgecolor="none", zorder=2)
                ax.add_patch(rect)
                ax.text(j, i, labels[i], ha="center", va="center", fontsize=13, fontweight="bold", color="#1F2937", zorder=3)
            elif j > i:
                # Upper triangle: R-style circles
                radius = 0.44 * np.sqrt(abs(r))
                circle = Circle((j, i), radius, facecolor=color, edgecolor="#374151" if abs(r) > 0.4 else "none", lw=0.8, zorder=3)
                ax.add_patch(circle)
            else:
                # Lower triangle: Correlation value
                font_weight = "bold" if abs(r) >= 0.3 else "normal"
                text_color = color if abs(r) >= 0.25 else "#6B7280"
                ax.text(j, i, f"{r:+.2f}", ha="center", va="center", fontsize=11, fontweight=font_weight, color=text_color, zorder=3)

    ax.set_xticks(range(n))
    ax.set_yticks(range(n))
    ax.set_xticklabels(labels, fontsize=12, fontweight="bold")
    ax.set_yticklabels(labels, fontsize=12, fontweight="bold")
    ax.xaxis.tick_top()
    ax.tick_params(top=False, bottom=False, left=False, right=False)

    for spine in ax.spines.values():
        spine.set_color("#9CA3AF")
        spine.set_linewidth(1.2)

    # Colorbar
    sm = plt.cm.ScalarMappable(cmap=cmap, norm=plt.Normalize(vmin=-1, vmax=1))
    sm.set_array([])
    cbar = fig.colorbar(sm, ax=ax, orientation="vertical", fraction=0.046, pad=0.04, aspect=25)
    cbar.set_label("Weighted Pearson Correlation ($r$)", fontsize=11, fontweight="bold", labelpad=10)
    cbar.ax.tick_params(labelsize=10)

    fig.suptitle(title, fontsize=14, fontweight="bold", y=0.98)
    fig.tight_layout()
    fig.savefig(f"{output_prefix}.png", dpi=300)
    fig.savefig(f"{output_prefix}.pdf")
    plt.close(fig)
    print(f"  -> Saved: {output_prefix}.png")

    # Plot 2: Elegant Full Heatmap with annotations
    fig_hm, ax_hm = plt.subplots(figsize=(9.5, 8.5), dpi=300)
    im = ax_hm.imshow(corr_mat, cmap=cmap, vmin=-1, vmax=1)

    ax_hm.set_xticks(range(n))
    ax_hm.set_yticks(range(n))
    ax_hm.set_xticklabels(labels, fontsize=11.5, fontweight="bold")
    ax_hm.set_yticklabels(labels, fontsize=11.5, fontweight="bold")

    # Overlay correlation values
    for i in range(n):
        for j in range(n):
            r = corr_mat[i, j]
            txt_color = "white" if abs(r) > 0.45 else "#111827"
            ax_hm.text(j, i, f"{r:+.2f}", ha="center", va="center", color=txt_color, fontsize=10.5, fontweight="bold")

    cbar_hm = fig_hm.colorbar(im, ax=ax_hm, fraction=0.046, pad=0.04)
    cbar_hm.set_label("Weighted Pearson Correlation ($r$)", fontsize=11, fontweight="bold")
    ax_hm.set_title(f"{title} (Heatmap Matrix)", fontsize=13, fontweight="bold", pad=15)
    fig_hm.tight_layout()
    fig_hm.savefig(f"{output_prefix}_heatmap.png", dpi=300)
    fig_hm.savefig(f"{output_prefix}_heatmap.pdf")
    plt.close(fig_hm)
    print(f"  -> Saved: {output_prefix}_heatmap.png")


def render_pairgrid_continuous(
    df: pd.DataFrame,
    param_dict: Dict[str, str],
    title: str,
    output_prefix: Path,
    accent_color: str = "#2563EB",
):
    """
    Generates a full pair grid where:
    - Diagonal: Continuous Smooth KDE density curve (no histogram blocks)
    - Lower triangle: Scatter plot with regression line
    - Upper triangle: Heatmap block with correlation score
    """
    params = list(param_dict.keys())
    n = len(params)
    w = np.array(df["weight"].to_numpy(float), copy=True)
    w = w / np.sum(w)

    fig, axes = plt.subplots(n, n, figsize=(14, 14), dpi=300)

    # Compute correlation matrix
    corr_mat = np.zeros((n, n))
    for i, p1 in enumerate(params):
        for j, p2 in enumerate(params):
            if i == j:
                corr_mat[i, j] = 1.0
            else:
                x1 = df[p1].to_numpy(float)
                x2 = df[p2].to_numpy(float)
                corr_mat[i, j] = weighted_corr(x1, x2, w)

    for i, p_row in enumerate(params):
        for j, p_col in enumerate(params):
            ax = axes[i, j]

            # ---------------------------------------------------------
            # Diagonal: Smooth Continuous KDE Density Curve
            # ---------------------------------------------------------
            if i == j:
                x = df[p_row].to_numpy(float)
                grid_x = np.linspace(np.min(x), np.max(x), 150)
                try:
                    kde = stats.gaussian_kde(x, weights=w)
                    dens = kde(grid_x)
                    ax.plot(grid_x, dens, color=accent_color, lw=2.0)
                    ax.fill_between(grid_x, 0, dens, color=accent_color, alpha=0.3)
                except Exception:
                    ax.hist(x, bins=15, weights=w, density=True, color=accent_color, alpha=0.5)

                ax.set_title(param_dict[p_row], fontsize=11, fontweight="bold", pad=4)
                ax.set_ylim(bottom=0)
                ax.tick_params(labelsize=8)
                ax.grid(True, linestyle=":", alpha=0.4)

            # ---------------------------------------------------------
            # Lower Triangle: Scatterplot with size scaled by particle weight
            # ---------------------------------------------------------
            elif i > j:
                x = df[p_col].to_numpy(float)
                y = df[p_row].to_numpy(float)
                r_val = corr_mat[i, j]

                sizes = 10 + 80 * (w / np.max(w))
                ax.scatter(x, y, s=sizes, color=accent_color, alpha=0.4, edgecolor="none")

                # Linear trend line
                poly = np.polyfit(x, y, deg=1, w=np.sqrt(w))
                x_trend = np.linspace(np.min(x), np.max(x), 50)
                y_trend = np.polyval(poly, x_trend)
                ax.plot(x_trend, y_trend, color="#DC2626", lw=1.5, ls="-")

                r_color = "#B91C1C" if abs(r_val) > 0.4 else "#374151"
                ax.text(
                    0.06,
                    0.88,
                    f"r = {r_val:+.2f}",
                    transform=ax.transAxes,
                    fontsize=9.5,
                    fontweight="bold",
                    color=r_color,
                    bbox=dict(boxstyle="round,pad=0.25", facecolor="#F9FAFB", edgecolor="#E5E7EB", alpha=0.85),
                )
                ax.tick_params(labelsize=8)
                ax.grid(True, linestyle=":", alpha=0.4)

            # ---------------------------------------------------------
            # Upper Triangle: Heatmap block with correlation score
            # ---------------------------------------------------------
            else:
                r_val = corr_mat[i, j]
                cmap = plt.cm.coolwarm
                color = cmap(0.5 + 0.5 * r_val)
                rect = plt.Rectangle((0, 0), 1, 1, facecolor=color, alpha=0.75, transform=ax.transAxes)
                ax.add_patch(rect)
                ax.text(
                    0.5,
                    0.5,
                    f"{r_val:+.2f}",
                    transform=ax.transAxes,
                    fontsize=12,
                    fontweight="bold",
                    ha="center",
                    va="center",
                    color="white" if abs(r_val) > 0.45 else "#1F2937",
                )
                ax.set_xticks([])
                ax.set_yticks([])

            if i == n - 1:
                ax.set_xlabel(param_dict[p_col], fontsize=10.5, fontweight="bold")
            else:
                if i != j:
                    ax.set_xticklabels([])

            if j == 0:
                ax.set_ylabel(param_dict[p_row], fontsize=10.5, fontweight="bold")
            else:
                if i != j:
                    ax.set_yticklabels([])

    fig.suptitle(title, fontsize=15, fontweight="bold", y=0.995)
    fig.tight_layout()
    fig.savefig(f"{output_prefix}_pairgrid.png", dpi=300)
    fig.savefig(f"{output_prefix}_pairgrid.pdf")
    plt.close(fig)
    print(f"  -> Saved: {output_prefix}_pairgrid.png")


def main():
    parser = argparse.ArgumentParser(description="Generate publication-grade R-style corrplots and heatmaps.")
    parser.add_argument("--global-dir", type=str, default="results/20260928_draft_map/sensitivity_abc_global")
    parser.add_argument("--locked-dir", type=str, default="results/20260928_draft_map/sensitivity_abc_global_locked")
    parser.add_argument("--output-dir", type=str, default="results/20260928_draft_map/figures_corrplots")
    args = parser.parse_args()

    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    df_glob = pd.read_csv(Path(args.global_dir) / "generation_09.csv")
    df_lock = pd.read_csv(Path(args.locked_dir) / "generation_09.csv")

    print("[1/2] Generating R-Style Corrplot & Heatmap for Global Model...")
    render_r_style_corrplot(
        df=df_glob,
        param_dict=LABELS_GLOBAL,
        title=r"Global Model: Parameter Correlations ($\nu \gg d$)",
        output_prefix=out_dir / "corrplot_global_model",
    )
    render_pairgrid_continuous(
        df=df_glob,
        param_dict=LABELS_GLOBAL,
        title=r"Global Model: Posterior Pairwise Dynamics ($\nu \gg d$)",
        output_prefix=out_dir / "corrplot_global_model",
        accent_color="#2563EB",
    )

    print("[2/2] Generating R-Style Corrplot & Heatmap for Locked Model...")
    render_r_style_corrplot(
        df=df_lock,
        param_dict=LABELS_LOCKED,
        title=r"Locked Model: Parameter Correlations ($\nu = d$)",
        output_prefix=out_dir / "corrplot_locked_model",
    )
    render_pairgrid_continuous(
        df=df_lock,
        param_dict=LABELS_LOCKED,
        title=r"Locked Model: Posterior Pairwise Dynamics ($\nu = d$)",
        output_prefix=out_dir / "corrplot_locked_model",
        accent_color="#DC2626",
    )

    print(f"\n[DONE] All R-style corrplots and heatmaps generated successfully in: {out_dir}")
    return 0


if __name__ == "__main__":
    import sys
    sys.exit(main())
