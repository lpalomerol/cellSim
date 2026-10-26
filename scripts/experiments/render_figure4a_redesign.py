#!/usr/bin/env python3
"""
render_figure4a_redesign.py — Publication-grade reconstruction and redesign
of Figure 4a (Posterior Parameter Relationships) for BRCA1-ONSET.

Data Source:
  /home/luis/Descargas/BRCA1_ONSET_frozen500_Figure4a_source_data.xlsx

Generates:
  1. Exact replica of Figure 4a style (contour plots + diagonal KDEs with 95% CIs and priors)
  2. Redesigned continuous density versions with smooth KDE contours
  3. Continuous ECDF vs Null Prior comparison for the 4 calibrated parameters
  4. R-style corrplot and publication heatmap
"""

from __future__ import annotations

import argparse
from pathlib import Path
from typing import Dict, List, Tuple

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from scipy import stats
from scipy.ndimage import gaussian_filter

plt.switch_backend("Agg")

# Manuscript parameter priors (Ref: Global Sensitivity Analysis & Manuscript Table)
# Beta ∈ [0.01, 0.10], delta_low ∈ [0.02, 0.20], nu ∈ [0.001, 0.30]
# For delta_high, bounded either as absolute [0.120, 0.600] or by ratio delta_high/delta_low ∈ [1.2, 4.0] (implied [0.024, 0.800]).
# We use [0.120, 0.600] which matches the calibration bounds, but allow global bounds.
PRIORS_GLOBAL = {
    "brca1_rate": (0.010, 0.100),
    "low_delta": (0.020, 0.200),
    "high_delta": (0.120, 0.600),
    "neoplastic_div_rate": (0.001, 0.300),
}

# Calibration priors (restricted stage)
PRIORS = PRIORS_GLOBAL

PARAM_LABELS = {
    "brca1_rate": r"$\beta$",
    "low_delta": r"$\delta_{\rm low}$",
    "high_delta": r"$\delta_{\rm high}$",
    "neoplastic_div_rate": r"$\nu$",
}

PARAM_TITLES = {
    "brca1_rate": r"$\beta$ (BRCA1 Second-Hit Rate)",
    "low_delta": r"$\delta_{\rm low}$ (Instability Heterozygous +/-)",
    "high_delta": r"$\delta_{\rm high}$ (Instability Biallelic -/-)",
    "neoplastic_div_rate": r"$\nu$ (Neoplastic Division Rate)",
}

PARAMS = ["brca1_rate", "low_delta", "high_delta", "neoplastic_div_rate"]


def weighted_stats(x: np.ndarray, w: np.ndarray) -> Tuple[float, float, float]:
    """Calculate median, q025, q975."""
    idx = np.argsort(x)
    sx = x[idx]
    sw = w[idx]
    cw = np.cumsum(sw) / np.sum(sw)
    q025 = float(np.interp(0.025, cw, sx))
    q50 = float(np.interp(0.50, cw, sx))
    q975 = float(np.interp(0.975, cw, sx))
    return q50, q025, q975


