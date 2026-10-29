#!/usr/bin/env python3
"""
run_lhs_sensitivity.py — Global Sensitivity Analysis for BRCA1-ONSET Agent-Based Model.

Scientific Context:
-------------------
Implements Latin Hypercube Sampling (LHS) across 1,000 parameter combinations evaluated
on 200 simulated tissues (500 cells, max_t=80 years) as described in Methods and
Supplementary Figure S1 of Palomero et al. (2026).

Quantifies the sensitivity of cumulative cancer penetrance and weighted calibration error
(SSE_w) to variations across 10 key biological model parameters:
  1. beta (brca1_rate): Somatic loss of remaining functional BRCA1 allele [0.01, 0.10]
  2. mu_TP53 (tp53_rate): TP53 loss-of-function mutation rate [3e-4, 6e-3]
  3. delta_low (low_delta): Latent DNA damage accumulation in heterozygous cells [0.02, 0.20]
  4. delta_ratio (high_delta / low_delta): Biallelic damage accumulation multiplier [1.2, 4.0]
  5. theta_D1 (d1_threshold): PRIMER mitotic checkpoint trigger threshold [1.5, 3.0]
  6. theta_D2 (d2_threshold): Clearance evasion / TUMORAL transition threshold [3.0, 7.0]
  7. d (division_rate): Normal epithelial basal division rate [3e-4, 3e-3]
  8. kappa_BRCA1 (kappa_brca1): BRCA1-dependent D2 accumulation multiplier [1.0, 3.0]
  9. A_it (age_drift): Cell age-dependent background drift [0.0, 5e-5]
 10. nu (neoplastic_div_rate): Transformed neoplastic cell division rate [0.001, 0.30]

Outputs:
--------
  - sensitivity_1000_lhs_evaluations.csv: Parameter vectors, SSE_w, and penetrance at ages 30-80.
  - rf_permutation_importance.csv: Normalized Random Forest permutation importance on SSE_w.
  - prcc_across_ages.csv: Partial Rank Correlation Coefficients (PRCC) at ages 30, 40, 50, 60, 70, 80.
"""

from __future__ import annotations

import argparse
import os
import re
import subprocess
import sys
import time
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

import numpy as np
import pandas as pd
from scipy import stats
from scipy.stats import qmc
from sklearn.ensemble import RandomForestRegressor
from sklearn.inspection import permutation_importance

# Clinical targets from Kuchenbaecker et al. 2017 (cumulative penetrance at ages 30–80)
AGES = np.array([30, 40, 50, 60, 70, 80], dtype=int)
OBS_PENETRANCE = np.array([4.0, 26.0, 46.0, 58.0, 65.0, 70.0])
CI_LO = np.array([2.0, 22.0, 41.0, 52.0, 56.0, 60.0])
CI_HI = np.array([7.0, 30.0, 52.0, 65.0, 73.0, 80.0])
SIGMA_OBS = (CI_HI - CI_LO) / 3.92

# Mathematical parameter bounds from manuscript Table S1 & Methods
PARAM_BOUNDS: Dict[str, Tuple[float, float]] = {
    "brca1_rate":          (0.010, 0.100),
    "tp53_rate":           (3e-4,  6e-3),
    "low_delta":           (0.020, 0.200),
    "delta_ratio":         (1.200, 4.000),     # high_delta = low_delta * delta_ratio
    "d1_threshold":        (1.500, 3.000),
    "d2_threshold":        (3.000, 7.000),
    "division_rate":       (3e-4,  3e-3),
    "kappa_brca1":         (1.000, 3.000),
    "age_drift":           (0.000, 5e-5),
    "neoplastic_div_rate": (0.001, 0.300),
}

# Regex to parse stdout from run_bootstrapping
_RE_WEIGHTED_SSE = re.compile(r"weighted SSE\s*=\s*([\d.eE+\-]+)")
_RE_AGE_ROW = re.compile(r"^\s*(\d{2})\s+([\d.]+)\s+", re.MULTILINE)


def sample_latin_hypercube(n_samples: int, seed: int = 42) -> pd.DataFrame:
    """Generate balanced Latin Hypercube Sample of parameter combinations.
    
    Uses SciPy's quasi-Monte Carlo LatinHypercube sampler to produce space-filling
    multi-dimensional parameter points uniformly distributed over PARAM_BOUNDS.
    """
    param_keys = list(PARAM_BOUNDS.keys())
    d = len(param_keys)
    sampler = qmc.LatinHypercube(d=d, seed=seed)
    sample_unit = sampler.random(n=n_samples)

    lower_bounds = np.array([PARAM_BOUNDS[k][0] for k in param_keys])
    upper_bounds = np.array([PARAM_BOUNDS[k][1] for k in param_keys])
    scaled = qmc.scale(sample_unit, lower_bounds, upper_bounds)

    df = pd.DataFrame(scaled, columns=param_keys)
    # Structural constraint: high_delta = low_delta * delta_ratio
    df["high_delta"] = df["low_delta"] * df["delta_ratio"]
    return df


