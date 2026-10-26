#!/usr/bin/env python3
"""Run exploratory parameter grid evaluations and verification checks for Figure 2."""
from __future__ import annotations

import argparse
import os
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Dict, List, Tuple

import numpy as np
import pandas as pd

REPO_ROOT = Path(__file__).resolve().parents[2]
BOOTSTRAP_BIN = REPO_ROOT / "build" / "run_bootstrapping"

AGES = np.array([30, 40, 50, 60, 70, 80], dtype=int)
OBSERVED_PENETRANCE = np.array([4.0, 26.0, 46.0, 58.0, 65.0, 70.0])
CI_LOWER = np.array([2.0, 22.0, 41.0, 52.0, 56.0, 60.0])
CI_UPPER = np.array([7.0, 30.0, 52.0, 65.0, 73.0, 80.0])
SIGMA_WEIGHT = (CI_UPPER - CI_LOWER) / 3.92

BETA_COARSE = [0.010, 0.020, 0.040, 0.060, 0.080, 0.100]
DLOW_COARSE = [0.020, 0.040, 0.060, 0.080, 0.100, 0.150]


def evaluate_simulation(
    *,
    n_runs: int,
    brca1_rate: float,
    low_delta: float,
    high_delta: float,
    neoplastic_div_rate: float,
    seed_offset: int,
    big_bang: bool,
    division_rate: float = 0.001,
) -> Dict[str, object]:
    """Execute run_bootstrapping binary for a single parameter combination."""
    if not BOOTSTRAP_BIN.exists():
        raise FileNotFoundError(f"Missing {BOOTSTRAP_BIN}. Please run 'make build' first.")

    with tempfile.NamedTemporaryFile(suffix=".csv", delete=False) as tmp:
        tmp_csv = tmp.name

    cmd = [
        str(BOOTSTRAP_BIN),
        "--n-runs", str(n_runs),
        "--n-cells", "500",
        "--max-t", "80",
        "--brca1-rate", f"{brca1_rate:.10g}",
        "--tp53-rate", "0.003",
        "--low-delta", f"{low_delta:.10g}",
        "--high-delta", f"{high_delta:.10g}",
        "--d1-threshold", "2.0",
        "--d2-threshold", "5.0",
        "--division-rate", f"{division_rate:.10g}",
        "--neoplastic-div-rate", f"{neoplastic_div_rate:.10g}",
        "--tumor-threshold", "0.05",
        "--kappa-brca1", "2.0",
        "--age-drift", "1e-5",
        "--seed-offset", str(seed_offset),
        "--output", tmp_csv,
    ]
    if big_bang:
        cmd.append("--big-bang")

    env = dict(os.environ)
    env["OMP_NUM_THREADS"] = "2"

    try:
        r = subprocess.run(cmd, capture_output=True, text=True, check=False, env=env)
        if r.returncode != 0:
            raise RuntimeError(f"run_bootstrapping execution failed:\n{r.stderr}\n{r.stdout}")

        df = pd.read_csv(tmp_csv)
        onsets = df["onset_year"].to_numpy()
        risks = np.array([100.0 * np.mean((onsets >= 0) & (onsets <= age)) for age in AGES])
        unweighted_sse = float(np.sum((risks - OBSERVED_PENETRANCE) ** 2))
        weighted_sse = float(np.sum(((risks - OBSERVED_PENETRANCE) / SIGMA_WEIGHT) ** 2))

        return {
            "risk": risks,
            "unweighted_sse": unweighted_sse,
            "weighted_sse": weighted_sse,
        }
    finally:
        if os.path.exists(tmp_csv):
            os.unlink(tmp_csv)


def run_smoke_test():
    """Ultra-fast smoke test checking binary invocation and pipeline."""
    print("🧪 Running smoke test for Figure 2 exploratory pipeline...")
    res = evaluate_simulation(
        n_runs=10,
        brca1_rate=0.045,
        low_delta=0.120,
        high_delta=0.240,
        neoplastic_div_rate=0.10,
        seed_offset=1234,
        big_bang=True,
    )
    print("  ✓ Simulation completed successfully!")
    print(f"  ✓ Simulated penetrance across ages {AGES}: {np.round(res['risk'], 1)}%")
    print(f"  ✓ Discrepancy SSE = {res['unweighted_sse']:.2f}")