def weighted_ecdf(values: np.ndarray, weights: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
    """Compute empirical CDF with SIS weights."""
    idx = np.argsort(values)
    sorted_vals = values[idx]
    sorted_weights = weights[idx]
    cum_w = np.cumsum(sorted_weights)
    cum_w = cum_w / cum_w[-1]
    return sorted_vals, cum_w


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


def render_fig4a_faithful(
    df: pd.DataFrame,
    out_dir: Path,
    bw_1d: float | str | None = None,
    bw_2d: float | str | None = None,
    suffix: str = "",
    use_prior_limits: bool = False,
):
    """
    Renders Fig 4a in the aesthetic of Palomero_Fig4_092326.pdf:
    - Diagonal: 1D KDE profile with vertical median, 95% dashed lines, and orange fill
    - Lower Triangle: 2D KDE contour map with warm orange/yellow gradient levels
    - Upper Triangle: Blank
    """
    w = np.array(df["weight"].to_numpy(float), copy=True)
    w /= np.sum(w)
    n = len(PARAMS)

    fig, axes = plt.subplots(n, n, figsize=(8.8, 8.5), dpi=300)

    for i, p_row in enumerate(PARAMS):
        for j, p_col in enumerate(PARAMS):
            ax = axes[i, j]

            # Diagonal: 1D KDE with 95% CI lines
            if i == j:
                x = df[p_row].to_numpy(float)
                lo, hi = PRIORS[p_row]
                if use_prior_limits:
                    grid_x = np.linspace(lo, hi, 300)
                else:
                    grid_x = np.linspace(np.min(x) * 0.95, np.max(x) * 1.05, 300)

                kde = stats.gaussian_kde(x, weights=w, bw_method=bw_1d)
                dens = kde(grid_x)
                dens_max = np.max(dens) if np.max(dens) > 0 else 1.0
                dens_norm = dens / dens_max

                ax.plot(grid_x, dens_norm, color="#7C2D12", lw=1.2, label="Posterior KDE")
                ax.fill_between(grid_x, 0, dens_norm, color="#FED7AA", alpha=0.7)

                # If showing full prior range, draw uniform prior indicator
                if use_prior_limits:
                    # Draw subtle baseline or prior indicator
                    prior_h = 0.15  # normalized height indicator for prior coverage
                    ax.axhline(prior_h, color="#9CA3AF", ls=":", lw=1.0, alpha=0.8)

                med, q025, q975 = weighted_stats(x, w)
                ax.axvline(med, color="#C2410C", lw=1.4)
                ax.axvline(q025, color="#C2410C", ls="--", lw=1.0)
                ax.axvline(q975, color="#C2410C", ls="--", lw=1.0)

                if use_prior_limits:
                    ax.set_xlim(lo, hi)
                ax.set_ylim(0, 1.05)
                ax.set_title(PARAM_LABELS[p_row], fontsize=13, fontweight="bold", pad=5)
                ax.tick_params(labelsize=8)

            # Lower Triangle: 2D Contour Density
            elif i > j:
                x = df[p_col].to_numpy(float)
                y = df[p_row].to_numpy(float)
                lo_x, hi_x = PRIORS[p_col]
                lo_y, hi_y = PRIORS[p_row]

                # Compute weighted 2D KDE
                try:
                    kde2d = stats.gaussian_kde(np.vstack([x, y]), weights=w, bw_method=bw_2d)
                    if use_prior_limits:
                        gx = np.linspace(lo_x, hi_x, 150)
                        gy = np.linspace(lo_y, hi_y, 150)
                    else:
                        gx = np.linspace(np.min(x) * 0.96, np.max(x) * 1.04, 120)
                        gy = np.linspace(np.min(y) * 0.96, np.max(y) * 1.04, 120)

                    X, Y = np.meshgrid(gx, gy)
                    Z = kde2d(np.vstack([X.ravel(), Y.ravel()])).reshape(X.shape)

                    # Filled contours (orange/yellow palette like original Fig 4a)
                    levels = np.linspace(Z.max() * 0.08, Z.max() * 0.95, 6)
                    ax.contourf(X, Y, Z, levels=levels, cmap="YlOrBr", alpha=0.75)
                    ax.contour(X, Y, Z, levels=levels, colors="#9A3412", linewidths=0.7, alpha=0.8)
                except Exception:
                    ax.scatter(x, y, s=15, color="#EA580C", alpha=0.5)

                if use_prior_limits:
                    ax.set_xlim(lo_x, hi_x)
                    ax.set_ylim(lo_y, hi_y)

                r_val = weighted_corr(x, y, w)
                ax.tick_params(labelsize=8)

            # Upper Triangle: Blank
            else:
                ax.axis("off")

            # Outer Axis labels
            if i == n - 1:
                ax.set_xlabel(PARAM_LABELS[p_col], fontsize=12, fontweight="bold")
            else:
                if i != j:
                    ax.set_xticklabels([])

            if j == 0:
                ax.set_ylabel(PARAM_LABELS[p_row], fontsize=12, fontweight="bold")
            else:
                if i != j:
                    ax.set_yticklabels([])

    subtitle = " (Original Granular / Low-Smooth)" if suffix == "_original_granular" else " (Standard Smooth)"
    fig.suptitle(f"Posterior Parameter Relationships (Figure 4a Reconstruction{subtitle})", fontsize=14, fontweight="bold", y=0.98)
    fig.tight_layout()
    base_name = f"Figure4a_faithful_contours{suffix}"
    fig.savefig(out_dir / f"{base_name}.png", dpi=300)
    fig.savefig(out_dir / f"{base_name}.pdf")
    fig.savefig(out_dir / f"{base_name}.eps", format="eps")
    plt.close(fig)
    print(f"  -> Saved: {out_dir}/{base_name}.png (and .pdf, .eps)")


def render_fig4a_continuous_cdf(df: pd.DataFrame, out_dir: Path):
    """
    Renders 1D continuous smooth densities (KDE) and CDFs vs the theoretical uniform null prior.
    """
    w = np.array(df["weight"].to_numpy(float), copy=True)
    w /= np.sum(w)

    fig, axes = plt.subplots(2, 4, figsize=(16, 7.5), dpi=300)

    for i, p in enumerate(PARAMS):
        ax_dens = axes[0, i]
        ax_cdf = axes[1, i]

        x = df[p].to_numpy(float)
        lo, hi = PRIORS[p]

        # 1. Continuous Density (KDE)
        grid_x = np.linspace(lo, hi, 300)
        kde = stats.gaussian_kde(x, weights=w)
        dens = kde(grid_x)
        prior_dens = np.full_like(grid_x, 1.0 / (hi - lo))

        ax_dens.plot(grid_x, dens, color="#1D4ED8", lw=2.2, label="Posterior KDE")
        ax_dens.fill_between(grid_x, 0, dens, color="#3B82F6", alpha=0.3)
        ax_dens.plot(grid_x, prior_dens, color="#6B7280", ls=":", lw=1.8, label="Uniform Prior")

        med, q025, q975 = weighted_stats(x, w)
        ax_dens.axvline(med, color="#1E3A8A", lw=1.5, ls="-", label=f"Median ({med:.3f})")
        ax_dens.axvline(q025, color="#1E3A8A", lw=1.0, ls="--")
        ax_dens.axvline(q975, color="#1E3A8A", lw=1.0, ls="--")

        ax_dens.set_title(PARAM_TITLES[p], fontsize=11, fontweight="bold")
        ax_dens.set_xlim(lo, hi)
        ax_dens.set_ylim(bottom=0)
        ax_dens.set_xlabel(PARAM_LABELS[p], fontsize=10.5, fontweight="bold")
        ax_dens.set_ylabel("Probability Density", fontsize=10)
        ax_dens.grid(True, linestyle=":", alpha=0.5)
        ax_dens.legend(fontsize=8, loc="upper right")

        # 2. Continuous ECDF vs Null Prior
        sorted_x, cdf_vals = weighted_ecdf(x, w)
        prior_cdf = (grid_x - lo) / (hi - lo)

        ax_cdf.plot(grid_x, prior_cdf, color="#6B7280", ls=":", lw=2.0, label="Theoretical Uniform CDF")
        ax_cdf.step(sorted_x, cdf_vals, where="post", color="#1D4ED8", lw=2.4, label="Posterior ECDF")

        # Kolmogorov-Smirnov test statistic vs Uniform
        ks_stat, pval = stats.kstest(x, stats.uniform(loc=lo, scale=hi-lo).cdf)
        ax_cdf.text(0.05, 0.15, f"KS vs Prior:\nD = {ks_stat:.3f}\np = {pval:.1e}",
                    transform=ax_cdf.transAxes, fontsize=8.5,
                    bbox=dict(boxstyle="round,pad=0.35", facecolor="#F3F4F6", edgecolor="#D1D5DB"))

        ax_cdf.set_title(f"Cumulative Distribution ({PARAM_LABELS[p]})", fontsize=11, fontweight="bold")
        ax_cdf.set_xlim(lo, hi)
        ax_cdf.set_ylim(-0.02, 1.02)
        ax_cdf.set_xlabel(PARAM_LABELS[p], fontsize=10.5, fontweight="bold")
        ax_cdf.set_ylabel("CDF", fontsize=10)
        ax_cdf.grid(True, linestyle=":", alpha=0.5)
        ax_cdf.legend(fontsize=8, loc="upper left")

    fig.suptitle("Calibrated Parameter Posteriors: Continuous Densities and ECDFs vs Null Prior", fontsize=15, fontweight="bold", y=0.99)
    fig.tight_layout()
    fig.savefig(out_dir / "Figure4a_continuous_density_and_cdf_v2.png", dpi=300)
    fig.savefig(out_dir / "Figure4a_continuous_density_and_cdf_v2.pdf")
    fig.savefig(out_dir / "Figure4a_continuous_density_and_cdf_v2.eps", format="eps")
    plt.close(fig)
    print(f"  -> Saved: {out_dir}/Figure4a_continuous_density_and_cdf_v2.png (and .pdf, .eps)")


def render_fig4a_corrplot(df: pd.DataFrame, out_dir: Path):
    """
    Renders an elegant R-style corrplot & heatmap for the 4 calibrated parameters.
    """
    w = np.array(df["weight"].to_numpy(float), copy=True)
    w /= np.sum(w)
    n = len(PARAMS)
    labels = [PARAM_LABELS[p] for p in PARAMS]

    corr_mat = np.zeros((n, n))
    for i, p1 in enumerate(PARAMS):
        for j, p2 in enumerate(PARAMS):
            if i == j:
                corr_mat[i, j] = 1.0
            else:
                x1 = df[p1].to_numpy(float)
                x2 = df[p2].to_numpy(float)
                corr_mat[i, j] = weighted_corr(x1, x2, w)

    # Save CSV
    corr_df = pd.DataFrame(corr_mat, index=PARAMS, columns=PARAMS)
    corr_df.to_csv(out_dir / "Figure4a_correlation_matrix.csv")

    # Heatmap
    fig, ax = plt.subplots(figsize=(6.8, 6.0), dpi=300)
    cmap = plt.cm.coolwarm
    im = ax.imshow(corr_mat, cmap=cmap, vmin=-1, vmax=1)

    ax.set_xticks(range(n))
    ax.set_yticks(range(n))
    ax.set_xticklabels(labels, fontsize=12, fontweight="bold")
    ax.set_yticklabels(labels, fontsize=12, fontweight="bold")

    for i in range(n):
        for j in range(n):
            r = corr_mat[i, j]
            txt_color = "white" if abs(r) > 0.45 else "#111827"
            ax.text(j, i, f"{r:+.2f}", ha="center", va="center", color=txt_color, fontsize=12, fontweight="bold")

    cbar = fig.colorbar(im, ax=ax, fraction=0.046, pad=0.04)
    cbar.set_label("Weighted Pearson Correlation ($r$)", fontsize=11, fontweight="bold")
    ax.set_title(r"Posterior Parameter Correlations ($\mathbf{Figure\ 4a}$)", fontsize=13, fontweight="bold", pad=12)
    fig.tight_layout()
    fig.savefig(out_dir / "Figure4a_corrplot_heatmap.png", dpi=300)
    fig.savefig(out_dir / "Figure4a_corrplot_heatmap.pdf")
    fig.savefig(out_dir / "Figure4a_corrplot_heatmap.eps", format="eps")
    plt.close(fig)
    print(f"  -> Saved: {out_dir}/Figure4a_corrplot_heatmap.png (and .pdf, .eps)")


def main():
    parser = argparse.ArgumentParser(description="Render Figure 4a redesigns from frozen source Excel.")
    parser.add_argument("--excel", type=str, default="/home/luis/Descargas/BRCA1_ONSET_frozen500_Figure4a_source_data.xlsx")
    parser.add_argument("--output-dir", type=str, default="results/20260928_draft_map/figure4a_redesign")
    args = parser.parse_args()

    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    print(f"Loading frozen Figure 4a source data: {args.excel}")
    df = pd.read_excel(args.excel)
    print(f"Loaded {len(df)} posterior particles.")

    print("\n[1a/4] Generating Fig 4a (Original Granular Bandwidth / Low-Smooth)...")
    render_fig4a_faithful(df, out_dir, bw_1d=0.09, bw_2d=0.28, suffix="_original_granular")

    print("\n[1b/4] Generating Fig 4a (Standard Smooth Bandwidth)...")
    render_fig4a_faithful(df, out_dir, bw_1d=None, bw_2d=None, suffix="_smooth")
    # Also save standard without suffix for backwards compatibility
    render_fig4a_faithful(df, out_dir, bw_1d=None, bw_2d=None, suffix="")

    print("\n[1c/4] Generating Fig 4a v2 (Full Prior Distribution Ranges)...")
    render_fig4a_faithful(df, out_dir, bw_1d=None, bw_2d=None, suffix="_v2", use_prior_limits=True)
    render_fig4a_faithful(df, out_dir, bw_1d=0.09, bw_2d=0.28, suffix="_v2_granular", use_prior_limits=True)

    print("\n[2/4] Generating Continuous Densities & ECDFs vs Null Prior...")
    render_fig4a_continuous_cdf(df, out_dir)

    print("\n[3/4] Generating Elegant Heatmap Corrplot...")
    render_fig4a_corrplot(df, out_dir)

    print(f"\n[DONE] All Figure 4a redesign figures generated successfully in: {out_dir}")


if __name__ == "__main__":
    main()
