#!/usr/bin/env python3
"""
analyse_abc_final.py — Comprehensive analysis of abc_final ABC-SMC results.

Generates:
  1. Marginal posteriors per parameter (prior overlay, all generations)
  2. Pairwise scatter plots of last generation
  3. SSE_w distribution evolution (violin per generation)
  4. Sensitivity analysis: Spearman ρ (params vs SSE_w) + scatter grid
  5. Summary table: median, MAP, 95% CI per parameter

Usage:
    python3 scripts/analyse_abc_final.py [--results-dir PATH] [--output-dir PATH]
"""

import argparse
import sys
from pathlib import Path

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from scipy import stats

REPO_ROOT = Path(__file__).resolve().parent.parent

PRIORS = {
    "brca1_rate":          (0.005, 0.015),
    "low_delta":           (0.20,  0.35),
    "high_delta":          (0.10,  0.60),
    "neoplastic_div_rate": (0.10,  0.22),
}
PARAM_NAMES = list(PRIORS.keys())
PARAM_LABELS = {
    "brca1_rate":          "BRCA1 mutation rate",
    "low_delta":           "Low delta (Δ₋)",
    "high_delta":          "High delta (Δ₊)",
    "neoplastic_div_rate": "Neoplastic division rate",
}

GEN_COLORS = ["#4477AA", "#EE6677", "#228833", "#CCBB44", "#66CCEE", "#AA3377"]


# ---------------------------------------------------------------------------
# Data loading
# ---------------------------------------------------------------------------

def load_generations(results_dir: Path) -> dict[int, pd.DataFrame]:
    gens = {}
    for f in sorted(results_dir.glob("gen_*.csv")):
        g = int(f.stem.split("_")[1])
        df = pd.read_csv(f)
        gens[g] = df
    return gens


def load_epsilon_schedule(results_dir: Path) -> pd.DataFrame | None:
    p = results_dir / "epsilon_schedule.csv"
    return pd.read_csv(p) if p.exists() else None


# ---------------------------------------------------------------------------
# 1. Marginal posteriors
# ---------------------------------------------------------------------------

def plot_marginals(gens: dict, output_dir: Path):
    fig, axes = plt.subplots(1, len(PARAM_NAMES), figsize=(16, 4))
    fig.suptitle("ABC-SMC Posterior Marginals (abc_final)", fontsize=13, y=1.02)

    for ax, param in zip(axes, PARAM_NAMES):
        lo, hi = PRIORS[param]
        # Prior (uniform) — shown as dashed line
        ax.axhline(1 / (hi - lo), color="gray", linestyle="--", linewidth=1.2,
                   label="Prior U[{:.3f},{:.2f}]".format(lo, hi))

        for g_idx, (g, df) in enumerate(sorted(gens.items())):
            color = GEN_COLORS[g_idx % len(GEN_COLORS)]
            label = f"Gen {g} (n={len(df)}, ε={df['sse_weighted'].quantile(0.5):.2f} med)"
            weights = df["weight"] / df["weight"].sum()
            values = df[param].values

            # Weighted KDE
            bw = 1.06 * np.average((values - np.average(values, weights=weights))**2,
                                   weights=weights)**0.5 * len(values)**(-0.2)
            bw = max(bw, (hi - lo) * 0.01)
            x = np.linspace(lo - (hi - lo) * 0.1, hi + (hi - lo) * 0.1, 300)
            kde = np.zeros_like(x)
            for v, w in zip(values, weights):
                kde += w * stats.norm.pdf(x, v, bw)
            kde /= np.trapz(kde, x)
            ax.plot(x, kde, color=color, linewidth=2, label=f"Gen {g}")
            ax.fill_between(x, kde, alpha=0.15, color=color)

        ax.set_xlabel(PARAM_LABELS[param], fontsize=9)
        ax.set_ylabel("Density" if param == PARAM_NAMES[0] else "")
        ax.set_xlim(lo - (hi - lo) * 0.05, hi + (hi - lo) * 0.05)
        ax.legend(fontsize=7)
        ax.set_title(param, fontsize=8)

    plt.tight_layout()
    out = output_dir / "1_marginal_posteriors.png"
    fig.savefig(out, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"  Saved: {out}")