def evaluate_single_point(
    binary_path: str,
    params: Dict[str, float],
    n_sim: int,
    seed_offset: int,
    timeout_sec: int = 180,
) -> Optional[Dict[str, float]]:
    """Runs run_bootstrapping for a single parameter vector and parses stdout."""
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
        res = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout_sec, env=env)
    except (subprocess.TimeoutExpired, OSError):
        return None

    if res.returncode != 0:
        return None

    m_sse = _RE_WEIGHTED_SSE.search(res.stdout)
    if not m_sse:
        return None
    weighted_sse = float(m_sse.group(1))

    # Parse simulated cumulative penetrance at ages 30, 40, 50, 60, 70, 80
    risk_dict: Dict[str, float] = {}
    for m in _RE_AGE_ROW.finditer(res.stdout):
        age_val = int(m.group(1))
        risk_val = float(m.group(2))
        if age_val in AGES:
            risk_dict[f"risk_{age_val}"] = risk_val

    if len(risk_dict) < len(AGES):
        return None

    return {"weighted_sse": weighted_sse, **risk_dict}


def compute_prcc(x: pd.DataFrame, y: pd.Series) -> pd.Series:
    """Computes Partial Rank Correlation Coefficients (PRCC) between columns of x and y.
    
    Transforms values into ranks, removes linear dependencies from other rank-transformed
    parameters via least-squares residuals, and calculates Pearson correlation between residuals.
    Reference: Blower & Dowlatabadi (1994), Marino et al. (2008).
    """
    rx = x.rank()
    ry = y.rank()

    cols = list(rx.columns)
    prcc_vals: Dict[str, float] = {}

    for c in cols:
        other_cols = [o for o in cols if o != c]
        x_design = np.c_[np.ones(len(rx)), rx[other_cols].to_numpy()]

        # Regress rx[c] on other_cols
        res_x = rx[c].to_numpy() - (x_design @ np.linalg.lstsq(x_design, rx[c].to_numpy(), rcond=None)[0])

        # Regress ry on other_cols
        res_y = ry.to_numpy() - (x_design @ np.linalg.lstsq(x_design, ry.to_numpy(), rcond=None)[0])

        # Pearson correlation of residual vectors
        corr, _ = stats.pearsonr(res_x, res_y)
        prcc_vals[c] = float(corr)

    return pd.Series(prcc_vals)


