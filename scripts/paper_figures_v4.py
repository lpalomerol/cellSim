#!/usr/bin/env python3
"""
paper_figures_v4.py — Reproducible publication figures for ABC-SMC v4.

Generates:
  paper_fig1_subtypes.png  — 4-panel: parameter space, clinical timeline,
                             progression speed, bimodality
  paper_fig2_summary_table.png — summary statistics table

Two tumour subtypes identified via posterior anticorrelation of
neoplastic_div_rate (nd) and low_delta (ld):
  - Explosive  (nd-dominant): high nd, low ld  → fast progression (~12y gap)
  - Inexorable (ld-dominant): low nd, high ld  → slow progression (~18y gap)

Usage:
    python3 scripts/paper_figures_v4.py [--gen 2] [--results-dir results/abc_final_v4]
"""

import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import matplotlib.gridspec as gridspec
import numpy as np
import pandas as pd
from scipy.stats import gaussian_kde

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

REPO_ROOT = Path(__file__).resolve().parent.parent

# ABC-SMC v4 prior ranges (used for normalisation)
PRIORS = {
    "brca1_rate":          (0.008, 0.017),
    "low_delta":           (0.15,  0.45),
    "high_delta":          (0.05,  1.20),
    "neoplastic_div_rate": (0.10,  0.24),
}

COLOR_EXP  = "#EE6677"   # Explosive  (red)
COLOR_INEX = "#4477AA"   # Inexorable (blue)
ALPHA      = 0.55

SUBTYPE_THRESHOLD = -0.51  # gradient = nd_norm - ld_norm; valley of KDE at gen2


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def load_gen(results_dir: Path, gen: int) -> pd.DataFrame:
    path = results_dir / f"gen_{gen:02d}.csv"
    df = pd.read_csv(path)
    df = df[df["weight"] > 0].copy()
    print(f"Loaded {len(df)} particles from {path.name}")
    return df


def normalise(df: pd.DataFrame, col: str) -> pd.Series:
    lo, hi = PRIORS[col]
    return (df[col] - lo) / (hi - lo)


def compute_gradient(df: pd.DataFrame) -> pd.Series:
    """nd_norm - ld_norm: positive → Explosive, negative → Inexorable."""
    return normalise(df, "neoplastic_div_rate") - normalise(df, "low_delta")


def split_subtypes(df: pd.DataFrame, threshold: float = SUBTYPE_THRESHOLD):
    grad = compute_gradient(df)
    mask_exp  = grad > threshold
    mask_inex = ~mask_exp
    return df[mask_exp].copy(), df[mask_inex].copy(), grad


def valley_peak_ratio(values: np.ndarray) -> float:
    """KDE-based valley/peak ratio as bimodality metric."""
    kde = gaussian_kde(values, bw_method=0.3)
    x   = np.linspace(values.min(), values.max(), 500)
    y   = kde(x)
    peaks  = [y[i] for i in range(1, len(y)-1) if y[i] > y[i-1] and y[i] > y[i+1]]
    valley = [y[i] for i in range(1, len(y)-1) if y[i] < y[i-1] and y[i] < y[i+1]]
    if len(peaks) < 2 or not valley:
        return 1.0
    return min(valley) / np.sort(peaks)[-1]


# ---------------------------------------------------------------------------
# Figure 1 — Four-panel subtype characterisation
# ---------------------------------------------------------------------------

