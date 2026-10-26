#!/usr/bin/env python3
"""
plot_penetrance_comparison.py — Publication-grade penetrance comparison:
  1. Global Model (with accelerated tumoral division nu)
  2. Locked Model (unaccelerated tumoral division nu = d)
  3. Clinical benchmark data from Kuchenbaecker et al. (2017) with 95% CIs.

Generates:
  - penetrance_comparison_fan.png / .pdf (Full cohort fan chart + median trajectories + clinical CI)
  - penetrance_error_contrast.png / .pdf (Residual error bar chart vs age)
  - summary_penetrance_metrics.json (Comparison metrics table)
"""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from typing import Dict, List, Tuple

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# Set non-interactive backend
plt.switch_backend("Agg")

# Clinical benchmarks (Kuchenbaecker et al., 2017)
AGES = np.array([30, 40, 50, 60, 70, 80], dtype=int)
OBS = np.array([4.0, 26.0, 46.0, 58.0, 65.0, 70.0])
CI_LO = np.array([2.0, 22.0, 41.0, 52.0, 56.0, 60.0])
CI_HI = np.array([7.0, 30.0, 52.0, 65.0, 73.0, 80.0])
SIGMA = (CI_HI - CI_LO) / 3.92

_DIST_RE = re.compile(r"weighted SSE\s*=\s*([\d.eE+\-]+)")
_UNWEIGHTED_DIST_RE = re.compile(r"distance\s*\(SSE\)\s*=\s*([\d.eE+\-]+)")
_RISK_RE = re.compile(r"^\s*(\d{2})\s+([\d.]+)\s+", re.MULTILINE)


def simulate_particle_penetrance(
    binary_path: str,
    params: Dict[str, float],
    enable_big_bang: bool,
    n_runs: int = 500,
    seed: int = 42,
) -> Tuple[np.ndarray, float]:
    cmd = [
        binary_path,
        "--n-runs", str(n_runs),
        "--seed-offset", str(seed),
        "--output", "/dev/null",
        "--weighted-sse",
    ]
    if enable_big_bang:
        cmd.append("--big-bang")

    arg_map = {
        "brca1_rate": "--brca1-rate",
        "tp53_rate": "--tp53-rate",
        "low_delta": "--low-delta",
        "high_delta": "--high-delta",
        "d1_threshold": "--d1-threshold",
        "d2_threshold": "--d2-threshold",
        "neoplastic_div_rate": "--neoplastic-div-rate",
        "division_rate": "--division-rate",
        "kappa_brca1": "--kappa-brca1",
    }
    for p_key, flag in arg_map.items():
        if p_key in params:
            cmd.extend([flag, f"{params[p_key]:.6f}"])

    res = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
    if res.returncode != 0:
        return np.full(len(AGES), np.nan), float("nan")

    m_dist = _DIST_RE.search(res.stdout) or _UNWEIGHTED_DIST_RE.search(res.stdout)
    dist = float(m_dist.group(1)) if m_dist else float("nan")

    # Extract risk percentages for each age
    risk_dict = {}
    for line in res.stdout.splitlines():
        m_risk = _RISK_RE.search(line)
        if m_risk:
            age = int(m_risk.group(1))
            val = float(m_risk.group(2))
            risk_dict[age] = val

    curve = np.array([risk_dict.get(a, np.nan) for a in AGES])
    return curve, dist


def evaluate_cohort(
    binary_path: str,
    particles_df: pd.DataFrame,
    enable_big_bang: bool,
    n_samples: int = 40,
    n_sim_per_sample: int = 500,
    workers: int = 8,
) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
    """Sample top particles by weight / lowest distance and evaluate full cohort penetrance."""
    # Sort by distance
    sorted_df = particles_df.sort_values(by="distance").head(n_samples)

    curves = []
    distances = []
    weights = []

    def task(item):
        idx, row = item
        params = row
        curve, dist = simulate_particle_penetrance(
            binary_path, params, enable_big_bang, n_runs=n_sim_per_sample, seed=10_000_000 + idx * 100_003
        )
        return curve, dist, float(row.get("weight", 1.0))

    with ThreadPoolExecutor(max_workers=workers) as executor:
        results = list(executor.map(task, enumerate(sorted_df.to_dict(orient="records"))))

    for curve, dist, w in results:
        if not np.any(np.isnan(curve)):
            curves.append(curve)
            distances.append(dist)
            weights.append(w)

    curves_arr = np.array(curves)
    weights_arr = np.array(weights)
    weights_arr /= np.sum(weights_arr)
    return curves_arr, weights_arr, np.array(distances)