# ---------------------------------------------------------------------------
# 2. Pairwise scatter (last generation)
# ---------------------------------------------------------------------------

def plot_pairwise(gens: dict, output_dir: Path):
    last_gen = max(gens.keys())
    df = gens[last_gen]
    n = len(PARAM_NAMES)
    fig, axes = plt.subplots(n, n, figsize=(12, 12))
    fig.suptitle(f"Pairwise Posterior — Gen {last_gen} (n={len(df)}) — coloured by onset_age",
                 fontsize=12)

    # Colour by onset_age (continuous) if available, otherwise fall back to SSE_w
    if "onset_age" in df.columns:
        colour_vals = df["onset_age"].values
        cmap_name = "plasma"
        cbar_label = "Onset age (years)"
        vmin, vmax = colour_vals.min(), colour_vals.max()
    else:
        colour_vals = df["sse_weighted"].values
        cmap_name = "viridis_r"
        cbar_label = "SSE_w"
        vmin, vmax = colour_vals.min(), colour_vals.max()

    norm = plt.Normalize(vmin=vmin, vmax=vmax)
    colors = plt.cm.get_cmap(cmap_name)(norm(colour_vals))

    for i, p_row in enumerate(PARAM_NAMES):
        for j, p_col in enumerate(PARAM_NAMES):
            ax = axes[i][j]
            if i == j:
                # Diagonal: marginal histogram
                lo, hi = PRIORS[p_row]
                ax.hist(df[p_row], bins=20, weights=df["weight"], color=GEN_COLORS[last_gen % len(GEN_COLORS)],
                        edgecolor="white", linewidth=0.5, density=True)
                ax.axvline(np.average(df[p_row], weights=df["weight"]),
                           color="red", linestyle="--", linewidth=1)
                ax.set_xlim(lo, hi)
            else:
                ax.scatter(df[p_col], df[p_row], c=colors, s=8, alpha=0.7)
                lo_x, hi_x = PRIORS[p_col]
                lo_y, hi_y = PRIORS[p_row]
                ax.set_xlim(lo_x, hi_x)
                ax.set_ylim(lo_y, hi_y)

            if i == n - 1:
                ax.set_xlabel(PARAM_LABELS[p_col], fontsize=7)
            else:
                ax.set_xticklabels([])
            if j == 0:
                ax.set_ylabel(PARAM_LABELS[p_row], fontsize=7)
            else:
                ax.set_yticklabels([])
            ax.tick_params(labelsize=6)

    sm = plt.cm.ScalarMappable(cmap=cmap_name, norm=norm)
    sm.set_array([])
    cbar = fig.colorbar(sm, ax=axes, shrink=0.6, pad=0.02)
    cbar.set_label(cbar_label, fontsize=9)

    out = output_dir / "2_pairwise_scatter.png"
    fig.savefig(out, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"  Saved: {out}")


# ---------------------------------------------------------------------------
# 3. SSE_w evolution
# ---------------------------------------------------------------------------

