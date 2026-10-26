#!/usr/bin/env python3
"""
plot_distributions_cdf.py — Publication-grade continuous density curves (weighted KDE)
and Empirical Cumulative Distribution Functions (ECDF) for each parameter comparing:
  1. Global Model (accelerated expansion nu)
  2. Locked Model (unaccelerated division nu = d)
  3. Theoretical Null Prior (Uniform density & CDF)

Generates:
  - Individual figures per parameter: <param>_hist_cdf.png and .pdf
  - Combined 3x3 multi-panel figure: all_parameters_hist_grid.png and .pdf (smooth densities)
  - Combined 3x3 multi-panel ECDF figure with KS-test statistics vs Prior: all_parameters_cdf_grid.png and .pdf
"""

from __future__ import annotations

import argparse
from pathlib import Path
from typing import Dict, List, Tuple

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from scipy import stats

plt.switch_backend("Agg")

# Comprehensive parameter priors
PRIORS: Dict[str, Tuple[float, float]] = {
    "brca1_rate":          (0.010, 0.080),
    "low_delta":           (0.050, 0.250),
    "high_delta":          (0.120, 0.600),
    "neoplastic_div_rate": (0.001, 0.250),
    "tp53_rate":           (0.001, 0.010),
    "d1_threshold":        (1.0,   4.0),
    "d2_threshold":        (3.0,   8.0),
    "division_rate":       (0.0005, 0.005),
    "kappa_brca1":         (1.0,   4.0),
}

PARAM_LABELS: Dict[str, str] = {
    "brca1_rate": r"$\beta$ (BRCA1 Second-Hit Rate)",
    "low_delta": r"$\delta_{\rm low}$ (Instability Heterozygous +/-)",
    "high_delta": r"$\delta_{\rm high}$ (Instability Biallelic -/-)",
    "neoplastic_div_rate": r"$\nu$ (Neoplastic Division Rate)",
    "tp53_rate": r"$\mu_{\rm TP53}$ (TP53 Loss Rate)",
    "d1_threshold": r"$\theta_{D1}$ (PRIMER Gate Threshold)",
    "d2_threshold": r"$\theta_{D2}$ (Immune Evasion / TUMORAL Gate)",
    "division_rate": r"$d$ (Basal Tissue Division Rate)",
    "kappa_brca1": r"$\kappa_{\rm BRCA1}$ (BRCA1 Accumulator Multiplier)",
}


