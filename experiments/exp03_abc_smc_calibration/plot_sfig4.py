#!/usr/bin/env python3
"""
plot_sfig4.py — Publication-grade visualization for Supplementary Figure S4 & S4b.

Reproduces Supplementary Figure S4 and S4b of Palomero et al. (2026):
  Joint and marginal posterior parameter distributions for the canonical parameters
  of the discrete cell-scale model.
  - Variant 4a (Figure S4): Absolute parameters (beta, delta_low, delta_high, nu)
                            in a 4x4 corner matrix.
  - Variant 4b (Figure S4b): Cross-parameter scatter plots and density contours
                             where the X-axis is normalized by BRCA1 mutation rate (theta / beta),
                             demonstrating parameter decoupling and time-scale identifiability.

Outputs:
--------
  - experiments/exp03_abc_smc_calibration/FigureS4.pdf / .png (Variant 4a)
  - experiments/exp03_abc_smc_calibration/FigureS4b.pdf / .png (Variant 4b)
"""

from __future__ import annotations

import argparse
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Tuple

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from scipy import stats

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


@dataclass
class VariantConfig:
    title: str
    file_stem: str
    params: List[str]
    labels: Dict[str, str]
    axis_limits: Dict[str, Tuple[float, float]]
    ticks: Dict[str, List[float]]


CONFIG_4A = VariantConfig(
    title="Figure S4",
    file_stem="FigureS4",
    params=["brca1_rate", "low_delta", "high_delta", "neoplastic_div_rate"],
    labels={
        "brca1_rate": r"$\beta$",
        "low_delta": r"$\delta_{\rm low}$",
        "high_delta": r"$\delta_{\rm high}$",
        "neoplastic_div_rate": r"$\nu$",
    },
    axis_limits={
        "brca1_rate": (0.038, 0.056),
        "low_delta": (0.075, 0.142),
        "high_delta": (0.120, 0.640),
        "neoplastic_div_rate": (0.080, 0.255),
    },
    ticks={
        "brca1_rate": [0.040, 0.045, 0.050, 0.055],
        "low_delta": [0.08, 0.10, 0.12, 0.14],
        "high_delta": [0.2, 0.4, 0.6],
        "neoplastic_div_rate": [0.10, 0.15, 0.20, 0.25],
    },
)

# Dimension metadata for Variant 4b
PARAM_LABELS = {
    "brca1_rate": r"$\beta$",
    "low_delta": r"$\delta_{\rm low}$",
    "high_delta": r"$\delta_{\rm high}$",
    "neoplastic_div_rate": r"$\nu$",
    "low_delta_norm": r"$\delta_{\rm low} / \beta$",
    "high_delta_norm": r"$\delta_{\rm high} / \beta$",
    "neoplastic_div_rate_norm": r"$\nu / \beta$",
}

AXIS_LIMITS = {
    "brca1_rate": (0.038, 0.056),
    "low_delta": (0.075, 0.142),
    "high_delta": (0.120, 0.640),
    "neoplastic_div_rate": (0.080, 0.255),
    "low_delta_norm": (1.70, 2.90),
    "high_delta_norm": (2.0, 15.0),
    "neoplastic_div_rate_norm": (1.5, 6.5),
}

TICKS = {
    "brca1_rate": [0.040, 0.045, 0.050, 0.055],
    "low_delta": [0.08, 0.10, 0.12, 0.14],
    "high_delta": [0.2, 0.4, 0.6],
    "neoplastic_div_rate": [0.10, 0.15, 0.20, 0.25],
    "low_delta_norm": [1.8, 2.2, 2.6],
    "high_delta_norm": [3.0, 6.0, 9.0, 12.0, 15.0],
    "neoplastic_div_rate_norm": [2.0, 3.5, 5.0, 6.5],
}


def weighted_stats(x: np.ndarray, w: np.ndarray) -> Tuple[float, float, float]:
    """Calculate weighted median, 2.5%, and 97.5% percentiles."""
    idx = np.argsort(x)
    sx = x[idx]
    sw = w[idx]
    cw = np.cumsum(sw) / np.sum(sw)
    q025 = float(np.interp(0.025, cw, sx))
    q50 = float(np.interp(0.50, cw, sx))
    q975 = float(np.interp(0.975, cw, sx))
    return q50, q025, q975


