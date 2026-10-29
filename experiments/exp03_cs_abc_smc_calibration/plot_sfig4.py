#!/usr/bin/env python3
"""
plot_sfig4.py — Publication-grade visualization for Supplementary Figures S4 & S4b.

Reproduces Supplementary Figure S4 and S4b of Palomero et al. (2026):
  Joint and marginal posterior parameter distributions for the canonical parameters
  of the discrete cell-scale model.
  - Variant 4a (Figure S4): Absolute parameters (beta, delta_low, delta_high, nu)
                            in a 4x4 corner matrix.
  - Variant 4b (Figure S4b): 1D posterior density distributions (SIS weighted KDE)
                             and 95% CrIs of all 6 pairwise parameter ratios.

Outputs:
--------
  - experiments/exp03_cs_abc_smc_calibration/FigureS4.pdf / .png / .eps (Variant 4a)
  - experiments/exp03_cs_abc_smc_calibration/FigureS4b.pdf / .png / .eps (Variant 4b)
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
    """Load posterior particles and compute all 6 pairwise parameter ratios."""
    pop_csv = data_dir / "posterior_particles_250.csv"
    if not pop_csv.exists():
        raise FileNotFoundError(f"Missing {pop_csv}")

    df = pd.read_csv(pop_csv)
    w = df["weight"].to_numpy(float)
    w = w / np.sum(w)

    # All 6 pairwise parameter ratios (Variant 4b / Figure S4b)
    beta = df["brca1_rate"]
    df["ratio_dh_dl"] = df["high_delta"] / df["low_delta"]
    df["ratio_dl_beta"] = df["low_delta"] / beta
    df["ratio_nu_beta"] = df["neoplastic_div_rate"] / beta
    df["ratio_nu_dl"] = df["neoplastic_div_rate"] / df["low_delta"]
    df["ratio_dh_beta"] = df["high_delta"] / beta
    df["ratio_nu_dh"] = df["neoplastic_div_rate"] / df["high_delta"]

    return df, w


def render_corner_matrix_4a(df: pd.DataFrame, w: np.ndarray, cfg: VariantConfig, out_dir: Path) -> Tuple[Path, Path]:
    """Renders the canonical 4x4 posterior corner matrix (Variant 4a / Figure S4)."""
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
                    if j > 0:
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
    eps_path = out_dir / f"{cfg.file_stem}.eps"
    fig.savefig(pdf_path, bbox_inches="tight")
    fig.savefig(png_path, dpi=300, bbox_inches="tight")
    fig.savefig(eps_path, format="eps", bbox_inches="tight")
    plt.close(fig)

    return pdf_path, png_path


def render_sfig4b_pairwise_ratios(df: pd.DataFrame, w: np.ndarray, out_dir: Path) -> Tuple[Path, Path]:
    """
    Renders Variant 4b (Figure S4b) as a 2x3 grid displaying 1D posterior distributions (SIS weighted KDE)
    versus parameter ratios across all 6 parameter pairs.
    """
    ratio_specs = [
        # (column, ratio_latex, description, panel_letter)
        ("ratio_dh_dl", r"$\delta_{\rm high} / \delta_{\rm low}$", "Accelerated vs Latent Accumulation", "a"),
        ("ratio_dl_beta", r"$\delta_{\rm low} / \beta$", "Latent Accumulation vs Mutation Rate", "b"),
        ("ratio_nu_beta", r"$\nu / \beta$", "Tumoral Proliferation vs Mutation Rate", "c"),
        ("ratio_nu_dl", r"$\nu / \delta_{\rm low}$", "Tumoral Proliferation vs Latent Accumulation", "d"),
        ("ratio_dh_beta", r"$\delta_{\rm high} / \beta$", "Accelerated Accumulation vs Mutation Rate", "e"),
        ("ratio_nu_dh", r"$\nu / \delta_{\rm high}$", "Tumoral Proliferation vs Accelerated Accumulation", "f"),
    ]

    fig, axes = plt.subplots(2, 3, figsize=(12.0, 7.8), dpi=300)

    # Master title and subtitle
    fig.text(0.06, 0.985, "Supplementary Figure S4b", fontsize=14, fontweight="bold", va="top")
    fig.text(0.06, 0.948,
             r"Posterior probability densities of all pairwise parameter ratios ($R = \theta_i / \theta_j$)",
             fontsize=10.0, color="#374151", va="top")

    for idx, (col_name, ratio_lbl, desc, letter) in enumerate(ratio_specs):
        row, col = divmod(idx, 3)
        ax = axes[row, col]

        vals = df[col_name].to_numpy(float)
        med, q025, q975 = weighted_stats(vals, w)

        val_min, val_max = np.min(vals), np.max(vals)
        val_span = val_max - val_min
        x_min = max(0.0, val_min - 0.15 * val_span)
        x_max = val_max + 0.15 * val_span

        grid_x = np.linspace(x_min, x_max, 300)
        kde = stats.gaussian_kde(vals, weights=w)
        dens = kde(grid_x)

        # 95% CrI mask
        cri_mask = (grid_x >= q025) & (grid_x <= q975)

        # Full distribution shading
        ax.fill_between(grid_x, 0, dens, color="#FED7AA", alpha=0.50, label="Posterior density")
        # 95% Credible Interval shading
        ax.fill_between(grid_x[cri_mask], 0, dens[cri_mask], color="#FDBA74", alpha=0.85, label="95% Credible interval")
        # KDE outline
        ax.plot(grid_x, dens, color="#7C2D12", linewidth=1.4)

        # Particle rug plot
        ax.scatter(vals, np.zeros_like(vals), color="#451A03", alpha=0.35, s=12, marker="|", zorder=3)

        # Reference lines
        ax.axvline(med, color="#C2410C", linewidth=1.6, label=f"Median ({med:.2f})")
        ax.axvline(q025, color="#9A3412", linestyle="--", linewidth=1.0)
        ax.axvline(q975, color="#9A3412", linestyle="--", linewidth=1.0)

        # Spines and styling
        ax.spines["top"].set_visible(False)
        ax.spines["right"].set_visible(False)
        ax.set_xlim(x_min, x_max)
        ax.set_ylim(bottom=0.0, top=np.max(dens) * 1.25)
        ax.set_xlabel(f"Ratio {ratio_lbl}", fontsize=10, labelpad=4)
        if col == 0:
            ax.set_ylabel("Posterior density", fontsize=10, labelpad=4)
        else:
            ax.set_ylabel("")

        ax.set_title(f"({letter}) {ratio_lbl}\n{desc}", fontsize=9.5, fontweight="bold", pad=6)
        ax.tick_params(labelsize=8.5, direction="out", length=2.5)

        # Annotation box
        annot_text = (
            f"Median: {med:.2f}\n"
            f"95% CrI: [{q025:.2f}, {q975:.2f}]"
        )
        ax.text(
            0.96, 0.94, annot_text, transform=ax.transAxes,
            fontsize=8.5, ha="right", va="top",
            bbox=dict(boxstyle="round,pad=0.3", facecolor="white", edgecolor="#D1D5DB", alpha=0.92)
        )

        if idx == 0:
            ax.legend(loc="upper left", fontsize=7.0, frameon=False)

    fig.subplots_adjust(left=0.07, right=0.96, bottom=0.08, top=0.84, hspace=0.45, wspace=0.22)

    pdf_path = out_dir / "FigureS4b.pdf"
    png_path = out_dir / "FigureS4b.png"
    eps_path = out_dir / "FigureS4b.eps"
    fig.savefig(pdf_path, bbox_inches="tight")
    fig.savefig(png_path, dpi=300, bbox_inches="tight")
    fig.savefig(eps_path, format="eps", bbox_inches="tight")
    plt.close(fig)

    return pdf_path, png_path


def render_sfig4(data_dir: Path, out_dir: Path, variant: str = "all") -> List[Tuple[str, Path, Path]]:
    """Generates Figure S4 (variant 4a), Figure S4b (variant 4b), or all (both S4 and S4b)."""
    df, w = load_particles_with_normalized_ratios(data_dir)
    results: List[Tuple[str, Path, Path]] = []

    if variant in ("4a", "s4", "all"):
        pdf_path, png_path = render_corner_matrix_4a(df, w, CONFIG_4A, out_dir)
        results.append(("4a", pdf_path, png_path))

    if variant in ("4b", "s4b", "all"):
        pdf_path, png_path = render_sfig4b_pairwise_ratios(df, w, out_dir)
        results.append(("4b", pdf_path, png_path))

    return results


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate publication-grade Supplementary Figure S4 / S4b.")
    parser.add_argument(
        "--variant",
        type=str,
        choices=["4a", "4b", "s4", "s4b", "all"],
        default="all",
        help="Variant to generate: '4a' / 's4' (canonical corner plot, Figure S4), '4b' / 's4b' (pairwise ratio distributions, Figure S4b), or 'all' (both)",
    )
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