def plot_sse_evolution(gens: dict, eps_df: pd.DataFrame | None, output_dir: Path):
    fig, axes = plt.subplots(1, 2, figsize=(12, 4))

    # Left: violin
    ax = axes[0]
    data = [gens[g]["sse_weighted"].values for g in sorted(gens)]
    parts = ax.violinplot(data, positions=sorted(gens.keys()), showmedians=True, showextrema=True)
    for i, pc in enumerate(parts["bodies"]):
        pc.set_facecolor(GEN_COLORS[i % len(GEN_COLORS)])
        pc.set_alpha(0.6)
    if eps_df is not None:
        for _, row in eps_df.iterrows():
            g = int(row["generation"])
            if g in gens:
                ax.axhline(row["epsilon"], color=GEN_COLORS[g % len(GEN_COLORS)],
                           linestyle=":", linewidth=1, alpha=0.8)
    ax.set_xticks(sorted(gens.keys()))
    ax.set_xticklabels([f"Gen {g}" for g in sorted(gens.keys())])
    ax.set_ylabel("SSE_w")
    ax.set_title("SSE_w distribution per generation")
    ax.axhline(12.59, color="black", linestyle="--", linewidth=1.2, label="χ²(0.95, df=6)=12.59")
    ax.legend(fontsize=8)

    # Right: % accepted below χ² threshold per generation
    ax2 = axes[1]
    pcts = []
    gen_labels = []
    for g in sorted(gens.keys()):
        df = gens[g]
        pct = (df["sse_weighted"] <= 12.59).mean() * 100
        pcts.append(pct)
        gen_labels.append(f"Gen {g}")
    bars = ax2.bar(gen_labels, pcts, color=[GEN_COLORS[i % len(GEN_COLORS)] for i in range(len(pcts))])
    for bar, pct in zip(bars, pcts):
        ax2.text(bar.get_x() + bar.get_width() / 2, bar.get_height() + 1,
                 f"{pct:.1f}%", ha="center", fontsize=9)
    ax2.set_ylabel("% particles with SSE_w ≤ 12.59")
    ax2.set_title("Acceptance rate per generation")
    ax2.set_ylim(0, 110)

    plt.tight_layout()
    out = output_dir / "3_sse_evolution.png"
    fig.savefig(out, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"  Saved: {out}")


# ---------------------------------------------------------------------------
# 4. Sensitivity analysis
# ---------------------------------------------------------------------------

def plot_sensitivity(gens: dict, output_dir: Path):
    last_gen = max(gens.keys())
    df = gens[last_gen]

    # Spearman correlations
    corrs = {}
    pvals = {}
    for p in PARAM_NAMES:
        r, pv = stats.spearmanr(df[p], df["sse_weighted"])
        corrs[p] = r
        pvals[p] = pv

    fig, axes = plt.subplots(1, 2, figsize=(14, 5))
    fig.suptitle(f"Parameter Sensitivity — Gen {last_gen}", fontsize=12)

    # Bar chart of Spearman ρ
    ax = axes[0]
    names = list(corrs.keys())
    rhos = [corrs[n] for n in names]
    pv   = [pvals[n] for n in names]
    colors_bar = ["#EE6677" if r > 0 else "#4477AA" for r in rhos]
    bars = ax.barh([PARAM_LABELS[n] for n in names], rhos, color=colors_bar, alpha=0.8)
    for bar, pv_val, r in zip(bars, pv, rhos):
        sig = "***" if pv_val < 0.001 else "**" if pv_val < 0.01 else "*" if pv_val < 0.05 else "n.s."
        ax.text(r + (0.01 if r >= 0 else -0.01), bar.get_y() + bar.get_height() / 2,
                f"{sig} (p={pv_val:.3f})", va="center", fontsize=8,
                ha="left" if r >= 0 else "right")
    ax.axvline(0, color="black", linewidth=0.8)
    ax.set_xlabel("Spearman ρ (param vs SSE_w)")
    ax.set_title("Sensitivity: Spearman correlation with SSE_w")
    ax.set_xlim(-1, 1)

    # Scatter grid: each param vs SSE_w
    ax2 = axes[1]
    ax2.axis("off")
    # Print summary table as text
    rows = []
    for p in PARAM_NAMES:
        lo, hi = PRIORS[p]
        vals = df[p].values
        wts  = df["weight"].values / df["weight"].sum()
        median = np.median(vals)
        mean_w = np.average(vals, weights=wts)
        std_w  = np.sqrt(np.average((vals - mean_w)**2, weights=wts))
        ci_lo  = np.percentile(vals, 2.5)
        ci_hi  = np.percentile(vals, 97.5)
        rows.append([PARAM_LABELS[p],
                     f"{mean_w:.4f}", f"{median:.4f}", f"{std_w:.4f}",
                     f"[{ci_lo:.4f}, {ci_hi:.4f}]",
                     f"{corrs[p]:+.3f}"])

    col_labels = ["Parameter", "Mean_w", "Median", "Std_w", "95% CI", "Spearman ρ"]
    table = ax2.table(cellText=rows, colLabels=col_labels,
                      loc="center", cellLoc="center")
    table.auto_set_font_size(False)
    table.set_fontsize(8)
    table.scale(1, 2.0)
    ax2.set_title("Summary statistics — posterior Gen {}".format(last_gen), fontsize=10)

    plt.tight_layout()
    out = output_dir / "4_sensitivity.png"
    fig.savefig(out, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"  Saved: {out}")

    return corrs, pvals


