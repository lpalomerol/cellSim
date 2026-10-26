#!/usr/bin/env python3
"""
exp_02_global_sensitivity_prcc.py — Global Sensitivity Analysis for BRCA1-ONSET.

Reproduces Supplementary Figure S1a & S1b:
1. Joint sampling of 1,000 parameter combinations across prespecified ranges:
   - brca1_rate ∈ [0.01, 0.10]
   - tp53_rate ∈ [3e-4, 6e-3]
   - low_delta ∈ [0.02, 0.20]
   - delta_ratio (high_delta / low_delta) ∈ [1.2, 4.0]  --> high_delta = low_delta * delta_ratio
   - d1_threshold ∈ [1.5, 3.0]
   - d2_threshold ∈ [3.0, 7.0]
   - division_rate ∈ [3e-4, 3e-3]
   - kappa_brca1 ∈ [1.0, 3.0]
   - age_drift ∈ [0.0, 5e-5]
   - neoplastic_div_rate ∈ [0.001, 0.30]
2. Evaluates each combination using 200 independent simulated tissues (500 cells, max_t=80).
3. Summarizes:
   - Cumulative penetrance at ages 30, 40, 50, 60, 70, 80 years.
   - Confidence-interval-weighted SSE (SSEw).
4. Machine Learning & Statistical sensitivity:
   - Random Forest Regression fitted to predict weighted SSE from the 10 parameters.
   - Normalized Permutation Feature Importance (Supplementary Fig. S1a).
   - Partial Rank Correlation Coefficients (PRCC) across ages 30–80 (Supplementary Fig. S1b).
"""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
import time
from concurrent.futures import FIRST_COMPLETED, ThreadPoolExecutor, wait
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from scipy import stats
from scipy.stats import qmc
from sklearn.ensemble import RandomForestRegressor
from sklearn.inspection import permutation_importance

plt.switch_backend("Agg")

# Clinical targets from Kuchenbaecker et al. 2017
AGES = np.array([30, 40, 50, 60, 70, 80], dtype=int)
OBS = np.array([4.0, 26.0, 46.0, 58.0, 65.0, 70.0])
CI_LO = np.array([2.0, 22.0, 41.0, 52.0, 56.0, 60.0])
CI_HI = np.array([7.0, 30.0, 52.0, 65.0, 73.0, 80.0])
SIGMA = (CI_HI - CI_LO) / 3.92

# Parameter bounds defined in the manuscript
PARAM_BOUNDS = {
    "brca1_rate":          (0.01, 0.10),
    "tp53_rate":           (3e-4, 6e-3),
    "low_delta":           (0.02, 0.20),
    "delta_ratio":         (1.2, 4.0),     # high_delta / low_delta
    "d1_threshold":        (1.5, 3.0),
    "d2_threshold":        (3.0, 7.0),
    "division_rate":       (3e-4, 3e-3),
    "kappa_brca1":         (1.0, 3.0),
    "age_drift":           (0.0, 5e-5),
    "neoplastic_div_rate": (0.001, 0.30),
}

PARAM_LABELS = {
    "brca1_rate":          r"$\beta$ (BRCA1 second-hit)",
    "tp53_rate":           r"$\mu_{\rm TP53}$ (TP53 loss-of-function)",
    "low_delta":           r"$\delta_{\rm low}$ (Heterozygous accumulation)",
    "high_delta":          r"$\delta_{\rm high}$ (Biallelic accumulation)",
    "d1_threshold":        r"$\theta_{D1}$ (PRIMER threshold)",
    "d2_threshold":        r"$\theta_{D2}$ (TUMORAL / clearance threshold)",
    "division_rate":       (r"$d$ (Basal division)"),
    "kappa_brca1":         r"$\kappa_{\rm BRCA1}$ (BRCA1 D2 weight)",
    "age_drift":           r"$A_{\rm it}$ (Cell age drift)",
    "neoplastic_div_rate": r"$\nu$ (Tumoral cell division)",
}

# Regex to parse stdout from run_bootstrapping
_DIST_RE = re.compile(r"weighted SSE\s*=\s*([\d.eE+\-]+)")
_AGE_ROW_RE = re.compile(r"^\s*(\d{2})\s+([\d.]+)\s+", re.MULTILINE)


