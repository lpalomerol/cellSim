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
# Figure P4-1 — Paper4: weighted SSE calibration vs Kuchenbaecker
# ---------------------------------------------------------------------------

def fig_p4_calibration() -> None:
    """Paper4 fig: weighted-SSE optimal params vs Kuchenbaecker + paper3 comparison."""
    out_dir = Path("docs/paper4")
    out_dir.mkdir(parents=True, exist_ok=True)

    cum_p3  = load_cumulative("results_final_n1000.csv")
    cum_p4  = load_cumulative("results_p4_final_n1000.csv")

    fig, ax = plt.subplots(figsize=(8, 5))
    add_kuchenbaecker(ax)

    ax.plot(AGES, cum_p3, "-", color="#e74c3c", lw=2, label="Paper3 (brca1=0.045, ld=0.120)")
    ax.plot(AGES, cum_p4, "-", color="#27ae60", lw=2.5, label="Paper4 weighted SSE (brca1=0.050, ld=0.140)")

    # Mark the 6 Kuchenbaecker anchor ages
    p4_vals = [cum_p4[a] for a in KUCH_AGES]
    ax.scatter(KUCH_AGES, p4_vals, color="#27ae60", zorder=5, s=50)

    ax.set_xlabel("Age (years)", fontsize=12)
    ax.set_ylabel("Cumulative breast cancer risk (%)", fontsize=12)
    ax.set_title("Paper4: Weighted SSE calibration vs. Kuchenbaecker 2017 (BRCA1)", fontsize=12)
    ax.legend(fontsize=10)
    ax.set_xlim(0, 80)
    ax.set_ylim(0, 100)
    ax.grid(True, alpha=0.3)

    for ext in ("png", "pdf"):
        path = out_dir / f"fig_p4_calibration.{ext}"
        fig.savefig(path, dpi=150, bbox_inches="tight")
        print(f"  Saved {path}")
    plt.close(fig)


def fig_p4_sse_landscape() -> None:
    """Paper4 fig: SSE_w landscape from fine sweep — shows shape of parameter space."""
    out_dir = Path("docs/paper4")
    out_dir.mkdir(parents=True, exist_ok=True)

    sweep_path = Path("results_sweep_p4_fine.csv")
    if not sweep_path.exists():
        print(f"  Skipping SSE landscape (missing {sweep_path})")
        return

    import collections
    data = collections.defaultdict(dict)
    brca1_vals, ld_vals = set(), set()
    with open(sweep_path) as f:
        for row in csv.DictReader(f):
            b = float(row["brca1_rate"])
            l = float(row["low_delta"])
            s = float(row["sse_weighted"])
            data[b][l] = s
            brca1_vals.add(b)
            ld_vals.add(l)

    brca1_vals = sorted(brca1_vals)
    ld_vals    = sorted(ld_vals)
    import numpy as np
    Z = np.array([[data[b].get(l, float("nan")) for l in ld_vals] for b in brca1_vals])

    fig, ax = plt.subplots(figsize=(8, 5))
    im = ax.contourf(ld_vals, brca1_vals, Z, levels=20, cmap="RdYlGn_r")
    plt.colorbar(im, ax=ax, label="Weighted SSE (χ² units)")

    # Mark chi²(6,0.95) contour
    cs = ax.contour(ld_vals, brca1_vals, Z, levels=[12.59], colors=["blue"], linewidths=[2])
    ax.clabel(cs, fmt="χ²=12.59 (p=0.05)", fontsize=9)

    # Mark optimum
    best_b, best_l, best_s = None, None, float("inf")
    for b in brca1_vals:
        for l in ld_vals:
            s = data[b].get(l, float("inf"))
            if s < best_s:
                best_s, best_b, best_l = s, b, l
    ax.scatter([best_l], [best_b], color="white", s=120, zorder=5,
               label=f"Optimum (brca1={best_b}, ld={best_l}, SSE_w={best_s:.1f})")

    ax.set_xlabel("low_delta", fontsize=12)
    ax.set_ylabel("brca1_rate", fontsize=12)
    ax.set_title("Paper4: Weighted SSE landscape — fine grid", fontsize=12)
    ax.legend(fontsize=9, loc="upper left")

    for ext in ("png", "pdf"):
        path = out_dir / f"fig_p4_sse_landscape.{ext}"
        fig.savefig(path, dpi=150, bbox_inches="tight")
        print(f"  Saved {path}")
    plt.close(fig)


# ---------------------------------------------------------------------------
# Validation
# ---------------------------------------------------------------------------

