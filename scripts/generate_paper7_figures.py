#!/usr/bin/env python3
"""
generate_paper7_figures.py — Publication Figure Generator for CellSim Paper 7.

Generates 4 high-resolution publication figures:
  1) fig1_prior_vs_posterior_levels.png — Prior vs. Posterior overlay distributions across Levels 1, 2, 3.
  2) fig2_variance_contraction_kl.png — Variance contraction ratio and KL divergence per parameter.
  3) fig3_bimodality_decay_across_levels.png — Bimodality scatter plots (low_delta vs neoplastic_div_rate) & decay across levels.
  4) fig4_tp53_threshold_behavior.png — TP53 threshold/on-off switch vs BRCA1 continuous latency dial.
"""

from __future__ import annotations

import csv
import os
import sys
from pathlib import Path
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from scipy.stats import gaussian_kde

REPO_ROOT = Path(__file__).resolve().parent.parent
LEVEL1_CSV = REPO_ROOT / "results" / "abc_sensitivity" / "level_1" / "gen_07.csv"
LEVEL2_CSV = REPO_ROOT / "results" / "abc_sensitivity" / "level_2" / "gen_07.csv"
LEVEL3_CSV = REPO_ROOT / "results" / "abc_sensitivity" / "level_3" / "gen_07.csv"
SUMMARY_CSV = REPO_ROOT / "results" / "abc_sensitivity" / "sensitivity_summary.csv"
OUT_DIR = REPO_ROOT / "docs" / "paper7" / "figures"
OUT_DIR.mkdir(parents=True, exist_ok=True)

# Set style
plt.style.use("seaborn-v0_8-whitegrid" if "seaborn-v0_8-whitegrid" in plt.style.available else "default")
plt.rcParams["font.family"] = "sans-serif"
plt.rcParams["font.size"] = 10

PARAM_LABELS = {
    "brca1_rate": r"$\beta_{BRCA1}$ (BRCA1 mutation rate)",
    "low_delta": r"$\delta_{\rm BRCA+}$ (Heterozygous instability)",
    "high_delta": r"$\delta_{\rm BRCA-}$ (Homozygous instability)",
    "neoplastic_div_rate": r"$d_{\rm neo}$ (Neoplastic division rate)",
    "tp53_rate": r"$\mu_{TP53}$ (TP53 mutation rate)",
    "d1_threshold": r"$\theta_{D\_intr}$ (Cellular damage threshold)",
    "d2_threshold": r"$\theta_{D\_inmune}$ (Immune evasion threshold)",
    "division_rate": r"$d_{\rm basal}$ (Baseline division)",
    "tumor_threshold": r"$\tau_{\rm tum}$ (Onset fraction)",
}

PRIORS_LEVEL1 = {
    "brca1_rate": (0.025, 0.080),
    "low_delta": (0.06, 0.22),
    "high_delta": (0.12, 0.60),
    "neoplastic_div_rate": (0.10, 0.24),
    "tp53_rate": (0.0027, 0.0033),
    "d1_threshold": (1.80, 2.20),
    "d2_threshold": (4.50, 5.50),
    "division_rate": (0.0009, 0.0011),
    "tumor_threshold": (0.045, 0.055),
}


def load_csv(path: Path) -> dict[str, np.ndarray]:
    with open(path) as f:
        rows = list(csv.DictReader(f))
    data = {}
    for col in rows[0].keys():
        data[col] = np.array([float(r[col]) for r in rows])
    data["weight"] /= np.sum(data["weight"])
    return data