def fig1_subtypes(df: pd.DataFrame, out_dir: Path, gen: int) -> None:
    df_exp, df_inex, grad = split_subtypes(df)

    fig = plt.figure(figsize=(12, 10))
    fig.suptitle(
        f"ABC-SMC v4 — Two tumour subtypes  (gen {gen}, "
        f"n={len(df_exp)} Explosive / {len(df_inex)} Inexorable)",
        fontsize=13, fontweight="bold", y=0.98,
    )

    gs = gridspec.GridSpec(2, 2, figure=fig, hspace=0.38, wspace=0.32)
    ax_a = fig.add_subplot(gs[0, 0])
    ax_b = fig.add_subplot(gs[0, 1])
    ax_c = fig.add_subplot(gs[1, 0])
    ax_d = fig.add_subplot(gs[1, 1])

    # ------------------------------------------------------------------
    # Panel A — Parameter space  (nd vs ld, colour = subtype)
    # ------------------------------------------------------------------
    ax_a.scatter(df_inex["low_delta"], df_inex["neoplastic_div_rate"],
                 c=COLOR_INEX, alpha=ALPHA, s=18, label="Inexorable")
    ax_a.scatter(df_exp["low_delta"],  df_exp["neoplastic_div_rate"],
                 c=COLOR_EXP,  alpha=ALPHA, s=18, label="Explosive")
    r = np.corrcoef(df["low_delta"], df["neoplastic_div_rate"])[0, 1]
    ax_a.set_xlabel("low_delta (Δ₋)", fontsize=10)
    ax_a.set_ylabel("neoplastic_div_rate", fontsize=10)
    ax_a.set_title(f"A — Parameter space  (r={r:.3f})", fontsize=10, fontweight="bold")
    ax_a.legend(fontsize=8)

    # ------------------------------------------------------------------
    # Panel B — Clinical timeline (onset → sat50 per subtype)
    # ------------------------------------------------------------------
    for df_sub, color, label in [
        (df_inex, COLOR_INEX, "Inexorable"),
        (df_exp,  COLOR_EXP,  "Explosive"),
    ]:
        gap = df_sub["sat50"] - df_sub["onset_age"]
        ax_b.scatter(df_sub["onset_age"], df_sub["sat50"],
                     c=color, alpha=ALPHA, s=18, label=f"{label} (gap={gap.median():.1f}y)")
    ax_b.set_xlabel("Tumour onset age (years)", fontsize=10)
    ax_b.set_ylabel("50% saturation age (years)", fontsize=10)
    ax_b.set_title("B — Clinical timeline", fontsize=10, fontweight="bold")
    ax_b.legend(fontsize=8)

    # ------------------------------------------------------------------
    # Panel C — Progression speed histogram (gap = sat50 − onset)
    # ------------------------------------------------------------------
    gap_exp  = (df_exp["sat50"]  - df_exp["onset_age"]).values
    gap_inex = (df_inex["sat50"] - df_inex["onset_age"]).values
    bins = np.linspace(min(gap_exp.min(), gap_inex.min()) - 1,
                       max(gap_exp.max(), gap_inex.max()) + 1, 30)
    ax_c.hist(gap_inex, bins=bins, color=COLOR_INEX, alpha=0.65, label="Inexorable")
    ax_c.hist(gap_exp,  bins=bins, color=COLOR_EXP,  alpha=0.65, label="Explosive")
    ax_c.axvline(np.median(gap_inex), color=COLOR_INEX, lw=2, linestyle="--",
                 label=f"median {np.median(gap_inex):.1f}y")
    ax_c.axvline(np.median(gap_exp),  color=COLOR_EXP,  lw=2, linestyle="--",
                 label=f"median {np.median(gap_exp):.1f}y")
    ax_c.set_xlabel("Progression gap: sat50 − onset (years)", fontsize=10)
    ax_c.set_ylabel("Count", fontsize=10)
    ax_c.set_title("C — Progression speed", fontsize=10, fontweight="bold")
    ax_c.legend(fontsize=8)

    # ------------------------------------------------------------------
    # Panel D — Bimodality: KDE of gradient (nd_norm − ld_norm)
    # ------------------------------------------------------------------
    grad_vals = grad.values
    kde = gaussian_kde(grad_vals, bw_method=0.3)
    x   = np.linspace(grad_vals.min() - 0.05, grad_vals.max() + 0.05, 500)
    y   = kde(x)
    vpr = valley_peak_ratio(grad_vals)

    ax_d.fill_between(x[x <= SUBTYPE_THRESHOLD], y[x <= SUBTYPE_THRESHOLD],
                      color=COLOR_INEX, alpha=0.35, label="Inexorable region")
    ax_d.fill_between(x[x >  SUBTYPE_THRESHOLD], y[x >  SUBTYPE_THRESHOLD],
                      color=COLOR_EXP,  alpha=0.35, label="Explosive region")
    ax_d.plot(x, y, "k-", lw=1.8)
    ax_d.axvline(SUBTYPE_THRESHOLD, color="black", lw=1.2, linestyle="--", alpha=0.6)
    ax_d.set_xlabel("Gradient  (nd_norm − ld_norm)", fontsize=10)
    ax_d.set_ylabel("Density", fontsize=10)
    ax_d.set_title(f"D — Bimodality  (valley/peak={vpr:.2f})", fontsize=10, fontweight="bold")
    ax_d.legend(fontsize=8)

    out_path = out_dir / "paper_fig1_subtypes.png"
    fig.savefig(out_path, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"Saved → {out_path}")