def load_particles_with_normalized_ratios(data_dir: Path) -> Tuple[pd.DataFrame, np.ndarray]:
    """Load posterior particles and compute ratios normalized by brca1_rate (beta)."""
    pop_csv = data_dir / "posterior_particles_250.csv"
    if not pop_csv.exists():
        raise FileNotFoundError(f"Missing {pop_csv}")

    df = pd.read_csv(pop_csv)
    w = df["weight"].to_numpy(float)
    w = w / np.sum(w)

    # Compute normalized ratios relative to beta (brca1_rate)
    beta = df["brca1_rate"]
    df["low_delta_norm"] = df["low_delta"] / beta
    df["high_delta_norm"] = df["high_delta"] / beta
    df["neoplastic_div_rate_norm"] = df["neoplastic_div_rate"] / beta

    return df, w


def render_corner_matrix_4a(df: pd.DataFrame, w: np.ndarray, cfg: VariantConfig, out_dir: Path) -> Tuple[Path, Path]:
    """Renders the canonical 4x4 posterior corner matrix (Variant 4a)."""
    n = len(cfg.params)
    fig, axes = plt.subplots(n, n, figsize=(8.5, 8.5), dpi=300)

    # Master title
    fig.text(0.06, 0.98, cfg.title, fontsize=15, fontweight="bold", va="top")

    for i, p_row in enumerate(cfg.params):
        for j, p_col in enumerate(cfg.params):
            ax = axes[i, j]

            # Diagonal: 1D KDE profile
            if i == j:
                vals = df[p_row].to_numpy(float)
                xlim = cfg.axis_limits[p_row]
                grid_x = np.linspace(xlim[0], xlim[1], 300)

                kde = stats.gaussian_kde(vals, weights=w)
                dens = kde(grid_x)
                dens_max = np.max(dens) if np.max(dens) > 0 else 1.0
                dens_norm = dens / dens_max

                ax.fill_between(grid_x, 0, dens_norm, color="#FED7AA", alpha=0.85)
                ax.plot(grid_x, dens_norm, color="#7C2D12", linewidth=1.1)

                med, q025, q975 = weighted_stats(vals, w)
                ax.axvline(med, color="#C2410C", linewidth=1.2)
                ax.axvline(q025, color="#C2410C", linestyle="--", linewidth=1.0)
                ax.axvline(q975, color="#C2410C", linestyle="--", linewidth=1.0)

                ax.set_xlim(xlim)
                ax.set_ylim(0.0, 1.05)
                ax.set_xticks(cfg.ticks[p_row])
                ax.set_yticks([0.0, 0.2, 0.4, 0.6, 0.8, 1.0])
                ax.set_title(cfg.labels[p_row], fontsize=12, pad=4)
                ax.tick_params(labelsize=8, direction="out", length=2.5)

                if i == 0 and j == 0:
                    ax.set_ylabel(cfg.labels[p_row], fontsize=12)
                else:
                    if j > 0:
                        ax.set_yticklabels([])

                if i == n - 1:
                    ax.set_xlabel(cfg.labels[p_col], fontsize=12)
                else:
                    ax.set_xticklabels([])

            # Lower Triangle: 2D Bivariate Density Contours
            elif i > j:
                x = df[p_col].to_numpy(float)
                y = df[p_row].to_numpy(float)
                xlim = cfg.axis_limits[p_col]
                ylim = cfg.axis_limits[p_row]

                kde2d = stats.gaussian_kde(np.vstack([x, y]), weights=w)
                gx = np.linspace(xlim[0], xlim[1], 120)
                gy = np.linspace(ylim[0], ylim[1], 120)
                X, Y = np.meshgrid(gx, gy)
                Z = kde2d(np.vstack([X.ravel(), Y.ravel()])).reshape(X.shape)
                z_max = np.max(Z) if np.max(Z) > 0 else 1.0
                Z_norm = Z / z_max

                levels = [0.08, 0.22, 0.42, 0.65, 0.88, 1.0]
                ax.contourf(X, Y, Z_norm, levels=levels, cmap="YlOrBr", alpha=0.9)
                ax.contour(X, Y, Z_norm, levels=levels, colors="#78350F", linewidths=0.7, alpha=0.9)

                ax.set_xlim(xlim)
                ax.set_ylim(ylim)
                ax.set_xticks(cfg.ticks[p_col])
                ax.set_yticks(cfg.ticks[p_row])
                ax.tick_params(labelsize=8, direction="out", length=2.5)

                if j == 0:
                    ax.set_ylabel(cfg.labels[p_row], fontsize=12)
                else:
                    ax.set_yticklabels([])

                if i == n - 1:
                    ax.set_xlabel(cfg.labels[p_col], fontsize=12)
                else:
                    ax.set_xticklabels([])

            # Upper Triangle: Blank or Color Gradient Legend Box
            else:
                ax.axis("off")
                if i == 0 and j == 1:
                    rect = plt.Rectangle((0.05, 0.15), 0.90, 0.75, transform=ax.transAxes,
                                         facecolor="white", edgecolor="#111827", linewidth=0.8)
                    ax.add_patch(rect)
                    ax.text(0.10, 0.78, "Normalized posterior density", transform=ax.transAxes,
                            fontsize=9, va="top", color="#111827")

                    cax = ax.inset_axes([0.15, 0.25, 0.10, 0.42])
                    norm = mpl.colors.Normalize(vmin=0.0, vmax=1.0)
                    cb = fig.colorbar(
                        mpl.cm.ScalarMappable(norm=norm, cmap="YlOrBr"),
                        cax=cax, ticks=[0.0, 0.5, 1.0]
                    )
                    cb.ax.tick_params(labelsize=7.5, length=2.0)
                    cb.outline.set_linewidth(0.6)

    fig.subplots_adjust(left=0.10, right=0.95, bottom=0.10, top=0.94, hspace=0.15, wspace=0.15)

    pdf_path = out_dir / f"{cfg.file_stem}.pdf"
    png_path = out_dir / f"{cfg.file_stem}.png"
    fig.savefig(pdf_path, bbox_inches="tight")
    fig.savefig(png_path, dpi=300, bbox_inches="tight")
    plt.close(fig)

    return pdf_path, png_path