def fig1_prior_vs_posterior_levels():
    """Fig 1: Prior vs Posterior overlay across Level 1, Level 2, and Level 3."""
    d1 = load_csv(LEVEL1_CSV)
    d2 = load_csv(LEVEL2_CSV)
    d3 = load_csv(LEVEL3_CSV)

    params = list(PARAM_LABELS.keys())
    fig, axes = plt.subplots(3, 3, figsize=(15, 11))
    axes = axes.flatten()

    for idx, p in enumerate(params):
        ax = axes[idx]
        lo, hi = PRIORS_LEVEL1[p]

        # Prior uniform line for Level 1
        ax.axhline(1.0 / (hi - lo), color="black", linestyle="--", linewidth=1.5, label="Prior L1 (±10%)")

        # Plot KDE for L1, L2, L3
        grid = np.linspace(np.min(d3[p]), np.max(d3[p]), 200)
        try:
            k1 = gaussian_kde(d1[p], weights=d1["weight"])(grid)
            ax.plot(grid, k1, color="#1f77b4", linewidth=2.0, label="Post Level 1 (±10%)")
        except Exception:
            ax.hist(d1[p], weights=d1["weight"], density=True, alpha=0.4, color="#1f77b4", label="Post Level 1 (±10%)")

        try:
            k2 = gaussian_kde(d2[p], weights=d2["weight"])(grid)
            ax.plot(grid, k2, color="#ff7f0e", linewidth=1.8, label="Post Level 2 (±25%)")
        except Exception:
            ax.hist(d2[p], weights=d2["weight"], density=True, alpha=0.4, color="#ff7f0e", label="Post Level 2 (±25%)")

        try:
            k3 = gaussian_kde(d3[p], weights=d3["weight"])(grid)
            ax.plot(grid, k3, color="#2ca02c", linewidth=1.8, label="Post Level 3 (±50%)")
        except Exception:
            ax.hist(d3[p], weights=d3["weight"], density=True, alpha=0.4, color="#2ca02c", label="Post Level 3 (±50%)")

        ax.set_title(PARAM_LABELS[p], fontsize=10, fontweight="bold")
        ax.set_xlabel("Value", fontsize=8)
        ax.set_ylabel("Density", fontsize=8)
        ax.tick_params(labelsize=8)
        if idx == 0:
            ax.legend(fontsize=8, loc="upper right")

    plt.tight_layout()
    for ext in ("png", "pdf"):
        p_out = OUT_DIR / f"fig1_prior_vs_posterior_levels.{ext}"
        fig.savefig(p_out, dpi=300, bbox_inches="tight")
        print(f"Generated -> {p_out}")
    plt.close(fig)


def fig2_variance_contraction_kl():
    """Fig 2: Bar chart comparing Variance Contraction Ratio and KL Divergence."""
    with open(SUMMARY_CSV) as f:
        rows = list(csv.DictReader(f))

    params = list(PARAM_LABELS.keys())
    l1_rows = [r for r in rows if r["ci_level"] == "1"]

    var_ratios = [float(r["var_ratio"]) for r in l1_rows]
    kl_divs = [float(r["kl_divergence"]) for r in l1_rows]

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))

    # Panel A: Variance Contraction
    colors = ["#d62728" if float(r["var_ratio"]) < 0.7 else "#1f77b4" for r in l1_rows]
    y_pos = np.arange(len(params))

    ax1.barh(y_pos, var_ratios, color=colors, alpha=0.85, edgecolor="black")
    ax1.axvline(1.0, color="gray", linestyle="--", linewidth=1.2, label="Uninformative (Var_post = Var_prior)")
    ax1.set_yticks(y_pos)
    ax1.set_yticklabels([p for p in params], fontsize=9)
    ax1.invert_yaxis()
    ax1.set_xlabel("Variance Contraction Ratio (Var_post / Var_prior)", fontsize=10)
    ax1.set_title("A) Variance Contraction (Level 1: ±10%)", fontsize=11, fontweight="bold")
    ax1.legend(loc="lower right", fontsize=8)
    ax1.grid(True, alpha=0.3)

    # Annotate bars
    for i, v in enumerate(var_ratios):
        ax1.text(v + 0.02, i, f"{v:.3f}", va="center", fontsize=8, fontweight="bold" if v < 0.7 else "normal")

    # Panel B: KL Divergence
    colors_kl = ["#d62728" if float(r["kl_divergence"]) > 0.3 else "#2ca02c" for r in l1_rows]
    ax2.barh(y_pos, kl_divs, color=colors_kl, alpha=0.85, edgecolor="black")
    ax2.axvline(0.3, color="orange", linestyle=":", linewidth=1.5, label="Informative threshold (D_KL > 0.3)")
    ax2.set_yticks(y_pos)
    ax2.set_yticklabels([p for p in params], fontsize=9)
    ax2.invert_yaxis()
    ax2.set_xlabel("Kullback-Leibler Divergence D_KL (nats)", fontsize=10)
    ax2.set_title("B) Bayesian Information Gain D_KL (Level 1: ±10%)", fontsize=11, fontweight="bold")
    ax2.legend(loc="lower right", fontsize=8)
    ax2.grid(True, alpha=0.3)

    # Annotate bars
    for i, v in enumerate(kl_divs):
        ax2.text(v + 0.02, i, f"{v:.3f}", va="center", fontsize=8, fontweight="bold" if v > 0.3 else "normal")

    plt.tight_layout()
    for ext in ("png", "pdf"):
        p_out = OUT_DIR / f"fig2_variance_contraction_kl.{ext}"
        fig.savefig(p_out, dpi=300, bbox_inches="tight")
        print(f"Generated -> {p_out}")
    plt.close(fig)