def sample_latin_hypercube(n_samples: int, seed: int = 42) -> pd.DataFrame:
    """Generate balanced Latin Hypercube Sample of parameter combinations."""
    param_keys = list(PARAM_BOUNDS.keys())
    d = len(param_keys)
    sampler = qmc.LatinHypercube(d=d, seed=seed)
    sample_unit = sampler.random(n=n_samples)

    lower_bounds = np.array([PARAM_BOUNDS[k][0] for k in param_keys])
    upper_bounds = np.array([PARAM_BOUNDS[k][1] for k in param_keys])
    scaled = qmc.scale(sample_unit, lower_bounds, upper_bounds)

    df = pd.DataFrame(scaled, columns=param_keys)
    # Derive high_delta = low_delta * delta_ratio
    df["high_delta"] = df["low_delta"] * df["delta_ratio"]
    return df


def evaluate_single_point(
    binary_path: str,
    params: Dict[str, float],
    n_sim: int,
    seed_offset: int,
) -> Optional[Dict[str, float]]:
    """Runs run_bootstrapping for a single parameter set."""
    cmd = [
        binary_path,
        "--n-runs", str(n_sim),
        "--seed-offset", str(seed_offset),
        "--output", "/dev/null",
        "--weighted-sse",
        "--big-bang",
        "--brca1-rate", f"{params['brca1_rate']:.8g}",
        "--tp53-rate", f"{params['tp53_rate']:.8g}",
        "--low-delta", f"{params['low_delta']:.8g}",
        "--high-delta", f"{params['high_delta']:.8g}",
        "--d1-threshold", f"{params['d1_threshold']:.8g}",
        "--d2-threshold", f"{params['d2_threshold']:.8g}",
        "--division-rate", f"{params['division_rate']:.8g}",
        "--neoplastic-div-rate", f"{params['neoplastic_div_rate']:.8g}",
        "--kappa-brca1", f"{params['kappa_brca1']:.8g}",
        "--age-drift", f"{params['age_drift']:.8g}",
    ]

    try:
        env = dict(os.environ)
        env["OMP_NUM_THREADS"] = "1"
        res = subprocess.run(cmd, capture_output=True, text=True, timeout=180, env=env)
    except (subprocess.TimeoutExpired, OSError):
        return None

    if res.returncode != 0:
        return None

    m_sse = _DIST_RE.search(res.stdout)
    if not m_sse:
        return None
    weighted_sse = float(m_sse.group(1))

    # Parse simulated penetrance at ages 30, 40, 50, 60, 70, 80
    risk_dict = {}
    for m in _AGE_ROW_RE.finditer(res.stdout):
        age_val = int(m.group(1))
        risk_val = float(m.group(2))
        if age_val in AGES:
            risk_dict[f"risk_{age_val}"] = risk_val

    if len(risk_dict) < len(AGES):
        return None

    return {"weighted_sse": weighted_sse, **risk_dict}


def compute_prcc(x: pd.DataFrame, y: pd.Series) -> pd.Series:
    """
    Computes Partial Rank Correlation Coefficients (PRCC) between columns of x and y.
    Transforms data to ranks and regresses residuals.
    """
    rx = x.rank()
    ry = y.rank()

    cols = list(rx.columns)
    prcc_vals = {}

    for c in cols:
        other_cols = [o for o in cols if o != c]
        # Regress rx[c] on other_cols
        res_x = rx[c] - np.linalg.lstsq(
            np.c_[np.ones(len(rx)), rx[other_cols].to_numpy()],
            rx[c].to_numpy(),
            rcond=None
        )[0] @ np.c_[np.ones(len(rx)), rx[other_cols].to_numpy()].T

        # Regress ry on other_cols
        res_y = ry - np.linalg.lstsq(
            np.c_[np.ones(len(ry)), rx[other_cols].to_numpy()],
            ry.to_numpy(),
            rcond=None
        )[0] @ np.c_[np.ones(len(ry)), rx[other_cols].to_numpy()].T

        # Pearson correlation of residuals
        corr, _ = stats.pearsonr(res_x, res_y)
        prcc_vals[c] = float(corr)

    return pd.Series(prcc_vals)


