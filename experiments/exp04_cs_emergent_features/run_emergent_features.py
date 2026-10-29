#!/usr/bin/env python3
"""
run_emergent_features.py — Evaluates emergent tumor features across posterior particles.
Simulates 20,000 virtual tissues (250 posterior particles x 80 tissues) using build/run_paper_features.
"""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
import tempfile
import time
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from typing import Tuple

import numpy as np
import pandas as pd
from scipy.stats import chi2_contingency

AGE_ORDER = ["<30", "30-39", "40-49", "50-59", "60-69", "70-79", "80"]
GENO_ORDER = ["Exclusively BRCA1+/-", "Mixed", "Exclusively BRCA1-/-"]
FOUNDER_ORDER = ["1", "2", "3", ">=4"]


def age_group(onset: int) -> str:
    if onset < 30:
        return "<30"
    if onset < 40:
        return "30-39"
    if onset < 50:
        return "40-49"
    if onset < 60:
        return "50-59"
    if onset < 70:
        return "60-69"
    if onset < 80:
        return "70-79"
    return "80"


def run_one_particle(item: Tuple[int, pd.Series], binary: str, n_per: int) -> pd.DataFrame:
    i, row = item
    with tempfile.NamedTemporaryFile(suffix=".csv", delete=False) as t:
        temp_path = t.name

    cmd = [
        binary,
        "--n-runs", str(n_per),
        "--seed-offset", str(20000 + i * n_per),
        "--brca1-rate", str(float(row["brca1_rate"])),
        "--low-delta", str(float(row["low_delta"])),
        "--high-delta", str(float(row["high_delta"])),
        "--neoplastic-div-rate", str(float(row["neoplastic_div_rate"])),
        "--tp53-rate", "0.003",
        "--d1-threshold", "2.0",
        "--d2-threshold", "5.0",
        "--division-rate", "0.001",
        "--tumor-threshold", "0.05",
        "--kappa-brca1", "2.0",
        "--age-drift", "0.00001",
        "--output", temp_path,
    ]
    env = dict(os.environ)
    env["OMP_NUM_THREADS"] = "1"
    res = subprocess.run(cmd, capture_output=True, text=True, env=env)
    if res.returncode != 0:
        if os.path.exists(temp_path):
            os.unlink(temp_path)
        raise RuntimeError(f"Error in run_paper_features: {res.stderr}")

    df = pd.read_csv(temp_path)
    if os.path.exists(temp_path):
        os.unlink(temp_path)

    df["particle"] = i
    w = float(row["weight"])
    df["posterior_weight"] = w
    df["tissue_weight"] = w / n_per
    return df


def simulate_cohort(post_csv: Path, binary: str, n_per: int, workers: int, out_dir: Path) -> pd.DataFrame:
    post = pd.read_csv(post_csv)
    post["weight"] = post["weight"] / post["weight"].sum()

    print(f"=========================================================================")
    print(f"Simulating Emergent Features: {len(post)} particles x {n_per} tissues = {len(post)*n_per:,} tissues")
    print(f"Binary: {binary} | Workers: {workers}")
    print(f"=========================================================================")

    t0 = time.time()
    jobs = list(post.iterrows())
    dfs = []
    with ThreadPoolExecutor(max_workers=workers) as ex:
        for k, d in enumerate(ex.map(lambda item: run_one_particle(item, binary, n_per), jobs), 1):
            dfs.append(d)
            if k % 25 == 0 or k == len(jobs):
                print(f"  -> Processed {k}/{len(jobs)} particles ({k*n_per:,} tissues)...", flush=True)

    elapsed = time.time() - t0
    all_df = pd.concat(dfs, ignore_index=True)
    raw_csv = out_dir / f"emergent_{len(post)*n_per}_tissues.csv"
    all_df.to_csv(raw_csv, index=False)
    print(f"[SUCCESS] Simulation completed in {elapsed:.2f}s! Saved: {raw_csv}")
    return all_df


