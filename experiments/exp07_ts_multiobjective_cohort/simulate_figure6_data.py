#!/usr/bin/env python3
"""
Run stochastic virtual tissue simulations for Figure 6 (Panels a, b, c, d, e).
Generates simulation CSVs using the C++ tissue_scale_sim engine.

Conditions simulated:
- Calibrated reference (10,000 tissues): final_multiobjective_validation_10000.csv
- Homogeneous model (3,000 tissues): homogeneous_validation_3000.csv
- Panel d: D2-threshold variation:
    - Low D2 variation (sigma=0.20, N=5,000 tissues): figure6d_sigma_020.csv
    - High D2 variation (sigma=0.60, N=5,000 tissues): figure6d_sigma_060.csv
- Panel e: Somatic driver checkpoint relaxation (alpha=0.5, N=3,000 tissues per condition):
    - gamma=1e-4: figure6e_gamma_1e4.csv
    - gamma=2e-4: figure6e_gamma_2e4.csv
    - gamma=3e-4: figure6e_gamma_3e4.csv
"""

from __future__ import annotations

import argparse
import json
import subprocess
import time
from pathlib import Path

EXP_DIR = Path(__file__).resolve().parent
ROOT_DIR = EXP_DIR.parent.parent
CONFIG_PATH = ROOT_DIR / "tissue_scale" / "configs" / "final_multiobjective.json"
DATA_DIR = EXP_DIR / "data"


def find_binary() -> Path:
    candidates = [
        ROOT_DIR / "build" / "tissue_scale_sim",
        ROOT_DIR / "tissue_scale" / "bin" / "tissue_scale_sim",
    ]
    for c in candidates:
        if c.exists() and c.is_file():
            return c
    # Try building
    subprocess.run(["cmake", "--build", str(ROOT_DIR / "build"), "--target", "tissue_scale_sim", "-j"], check=True)
    for c in candidates:
        if c.exists() and c.is_file():
            return c
    raise FileNotFoundError("Could not find or build tissue_scale_sim binary")


def run_sim(
    bin_path: Path,
    params_dict: dict,
    n_sim: int,
    seed: int,
    output_path: Path,
    gamma: float = 0.0,
    alpha: float = 0.0,
):
    """Invoke tissue_scale_sim binary and write CSV directly."""
    cmd = [
        str(bin_path),
        str(params_dict["mu_TP53"]),
        str(params_dict["beta_BRCA1"]),
        str(params_dict["delta_low"]),
        str(params_dict["delta_high"]),
        str(params_dict["theta_D1"]),
        str(params_dict["theta_D2_mean"]),
        str(params_dict["tau_T_years"]),
        str(params_dict["sigma_thetaD2"]),
        str(n_sim),
        str(seed),
        str(gamma),
        str(alpha),
    ]

    output_path.parent.mkdir(parents=True, exist_ok=True)
    start_t = time.time()
    with output_path.open("w") as fh:
        subprocess.run(cmd, stdout=fh, check=True)
    dur = time.time() - start_t
    print(f"  [DONE] {output_path.name}: {n_sim} tissues simulated in {dur:.2f}s")


def main():
    parser = argparse.ArgumentParser(description="Simulate datasets for Figure 6.")
    parser.add_argument("--smoke-test", action="store_true", help="Run ultra-fast smoke test (N=50 per setting)")
    parser.add_argument("--n-runs-panel-d", type=int, default=5000, help="Tissues for Panel d (default: 5000)")
    parser.add_argument("--n-runs-panel-e", type=int, default=3000, help="Tissues for Panel e (default: 3000)")
    args = parser.parse_args()

    n_d = 50 if args.smoke_test else args.n_runs_panel_d
    n_e = 50 if args.smoke_test else args.n_runs_panel_e

    bin_path = find_binary()
    calib_cfg = json.loads(CONFIG_PATH.read_text())

    print(f"🔬 Starting Figure 6 stochastic simulations (Panel d: N={n_d}, Panel e: N={n_e})...")

    # --- Panel d: Heterogeneity sigma = 0.20 and sigma = 0.60 ---
    print("\n▶ Simulating Panel d: D2-threshold heterogeneity...")
    cfg_sigma_020 = dict(calib_cfg)
    cfg_sigma_020["sigma_thetaD2"] = 0.20
    run_sim(bin_path, cfg_sigma_020, n_d, seed=20261001, output_path=DATA_DIR / "figure6d_sigma_020.csv")

    cfg_sigma_060 = dict(calib_cfg)
    cfg_sigma_060["sigma_thetaD2"] = 0.60
    run_sim(bin_path, cfg_sigma_060, n_d, seed=20261002, output_path=DATA_DIR / "figure6d_sigma_060.csv")

    # --- Panel e: Checkpoint relaxation (alpha=0.5, gamma in 1e-4, 2e-4, 3e-4) ---
    print("\n▶ Simulating Panel e: Checkpoint relaxation by somatic driver...")
    run_sim(bin_path, calib_cfg, n_e, seed=20261003, output_path=DATA_DIR / "figure6e_gamma_1e4.csv", gamma=1e-4, alpha=0.5)
    run_sim(bin_path, calib_cfg, n_e, seed=20261004, output_path=DATA_DIR / "figure6e_gamma_2e4.csv", gamma=2e-4, alpha=0.5)
    run_sim(bin_path, calib_cfg, n_e, seed=20261005, output_path=DATA_DIR / "figure6e_gamma_3e4.csv", gamma=3e-4, alpha=0.5)

    print("\n🎉 All Figure 6 simulations completed successfully!")


if __name__ == "__main__":
    main()
