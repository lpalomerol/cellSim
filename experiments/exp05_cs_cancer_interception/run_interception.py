#!/usr/bin/env python3
"""
run_interception.py — Matched in silico cancer interception experiments.
Simulates matched cohorts across intervention ages (30, 40, 50) and doses (10%, 25%, 50%)
for D1, D2, and D1+D2 attenuation using build/run_paper_interventions.
"""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
import tempfile
import time
from pathlib import Path

import numpy as np
import pandas as pd
import statsmodels.api as sm
import statsmodels.formula.api as smf
from statsmodels.genmod.cov_struct import Exchangeable
from statsmodels.stats.contingency_tables import mcnemar

AGES = (30, 40, 50)
DOSES = (0.10, 0.25, 0.50)


def make_design(post: pd.DataFrame, out_path: Path, seed: int = 20260922) -> pd.DataFrame:
    print(f"Generating matched intervention design across ages {AGES} and doses {DOSES}...")
    rng = np.random.default_rng(seed)
    w = post["weight"].to_numpy(float, copy=True)
    w = w / w.sum()
    rows = []
    tissue_global = 0

    for age in AGES:
        # 8,000 base tissues; the first 6,000 also receive the 25% and 50% conditions.
        idx = rng.choice(len(post), size=8000, replace=True, p=w)
        for i, pidx in enumerate(idx):
            r = post.iloc[pidx]
            tid = tissue_global + i
            tissue_seed = 10000 + (age // 10) * 10000 + i
            base = {
                "tissue_id": tid,
                "seed": tissue_seed,
                "brca1_rate": float(r["brca1_rate"]),
                "low_delta": float(r["low_delta"]),
                "high_delta": float(r["high_delta"]),
                "neoplastic_div_rate": float(r["neoplastic_div_rate"]),
                "tp53_rate": 0.003,
                "d1_threshold": 2.0,
                "d2_threshold": 5.0,
                "division_rate": 0.001,
                "tumor_threshold": 0.05,
                "kappa_brca1": 2.0,
                "age_drift": 1e-5,
                "intervention_age": age,
            }
            rows.append({**base, "condition": "none", "dose": 0.0, "r_d1": 0.0, "r_d2": 0.0})
            for dose in DOSES:
                if dose > 0.10 and i >= 6000:
                    continue
                rows.append({**base, "condition": "D1", "dose": dose, "r_d1": dose, "r_d2": 0.0})
                rows.append({**base, "condition": "D2", "dose": dose, "r_d1": 0.0, "r_d2": dose})
                rows.append({**base, "condition": "D1+D2", "dose": dose, "r_d1": dose, "r_d2": dose})
        tissue_global += 10000

    df = pd.DataFrame(rows)
    df.to_csv(out_path, index=False)
    print(f"Design matrix generated: {len(df):,} combinations saved to {out_path}")
    return df


def run_binary(binary_path: str, design_path: Path, result_path: Path, threads: int = 8):
    print(f"Launching C++ binary {binary_path} with OMP_NUM_THREADS={threads}...")
    env = dict(os.environ)
    env["OMP_NUM_THREADS"] = str(threads)
    t0 = time.time()
    cmd = [binary_path, "--design", str(design_path), "--output", str(result_path)]
    subprocess.check_call(cmd, env=env)
    elapsed = time.time() - t0
    print(f"[SUCCESS] C++ simulation finished in {elapsed:.2f}s! Outputs: {result_path}")


def cumulative_curve(d: pd.DataFrame, age0: int, end: int = 80) -> pd.DataFrame:
    ages = np.arange(age0, end + 1)
    return pd.DataFrame({
        "age": ages,
        "risk": [100.0 * np.mean((d["onset_year"] >= 0) & (d["onset_year"] <= a)) for a in ages]
    })


def matched_set(res: pd.DataFrame, age: int, dose: float, condition: str) -> pd.DataFrame:
    none = res[(res["intervention_age"] == age) & (res["condition"] == "none")][
        ["tissue_id", "eligible", "tumor_by70", "tumor_by80", "onset_year"]
    ]
    tr = res[(res["intervention_age"] == age) & (res["condition"] == condition) & np.isclose(res["dose"], dose)][
        ["tissue_id", "eligible", "tumor_by70", "tumor_by80", "onset_year"]
    ]
    m = none.merge(tr, on="tissue_id", suffixes=("_none", "_tr"))
    if not np.array_equal(m["eligible_none"].to_numpy(), m["eligible_tr"].to_numpy()):
        raise RuntimeError("Eligibility mismatch between matched conditions.")
    return m[m["eligible_none"] == 1].copy()


def summarize(res: pd.DataFrame, out_dir: Path, boot: int = 1000, seed: int = 123):
    print("Computing matched summaries, ARR, dose-response, and interaction contrasts...")
    
    # 1. Figure 5a: Risk curves for age 30, 50% attenuation
    curves = []
    for condition in ("none", "D1", "D2", "D1+D2"):
        if condition == "none":
            z = res[(res["intervention_age"] == 30) & (res["condition"] == "none") & (res["eligible"] == 1)]
            tids = res[(res["intervention_age"] == 30) & (res["condition"] == "D1") & np.isclose(res["dose"], 0.50)]["tissue_id"]
            z = z[z["tissue_id"].isin(tids)]
        else:
            z = res[(res["intervention_age"] == 30) & (res["condition"] == condition) & np.isclose(res["dose"], 0.50) & (res["eligible"] == 1)]
        c = cumulative_curve(z, 30, 80)
        c["condition"] = condition
        curves.append(c)
    curves_df = pd.concat(curves, ignore_index=True)
    curves_df.to_csv(out_dir / "Figure5a_risk_curves.csv", index=False)
    print("  -> Saved Figure5a_risk_curves.csv")

    # 2. Figure 5b: ARR by age 70 for 50% dose across start ages 30, 40, 50
    arr = []
    tests = []
    for age in AGES:
        for condition in ("D1", "D2", "D1+D2"):
            m = matched_set(res, age, 0.50, condition)
            r0 = 100.0 * m["tumor_by70_none"].mean()
            ri = 100.0 * m["tumor_by70_tr"].mean()
            arr.append({
                "intervention_age": age,
                "condition": condition,
                "dose": 0.50,
                "risk_none_70": r0,
                "risk_intervention_70": ri,
                "ARR_pp": r0 - ri,
                "n": len(m)
            })
            table = pd.crosstab(m["tumor_by70_none"], m["tumor_by70_tr"]).reindex(index=[0, 1], columns=[0, 1], fill_value=0)
            test = mcnemar(table.to_numpy(), exact=False, correction=False)
            tests.append({
                "analysis": "McNemar_age70",
                "intervention_age": age,
                "condition": condition,
                "dose": 0.50,
                "statistic": float(test.statistic),
                "p": float(test.pvalue),
                "n": len(m)
            })
    arr_df = pd.DataFrame(arr)
    arr_df.to_csv(out_dir / "Figure5b_ARR_age70_50pct.csv", index=False)
    pd.DataFrame(tests).to_csv(out_dir / "Figure5b_McNemar_tests.csv", index=False)
    print("  -> Saved Figure5b_ARR_age70_50pct.csv and Figure5b_McNemar_tests.csv")

    # 3. Figure 5c: Dose-response at age 30
    dose_rows = []
    gee_rows = []
    for condition in ("D1", "D2", "D1+D2"):
        pieces = []
        for dose in DOSES:
            m = matched_set(res, 30, dose, condition)
            r0 = 100.0 * m["tumor_by70_none"].mean()
            ri = 100.0 * m["tumor_by70_tr"].mean()
            dose_rows.append({
                "condition": condition,
                "dose": dose,
                "risk_none_70": r0,
                "risk_intervention_70": ri,
                "ARR_pp": r0 - ri,
                "n": len(m)
            })
            x = m[["tissue_id", "tumor_by70_tr"]].copy()
            x["dose"] = dose
            x.rename(columns={"tumor_by70_tr": "tumor"}, inplace=True)
            pieces.append(x)
        g = pd.concat(pieces, ignore_index=True)
        model = smf.gee("tumor ~ dose", groups="tissue_id", data=g,
                        family=sm.families.Binomial(), cov_struct=Exchangeable()).fit()
        gee_rows.append({
            "condition": condition,
            "coef_dose": float(model.params["dose"]),
            "se": float(model.bse["dose"]),
            "p": float(model.pvalues["dose"])
        })
    pd.DataFrame(dose_rows).to_csv(out_dir / "Figure5c_dose_response_age30.csv", index=False)
    pd.DataFrame(gee_rows).to_csv(out_dir / "Figure5c_GEE_tests.csv", index=False)
    print("  -> Saved Figure5c_dose_response_age30.csv and Figure5c_GEE_tests.csv")

    # 4. Figure S5: Interaction on absolute-risk scale through age 80 with paired bootstrap
    rng = np.random.default_rng(seed)
    inter = []
    for age in AGES:
        for dose in DOSES:
            d1 = matched_set(res, age, dose, "D1")
            d2 = matched_set(res, age, dose, "D2")
            both = matched_set(res, age, dose, "D1+D2")
            x = d1[["tissue_id", "tumor_by80_none", "tumor_by80_tr"]].rename(columns={"tumor_by80_tr": "y_d1"})
            x = x.merge(d2[["tissue_id", "tumor_by80_tr"]].rename(columns={"tumor_by80_tr": "y_d2"}), on="tissue_id")
            x = x.merge(both[["tissue_id", "tumor_by80_tr"]].rename(columns={"tumor_by80_tr": "y_both"}), on="tissue_id")
            y0 = x["tumor_by80_none"].to_numpy(float)
            y1 = x["y_d1"].to_numpy(float)
            y2 = x["y_d2"].to_numpy(float)
            yb = x["y_both"].to_numpy(float)

            def icontrast(ii):
                r0 = y0[ii].mean()
                a1 = r0 - y1[ii].mean()
                a2 = r0 - y2[ii].mean()
                ab = r0 - yb[ii].mean()
                return 100.0 * (ab - a1 - a2)

            base = np.arange(len(x))
            est = icontrast(base)
            bs = np.empty(boot)
            for b in range(boot):
                ii = rng.integers(0, len(x), len(x))
                bs[b] = icontrast(ii)
            inter.append({
                "intervention_age": age,
                "dose": dose,
                "interaction_pp": est,
                "ci_lo": float(np.quantile(bs, 0.025)),
                "ci_hi": float(np.quantile(bs, 0.975)),
                "n": len(x)
            })
    pd.DataFrame(inter).to_csv(out_dir / "FigureS5_interaction.csv", index=False)
    print("  -> Saved FigureS5_interaction.csv")


def run_smoke_test(binary_path: str, out_dir: Path) -> int:
    print("[SMOKE TEST] Running fast verification of run_paper_interventions binary...")
    t0 = time.time()
    design_content = (
        "tissue_id,seed,condition,dose,intervention_age,r_d1,r_d2,brca1_rate,low_delta,high_delta,neoplastic_div_rate\n"
        "1,1001,none,0.0,30,0.0,0.0,0.0457,0.104,0.392,0.159\n"
        "1,1001,D2,0.5,30,0.0,0.5,0.0457,0.104,0.392,0.159\n"
    )
    with tempfile.NamedTemporaryFile(suffix=".csv", mode="w", delete=False) as f:
        f.write(design_content)
        des_path = f.name

    test_out = out_dir / "smoke_intervention_results.csv"
    cmd = [binary_path, "--design", des_path, "--output", str(test_out)]
    res = subprocess.run(cmd, capture_output=True, text=True)
    if os.path.exists(des_path):
        os.unlink(des_path)

    if res.returncode != 0:
        print(f"[ERROR] Failed to run binary: {res.stderr}")
        return 1

    if test_out.exists():
        df = pd.read_csv(test_out)
        test_out.unlink()
        elapsed = time.time() - t0
        print(f"[OK] Smoke test successful in {elapsed:.2f}s ({len(df)} outcomes evaluated)!")
        return 0
    return 1


def main() -> int:
    print("=== Cancer Interception Simulation Runner ===")
    parser = argparse.ArgumentParser(description="Cancer interception runner.")
    parser.add_argument("--smoke-test", action="store_true", help="Fast smoke test (<1s)")
    parser.add_argument("--simulate", action="store_true", help="Run full 204,000 condition simulation")
    parser.add_argument("--workers", type=int, default=8, help="OMP worker threads (default: 8)")
    parser.add_argument("--binary", type=str, default="./build/run_paper_interventions")
    parser.add_argument("--post-csv", type=str, default="experiments/exp04_emergent_features/data/Figure4a_parameters.csv")
    parser.add_argument("--output-dir", type=str, default=str(Path(__file__).resolve().parent / "data"))
    args = parser.parse_args()

    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    print(f"[INFO] Running in mode {'smoke test' if args.smoke_test else 'full simulation' if args.simulate else 'no action'}...")

    if args.smoke_test:
        return run_smoke_test(args.binary, out_dir)

    if args.simulate:
        print("[INFO] Running full matched intervention simulation...")
        post = pd.read_csv(args.post_csv)
        design_path = out_dir / "intervention_design.csv"
        results_path = out_dir / "intervention_results.csv"
        make_design(post, design_path)
        run_binary(args.binary, design_path, results_path, threads=args.workers)
        res = pd.read_csv(results_path)
        summarize(res, out_dir)
        return 0

    print("[INFO] Use plot_figure5_sfig5.py to render figures from calibrated intervention summaries.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