def summarize_features(raw_df: pd.DataFrame, out_dir: Path):
    d = raw_df[raw_df["onset_year"] >= 0].copy()
    d["age_group"] = d["onset_year"].map(age_group)
    d["founder_category"] = d["founder_count"].map(lambda x: ">=4" if x >= 4 else str(int(x)))

    # Map C++ brca1_category to standard naming
    mapping = {
        "BRCA1+/-": "Exclusively BRCA1+/-",
        "mixed": "Mixed",
        "BRCA1-/-": "Exclusively BRCA1-/-",
    }
    d["genotype_cat"] = d["brca1_category"].map(mapping)

    # 1. Figure 4b: Overall Genotype Breakdown
    total_w = d["tissue_weight"].sum()
    rows_b = []
    for cat in GENO_ORDER:
        cat_w = d.loc[d["genotype_cat"] == cat, "tissue_weight"].sum()
        pct = (cat_w / total_w) * 100.0 if total_w > 0 else 0.0
        rows_b.append({"category": cat, "percentage": pct, "label": f"{pct:.0f}%"})
    df_b = pd.DataFrame(rows_b)
    df_b.to_csv(out_dir / "Figure4b_genotype_composition.csv", index=False)
    print("\n[Figure 4b Summary - BRCA1 Genotype Breakdown]:")
    for r in rows_b:
        print(f"  {r['category']:<25}: {r['percentage']:.2f}%")

    # 2. Figure 4c: Founder Lineages across Age Groups
    rows_c = []
    for ag in AGE_ORDER:
        sub = d[d["age_group"] == ag]
        ag_w = sub["tissue_weight"].sum()
        p_dict = {"age_group": ag}
        for f_cat, f_col in zip(FOUNDER_ORDER, ["founder_1_pct", "founder_2_pct", "founder_3_pct", "founder_ge4_pct"]):
            num = sub.loc[sub["founder_category"] == f_cat, "tissue_weight"].sum()
            pct = (num / ag_w * 100.0) if ag_w > 0 else 0.0
            p_dict[f_col] = round(pct, 2)
        rows_c.append(p_dict)
    df_c = pd.DataFrame(rows_c)
    df_c.to_csv(out_dir / "Figure4c_founder_lineages.csv", index=False)

    print("\n[Figure 4c Summary - Founder Lineage Stack Percentages]:")
    print(df_c.to_string(index=False))

    # 3. Chi-square test
    ct = pd.crosstab(d["age_group"], d["founder_category"]).reindex(index=AGE_ORDER, columns=FOUNDER_ORDER, fill_value=0)
    chi2_val, p_val, dof, _ = chi2_contingency(ct)
    print(f"\n[Statistical Test]:")
    print(f"  Chi-square = {chi2_val:.2f}, df = {dof}, P-value = {p_val:.2e} (n = {len(d):,} tumor-positive tissues)")

    df_test = pd.DataFrame([{
        "test": "chi-square",
        "outcome": "founder category across age groups",
        "statistic": chi2_val,
        "df": dof,
        "p_value": p_val,
        "n_tumors": len(d)
    }])
    df_test.to_csv(out_dir / "Figure4_statistical_tests.csv", index=False)


def run_smoke_test(binary_path: str, out_dir: Path) -> int:
    print("[SMOKE TEST] Running fast verification of run_paper_features binary...")
    t0 = time.time()
    test_csv = out_dir / "smoke_features.csv"
    cmd = [
        binary_path,
        "--n-runs", "5",
        "--seed-offset", "1000",
        "--brca1-rate", "0.0457",
        "--low-delta", "0.104",
        "--high-delta", "0.392",
        "--neoplastic-div-rate", "0.159",
        "--output", str(test_csv),
    ]
    res = subprocess.run(cmd, capture_output=True, text=True)
    if res.returncode != 0:
        print(f"[ERROR] Failed to run binary: {res.stderr}")
        return 1
    if test_csv.exists():
        df = pd.read_csv(test_csv)
        test_csv.unlink()
        elapsed = time.time() - t0
        print(f"[OK] Smoke test successful in {elapsed:.2f}s ({len(df)} tissues evaluated)!")
        return 0
    return 1


def main() -> int:
    parser = argparse.ArgumentParser(description="Emergent tumor features runner.")
    parser.add_argument("--smoke-test", action="store_true", help="Fast smoke test (<1s)")
    parser.add_argument("--simulate", action="store_true", help="Run full 20,000 tissue simulation")
    parser.add_argument("--n-per-particle", type=int, default=80, help="Tissues per particle (default: 80)")
    parser.add_argument("--workers", type=int, default=8, help="Worker threads (default: 8)")
    parser.add_argument("--binary", type=str, default="./build/run_paper_features")
    parser.add_argument("--post-csv", type=str, default=str(Path(__file__).resolve().parent / "data" / "Figure4a_parameters.csv"))
    parser.add_argument("--output-dir", type=str, default=str(Path(__file__).resolve().parent / "data"))
    args = parser.parse_args()

    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    if args.smoke_test:
        return run_smoke_test(args.binary, out_dir)

    raw_df = simulate_cohort(
        post_csv=Path(args.post_csv),
        binary=args.binary,
        n_per=args.n_per_particle,
        workers=args.workers,
        out_dir=out_dir
    )
    summarize_features(raw_df, out_dir)
    return 0


if __name__ == "__main__":
    sys.exit(main())