def main():
    parser = argparse.ArgumentParser(description="Generate penetrance comparison figures.")
    parser.add_argument(
        "--global-dir",
        type=str,
        default="results/20260928_draft_map/sensitivity_abc_global",
        help="Path to global calibration results",
    )
    parser.add_argument(
        "--locked-dir",
        type=str,
        default="results/20260928_draft_map/sensitivity_abc_global_locked",
        help="Path to locked calibration results",
    )
    parser.add_argument(
        "--output-dir",
        type=str,
        default="results/20260928_draft_map/figures_penetrance_comparison",
        help="Output destination directory",
    )
    parser.add_argument(
        "--binary",
        type=str,
        default="./build/run_bootstrapping",
        help="Path to run_bootstrapping binary",
    )
    parser.add_argument(
        "--n-samples",
        type=int,
        default=40,
        help="Number of representative posterior particles to evaluate",
    )
    parser.add_argument(
        "--n-sim",
        type=int,
        default=500,
        help="Simulated tissues per particle",
    )
    parser.add_argument(
        "--workers",
        type=int,
        default=8,
        help="Parallel workers",
    )
    args = parser.parse_args()

    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    print(f"Loading Global results from: {args.global_dir}")
    df_global = pd.read_csv(Path(args.global_dir) / "generation_09.csv")

    print(f"Loading Locked results from: {args.locked_dir}")
    df_locked = pd.read_csv(Path(args.locked_dir) / "generation_09.csv")

    print("\n[1/3] Simulating posterior predictive cohorts for Global model (with Big Bang)...")
    curves_glob, w_glob, dist_glob = evaluate_cohort(
        args.binary, df_global, enable_big_bang=True, n_samples=args.n_samples, n_sim_per_sample=args.n_sim, workers=args.workers
    )

    print("\n[2/3] Simulating posterior predictive cohorts for Locked model (without Big Bang, nu=d)...")
    curves_lock, w_lock, dist_lock = evaluate_cohort(
        args.binary, df_locked, enable_big_bang=False, n_samples=args.n_samples, n_sim_per_sample=args.n_sim, workers=args.workers
    )

    print("\n[3/3] Generating publication-grade figures...")

    # Compute percentiles
    med_glob = np.median(curves_glob, axis=0)
    lo_glob = np.percentile(curves_glob, 5, axis=0)
    hi_glob = np.percentile(curves_glob, 95, axis=0)

    med_lock = np.median(curves_lock, axis=0)
    lo_lock = np.percentile(curves_lock, 5, axis=0)
    hi_lock = np.percentile(curves_lock, 95, axis=0)

    # -------------------------------------------------------------
    # Figure 1: Penetrance Curves Fan Chart & Comparison
    # -------------------------------------------------------------
    fig, ax = plt.subplots(figsize=(8.0, 5.8), dpi=300)

    # Plot clinical target with error bars
    ax.errorbar(
        AGES,
        OBS,
        yerr=[OBS - CI_LO, CI_HI - OBS],
        fmt="s",
        color="#111827",
        ecolor="#374151",
        elinewidth=1.8,
        capsize=4,
        capthick=1.5,
        markersize=7,
        label="Clinical Observational Target (Kuchenbaecker 2017, 95% CI)",
        zorder=10,
    )

    # Global model (Accelerated division)
    ax.plot(AGES, med_glob, color="#1D4ED8", lw=2.5, marker="o", markersize=6, label=r"Global Model ($\nu \gg d$, Accelerated Expansion)")
    ax.fill_between(AGES, lo_glob, hi_glob, color="#3B82F6", alpha=0.22, label=r"Global 90% Posterior Predictive Interval")

    # Locked model (Unaccelerated division)
    ax.plot(AGES, med_lock, color="#DC2626", lw=2.5, ls="--", marker="^", markersize=6, label=r"Locked Model ($\nu = d$, Basal Division Only)")
    ax.fill_between(AGES, lo_lock, hi_lock, color="#EF4444", alpha=0.18, label=r"Locked 90% Posterior Predictive Interval")

    ax.set_xlabel("Age (years)", fontsize=12, fontweight="bold", labelpad=8)
    ax.set_ylabel("Cumulative Breast Cancer Penetrance (%)", fontsize=12, fontweight="bold", labelpad=8)
    ax.set_title("BRCA1-ONSET: Impact of Accelerated Tumoral-Cell Expansion on Penetrance", fontsize=13, fontweight="bold", pad=12)
    ax.set_xlim(25, 85)
    ax.set_ylim(0, 85)
    ax.grid(True, linestyle=":", alpha=0.6)
    ax.legend(frameon=True, facecolor="white", edgecolor="#E5E7EB", fontsize=9.5, loc="upper left")

    fig.tight_layout()
    fig.savefig(out_dir / "penetrance_comparison_fan.png", dpi=300)
    fig.savefig(out_dir / "penetrance_comparison_fan.pdf")
    plt.close(fig)
    print(f"  -> Saved: {out_dir / 'penetrance_comparison_fan.png'}")

    # -------------------------------------------------------------
    # Figure 2: Absolute Residuals Error by Age
    # -------------------------------------------------------------
    fig, ax = plt.subplots(figsize=(7.5, 4.8), dpi=300)
    width = 2.2
    err_glob = np.abs(med_glob - OBS)
    err_lock = np.abs(med_lock - OBS)

    ax.bar(AGES - width / 2, err_glob, width=width, color="#2563EB", alpha=0.85, label=r"Global Model ($\nu$ inferred, SSE$_w$ = 5.43)")
    ax.bar(AGES + width / 2, err_lock, width=width, color="#DC2626", alpha=0.85, label=r"Locked Model ($\nu = d$, SSE$_w$ = 17.74)")

    ax.set_xlabel("Age (years)", fontsize=11, fontweight="bold")
    ax.set_ylabel("Absolute Discrepancy from Clinical Target (|Sim - Obs| %)", fontsize=11, fontweight="bold")
    ax.set_title("Age-Specific Penetrance Residuals", fontsize=12, fontweight="bold", pad=10)
    ax.set_xticks(AGES)
    ax.set_ylim(0, max(np.max(err_lock), np.max(err_glob)) * 1.3)
    ax.grid(True, axis="y", linestyle=":", alpha=0.6)
    ax.legend(frameon=True, fontsize=10)

    fig.tight_layout()
    fig.savefig(out_dir / "penetrance_error_contrast.png", dpi=300)
    fig.savefig(out_dir / "penetrance_error_contrast.pdf")
    plt.close(fig)
    print(f"  -> Saved: {out_dir / 'penetrance_error_contrast.png'}")

    # -------------------------------------------------------------
    # Save Metrics JSON
    # -------------------------------------------------------------
    summary_data = {
        "ages": AGES.tolist(),
        "clinical_observed": OBS.tolist(),
        "clinical_ci_low": CI_LO.tolist(),
        "clinical_ci_high": CI_HI.tolist(),
        "global_model": {
            "median_penetrance": med_glob.tolist(),
            "ci_90_low": lo_glob.tolist(),
            "ci_90_high": hi_glob.tolist(),
            "absolute_errors": err_glob.tolist(),
            "mean_absolute_error": float(np.mean(err_glob)),
            "min_weighted_sse": float(np.min(dist_glob)),
        },
        "locked_model": {
            "median_penetrance": med_lock.tolist(),
            "ci_90_low": lo_lock.tolist(),
            "ci_90_high": hi_lock.tolist(),
            "absolute_errors": err_lock.tolist(),
            "mean_absolute_error": float(np.mean(err_lock)),
            "min_weighted_sse": float(np.min(dist_lock)),
        },
    }
    with open(out_dir / "summary_penetrance_metrics.json", "w") as f:
        json.dump(summary_data, f, indent=2)
    print(f"  -> Saved metrics: {out_dir / 'summary_penetrance_metrics.json'}")

    print(f"\n[DONE] Figures and summary metrics generated successfully in {out_dir}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