# ---------------------------------------------------------------------------
# 5. Param-vs-SSE_w scatter grid
# ---------------------------------------------------------------------------

def plot_param_vs_sse(gens: dict, output_dir: Path):
    last_gen = max(gens.keys())
    df = gens[last_gen]
    n = len(PARAM_NAMES)
    fig, axes = plt.subplots(1, n, figsize=(16, 4))
    fig.suptitle(f"Parameter vs SSE_w — Gen {last_gen}", fontsize=12)

    for ax, p in zip(axes, PARAM_NAMES):
        lo, hi = PRIORS[p]
        ax.scatter(df[p], df["sse_weighted"], s=10, alpha=0.5, color="#4477AA")
        # LOWESS trend
        from scipy.stats import spearmanr
        r, pv = spearmanr(df[p], df["sse_weighted"])
        ax.axhline(12.59, color="red", linestyle="--", linewidth=1, label="ε=12.59")
        # Linear trend
        m, b, *_ = stats.linregress(df[p], df["sse_weighted"])
        xr = np.linspace(lo, hi, 100)
        ax.plot(xr, m * xr + b, color="orange", linewidth=1.5,
                label=f"ρ={r:+.3f} (p={pv:.3f})")
        ax.set_xlabel(PARAM_LABELS[p], fontsize=9)
        ax.set_ylabel("SSE_w" if p == PARAM_NAMES[0] else "")
        ax.set_xlim(lo, hi)
        ax.legend(fontsize=7)
        ax.set_title(p, fontsize=8)

    plt.tight_layout()
    out = output_dir / "5_param_vs_sse.png"
    fig.savefig(out, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"  Saved: {out}")


# ---------------------------------------------------------------------------
# 6. onset_age vs each parameter (continuous, regression)
# ---------------------------------------------------------------------------

def plot_onset_vs_params(gens: dict, output_dir: Path):
    """Scatter of onset_age vs each free parameter with Spearman ρ + OLS trend."""
    last_gen = max(gens.keys())
    df = gens[last_gen]

    if "onset_age" not in df.columns:
        print("  [skip] onset_age column not found — skipping onset_vs_params figure")
        return

    n = len(PARAM_NAMES)
    fig, axes = plt.subplots(1, n, figsize=(4 * n, 4))
    fig.suptitle(f"Onset age vs parameters — Gen {last_gen} (n={len(df)})", fontsize=12)

    for ax, p in zip(axes, PARAM_NAMES):
        lo, hi = PRIORS[p]
        x = df[p].values
        y = df["onset_age"].values

        r, pv = stats.spearmanr(x, y)
        sig = "***" if pv < 0.001 else "**" if pv < 0.01 else "*" if pv < 0.05 else "n.s."

        # Colour points by SSE_w
        sse_norm = plt.Normalize(df["sse_weighted"].min(), df["sse_weighted"].max())
        colors = plt.cm.viridis_r(sse_norm(df["sse_weighted"].values))
        ax.scatter(x, y, c=colors, s=12, alpha=0.7)

        # OLS trend line
        m, b, *_ = stats.linregress(x, y)
        xr = np.linspace(lo, hi, 100)
        ax.plot(xr, m * xr + b, color="red", linewidth=1.8,
                label=f"ρ={r:+.3f} {sig}")

        ax.set_xlabel(PARAM_LABELS[p], fontsize=9)
        ax.set_ylabel("Onset age (years)" if p == PARAM_NAMES[0] else "")
        ax.set_xlim(lo, hi)
        ax.legend(fontsize=8)
        ax.set_title(p, fontsize=8)

    plt.tight_layout()
    out = output_dir / "6_onset_age_vs_params.png"
    fig.savefig(out, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"  Saved: {out}")