def run_sensitivity_analysis(
    df_evals: pd.DataFrame,
    feature_cols: List[str],
    seed: int = 42,
) -> Tuple[pd.DataFrame, pd.DataFrame]:
    """Fits Random Forest Regressor on SSE_w and computes age-specific PRCCs."""
    X = df_evals[feature_cols]
    y_sse = df_evals["weighted_sse"]

    # 1. Random Forest Regressor on SSE_w
    rf = RandomForestRegressor(n_estimators=500, max_features="sqrt", random_state=seed, n_jobs=-1)
    rf.fit(X, y_sse)
    r2 = rf.score(X, y_sse)
    print(f"  -> Random Forest R^2 on weighted SSE: {r2:.3f}")

    # Permutation importance (normalized to sum to 100%)
    perm_res = permutation_importance(rf, X, y_sse, n_repeats=30, random_state=seed, n_jobs=-1)
    raw_mean = np.maximum(perm_res.importances_mean, 0)
    total_raw = np.sum(raw_mean) if np.sum(raw_mean) > 0 else 1.0
    norm_mean = 100.0 * raw_mean / total_raw
    norm_std = 100.0 * perm_res.importances_std / total_raw

    perm_df = pd.DataFrame({
        "parameter": feature_cols,
        "importance_pct": norm_mean,
        "std_pct": norm_std,
    }).sort_values(by="importance_pct", ascending=True)

    # 2. PRCC across clinical ages (30 to 80)
    prcc_by_age = {}
    for age in AGES:
        col_risk = f"risk_{age}"
        if col_risk in df_evals.columns:
            prcc_by_age[age] = compute_prcc(X, df_evals[col_risk])

    prcc_df = pd.DataFrame(prcc_by_age)
    return perm_df, prcc_df


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Run Latin Hypercube Sampling (LHS) Global Sensitivity Analysis."
    )
    parser.add_argument("--n-samples", type=int, default=1000, help="Number of LHS combinations (default: 1000)")
    parser.add_argument("--n-sim", type=int, default=200, help="Simulated tissues per combination (default: 200)")
    parser.add_argument("--workers", type=int, default=8, help="Worker threads (default: 8)")
    parser.add_argument("--seed", type=int, default=42, help="RNG seed")
    parser.add_argument("--binary", type=str, default="./build/run_bootstrapping", help="Path to run_bootstrapping binary")
    parser.add_argument("--output-dir", type=str, default=str(Path(__file__).resolve().parent / "data"), help="Output directory")
    parser.add_argument("--smoke-test", action="store_true", help="Run ultra-fast smoke test (2 samples, 5 sims)")
    parser.add_argument("--skip-sim", action="store_true", help="Skip simulation if evaluation CSV already exists")

    args = parser.parse_args()
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    eval_csv_path = out_dir / "sensitivity_1000_lhs_evaluations.csv"

    # Fast smoke test mode
    if args.smoke_test:
        print("[SMOKE TEST] Verifying LHS sampling, binary invocation, and output parsing...")
        t0 = time.time()
        test_lhs = sample_latin_hypercube(n_samples=2, seed=args.seed)
        row = test_lhs.iloc[0].to_dict()
        res = evaluate_single_point(args.binary, row, n_sim=5, seed_offset=101)
        if res is None or "weighted_sse" not in res:
            print("[ERROR] Smoke test failed to evaluate single point with binary.")
            return 1
        elapsed = time.time() - t0
        print(f"[OK] Smoke test completed successfully in {elapsed:.2f}s! (weighted_sse = {res['weighted_sse']:.2f})")
        return 0

    feature_cols = [
        "brca1_rate", "tp53_rate", "low_delta", "delta_ratio",
        "d1_threshold", "d2_threshold", "division_rate",
        "kappa_brca1", "age_drift", "neoplastic_div_rate"
    ]

    if not args.skip_sim or not eval_csv_path.exists():
        print(f"=========================================================================")
        print(f"Global Sensitivity Analysis (LHS: {args.n_samples} points x {args.n_sim} sims)")
        print(f"Binary: {args.binary}")
        print(f"Output directory: {out_dir}")
        print(f"Workers: {args.workers}")
        print(f"=========================================================================\n")

        print(f"[1/3] Generating {args.n_samples} Latin Hypercube parameter samples...")
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
                    results[idx] = fut.result()
                except Exception as ex:
                    print(f"Error on sample {idx}: {ex}")
                    results[idx] = None
                completed += 1
                if completed % 50 == 0 or completed == len(lhs_df):
                    elapsed = time.time() - start_time
                    rate = completed / max(elapsed, 1e-5)
                    eta = (len(lhs_df) - completed) / max(rate, 1e-5)
                    print(f"  -> Completed {completed}/{len(lhs_df)} ({completed/len(lhs_df)*100:.1f}%) | "
                          f"Elapsed: {elapsed:.1f}s | Rate: {rate:.2f} pts/s | ETA: {eta:.1f}s")

        eval_rows = []
        for idx, row in lhs_df.iterrows():
            r = results[idx]
            if r is not None:
                eval_rows.append({**row.to_dict(), **r})

        df_evals = pd.DataFrame(eval_rows)
        df_evals.to_csv(eval_csv_path, index=False)
        print(f"\n[DONE] Successfully evaluated {len(df_evals)}/{len(lhs_df)} points. Saved to: {eval_csv_path}")
    else:
        print(f"Loading pre-existing LHS evaluations from: {eval_csv_path}")
        df_evals = pd.read_csv(eval_csv_path)

    print("\n[3/3] Performing Random Forest Permutation Importance and PRCC Analysis...")
    perm_df, prcc_df = run_sensitivity_analysis(df_evals, feature_cols, seed=args.seed)

    rf_csv = out_dir / "rf_permutation_importance.csv"
    prcc_csv = out_dir / "prcc_across_ages.csv"
    perm_df.to_csv(rf_csv, index=False)
    prcc_df.to_csv(prcc_csv)
    print(f"  -> Saved RF Permutation Importance to: {rf_csv}")
    print(f"  -> Saved PRCC metrics to: {prcc_csv}")

    print("\n[COMPLETE] Global Sensitivity Analysis data pipeline finished successfully.\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