def weighted_ecdf(values: np.ndarray, weights: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
    """Compute empirical CDF incorporating SIS particle weights."""
    idx = np.argsort(values)
    sorted_vals = values[idx]
    sorted_weights = weights[idx]
    cum_w = np.cumsum(sorted_weights)
    cum_w = cum_w / cum_w[-1]
    return sorted_vals, cum_w


def weighted_kde(values: np.ndarray, weights: np.ndarray, grid: np.ndarray, bw_method="scott") -> np.ndarray:
    """Compute smooth continuous probability density using weighted Gaussian KDE."""
    kde = stats.gaussian_kde(values, weights=weights, bw_method=bw_method)
    return kde(grid)


def main():
    parser = argparse.ArgumentParser(description="Generate continuous densities and CDFs comparing Global vs Locked posteriors against prior.")
    parser.add_argument("--global-dir", type=str, default="results/20260928_draft_map/sensitivity_abc_global")
    parser.add_argument("--locked-dir", type=str, default="results/20260928_draft_map/sensitivity_abc_global_locked")
    parser.add_argument("--output-dir", type=str, default="results/20260928_draft_map/figures_distributions_cdf")
    args = parser.parse_args()

    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    df_glob = pd.read_csv(Path(args.global_dir) / "generation_09.csv")
    df_lock = pd.read_csv(Path(args.locked_dir) / "generation_09.csv")

    w_glob = np.array(df_glob["weight"].to_numpy(float), copy=True)
    w_glob = w_glob / np.sum(w_glob)

    w_lock = np.array(df_lock["weight"].to_numpy(float), copy=True)
    w_lock = w_lock / np.sum(w_lock)

    # -------------------------------------------------------------
    # 1. Individual 2-panel figure per parameter (Continuous Density + CDF)
    # -------------------------------------------------------------
    for param, (lo, hi) in PRIORS.items():
        fig, (ax_dens, ax_cdf) = plt.subplots(1, 2, figsize=(11.5, 4.6), dpi=300)

        # Theoretical Null Prior
        grid_x = np.linspace(lo, hi, 300)
        prior_cdf = (grid_x - lo) / (hi - lo)
        prior_density = np.full_like(grid_x, 1.0 / (hi - lo))

        # Global Model
        vals_glob = df_glob[param].to_numpy(float)
        dens_g = weighted_kde(vals_glob, w_glob, grid_x)
        sorted_g, cdf_g = weighted_ecdf(vals_glob, w_glob)

        # Plot Smooth Densities
        ax_dens.plot(grid_x, dens_g, color="#1D4ED8", lw=2.4, label="Global Posterior")
        ax_dens.fill_between(grid_x, 0, dens_g, color="#3B82F6", alpha=0.3)

        has_locked = param in df_lock.columns
        if has_locked:
            vals_lock = df_lock[param].to_numpy(float)
            dens_l = weighted_kde(vals_lock, w_lock, grid_x)
            sorted_l, cdf_l = weighted_ecdf(vals_lock, w_lock)

            ax_dens.plot(grid_x, dens_l, color="#DC2626", lw=2.4, ls="--", label="Locked Posterior")
            ax_dens.fill_between(grid_x, 0, dens_l, color="#EF4444", alpha=0.25)

        ax_dens.plot(grid_x, prior_density, color="#4B5563", ls=":", lw=2, label="Null Prior (Uniform)")

        ax_dens.set_xlabel(PARAM_LABELS[param], fontsize=11, fontweight="bold")
        ax_dens.set_ylabel("Posterior Probability Density", fontsize=11, fontweight="bold")
        ax_dens.set_title(f"Continuous Posterior Density: {PARAM_LABELS[param]}", fontsize=12, fontweight="bold")
        ax_dens.set_xlim(lo, hi)
        ax_dens.set_ylim(bottom=0)
        ax_dens.grid(True, linestyle=":", alpha=0.6)
        ax_dens.legend(frameon=True, fontsize=9.5)

        # Plot CDFs
        ax_cdf.plot(grid_x, prior_cdf, color="#4B5563", ls=":", lw=2.2, label="Null Prior (Theoretical Uniform)")
        ax_cdf.step(sorted_g, cdf_g, where="post", color="#1D4ED8", lw=2.5, label="Global Model ECDF")
        if has_locked:
            ax_cdf.step(sorted_l, cdf_l, where="post", color="#DC2626", lw=2.5, ls="--", label="Locked Model ECDF")

        # Compute KS-test statistic against Uniform
        ks_stat_g, pval_g = stats.kstest(vals_glob, stats.uniform(loc=lo, scale=hi-lo).cdf)
        ks_text = f"KS vs Prior (Global): D={ks_stat_g:.2f} (p={pval_g:.1e})"
        if has_locked:
            ks_stat_l, pval_l = stats.kstest(vals_lock, stats.uniform(loc=lo, scale=hi-lo).cdf)
            ks_text += f"\nKS vs Prior (Locked): D={ks_stat_l:.2f} (p={pval_l:.1e})"
        ax_cdf.text(0.04, 0.15, ks_text, transform=ax_cdf.transAxes, fontsize=8.5,
                    bbox=dict(boxstyle="round,pad=0.4", facecolor="#F3F4F6", edgecolor="#D1D5DB"))

        ax_cdf.set_xlabel(PARAM_LABELS[param], fontsize=11, fontweight="bold")
        ax_cdf.set_ylabel("Cumulative Density Function (CDF)", fontsize=11, fontweight="bold")
        ax_cdf.set_title(f"Empirical CDF vs Null Prior: {PARAM_LABELS[param]}", fontsize=12, fontweight="bold")
        ax_cdf.set_xlim(lo, hi)
        ax_cdf.set_ylim(-0.02, 1.02)
        ax_cdf.grid(True, linestyle=":", alpha=0.6)
        ax_cdf.legend(frameon=True, fontsize=9.5, loc="upper left")

        fig.tight_layout()
        fig.savefig(out_dir / f"{param}_hist_cdf.png", dpi=300)
        fig.savefig(out_dir / f"{param}_hist_cdf.pdf")
        plt.close(fig)

    # -------------------------------------------------------------
    # 2. Combined 3x3 Grid: Cumulative Density Functions (ECDF)
    # -------------------------------------------------------------
    fig, axes = plt.subplots(3, 3, figsize=(15, 12), dpi=300)
    axes = axes.flatten()

    for i, (param, (lo, hi)) in enumerate(PRIORS.items()):
        ax = axes[i]
        prior_x = np.linspace(lo, hi, 200)
        prior_cdf = (prior_x - lo) / (hi - lo)

        vals_glob = df_glob[param].to_numpy(float)
        sorted_g, cdf_g = weighted_ecdf(vals_glob, w_glob)

        ax.plot(prior_x, prior_cdf, color="#6B7280", ls=":", lw=1.8, label="Uniform Prior")
        ax.step(sorted_g, cdf_g, where="post", color="#1D4ED8", lw=2.2, label="Global")

        if param in df_lock.columns:
            vals_lock = df_lock[param].to_numpy(float)
            sorted_l, cdf_l = weighted_ecdf(vals_lock, w_lock)
            ax.step(sorted_l, cdf_l, where="post", color="#DC2626", lw=2.0, ls="--", label="Locked")

        ax.set_title(PARAM_LABELS[param], fontsize=11, fontweight="bold")
        ax.set_xlim(lo, hi)
        ax.set_ylim(-0.02, 1.02)
        ax.set_xlabel("Value", fontsize=9.5)
        ax.set_ylabel("CDF", fontsize=9.5)
        ax.grid(True, linestyle=":", alpha=0.5)
        if i == 0:
            ax.legend(frameon=True, fontsize=8.5, loc="upper left")

    fig.suptitle("Cumulative Density Functions (ECDF) vs Null Prior across All Parameters", fontsize=15, fontweight="bold", y=0.995)
    fig.tight_layout()
    fig.savefig(out_dir / "all_parameters_cdf_grid.png", dpi=300)
    fig.savefig(out_dir / "all_parameters_cdf_grid.pdf")
    plt.close(fig)

    # -------------------------------------------------------------
    # 3. Combined 3x3 Grid: Continuous Densities (Smooth KDE)
    # -------------------------------------------------------------
    fig, axes = plt.subplots(3, 3, figsize=(15, 12), dpi=300)
    axes = axes.flatten()

    for i, (param, (lo, hi)) in enumerate(PRIORS.items()):
        ax = axes[i]
        grid_x = np.linspace(lo, hi, 300)
        prior_density = np.full_like(grid_x, 1.0 / (hi - lo))

        vals_glob = df_glob[param].to_numpy(float)
        dens_g = weighted_kde(vals_glob, w_glob, grid_x)

        ax.plot(grid_x, dens_g, color="#1D4ED8", lw=2.2, label="Global")
        ax.fill_between(grid_x, 0, dens_g, color="#3B82F6", alpha=0.3)

        if param in df_lock.columns:
            vals_lock = df_lock[param].to_numpy(float)
            dens_l = weighted_kde(vals_lock, w_lock, grid_x)
            ax.plot(grid_x, dens_l, color="#DC2626", lw=2.2, ls="--", label="Locked")
            ax.fill_between(grid_x, 0, dens_l, color="#EF4444", alpha=0.25)

        ax.plot(grid_x, prior_density, color="#4B5563", ls=":", lw=1.6, label="Prior")
        ax.set_title(PARAM_LABELS[param], fontsize=11, fontweight="bold")
        ax.set_xlim(lo, hi)
        ax.set_ylim(bottom=0)
        ax.set_xlabel("Value", fontsize=9.5)
        ax.set_ylabel("Density", fontsize=9.5)
        ax.grid(True, linestyle=":", alpha=0.5)
        if i == 0:
            ax.legend(frameon=True, fontsize=8.5, loc="upper right")

    fig.suptitle("Continuous Posterior Density Distributions vs Null Prior across All Parameters", fontsize=15, fontweight="bold", y=0.995)
    fig.tight_layout()
    fig.savefig(out_dir / "all_parameters_hist_grid.png", dpi=300)
    fig.savefig(out_dir / "all_parameters_hist_grid.pdf")
    plt.close(fig)

    print(f"\n[DONE] Continuous Densities and CDFs successfully generated in: {out_dir}")
    return 0


if __name__ == "__main__":
    import sys
    sys.exit(main())
