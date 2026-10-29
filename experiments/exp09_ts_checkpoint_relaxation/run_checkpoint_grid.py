#!/usr/bin/env python3
"""
Simulate grid of somatic driver checkpoint relaxation parameters:
- gamma (somatic driver rate): [0, 1e-5, 2e-5, 1e-4, 2e-4, 3e-4]
- p_bypass (checkpoint bypass probability): [0.25, 0.50, 0.75, 1.00]

Outputs:
- data/checkpoint_grid_results.csv (SSE vs clinical penetrance & % tumors enabled)
"""

from __future__ import annotations

import argparse
import json
import subprocess
import tempfile
import time
from pathlib import Path
import numpy as np
import pandas as pd

EXP_DIR = Path(__file__).resolve().parent
ROOT_DIR = EXP_DIR.parent.parent
CONFIG_PATH = ROOT_DIR / "tissue_scale" / "configs" / "final_multiobjective.json"
CLINICAL_PATH = ROOT_DIR / "tissue_scale" / "data" / "clinical_penetrance.csv"
DATA_DIR = EXP_DIR / "data"

CLINICAL_AGES = np.array([30, 40, 50, 60, 70, 80])
GAMMAS = [
    ("Reference (gamma=0)", 0.0),
    ("1 x 10^-5", 1e-5),
    ("2 x 10^-5", 2e-5),
    ("1 x 10^-4", 1e-4),
    ("2 x 10^-4", 2e-4),
    ("3 x 10^-4", 3e-4),
]
BYPASS_PROBS = [0.25, 0.50, 0.75, 1.00]


def find_binary() -> Path:
    candidates = [
        ROOT_DIR / "build" / "tissue_scale_sim",
        ROOT_DIR / "tissue_scale" / "bin" / "tissue_scale_sim",
    ]
    for c in candidates:
        if c.exists() and c.is_file():
            return c
    subprocess.run(["cmake", "--build", str(ROOT_DIR / "build"), "--target", "tissue_scale_sim", "-j"], check=True)
    for c in candidates:
        if c.exists() and c.is_file():
            return c
    raise FileNotFoundError("Could not find tissue_scale_sim binary")


def run_single(bin_path: Path, params: dict, n_sim: int, seed: int, gamma: float, alpha: float) -> tuple[float, float]:
    """Run one grid condition, return (unweighted SSE, tumors enabled %)."""
    cmd = [
        str(bin_path),
        str(params["mu_TP53"]),
        str(params["beta_BRCA1"]),
        str(params["delta_low"]),
        str(params["delta_high"]),
        str(params["theta_D1"]),
        str(params["theta_D2_mean"]),
        str(params["tau_T_years"]),
        str(params["sigma_thetaD2"]),
        str(n_sim),
        str(seed),
        str(gamma),
        str(alpha),
    ]

    with tempfile.NamedTemporaryFile("w+", suffix=".csv") as tmp:
        subprocess.run(cmd, stdout=tmp, check=True)
        tmp.seek(0)
        df = pd.read_csv(tmp.name)

    clin_df = pd.read_csv(CLINICAL_PATH).set_index("age")
    obs_pen = clin_df.loc[CLINICAL_AGES, "penetrance"].to_numpy(float)

    sim_pen = np.array([(df["detect_age"].between(0, a)).mean() * 100.0 for a in CLINICAL_AGES])
    sse = float(np.sum((sim_pen - obs_pen) ** 2))

    tumors = df[df["detect_age"] >= 0]
    if len(tumors) > 0 and "checkpoint_relaxed" in tumors.columns:
        enabled_pct = float(100.0 * tumors["checkpoint_relaxed"].mean())
    else:
        enabled_pct = 0.0

    return sse, enabled_pct


def main():
    parser = argparse.ArgumentParser(description="Run checkpoint relaxation grid.")
    parser.add_argument("--smoke-test", action="store_true", help="Fast smoke test (N=50 per cell)")
    parser.add_argument("--n-sim", type=int, default=3000, help="Simulations per condition (default: 3000)")
    parser.add_argument("--output", default=str(DATA_DIR / "checkpoint_grid_results.csv"))
    args = parser.parse_args()

    n_sim = 50 if args.smoke_test else args.n_sim
    bin_path = find_binary()
    calib_cfg = json.loads(CONFIG_PATH.read_text())

    print(f"🔬 Running Checkpoint Relaxation Grid (N={n_sim} per condition)...")
    records = []
    base_seed = 20261029

    step = 0
    total = len(GAMMAS) * len(BYPASS_PROBS)
    start_all = time.time()

    for g_label, g_val in GAMMAS:
        for p_byp in BYPASS_PROBS:
            step += 1
            seed = base_seed + step * 101
            t0 = time.time()
            sse, enabled_pct = run_single(bin_path, calib_cfg, n_sim, seed, g_val, p_byp)
            dur = time.time() - t0
            print(f"  [{step}/{total}] gamma={g_val:8.1e}, p_byp={p_byp:4.2f} -> SSE={sse:6.1f}, Enabled={enabled_pct:5.1f}% ({dur:.2f}s)")
            records.append({
                "gamma_label": g_label,
                "gamma_val": g_val,
                "p_bypass": p_byp,
                "sse": round(sse, 1),
                "tumors_enabled_pct": round(enabled_pct, 1),
            })

    out_df = pd.DataFrame(records)
    out_path = Path(args.output)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_df.to_csv(out_path, index=False)
    print(f"\n🎉 Grid complete in {time.time() - start_all:.2f}s. Saved to {out_path}")


if __name__ == "__main__":
    main()