def validate_and_report(csv_path: str, out_dir: Path = None, weighted: bool = False) -> None:
    """Compute simulated vs Kuchenbaecker cumulative risk, write JSON + print."""
    import json
    from datetime import datetime

    if out_dir is None:
        out_dir = OUT_DIR

    cum = load_cumulative(csv_path)

    # Count total runs and non-penetrant runs
    n_total = 0
    n_no_cancer = 0
    with open(csv_path) as f:
        for row in csv.DictReader(f):
            n_total += 1
            yr = row.get("onset_year", "")
            if yr and float(yr) < 0:
                n_no_cancer += 1

    sse = sum((cum[age] - mean) ** 2 for age, mean in zip(KUCH_AGES, KUCH_MEAN))

    # Weighted SSE: Σ (sim-target)² / σᵢ²  where σᵢ = (CI_hi - CI_lo) / 3.92
    kuch_sigma = [(hi - lo) / 3.92 for lo, hi in zip(KUCH_LO, KUCH_HI)]
    sse_w = sum((cum[age] - mean) ** 2 / (s * s)
                for age, mean, s in zip(KUCH_AGES, KUCH_MEAN, kuch_sigma))
    chi2_threshold = 12.59  # chi²(6, 0.95)

    non_penetrance_pct = 100.0 * n_no_cancer / n_total if n_total > 0 else 0.0

    risks = {}
    ci_pass_count = 0
    for age, mean, lo, hi in zip(KUCH_AGES, KUCH_MEAN, KUCH_LO, KUCH_HI):
        sim = cum[age]
        in_ci = lo <= sim <= hi
        if in_ci:
            ci_pass_count += 1
        risks[str(age)] = {
            "simulated": round(sim, 2),
            "target": mean,
            "diff": round(sim - mean, 2),
            "ci": [lo, hi],
            "in_ci": in_ci,
        }

    # Status: weighted mode uses chi² criterion; unweighted uses legacy heuristic
    if weighted:
        if sse_w < chi2_threshold and ci_pass_count >= 5:
            status = "PASS"
        elif sse_w < chi2_threshold * 2 or ci_pass_count >= 4:
            status = "WARN"
        else:
            status = "FAIL"
    else:
        if ci_pass_count >= 5 and sse < 300:
            status = "PASS"
        elif ci_pass_count >= 3:
            status = "WARN"
        else:
            status = "FAIL"

    report = {
        "timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        "csv_file": csv_path,
        "n_runs": n_total,
        "sse": round(sse, 2),
        "sse_weighted": round(sse_w, 2),
        "chi2_threshold": chi2_threshold,
        "non_penetrance_pct": round(non_penetrance_pct, 2),
        "ci_pass_count": ci_pass_count,
        "status": status,
        "risks": risks,
    }

    json_path = out_dir / "validation_report.json"
    with open(json_path, "w") as f:
        json.dump(report, f, indent=2)

    icon = {"PASS": "✅", "WARN": "⚠️", "FAIL": "❌"}[status]
    chi2_flag = f"SSE_w={sse_w:.2f} {'<' if sse_w < chi2_threshold else '≥'} χ²(0.95,6)={chi2_threshold}"
    print(f"\n{icon}  Status: {status}  (SSE={sse:.1f}, {chi2_flag}, CI coverage: {ci_pass_count}/6, non-penetrance: {non_penetrance_pct:.1f}%)")
    print(f"{'Age':>5}  {'Simulated':>10}  {'Target':>8}  {'Diff':>7}  {'95% CI':>16}  In CI")
    print("-" * 62)
    for age, mean, lo, hi in zip(KUCH_AGES, KUCH_MEAN, KUCH_LO, KUCH_HI):
        sim = cum[age]
        in_ci = lo <= sim <= hi
        flag = "✅" if in_ci else "❌"
        print(f"{age:>5}  {sim:>9.1f}%  {mean:>7.1f}%  {sim-mean:>+6.1f}pp  [{lo:.1f}–{hi:.1f}]%  {flag}")
    print(f"\nValidation report saved: {json_path}")
    html_path = out_dir / "qa_report.html"
    print(f"Generate HTML report with:")
    print(f"  python3 scripts/qa_report.py {json_path} {html_path}")


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

    # Paper3 validation
    print("\n" + "=" * 70)
    print("Validation — Paper3 (unweighted SSE)")
    print("=" * 70)
    validate_and_report("results_final_n1000.csv", OUT_DIR, weighted=False)

    # Paper4 figures + validation (only if CSV exists)
    p4_csv = Path("results_p4_final_n1000.csv")
    if p4_csv.exists():
        print("\n" + "=" * 70)
        print("Paper4 — Weighted SSE calibration")
        print("=" * 70)
        p4_dir = Path("docs/paper4")
        fig_p4_calibration()
        fig_p4_sse_landscape()
        print("\n" + "=" * 70)
        print("Validation — Paper4 (weighted SSE, χ² criterion)")
        print("=" * 70)
        validate_and_report(str(p4_csv), p4_dir, weighted=True)

    print("\n✅ All done. Open docs/paper3/qa_report.html for full report.")
