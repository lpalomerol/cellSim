#!/usr/bin/env python3
"""
Generate figures for Working Document 6 (ABC-SMC calibration).

Figures:
  gen_03_density_bimodality.pdf  — KDE of all 4 posterior marginals
  gen_03_phenotype_scatter.pdf   — Scatter δ_low vs ν with phenotype clusters
"""

from pathlib import Path
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.gridspec import GridSpec
from scipy.stats import gaussian_kde

# ── paths ─────────────────────────────────────────────────────────────────────
ROOT = Path(__file__).resolve().parent.parent.parent
DATA = ROOT / "results" / "abc_final_v4" / "gen_03.csv"
OUT  = ROOT / "docs" / "paper6"

# ── load data ─────────────────────────────────────────────────────────────────
import csv

def load_gen(path):
    rows = list(csv.DictReader(open(path)))
    keys = ["brca1_rate", "low_delta", "high_delta", "neoplastic_div_rate"]
    data = {k: np.array([float(r[k]) for r in rows]) for k in keys + ["weight"]}
    data["weight"] = data["weight"] / data["weight"].sum()
    return data

df = load_gen(DATA)

LABELS = {
    "brca1_rate":          r"$\beta_{\mathrm{BRCA1}}$",
    "low_delta":           r"$\delta_{\mathrm{low}}$ (Selective Pressure)",
    "high_delta":          r"$\delta_{\mathrm{high}}$ (Intrinsic Instability)",
    "neoplastic_div_rate": r"$\nu$ (Neoplastic Division Rate)",
}
PRIORS = {
    "brca1_rate":          (0.008, 0.017),
    "low_delta":           (0.150, 0.450),
    "high_delta":          (0.050, 1.200),
    "neoplastic_div_rate": (0.100, 0.240),
}

PARAMS = list(LABELS.keys())
BLUE   = "#2166AC"
RED    = "#D6604D"
GREY   = "#999999"

# ── Figure 1: KDE marginals ────────────────────────────────────────────────────

def weighted_kde(vals, weights, n_pts=500, bw_factor=1.0):
    """Return (x, y) for a weighted KDE using Silverman's rule."""
    n   = len(vals)
    std = np.sqrt(np.average((vals - np.average(vals, weights=weights))**2,
                             weights=weights))
    bw  = bw_factor * 1.06 * std * n**(-0.2)
    x   = np.linspace(vals.min() - 3*bw, vals.max() + 3*bw, n_pts)
    y   = np.sum(
        weights[:, None] * np.exp(-0.5 * ((x[None, :] - vals[:, None]) / bw)**2)
        / (bw * np.sqrt(2 * np.pi)),
        axis=0
    )
    return x, y


fig, axes = plt.subplots(2, 2, figsize=(11, 8), dpi=300)
axes = axes.flatten()

Q1, Q3 = np.quantile(df["low_delta"], [0.25, 0.75])

for ax, param in zip(axes, PARAMS):
    vals    = df[param]
    w       = df["weight"]
    lo, hi  = PRIORS[param]

    # KDE
    x, y = weighted_kde(vals, w)
    # clip to prior range
    mask = (x >= lo) & (x <= hi)
    ax.plot(x[mask], y[mask], color=BLUE, linewidth=2.0, zorder=3)
    ax.fill_between(x[mask], y[mask], alpha=0.25, color=BLUE, zorder=2)

    # Prior as a horizontal band at the bottom
    ax.axhspan(0, ax.get_ylim()[1] * 0.02 if ax.get_ylim()[1] > 0 else 0.01,
               xmin=0, xmax=1, alpha=0, zorder=0)  # placeholder; drawn after
    ax.axhline(0, color="black", linewidth=0.5, zorder=1)

    # Posterior mean
    mu = np.average(vals, weights=w)
    ax.axvline(mu, color=RED, linestyle="--", linewidth=1.5, label=f"Mean = {mu:.4f}")

    # Q1/Q3 lines only for δ_low
    if param == "low_delta":
        ax.axvline(Q1, color="black", linestyle=":", linewidth=1.2,
                   label=f"$Q_1$ = {Q1:.3f}")
        ax.axvline(Q3, color="black", linestyle=":", linewidth=1.2,
                   label=f"$Q_3$ = {Q3:.3f}")
        # shade the two modes
        xm, ym = weighted_kde(vals, w)
        mode1_mask = (xm >= lo) & (xm < Q1)
        mode2_mask = (xm > Q3) & (xm <= hi)
        ax.fill_between(xm[mode1_mask], ym[mode1_mask],
                        alpha=0.45, color=RED, label="Mode A (Aggressive)")
        ax.fill_between(xm[mode2_mask], ym[mode2_mask],
                        alpha=0.45, color="steelblue", label="Mode B (Basal)")

    # Prior range bar at the bottom
    ax.annotate("", xy=(hi, -ax.get_ylim()[1]*0.04),
                xytext=(lo, -ax.get_ylim()[1]*0.04),
                xycoords="data", textcoords="data",
                annotation_clip=False)

    ax.set_xlabel(LABELS[param], fontsize=11)
    ax.set_ylabel("Posterior density", fontsize=10)
    ax.set_xlim(lo - 0.05*(hi-lo), hi + 0.05*(hi-lo))
    ax.set_ylim(bottom=0)
    ax.legend(fontsize=8, framealpha=0.8)
    ax.grid(True, alpha=0.25, linestyle=":")
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)