def run_grid_sweep(n_runs: int, output_dir: Path):
    """Run full coarse grid sweep for no-expansion and accelerated expansion."""
    output_dir.mkdir(parents=True, exist_ok=True)
    print(f"🔬 Running coarse grid sweep (N={n_runs} runs per setting)...")

    # 1. No accelerated expansion (nu = d = 0.001)
    rows_noexp = []
    seed = 100000
    for beta in BETA_COARSE:
        for dl in DLOW_COARSE:
            dh = 2.0 * dl
            res = evaluate_simulation(
                n_runs=n_runs,
                brca1_rate=beta,
                low_delta=dl,
                high_delta=dh,
                neoplastic_div_rate=0.001,
                seed_offset=seed,
                big_bang=False,
            )
            row = {
                "brca1_rate": beta,
                "low_delta": dl,
                "high_delta": dh,
                "neoplastic_div_rate": 0.001,
                "sse": res["unweighted_sse"],
                "weighted_sse": res["weighted_sse"],
            }
            row.update({f"risk_{age}": r for age, r in zip(AGES, res["risk"])})
            rows_noexp.append(row)
            seed += 10003
            print(f"  No-exp: β={beta:.3f}, δ_low={dl:.3f} -> SSE={res['unweighted_sse']:.1f}")

    df_noexp = pd.DataFrame(rows_noexp).sort_values("sse")
    df_noexp.to_csv(output_dir / "coarse_no_accelerated_expansion.csv", index=False)
    print(f"✓ Saved: {output_dir / 'coarse_no_accelerated_expansion.csv'}")

    # 2. Accelerated expansion at provisional setting
    res_acc = evaluate_simulation(
        n_runs=max(500, n_runs),
        brca1_rate=0.045,
        low_delta=0.120,
        high_delta=0.240,
        neoplastic_div_rate=0.10,
        seed_offset=771000,
        big_bang=True,
    )
    res_basal = evaluate_simulation(
        n_runs=max(500, n_runs),
        brca1_rate=0.045,
        low_delta=0.120,
        high_delta=0.240,
        neoplastic_div_rate=0.001,
        seed_offset=771000,
        big_bang=False,
    )
    df_comp = pd.DataFrame([
        {
            "setting": "Accelerated (nu=0.10)",
            "brca1_rate": 0.045, "low_delta": 0.120, "high_delta": 0.240,
            "neoplastic_div_rate": 0.10, "sse": res_acc["unweighted_sse"],
            **{f"risk_{age}": r for age, r in zip(AGES, res_acc["risk"])}
        },
        {
            "setting": "Basal (nu=0.001)",
            "brca1_rate": 0.045, "low_delta": 0.120, "high_delta": 0.240,
            "neoplastic_div_rate": 0.001, "sse": res_basal["unweighted_sse"],
            **{f"risk_{age}": r for age, r in zip(AGES, res_basal["risk"])}
        }
    ])
    df_comp.to_csv(output_dir / "expansion_comparison.csv", index=False)
    print(f"✓ Saved: {output_dir / 'expansion_comparison.csv'}")


def main():
    parser = argparse.ArgumentParser(description="Exploratory grid runner for Figure 2")
    parser.add_argument("--smoke-test", action="store_true", help="Run fast verification")
    parser.add_argument("--run", action="store_true", help="Run full simulation grid")
    parser.add_argument("--n-runs", type=int, default=200, help="Tissues per setting")
    parser.add_argument("--output-dir", type=str, default=str(Path(__file__).parent / "data"))

    args = parser.parse_args()

    if args.smoke_test:
        run_smoke_test()
    elif args.run:
        run_grid_sweep(args.n_runs, Path(args.output_dir))
    else:
        run_smoke_test()


if __name__ == "__main__":
    main()