def main():
    parser = argparse.ArgumentParser(description="Run 1,000 parameter combinations Global Sensitivity Analysis (LHS + PRCC + Random Forest).")
    parser.add_argument("--n-samples", type=int, default=1000, help="Number of LHS parameter combinations (default: 1000)")
    parser.add_argument("--n-sim", type=int, default=200, help="Simulations per combination (default: 200)")
    parser.add_argument("--workers", type=int, default=10, help="Parallel worker threads (default: 10)")
    parser.add_argument("--seed", type=int, default=42, help="RNG seed")
    parser.add_argument("--output-dir", type=str, default="results/20260928_draft_map/exp_02_global_sensitivity")
    parser.add_argument("--binary", type=str, default="./build/run_bootstrapping")
    parser.add_argument("--skip-sim", action="store_true", help="Skip simulation if results CSV already exists")
    args = parser.parse_args()

    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    csv_path = out_dir / "sensitivity_1000_lhs_evaluations.csv"

    if not args.skip_sim or not csv_path.exists():
        print(f"=========================================================================")
        print(f"Starting Global Sensitivity Analysis (1,000 parameter combinations x {args.n_sim} sims)")
        print(f"Binary: {args.binary}")
        print(f"Output directory: {out_dir}")
        print(f"Workers: {args.workers}")
        print(f"=========================================================================\n")

        print("[1/3] Generating 1,000 Latin Hypercube parameter samples...")
        lhs_df = sample_latin_hypercube(n_samples=args.n_samples, seed=args.seed)

        start_time = time.time()
        results: List[Optional[Dict[str, float]]] = [None] * len(lhs_df)

        print(f"[2/3] Simulating {len(lhs_df)} points with {args.workers} workers...")
        with ThreadPoolExecutor(max_workers=args.workers) as executor:
            future_to_idx = {}
            for idx, row in lhs_df.iterrows():
                p_dict = row.to_dict()
                seed_offset = 50_000_000 + int(idx) * 100_003
                fut = executor.submit(evaluate_single_point, args.binary, p_dict, args.n_sim, seed_offset)
                future_to_idx[fut] = idx

            completed = 0
            for fut in future_to_idx:
                idx = future_to_idx[fut]
                try:
                    res = fut.result()
                    results[idx] = res
                except Exception as ex:
                    print(f"Error on sample {idx}: {ex}")
                    results[idx] = None
                completed += 1
                if completed % 50 == 0 or completed == len(lhs_df):
                    elapsed = time.time() - start_time
                    rate = completed / elapsed
                    eta = (len(lhs_df) - completed) / max(rate, 1e-5)
                    print(f"  -> Completed {completed}/{len(lhs_df)} ({completed/len(lhs_df)*100:.1f}%) | "
                          f"Elapsed: {elapsed:.1f}s | Rate: {rate:.2f} pts/s | ETA: {eta:.1f}s")

        # Collect successful rows
        eval_rows = []
        for idx, row in lhs_df.iterrows():
            r = results[idx]
            if r is not None:
                combined = {**row.to_dict(), **r}
                eval_rows.append(combined)

        df_full = pd.DataFrame(eval_rows)
        df_full.to_csv(csv_path, index=False)
        print(f"\n[DONE] Successfully evaluated {len(df_full)}/{len(lhs_df)} points. Saved to: {csv_path}")
    else:
        print(f"Loading existing evaluations from: {csv_path}")
        df_full = pd.read_csv(csv_path)

    # -------------------------------------------------------------------------
    # 3. Sensitivity Analysis (Random Forest Permutation Importance & PRCC)
    # -------------------------------------------------------------------------
    print("\n[3/3] Performing Random Forest Permutation Importance and PRCC Analysis...")

    feature_cols = [
        "brca1_rate", "tp53_rate", "low_delta", "high_delta",
        "d1_threshold", "d2_threshold", "division_rate",
        "kappa_brca1", "age_drift", "neoplastic_div_rate"
    ]

    X = df_full[feature_cols]
    y_sse = df_full["weighted_sse"]

    # Fit Random Forest Regressor
    rf = RandomForestRegressor(n_estimators=500, max_features="sqrt", random_state=args.seed, n_jobs=-1)
    rf.fit(X, y_sse)
    r2_score = rf.score(X, y_sse)
    print(f"  -> Random Forest R^2 on weighted SSE: {r2_score:.3f}")

    # Permutation Importance
    perm_res = permutation_importance(rf, X, y_sse, n_repeats=30, random_state=args.seed, n_jobs=-1)
    # Normalize permutation importance to sum to 100%
    raw_importances = np.maximum(perm_res.importances_mean, 0)
    norm_importances = 100.0 * raw_importances / np.sum(raw_importances)

    perm_df = pd.DataFrame({
        "parameter": feature_cols,
        "importance_pct": norm_importances,
        "std_pct": 100.0 * perm_res.importances_std / np.sum(raw_importances),
    }).sort_values(by="importance_pct", ascending=True)

    perm_df.to_csv(out_dir / "rf_permutation_importance.csv", index=False)

    # Compute PRCC for each age
    prcc_by_age = {}
    for age in AGES:
        y_age = df_full[f"risk_{age}"]
        prcc_by_age[age] = compute_prcc(X, y_age)

    prcc_df = pd.DataFrame(prcc_by_age)
    prcc_df.to_csv(out_dir / "prcc_across_ages.csv")

    # -------------------------------------------------------------------------
    # 4. Generate Publication-Grade Supplementary Figure S1
    # -------------------------------------------------------------------------
    print("\nGenerating Supplementary Figure S1 (Panels a & b)...")
    fig = plt.figure(figsize=(14.2, 6.2), dpi=300)
    gs = fig.add_gridspec(1, 2, width_ratios=[1, 1.25], wspace=0.45)

    # Panel (a): Random Forest Permutation Importance
    ax_a = fig.add_subplot(gs[0, 0])
    y_pos = np.arange(len(perm_df))
    bars = ax_a.barh(y_pos, perm_df["importance_pct"], xerr=perm_df["std_pct"],
                     color="#1D4ED8", alpha=0.85, edgecolor="#1E3A8A", capsize=3, height=0.65)
    ax_a.set_yticks(y_pos)
    ax_a.set_yticklabels([PARAM_LABELS[p] for p in perm_df["parameter"]], fontsize=10.5, fontweight="bold")
    ax_a.set_xlabel("Normalized Permutation Importance (%)", fontsize=11, fontweight="bold")
    ax_a.set_title(r"$\mathbf{a}$  Overall Parameter Influence (Random Forest on $SSE_w$)",
                   loc="left", fontsize=12.5, fontweight="bold", pad=15)
    ax_a.grid(True, axis="x", linestyle=":", alpha=0.6)
    ax_a.set_xlim(0, max(perm_df["importance_pct"]) * 1.2)

    for bar, val in zip(bars, perm_df["importance_pct"]):
        ax_a.text(val + 0.8, bar.get_y() + bar.get_height() / 2, f"{val:.1f}%",
                  va="center", fontsize=9.5, fontweight="bold", color="#1F2937")

    # Panel (b): Age-specific PRCC
    ax_b = fig.add_subplot(gs[0, 1])
    ax_b.set_title(r"$\mathbf{b}$  Age-Specific Parameter Influence on Penetrance (PRCC)",
                   loc="left", fontsize=12.5, fontweight="bold", pad=15)
    # Highlight the key parameters highlighted in manuscript text:
    # neoplastic_div_rate (nu), tp53_rate, d2_threshold, brca1_rate, etc.
    color_map = {
        "tp53_rate":           "#DC2626",  # Red
        "neoplastic_div_rate": "#2563EB",  # Blue
        "d2_threshold":        "#059669",  # Green
        "brca1_rate":          "#D97706",  # Amber
        "low_delta":           "#8B5CF6",  # Purple
        "high_delta":          "#EC4899",  # Pink
        "d1_threshold":        "#6B7280",  # Gray
        "division_rate":       "#14B8A6",  # Teal
        "kappa_brca1":         "#F97316",  # Orange
        "age_drift":           "#9CA3AF",  # Light gray
    }
    marker_map = {
        "tp53_rate": "o", "neoplastic_div_rate": "s", "d2_threshold": "^",
        "brca1_rate": "D", "low_delta": "v", "high_delta": "<",
        "d1_threshold": ">", "division_rate": "p", "kappa_brca1": "*", "age_drift": "x"
    }

    for p in feature_cols:
        series = prcc_df.loc[p]
        is_highlighted = p in ["tp53_rate", "neoplastic_div_rate", "d2_threshold", "brca1_rate"]
        lw = 2.4 if is_highlighted else 1.2
        alpha = 1.0 if is_highlighted else 0.45
        ax_b.plot(AGES, series.values, label=PARAM_LABELS[p], color=color_map[p],
                  marker=marker_map[p], lw=lw, alpha=alpha, markersize=6 if is_highlighted else 4)

    ax_b.axhline(0, color="#111827", ls="--", lw=1.2, alpha=0.7)
    ax_b.set_xlabel("Age (years)", fontsize=11, fontweight="bold")
    ax_b.set_ylabel("Partial Rank Correlation Coefficient (PRCC)", fontsize=11, fontweight="bold")
    ax_b.set_xticks(AGES)
    ax_b.set_ylim(-0.85, 0.85)
    ax_b.grid(True, linestyle=":", alpha=0.5)
    ax_b.legend(bbox_to_anchor=(1.02, 1), loc="upper left", frameon=True, fontsize=8.5)

    fig_png = out_dir / "Supplementary_Figure_S1.png"
    fig_pdf = out_dir / "Supplementary_Figure_S1.pdf"
    fig.savefig(fig_png, dpi=300, bbox_inches="tight")
    fig.savefig(fig_pdf, bbox_inches="tight")
    plt.close(fig)

    print(f"  -> Saved: {fig_png}")
    print(f"  -> Saved: {fig_pdf}")
    print(f"\n[COMPLETE] Global Sensitivity Analysis finished successfully in: {out_dir}\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