def render_sfig4b_cross_grid(df: pd.DataFrame, w: np.ndarray, out_dir: Path) -> Tuple[Path, Path]:
    """
    Renders Variant 4b as a 3x2 cross-parameter scatterplot grid where the X-axis
    is always a normalized rate ratio (theta / beta) and the Y-axis is an unnormalized parameter.
    """
    cross_pairs = [
        # Row 1: delta_low & delta_high
        ("high_delta_norm", "low_delta", r"(a) $\delta_{\rm low}$ vs $\delta_{\rm high} / \beta$"),
        ("low_delta_norm", "high_delta", r"(b) $\delta_{\rm high}$ vs $\delta_{\rm low} / \beta$"),
        # Row 2: delta_low & nu
        ("low_delta_norm", "neoplastic_div_rate", r"(c) $\nu$ vs $\delta_{\rm low} / \beta$"),
        ("neoplastic_div_rate_norm", "low_delta", r"(d) $\delta_{\rm low}$ vs $\nu / \beta$"),
        # Row 3: delta_high & nu
        ("high_delta_norm", "neoplastic_div_rate", r"(e) $\nu$ vs $\delta_{\rm high} / \beta$"),
        ("neoplastic_div_rate_norm", "high_delta", r"(f) $\delta_{\rm high}$ vs $\nu / \beta$"),
    ]

    fig, axes = plt.subplots(3, 2, figsize=(9.5, 11.5), dpi=300)

    # Master title and subtitle explaining structural identifiability
    fig.text(0.06, 0.985, "Supplementary Figure S4b", fontsize=15, fontweight="bold", va="top")
    fig.text(0.06, 0.965,
             "Cross-parameter posterior distributions with rate ratios normalized by BRCA1 mutation rate (X-axis: $\\theta / \\beta$)",
             fontsize=10.5, color="#374151", va="top")

    levels = [0.08, 0.22, 0.42, 0.65, 0.88, 1.0]

    for idx, (x_col, y_col, panel_title) in enumerate(cross_pairs):
        row, col = divmod(idx, 2)
        ax = axes[row, col]

        x = df[x_col].to_numpy(float)
        y = df[y_col].to_numpy(float)
        xlim = AXIS_LIMITS[x_col]
        ylim = AXIS_LIMITS[y_col]

        # 2D weighted KDE contours
        kde2d = stats.gaussian_kde(np.vstack([x, y]), weights=w)
        gx = np.linspace(xlim[0], xlim[1], 120)
        gy = np.linspace(ylim[0], ylim[1], 120)
        X, Y = np.meshgrid(gx, gy)
        Z = kde2d(np.vstack([X.ravel(), Y.ravel()])).reshape(X.shape)
        z_max = np.max(Z) if np.max(Z) > 0 else 1.0
        Z_norm = Z / z_max

        ax.contourf(X, Y, Z_norm, levels=levels, cmap="YlOrBr", alpha=0.85)
        ax.contour(X, Y, Z_norm, levels=levels, colors="#78350F", linewidths=0.7, alpha=0.85)

        # Subtle scatter points showing individual posterior particles
        ax.scatter(x, y, s=14, color="#451A03", alpha=0.35, edgecolors="none")

        # Vertical line for weighted median of X-axis ratio
        med_x, q025_x, q975_x = weighted_stats(x, w)
        ax.axvline(med_x, color="#C2410C", linestyle="--", linewidth=1.1,
                   label=f"Median {PARAM_LABELS[x_col].split()[0]} = {med_x:.2f}")

        # Compute weighted Pearson correlation coefficient r
        mean_x = np.sum(w * x)
        mean_y = np.sum(w * y)
        cov_xy = np.sum(w * (x - mean_x) * (y - mean_y))
        std_x = np.sqrt(np.sum(w * (x - mean_x)**2))
        std_y = np.sqrt(np.sum(w * (y - mean_y)**2))
        r_corr = cov_xy / (std_x * std_y) if (std_x * std_y) > 0 else 0.0

        ax.set_xlim(xlim)
        ax.set_ylim(ylim)
        ax.set_xticks(TICKS[x_col])
        ax.set_yticks(TICKS[y_col])
        ax.set_xlabel(PARAM_LABELS[x_col], fontsize=10, labelpad=4)
        ax.set_ylabel(PARAM_LABELS[y_col], fontsize=10, labelpad=4)
        ax.set_title(panel_title, fontsize=11, fontweight="bold", pad=6)
        ax.tick_params(labelsize=8.5, direction="out", length=2.5)

        # Correlation annotation box
        ax.text(0.95, 0.93, f"$r = {r_corr:+.2f}$", transform=ax.transAxes,
                fontsize=9, ha="right", va="top",
                bbox=dict(boxstyle="round,pad=0.25", facecolor="white", edgecolor="#D1D5DB", alpha=0.9))

    fig.subplots_adjust(left=0.09, right=0.95, bottom=0.06, top=0.93, hspace=0.32, wspace=0.26)

    pdf_path = out_dir / "FigureS4b.pdf"
    png_path = out_dir / "FigureS4b.png"
    fig.savefig(pdf_path, bbox_inches="tight")
    fig.savefig(png_path, dpi=300, bbox_inches="tight")
    plt.close(fig)

    return pdf_path, png_path