# ---------------------------------------------------------------------------
# 7. Print summary report
# ---------------------------------------------------------------------------

def print_summary(gens: dict, corrs: dict, pvals: dict):
    last_gen = max(gens.keys())
    df = gens[last_gen]
    print("\n" + "=" * 70)
    print(f"  ABC-FINAL — Posterior summary (Gen {last_gen}, n={len(df)})")
    print("=" * 70)
    print(f"  SSE_w:  min={df['sse_weighted'].min():.3f}  "
          f"median={df['sse_weighted'].median():.3f}  "
          f"max={df['sse_weighted'].max():.3f}")
    pct = (df["sse_weighted"] <= 12.59).mean() * 100
    print(f"  Particles ≤ χ²(12.59): {pct:.1f}%\n")

    wts = df["weight"].values / df["weight"].sum()
    print(f"  {'Parameter':<28} {'Mean_w':>8} {'Std_w':>8} {'95% CI':>22}  {'ρ(SSE)':>8}")
    print("  " + "-" * 78)
    for p in PARAM_NAMES:
        vals = df[p].values
        mean_w = np.average(vals, weights=wts)
        std_w  = np.sqrt(np.average((vals - mean_w) ** 2, weights=wts))
        ci_lo  = np.percentile(vals, 2.5)
        ci_hi  = np.percentile(vals, 97.5)
        r = corrs[p]
        pv = pvals[p]
        sig = "***" if pv < 0.001 else "**" if pv < 0.01 else "*" if pv < 0.05 else "n.s."
        print(f"  {PARAM_LABELS[p]:<28} {mean_w:>8.4f} {std_w:>8.4f} "
              f"[{ci_lo:.4f}, {ci_hi:.4f}]  {r:>+.3f} {sig}")

    print("\n  Sensitivity interpretation:")
    print("  |ρ| < 0.2  → weak / negligible influence on SSE_w")
    print("  |ρ| 0.2–0.4 → moderate")
    print("  |ρ| > 0.4  → strong — parameter is identifiable")
    print("=" * 70)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--results-dir", default="results/abc_final",
                        help="Directory with gen_XX.csv files")
    parser.add_argument("--output-dir", default="results/abc_final/figures",
                        help="Where to save figures")
    args = parser.parse_args()

    results_dir = REPO_ROOT / args.results_dir
    output_dir  = REPO_ROOT / args.output_dir
    output_dir.mkdir(parents=True, exist_ok=True)

    gens = load_generations(results_dir)
    if not gens:
        print(f"ERROR: no gen_XX.csv files found in {results_dir}", file=sys.stderr)
        sys.exit(1)

    eps_df = load_epsilon_schedule(results_dir)
    print(f"Loaded generations: {sorted(gens.keys())} from {results_dir}")

    print("\nGenerating figures...")
    plot_marginals(gens, output_dir)
    plot_pairwise(gens, output_dir)
    plot_sse_evolution(gens, eps_df, output_dir)
    corrs, pvals = plot_sensitivity(gens, output_dir)
    plot_param_vs_sse(gens, output_dir)
    plot_onset_vs_params(gens, output_dir)
    print_summary(gens, corrs, pvals)

    print(f"\nAll figures saved to: {output_dir}")


if __name__ == "__main__":
    main()