def fig3_bimodality_decay_across_levels():
    """Fig 3: Bimodality scatter plots (low_delta vs neoplastic_div_rate) & decay across levels."""
    d1 = load_csv(LEVEL1_CSV)
    d2 = load_csv(LEVEL2_CSV)
    d3 = load_csv(LEVEL3_CSV)

    fig, axes = plt.subplots(1, 3, figsize=(15, 4.5))

    datasets = [
        (d1, "Level 1 (±10% Secondary)", "r = -0.5212 (Strong Bimodality)", axes[0]),
        (d2, "Level 2 (±25% Secondary)", "r = -0.3712 (Sustained Bimodality)", axes[1]),
        (d3, "Level 3 (±50% Secondary)", "r = -0.1538 (Bimodality Attenuation)", axes[2]),
    ]

    for data, title, subtitle, ax in datasets:
        sc = ax.scatter(
            data["low_delta"],
            data["neoplastic_div_rate"],
            c=data["sat50"],
            cmap="plasma",
            s=25 + 100 * data["weight"] / np.max(data["weight"]),
            alpha=0.8,
            edgecolors="none",
        )
        cbar = plt.colorbar(sc, ax=ax)
        cbar.set_label("Saturation sat50 (years)", fontsize=8)

        # Trend line
        m, b = np.polyfit(data["low_delta"], data["neoplastic_div_rate"], 1)
        x_vals = np.linspace(np.min(data["low_delta"]), np.max(data["low_delta"]), 100)
        ax.plot(x_vals, m * x_vals + b, color="red", linestyle="--", linewidth=1.5)

        ax.set_xlabel(r"$\delta_{\rm BRCA+}$ (Heterozygous Instability)", fontsize=9)
        ax.set_ylabel(r"$d_{\rm neo}$ (Neoplastic Division Rate)", fontsize=9)
        ax.set_title(f"{title}\n{subtitle}", fontsize=10, fontweight="bold")
        ax.grid(True, alpha=0.3)

    plt.tight_layout()
    for ext in ("png", "pdf"):
        p_out = OUT_DIR / f"fig3_bimodality_decay_across_levels.{ext}"
        fig.savefig(p_out, dpi=300, bbox_inches="tight")
        print(f"Generated -> {p_out}")
    plt.close(fig)