def render_sfig4c_self_ratio_strip(df: pd.DataFrame, w: np.ndarray, out_dir: Path) -> Tuple[Path, Path]:
    """
    Renders Variant 4c as a 1x3 horizontal strip where each panel displays a parameter
    against its own normalized rate ratio (X-axis: theta / beta, Y-axis: theta).
    """
    self_pairs = [
        ("low_delta_norm", "low_delta", r"(a) $\delta_{\rm low}$ vs $\delta_{\rm low} / \beta$"),
        ("high_delta_norm", "high_delta", r"(b) $\delta_{\rm high}$ vs $\delta_{\rm high} / \beta$"),
        ("neoplastic_div_rate_norm", "neoplastic_div_rate", r"(c) $\nu$ vs $\nu / \beta$"),
    ]

    fig, axes = plt.subplots(1, 3, figsize=(13.2, 4.4), dpi=300)

    # Master title and subtitle
    fig.text(0.06, 0.985, "Supplementary Figure S4c", fontsize=15, fontweight="bold", va="top")
    fig.text(0.06, 0.930,
             "Self-parameter posterior distributions vs rate ratios normalized by BRCA1 mutation rate (X-axis: $\\theta / \\beta$)",
             fontsize=10.5, color="#374151", va="top")

    levels = [0.08, 0.22, 0.42, 0.65, 0.88, 1.0]

    for idx, (x_col, y_col, panel_title) in enumerate(self_pairs):
        ax = axes[idx]

        x = df[x_col].to_numpy(float)
        y = df[y_col].to_numpy(float)
        xlim = AXIS_LIMITS[x_col]
        ylim = AXIS_LIMITS[y_col]

        # 2D weighted KDE contours
        kde2d = stats.gaussian_kde(np.vstack([x, y]), weights=w)
        gx = np.linspace(xlim[0], xlim[1], 120)
        gy = np.linspace(ylim[0], ylim[1], 120)
        X, Y = np.meshgrid(gx, gy)
        Z = kde2d(np.vstack([X.ravel(), Y.ravel()])).reshape(X.shape)
        z_max = np.max(Z) if np.max(Z) > 0 else 1.0
        Z_norm = Z / z_max

        ax.contourf(X, Y, Z_norm, levels=levels, cmap="YlOrBr", alpha=0.85)
        ax.contour(X, Y, Z_norm, levels=levels, colors="#78350F", linewidths=0.7, alpha=0.85)

        # Subtle scatter points showing individual posterior particles
        ax.scatter(x, y, s=14, color="#451A03", alpha=0.35, edgecolors="none")

        # Vertical line for weighted median of X-axis ratio
        med_x, q025_x, q975_x = weighted_stats(x, w)
        ax.axvline(med_x, color="#C2410C", linestyle="--", linewidth=1.1,
                   label=f"Median {PARAM_LABELS[x_col]} = {med_x:.2f}")

        # Compute weighted Pearson correlation coefficient r
        mean_x = np.sum(w * x)
        mean_y = np.sum(w * y)
        cov_xy = np.sum(w * (x - mean_x) * (y - mean_y))
        std_x = np.sqrt(np.sum(w * (x - mean_x)**2))
        std_y = np.sqrt(np.sum(w * (y - mean_y)**2))
        r_corr = cov_xy / (std_x * std_y) if (std_x * std_y) > 0 else 0.0

        ax.set_xlim(xlim)
        ax.set_ylim(ylim)
        ax.set_xticks(TICKS[x_col])
        ax.set_yticks(TICKS[y_col])
        ax.set_xlabel(PARAM_LABELS[x_col], fontsize=11, labelpad=4)
        ax.set_ylabel(PARAM_LABELS[y_col], fontsize=11, labelpad=4)
        ax.set_title(panel_title, fontsize=12, fontweight="bold", pad=6)
        ax.tick_params(labelsize=9, direction="out", length=2.5)

        # Correlation annotation box
        ax.text(0.95, 0.93, f"$r = {r_corr:+.2f}$", transform=ax.transAxes,
                fontsize=9.5, ha="right", va="top",
                bbox=dict(boxstyle="round,pad=0.25", facecolor="white", edgecolor="#D1D5DB", alpha=0.9))

    fig.subplots_adjust(left=0.06, right=0.97, bottom=0.16, top=0.84, wspace=0.28)

    pdf_path = out_dir / "FigureS4c.pdf"
    png_path = out_dir / "FigureS4c.png"
    fig.savefig(pdf_path, bbox_inches="tight")
    fig.savefig(png_path, dpi=300, bbox_inches="tight")
    plt.close(fig)

    return pdf_path, png_path


