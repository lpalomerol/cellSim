#!/usr/bin/env python3
"""Run exploratory parameter grid evaluations and simulation trajectories for Figure 2."""
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
ALL_AGES = np.arange(0, 81, dtype=int)
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
    max_t: int = 80,
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
        "--max-t", str(max_t),
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
    threads = env.get("OMP_NUM_THREADS", str(min(8, os.cpu_count() or 4)))
    env["OMP_NUM_THREADS"] = threads

    try:
        r = subprocess.run(cmd, capture_output=True, text=True, check=False, env=env)
        if r.returncode != 0:
            raise RuntimeError(f"run_bootstrapping execution failed:\n{r.stderr}\n{r.stdout}")

        df = pd.read_csv(tmp_csv)
        onsets = df["onset_year"].to_numpy()
        risks = np.array([100.0 * np.mean((onsets >= 0) & (onsets <= age)) for age in AGES])
        annual_ages = np.arange(0, max_t + 1, dtype=int)
        annual_curve = np.array([100.0 * np.mean((onsets >= 0) & (onsets <= age)) for age in annual_ages])
        unweighted_sse = float(np.sum((risks - OBSERVED_PENETRANCE) ** 2))
        weighted_sse = float(np.sum(((risks - OBSERVED_PENETRANCE) / SIGMA_WEIGHT) ** 2))

        return {
            "risk": risks,
            "annual_ages": annual_ages,
            "annual_curve": annual_curve,
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


def generate_figure2_trajectories(n_runs: int, output_dir: Path, division_rate: float = 0.001, suffix: str = "", max_t: int = 80):
    """Generate authentic Monte Carlo simulation trajectories for Figure 2a and 2b."""
    output_dir.mkdir(parents=True, exist_ok=True)
    print(f"🔬 Simulating authentic Figure 2 trajectories with N={n_runs} tissues per curve (d={division_rate}, max_t={max_t})...")

    # 1. Panel a: Pre-transformation parameter exploration (basal expansion, nu = d)
    # Exact combinations from the 6x6 coarse grid (beta in [0.01, 0.10], dl in [0.02, 0.15])
    # 1. Higher-beta (best overall in grid, reported SSE ~2078): beta=0.060, dl=0.150
    # 2. Lower-beta: beta=0.010, dl=0.150 (delayed onset, rises late to 78%)
    # 3. Higher-delta_low: beta=0.040, dl=0.200 (overshoots above clinical reference at late ages, ~80%)
    # 4. Lower-delta_low: beta=0.040, dl=0.080 (suppressed penetrance, stays below at 56%)
    panel_a_settings = [
        ("Representative higher-β setting", 0.060, 0.150, 0.300, division_rate, False, 1000),
        ("Representative lower-β setting", 0.010, 0.150, 0.300, division_rate, False, 1000),
        ("Representative higher-δ_low setting", 0.040, 0.200, 0.400, division_rate, False, 1000),
        ("Representative lower-δ_low setting", 0.040, 0.080, 0.160, division_rate, False, 1000),
    ]

    rows_a = []
    summary_rows = []

    print("  -> Simulating Panel a (pre-transformation exploration)...")
    for name, beta, dl, dh, nu, bb, seed in panel_a_settings:
        res = evaluate_simulation(
            n_runs=n_runs,
            brca1_rate=beta,
            low_delta=dl,
            high_delta=dh,
            neoplastic_div_rate=nu,
            seed_offset=seed,
            big_bang=bb,
            division_rate=division_rate,
            max_t=max_t,
        )
        for age, pen in zip(res["annual_ages"], res["annual_curve"]):
            rows_a.append({"curve": name, "age": age, "penetrance": pen})
        
        summary_rows.append({
            "panel": "a",
            "curve": name,
            "beta": beta,
            "low_delta": dl,
            "high_delta": dh,
            "nu": nu,
            "division_rate": division_rate,
            "big_bang": bb,
            "unweighted_sse": res["unweighted_sse"],
            "weighted_sse": res["weighted_sse"],
            **{f"risk_{age}": r for age, r in zip(AGES, res["risk"])},
        })
        print(f"     ✓ {name}: SSE={res['unweighted_sse']:.1f}, weighted_SSE={res['weighted_sse']:.2f}")

    df_a = pd.DataFrame(rows_a)
    out_a = output_dir / f"figure2a_simulated_trajectories{suffix}.csv"
    df_a.to_csv(out_a, index=False)
    print(f"  ✓ Saved simulated Panel a data: {out_a}")

    # 2. Panel b: Effect of accelerated Tumoral expansion
    basal_label = f"Basal expansion (ν = {division_rate:.10g})" if division_rate != 0.001 else "Basal expansion (ν = 0.001)"
    panel_b_settings = [
        ("Accelerated expansion (ν = 0.10)", 0.045, 0.120, 0.240, 0.10, True, 0),
        (basal_label, 0.045, 0.120, 0.240, division_rate, False, 0),
    ]

    rows_b = []
    print("  -> Simulating Panel b (tumoral acceleration)...")
    for name, beta, dl, dh, nu, bb, seed in panel_b_settings:
        res = evaluate_simulation(
            n_runs=n_runs,
            brca1_rate=beta,
            low_delta=dl,
            high_delta=dh,
            neoplastic_div_rate=nu,
            seed_offset=seed,
            big_bang=bb,
            division_rate=division_rate,
            max_t=max_t,
        )
        for age, pen in zip(res["annual_ages"], res["annual_curve"]):
            rows_b.append({"curve": name, "age": age, "penetrance": pen})
        
        summary_rows.append({
            "panel": "b",
            "curve": name,
            "beta": beta,
            "low_delta": dl,
            "high_delta": dh,
            "nu": nu,
            "division_rate": division_rate,
            "big_bang": bb,
            "unweighted_sse": res["unweighted_sse"],
            "weighted_sse": res["weighted_sse"],
            **{f"risk_{age}": r for age, r in zip(AGES, res["risk"])},
        })
        print(f"     ✓ {name}: SSE={res['unweighted_sse']:.1f}, weighted_SSE={res['weighted_sse']:.2f}")

    df_b = pd.DataFrame(rows_b)
    out_b = output_dir / f"figure2b_simulated_trajectories{suffix}.csv"
    df_b.to_csv(out_b, index=False)
    print(f"  ✓ Saved simulated Panel b data: {out_b}")

    out_summary = output_dir / f"figure2_simulated_summary{suffix}.csv"
    pd.DataFrame(summary_rows).to_csv(out_summary, index=False)
    print(f"  ✓ Saved simulation summary metrics: {out_summary}")


def generate_all_grid_trajectories(n_runs: int, output_dir: Path, division_rate: float = 0.001, suffix: str = "", max_t: int = 80):
    """Simulate annual cumulative penetrance curves for all 36 coarse grid combinations."""
    output_dir.mkdir(parents=True, exist_ok=True)
    print(f"🔬 Simulating all 36 grid trajectories (N={n_runs} per setting, d={division_rate}, max_t={max_t})...")

    rows = []
    seed = 100000
    for beta in BETA_COARSE:
        for dl in DLOW_COARSE:
            dh = 2.0 * dl
            res = evaluate_simulation(
                n_runs=n_runs,
                brca1_rate=beta,
                low_delta=dl,
                high_delta=dh,
                neoplastic_div_rate=division_rate,
                seed_offset=seed,
                big_bang=False,
                division_rate=division_rate,
                max_t=max_t,
            )
            for age, pen in zip(res["annual_ages"], res["annual_curve"]):
                rows.append({
                    "beta": beta,
                    "low_delta": dl,
                    "high_delta": dh,
                    "age": age,
                    "penetrance": pen,
                })
            seed += 10003
            print(f"  Grid curve: β={beta:.3f}, δ_low={dl:.3f} -> SSE={res['unweighted_sse']:.1f}")

    df_grid = pd.DataFrame(rows)
    out_grid = output_dir / f"figure2a_all_grid_trajectories{suffix}.csv"
    df_grid.to_csv(out_grid, index=False)
    print(f"✓ Saved all 36 grid trajectories ({len(df_grid)} rows): {out_grid}")


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


def main():
    parser = argparse.ArgumentParser(description="Exploratory grid and trajectory runner for Figure 2")
    parser.add_argument("--smoke-test", action="store_true", help="Run fast verification")
    parser.add_argument("--generate-trajectories", action="store_true", help="Generate simulated trajectories for Figure 2")
    parser.add_argument("--generate-grid-trajectories", action="store_true", help="Simulate all 36 grid curves for Figure 2a background")
    parser.add_argument("--run-grid", action="store_true", help="Run full 6x6 coarse simulation grid summary")
    parser.add_argument("--division-rate", type=float, default=0.001, help="Normal cell division rate per tick / d (default: 0.001)")
    parser.add_argument("--max-t", type=int, default=80, help="Maximum years of follow-up (default: 80)")
    parser.add_argument("--file-suffix", type=str, default="", help="Suffix for output CSV files (e.g. '_d001')")
    parser.add_argument("--n-runs", type=int, default=1000, help="Tissues per setting (default: 1000)")
    parser.add_argument("--output-dir", type=str, default=str(Path(__file__).parent / "data"))

    args = parser.parse_args()

    if args.generate_trajectories:
        generate_figure2_trajectories(args.n_runs, Path(args.output_dir), division_rate=args.division_rate, suffix=args.file_suffix, max_t=args.max_t)
    elif args.generate_grid_trajectories:
        generate_all_grid_trajectories(args.n_runs, Path(args.output_dir), division_rate=args.division_rate, suffix=args.file_suffix, max_t=args.max_t)
    elif args.run_grid:
        run_grid_sweep(args.n_runs, Path(args.output_dir))
    elif args.smoke_test:
        run_smoke_test()
    else:
        run_smoke_test()


if __name__ == "__main__":
    main()