# ---------------------------------------------------------------------------
# Figure 2 — Summary table
# ---------------------------------------------------------------------------

def fig2_summary_table(df: pd.DataFrame, out_dir: Path, gen: int) -> None:
    df_exp, df_inex, _ = split_subtypes(df)

    def stats_row(label, series_exp, series_inex):
        return [
            label,
            f"{series_exp.median():.2f}  (±{series_exp.std():.2f})",
            f"{series_inex.median():.2f}  (±{series_inex.std():.2f})",
        ]

    gap_exp  = df_exp["sat50"]  - df_exp["onset_age"]
    gap_inex = df_inex["sat50"] - df_inex["onset_age"]

    rows = [
        ["Parameter",                  "Explosive",            "Inexorable"],
        ["n particles",                str(len(df_exp)),       str(len(df_inex))],
        *[stats_row(lbl, df_exp[col], df_inex[col]) for col, lbl in [
            ("neoplastic_div_rate", "neoplastic_div_rate"),
            ("low_delta",           "low_delta (Δ₋)"),
            ("high_delta",          "high_delta (Δ₊)"),
            ("brca1_rate",          "brca1_rate"),
            ("onset_age",           "Tumour onset (y)"),
            ("sat50",               "50% saturation (y)"),
        ]],
        stats_row("Progression gap (y)", gap_exp, gap_inex),
    ]

    fig, ax = plt.subplots(figsize=(10, len(rows) * 0.55 + 1))
    ax.axis("off")
    tbl = ax.table(
        cellText=rows[1:],
        colLabels=rows[0],
        cellLoc="center",
        loc="center",
    )
    tbl.auto_set_font_size(False)
    tbl.set_fontsize(10)
    tbl.scale(1.0, 1.6)

    for (row, col), cell in tbl.get_celld().items():
        if row == 0:
            cell.set_facecolor("#2c3e50")
            cell.set_text_props(color="white", fontweight="bold")
        elif col == 1:
            cell.set_facecolor("#fde8ea")   # Explosive (light red)
        elif col == 2:
            cell.set_facecolor("#e8eef8")   # Inexorable (light blue)
        if col == 0:
            cell.set_text_props(fontweight="bold")

    fig.suptitle(
        f"Tumour subtype summary — ABC-SMC v4 gen {gen}",
        fontsize=12, fontweight="bold",
    )

    out_path = out_dir / "paper_fig2_summary_table.png"
    fig.savefig(out_path, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"Saved → {out_path}")


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def parse_args():
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--gen",         type=int,  default=2,
                   help="Generation to load (default: 2)")
    p.add_argument("--results-dir", type=Path,
                   default=REPO_ROOT / "results" / "abc_final_v4",
                   help="Directory containing gen_XX.csv files")
    p.add_argument("--output-dir",  type=Path, default=None,
                   help="Output directory (defaults to --results-dir)")
    p.add_argument("--threshold",   type=float, default=SUBTYPE_THRESHOLD,
                   help="Gradient threshold to split subtypes (default: 0.0)")
    return p.parse_args()


def main():
    args = parse_args()
    out_dir = args.output_dir or args.results_dir
    out_dir.mkdir(parents=True, exist_ok=True)

    global SUBTYPE_THRESHOLD
    SUBTYPE_THRESHOLD = args.threshold

    df = load_gen(args.results_dir, args.gen)
    fig1_subtypes(df, out_dir, args.gen)
    fig2_summary_table(df, out_dir, args.gen)
    print("Done.")


if __name__ == "__main__":
    main()