def render_sfig4(data_dir: Path, out_dir: Path, variant: str = "all") -> List[Tuple[str, Path, Path]]:
    """Generates Figure S4 (variant 4a), Figure S4b (variant 4b), Figure S4c (variant 4c), or all."""
    df, w = load_particles_with_normalized_ratios(data_dir)
    results: List[Tuple[str, Path, Path]] = []

    if variant in ("4a", "all"):
        pdf_path, png_path = render_corner_matrix_4a(df, w, CONFIG_4A, out_dir)
        results.append(("4a", pdf_path, png_path))

    if variant in ("4b", "all"):
        pdf_path, png_path = render_sfig4b_cross_grid(df, w, out_dir)
        results.append(("4b", pdf_path, png_path))

    if variant in ("4c", "all"):
        pdf_path, png_path = render_sfig4c_self_ratio_strip(df, w, out_dir)
        results.append(("4c", pdf_path, png_path))

    return results


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate publication-grade Supplementary Figure S4 / S4b / S4c.")
    parser.add_argument(
        "--variant",
        type=str,
        choices=["4a", "4b", "4c", "all"],
        default="all",
        help="Variant to generate: '4a' (canonical corner plot), '4b' (cross-parameter ratio grid), '4c' (self-ratio strip), or 'all' (all three)",
    )
    parser.add_argument(
        "--data-dir",
        type=str,
        default="experiments/exp03_abc_smc_calibration/data",
        help="Path to precomputed data directory",
    )
    parser.add_argument(
        "--output-dir",
        type=str,
        default="experiments/exp03_abc_smc_calibration",
        help="Output directory",
    )

    args = parser.parse_args()
    data_dir = Path(args.data_dir)
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    print("=========================================================================")
    print(f"Rendering Supplementary Figure S4 (Variant: {args.variant})")
    print(f"Data directory: {data_dir}")
    print(f"Output directory: {out_dir}")
    print("=========================================================================")

    generated = render_sfig4(data_dir, out_dir, variant=args.variant)

    for var_name, pdf_path, png_path in generated:
        print(f"[SUCCESS] Figure for variant '{var_name}' generated successfully:")
        print(f"  -> Vector: {pdf_path}")
        print(f"  -> Raster: {png_path} (300 DPI)")
    print()
    return 0


if __name__ == "__main__":
    sys.exit(main())