fig.suptitle(
    "ABC-SMC Gen 3 — Posterior Marginals (KDE, $N=150$)",
    fontsize=13, fontweight="bold", y=1.01
)
plt.tight_layout()
fig.savefig(OUT / "gen_03_density_bimodality.pdf", dpi=300, bbox_inches="tight")
fig.savefig(OUT / "gen_03_density_bimodality.png", dpi=300, bbox_inches="tight")
print("✅  gen_03_density_bimodality.pdf")

# ── Figure 2: Phenotype scatter ────────────────────────────────────────────────

fig2, axes2 = plt.subplots(1, 2, figsize=(12, 5), dpi=300)

ld = df["low_delta"]
nd = df["neoplastic_div_rate"]
w  = df["weight"]

agg  = ld < Q1
bas  = ld > Q3
mid  = ~(agg | bas)

# ── left: scatter δ_low vs ν ──────────────────────────────────────────────────
ax = axes2[0]
ax.scatter(ld[mid],  nd[mid],  s=55, alpha=0.30, color=GREY,    zorder=2,
           label=f"Transition ($n={mid.sum()}$)")
ax.scatter(ld[agg],  nd[agg],  s=80, alpha=0.75, color=RED,     zorder=4,
           label=f"Phenotype A — Aggressive ($n={agg.sum()}$)")
ax.scatter(ld[bas],  nd[bas],  s=80, alpha=0.75, color=BLUE,    zorder=4,
           label=f"Phenotype B — Basal ($n={bas.sum()}$)")

ax.axvline(Q1, color=RED,  linestyle="--", linewidth=1.2, alpha=0.7,
           label=f"$Q_1 = {Q1:.3f}$")
ax.axvline(Q3, color=BLUE, linestyle="--", linewidth=1.2, alpha=0.7,
           label=f"$Q_3 = {Q3:.3f}$")

# regression line
r = np.corrcoef(ld, nd)[0, 1]
m, b = np.polyfit(ld, nd, 1)
xfit = np.linspace(ld.min(), ld.max(), 200)
ax.plot(xfit, m*xfit + b, color="black", linewidth=1.0, linestyle="-",
        alpha=0.5, label=f"OLS  ($r = {r:.3f}$)")

ax.set_xlabel(r"$\delta_{\mathrm{low}}$ (Selective Pressure)", fontsize=11)
ax.set_ylabel(r"$\nu$ (Neoplastic Division Rate)", fontsize=11)
ax.set_title("Phenotype Bifurcation in $(\delta_{\mathrm{low}}, \\nu)$ Space",
             fontsize=11, fontweight="bold")
ax.legend(fontsize=8, framealpha=0.9, loc="upper right")
ax.grid(True, alpha=0.25, linestyle=":")
ax.spines["top"].set_visible(False)
ax.spines["right"].set_visible(False)

# ── right: side-by-side KDE for δ_low by phenotype ────────────────────────────
ax2 = axes2[1]

# KDE for each cluster
for vals, color, label in [
    (ld[agg], RED,  "Phenotype A (Aggressive)"),
    (ld[bas], BLUE, "Phenotype B (Basal)"),
    (ld,      GREY, "All particles (marginal)"),
]:
    bw = 1.06 * vals.std() * len(vals)**(-0.2)
    x  = np.linspace(0.10, 0.50, 500)
    y  = np.mean(
        np.exp(-0.5 * ((x[None, :] - vals[:, None]) / bw)**2)
        / (bw * np.sqrt(2 * np.pi)),
        axis=0
    )
    if label.startswith("All"):
        ax2.plot(x, y, color=color, linewidth=1.0, linestyle="--",
                 alpha=0.6, label=label)
    else:
        ax2.fill_between(x, y, alpha=0.35, color=color)
        ax2.plot(x, y, color=color, linewidth=2.0, label=label)

ax2.axvline(Q1, color="black", linestyle=":", linewidth=1.2, alpha=0.6,
            label=f"$Q_1 = {Q1:.3f}$, $Q_3 = {Q3:.3f}$")
ax2.axvline(Q3, color="black", linestyle=":", linewidth=1.2, alpha=0.6)

ax2.set_xlabel(r"$\delta_{\mathrm{low}}$ (Selective Pressure)", fontsize=11)
ax2.set_ylabel("Density", fontsize=10)
ax2.set_title(r"KDE of $\delta_{\mathrm{low}}$ by Phenotype",
              fontsize=11, fontweight="bold")
ax2.set_xlim(0.10, 0.50)
ax2.set_ylim(bottom=0)
ax2.legend(fontsize=8, framealpha=0.9)
ax2.grid(True, alpha=0.25, linestyle=":")
ax2.spines["top"].set_visible(False)
ax2.spines["right"].set_visible(False)

fig2.suptitle(
    "Phenotype Identification from ABC-SMC Gen 3 Posterior",
    fontsize=13, fontweight="bold"
)
plt.tight_layout()
fig2.savefig(OUT / "gen_03_phenotype_scatter.pdf", dpi=300, bbox_inches="tight")
fig2.savefig(OUT / "gen_03_phenotype_scatter.png", dpi=300, bbox_inches="tight")
print("✅  gen_03_phenotype_scatter.pdf")

print("\nAll figures saved to", OUT)
