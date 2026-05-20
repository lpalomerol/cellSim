#!/usr/bin/env python3
"""
generate_figures.py — CellSim calibration figures for paper3 / resumen_para_MA

Generates all 4 publication figures from pre-computed CSV results.
Run from repository root:
    python3 scripts/generate_figures.py

Prerequisites:
    pip install matplotlib
    CSV files in repo root (run scripts/run_experiments.sh first)

Output:
    docs/paper3/fig1_initial_vs_calibrated.{png,pdf}
    docs/paper3/fig2_bigbang_effect.{png,pdf}
    docs/paper3/fig3_calibration_kuchenbaecker.{png,pdf}
    docs/paper3/fig4_bigbang_comparison.{png,pdf}
"""

import csv
import sys
from pathlib import Path

# Always resolve paths relative to repo root (parent of this script's directory)
REPO_ROOT = Path(__file__).resolve().parent.parent
import os; os.chdir(REPO_ROOT)

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

# ---------------------------------------------------------------------------
# Kuchenbaecker 2017 reference data (BRCA1 cohort)
# ---------------------------------------------------------------------------
KUCH_AGES = [30, 40, 50, 60, 70, 80]
KUCH_MEAN = [4.0, 26.0, 46.0, 58.0, 65.0, 70.0]
KUCH_LO   = [2.0, 22.0, 41.0, 52.0, 56.0, 60.0]
KUCH_HI   = [7.0, 30.0, 52.0, 65.0, 73.0, 80.0]

OUT_DIR = Path("docs/paper3")
OUT_DIR.mkdir(parents=True, exist_ok=True)


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def load_cumulative(csv_path: str) -> list[float]:
    """Load per-run onset CSV and return cumulative risk curve (0–100, years 0–100)."""
    counts = [0] * 101
    n = 0
    with open(csv_path) as f:
        for row in csv.DictReader(f):
            n += 1
            yr = row.get("onset_year", "")
            if yr and yr.strip() and float(yr) >= 0:
                y = int(float(yr))
                if 0 <= y <= 100:
                    counts[y] += 1
    cum, s = [], 0.0
    for c in counts:
        s += c / n * 100
        cum.append(s)
    return cum


def save_fig(fig: plt.Figure, name: str) -> None:
    for ext in ("png", "pdf"):
        path = OUT_DIR / f"{name}.{ext}"
        fig.savefig(path, dpi=150, bbox_inches="tight")
        print(f"  Saved {path}")
    plt.close(fig)


def add_kuchenbaecker(ax: plt.Axes, label: bool = True) -> None:
    ax.fill_between(
        KUCH_AGES, KUCH_LO, KUCH_HI,
        alpha=0.2, color="#3498db",
        label="IC 95% Kuchenbaecker" if label else "_nolegend_"
    )
    ax.plot(
        KUCH_AGES, KUCH_MEAN, "o--",
        color="#2980b9", lw=2, ms=7,
        label="Kuchenbaecker 2017" if label else "_nolegend_"
    )


AGES = list(range(101))


# ---------------------------------------------------------------------------
# Figure 1 — Initial params vs calibrated (side-by-side)
# ---------------------------------------------------------------------------

def fig1_initial_vs_calibrated() -> None:
    print("[Fig 1] Initial params vs calibrated...")
    cum_init = load_cumulative("results_initial_params.csv")
    cum_cal  = load_cumulative("results_final_n1000.csv")

    fig, axes = plt.subplots(1, 2, figsize=(13, 5))

    for ax, cum, title, sse, color in [
        (axes[0], cum_init,
         "Parámetros iniciales\n(brca1=0.008, δ=0.15/0.40)", 6139, "#e74c3c"),
        (axes[1], cum_cal,
         "Parámetros calibrados\n(brca1=0.045, δ=0.12/0.24)",   49, "#27ae60"),
    ]:
        add_kuchenbaecker(ax)
        ax.plot(AGES, cum, color=color, lw=2.2, label=f"CellSim (SSE={sse})")
        ax.set_xlim(0, 85)
        ax.set_ylim(0, 105)
        ax.set_xlabel("Edad (años)", fontsize=11)
        ax.set_ylabel("Riesgo acumulado (%)", fontsize=11)
        ax.set_title(title, fontsize=11, fontweight="bold")
        ax.legend(fontsize=9, loc="upper left")
        ax.grid(True, alpha=0.3)

    fig.suptitle(
        "CellSim — Parámetros iniciales vs. calibrados  (Big Bang activo, N=1000)",
        fontsize=12, fontweight="bold"
    )
    plt.tight_layout()
    save_fig(fig, "fig1_initial_vs_calibrated")


# ---------------------------------------------------------------------------
# Figure 2 — Big bang effect (side-by-side, same best params)
# ---------------------------------------------------------------------------