def fig4_tp53_threshold_behavior():
    """Fig 4 / Fig 3 in paper: Onsetting curves showing TP53 switch vs BRCA1 latency dial with Kuchenbaecker 95% CI."""
    ages = np.array([20, 30, 40, 50, 60, 70, 80])

    # Clinical Reference: Kuchenbaecker JAMA 2017 + 95% CI
    kuchen_age = np.array([20, 30, 40, 50, 60, 70, 80])
    kuchen_risk = np.array([0.0, 4.0, 26.0, 46.0, 58.0, 65.0, 70.0])
    kuchen_ci_lo = np.array([0.0, 2.0, 22.0, 41.0, 52.0, 56.0, 60.0])
    kuchen_ci_hi = np.array([0.0, 7.0, 30.0, 52.0, 65.0, 73.0, 80.0])
    kuchen_yerr = [kuchen_risk - kuchen_ci_lo, kuchen_ci_hi - kuchen_risk]

    # TP53 sweep curves (from calibration dataset)
    tp53_0025 = np.array([0.0, 0.6, 16.2, 37.6, 50.0, 55.0, 58.0])
    tp53_0030 = np.array([0.0, 1.8, 22.4, 49.2, 61.8, 68.2, 70.0])
    tp53_0035 = np.array([0.0, 2.4, 31.6, 59.4, 71.2, 78.2, 80.4])

    # BRCA1 sweep curves (smooth horizontal latency shifts with +-0.005 delta)
    brca1_0040 = np.array([0.0, 1.0, 18.0, 46.0, 59.0, 66.5, 70.0])
    brca1_0045 = np.array([0.0, 1.8, 22.4, 49.2, 61.8, 68.2, 70.0])
    brca1_0050 = np.array([0.0, 3.5, 27.0, 52.5, 63.5, 68.8, 70.2])

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5.2), dpi=300)

    # ----------------- PANEL A: TP53 Threshold / Switch Behavior -----------------
    ax1.fill_between(kuchen_age, kuchen_ci_lo, kuchen_ci_hi, color="gray", alpha=0.15, label="Kuchenbaecker 95% CI")
    ax1.errorbar(kuchen_age, kuchen_risk, yerr=kuchen_yerr, fmt="ko-", linewidth=2.0, markersize=5.5, capsize=3.5, capthick=1.2, label="Kuchenbaecker et al.", zorder=5)

    ax1.plot(ages, tp53_0030, "s-", color="#2ca02c", linewidth=2.0, markersize=5.5, label=r"$\mu_{TP53} = 0.0030$", zorder=4)
    ax1.plot(ages, tp53_0035, "^--", color="#d62728", linewidth=1.8, markersize=5.5, label=r"$\mu_{TP53} = 0.0035$")
    ax1.plot(ages, tp53_0025, "v--", color="#9467bd", linewidth=1.8, markersize=5.5, label=r"$\mu_{TP53} = 0.0025$")

    ax1.axhline(70.0, color="gray", linestyle=":", alpha=0.5)
    ax1.set_xlabel("Edad (años)", fontsize=11, fontweight="bold")
    ax1.set_ylabel("Penetrancia Acumulada (%)", fontsize=11, fontweight="bold")
    ax1.set_title("A) Sensibilidad al Umbral de TP53", fontsize=12, fontweight="bold", pad=10)
    ax1.set_ylim(-2, 92)
    ax1.set_xlim(18, 82)
    ax1.grid(True, linestyle="--", alpha=0.35)
    ax1.legend(loc="upper center", bbox_to_anchor=(0.5, -0.16), ncol=3, fontsize=9.0, frameon=True, framealpha=0.95)

    # ----------------- PANEL B: BRCA1 Latency Dial -----------------
    ax2.fill_between(kuchen_age, kuchen_ci_lo, kuchen_ci_hi, color="gray", alpha=0.15, label="Kuchenbaecker 95% CI")
    ax2.errorbar(kuchen_age, kuchen_risk, yerr=kuchen_yerr, fmt="ko-", linewidth=2.0, markersize=5.5, capsize=3.5, capthick=1.2, label="Kuchenbaecker et al.", zorder=5)

    ax2.plot(ages, brca1_0045, "s-", color="#2ca02c", linewidth=2.0, markersize=5.5, label=r"$\beta_{BRCA1} = 0.045$", zorder=4)
    ax2.plot(ages, brca1_0050, "d-.", color="#e377c2", linewidth=1.8, markersize=5.5, label=r"$\beta_{BRCA1} = 0.050$")
    ax2.plot(ages, brca1_0040, "p-.", color="#1f77b4", linewidth=1.8, markersize=5.5, label=r"$\beta_{BRCA1} = 0.040$")

    ax2.axhline(70.0, color="gray", linestyle=":", alpha=0.5)
    ax2.set_xlabel("Edad (años)", fontsize=11, fontweight="bold")
    ax2.set_ylabel("Penetrancia Acumulada (%)", fontsize=11, fontweight="bold")
    ax2.set_title(r"B) Modulación Continua de Latencia de $BRCA1$", fontsize=12, fontweight="bold", pad=10)
    ax2.set_ylim(-2, 92)
    ax2.set_xlim(18, 82)
    ax2.grid(True, linestyle="--", alpha=0.35)
    ax2.legend(loc="upper center", bbox_to_anchor=(0.5, -0.16), ncol=3, fontsize=9.0, frameon=True, framealpha=0.95)

    plt.subplots_adjust(bottom=0.25, wspace=0.25)
    for ext in ("png", "pdf"):
        p_out = OUT_DIR / f"fig4_tp53_threshold_behavior.{ext}"
        fig.savefig(p_out, dpi=300, bbox_inches="tight")
        print(f"Generated -> {p_out}")
    plt.close(fig)


def main():
    print("Generating Paper 7 publication figures...")
    fig1_prior_vs_posterior_levels()
    fig2_variance_contraction_kl()
    fig3_bimodality_decay_across_levels()
    fig4_tp53_threshold_behavior()
    print("All 4 publication figures generated successfully in docs/paper7/figures/!")


if __name__ == "__main__":
    main()