def fig2_bigbang_effect() -> None:
    print("[Fig 2] Big bang effect (side-by-side)...")
    cum_nobb = load_cumulative("results_final_nobb_n1000.csv")
    cum_bb   = load_cumulative("results_final_n1000.csv")

    fig, axes = plt.subplots(1, 2, figsize=(13, 5))

    for ax, cum, title, sse, color in [
        (axes[0], cum_nobb,
         "Sin Big Bang\n(brca1=0.045, δ=0.12/0.24)", 3079, "#e67e22"),
        (axes[1], cum_bb,
         "Con Big Bang\n(brca1=0.045, δ=0.12/0.24)",   49, "#27ae60"),
    ]:
        add_kuchenbaecker(ax)
        ax.plot(AGES, cum, color=color, lw=2.2, label=f"CellSim (SSE={sse})")
        ax.set_xlim(0, 85)
        ax.set_ylim(0, 105)
        ax.set_xlabel("Edad (años)", fontsize=11)
        ax.set_ylabel("Riesgo acumulado (%)", fontsize=11)
        ax.set_title(title, fontsize=11, fontweight="bold")
        ax.legend(fontsize=9, loc="upper left")
        ax.grid(True, alpha=0.3)

    fig.suptitle(
        "CellSim — Efecto del Big Bang (mismos parámetros, N=1000)",
        fontsize=12, fontweight="bold"
    )
    plt.tight_layout()
    save_fig(fig, "fig2_bigbang_effect")


# ---------------------------------------------------------------------------
# Figure 3 — Final calibration vs Kuchenbaecker (single panel, with CI)
# ---------------------------------------------------------------------------

def fig3_calibration_kuchenbaecker() -> None:
    print("[Fig 3] Final calibration vs Kuchenbaecker...")
    cum = load_cumulative("results_final_n1000.csv")

    fig, ax = plt.subplots(figsize=(8, 5))
    add_kuchenbaecker(ax)
    ax.plot(AGES, cum, color="#27ae60", lw=2.5, label="CellSim calibrado (N=1000)")
    ax.set_xlim(0, 85)
    ax.set_ylim(0, 105)
    ax.set_xlabel("Edad (años)", fontsize=12)
    ax.set_ylabel("Riesgo acumulado de cáncer de mama (%)", fontsize=12)
    ax.set_title(
        "CellSim vs. Kuchenbaecker 2017 (BRCA1)\n"
        "brca1=0.045, δ=0.12/0.24, big_bang=true, tp53=0.003",
        fontsize=11, fontweight="bold"
    )
    ax.legend(fontsize=10)
    ax.grid(True, alpha=0.3)

    # Annotate key ages
    kuch_dict = dict(zip(KUCH_AGES, KUCH_MEAN))
    for age in [40, 60, 80]:
        sim_val = cum[age]
        ax.annotate(
            f"{sim_val:.1f}%",
            xy=(age, sim_val), xytext=(age + 1.5, sim_val + 4),
            fontsize=8, color="#27ae60",
            arrowprops=dict(arrowstyle="->", color="#27ae60", lw=0.8)
        )

    plt.tight_layout()
    save_fig(fig, "fig3_calibration_kuchenbaecker")


# ---------------------------------------------------------------------------
# Figure 4 — Big bang comparison (single panel, overlaid)
# ---------------------------------------------------------------------------

def fig4_bigbang_comparison() -> None:
    print("[Fig 4] Big bang comparison (single panel)...")
    cum_nobb = load_cumulative("results_final_nobb_n1000.csv")
    cum_bb   = load_cumulative("results_final_n1000.csv")

    fig, ax = plt.subplots(figsize=(8, 5))
    add_kuchenbaecker(ax)
    ax.plot(AGES, cum_nobb, color="#e67e22", lw=2.2, linestyle="--",
            label="CellSim sin Big Bang (SSE=3079)")
    ax.plot(AGES, cum_bb,   color="#27ae60", lw=2.2,
            label="CellSim con Big Bang (SSE=49)")
    ax.set_xlim(0, 85)
    ax.set_ylim(0, 105)
    ax.set_xlabel("Edad (años)", fontsize=12)
    ax.set_ylabel("Riesgo acumulado (%)", fontsize=12)
    ax.set_title(
        "Impacto del Big Bang — mismos parámetros calibrados (N=1000)",
        fontsize=11, fontweight="bold"
    )
    ax.legend(fontsize=10)
    ax.grid(True, alpha=0.3)
    plt.tight_layout()
    save_fig(fig, "fig4_bigbang_comparison")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    missing = [
        f for f in [
            "results_initial_params.csv",
            "results_final_n1000.csv",
            "results_final_nobb_n1000.csv",
        ]
        if not Path(f).exists()
    ]
    if missing:
        print("ERROR: Missing CSV files (run scripts/run_experiments.sh first):")
        for f in missing:
            print(f"  {f}")
        sys.exit(1)

    print("=" * 70)
    print("CellSim — Figure Generation + Validation")
    print("=" * 70)

    fig1_initial_vs_calibrated()
    fig2_bigbang_effect()
    fig3_calibration_kuchenbaecker()
    fig4_bigbang_comparison()

    print("\n" + "=" * 70)
    print("Figures saved to docs/paper3/")
    print("=" * 70)

    # NEW: Validate final calibration results
    print("\n" + "=" * 70)
    print("Validation")
    print("=" * 70)
    validate_and_report("results_final_n1000.csv")

    print("\n✅ All done. Open docs/paper3/qa_report.html for full report.")
