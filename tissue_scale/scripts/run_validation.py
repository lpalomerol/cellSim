#!/usr/bin/env python3
"""
Run the stochastic BRCA1-ONSET tissue-scale simulator with fast smoke-test support.
"""

from __future__ import annotations

import argparse
import json
import subprocess
from pathlib import Path


def main():
    base_dir = Path(__file__).resolve().parent.parent
    parser = argparse.ArgumentParser(description="Run the exact stochastic BRCA1-ONSET tissue-scale simulator.")
    parser.add_argument("--config", default=str(base_dir / "configs" / "final_multiobjective.json"))
    parser.add_argument("--nsim", type=int, default=None)
    parser.add_argument("--seed", type=int, default=None)
    parser.add_argument("--binary", default=str(base_dir / "bin" / "tissue_scale_sim"))
    parser.add_argument("--output", default=str(base_dir / "results" / "smoke.csv"))
    parser.add_argument("--smoke-test", action="store_true", help="Run ultra-fast test (N=10) in < 2 seconds")

    args = parser.parse_args()

    cfg_path = Path(args.config)
    if not cfg_path.exists():
        raise FileNotFoundError(f"Config not found: {cfg_path}")

    cfg = json.loads(cfg_path.read_text())

    if args.smoke_test:
        nsim = 10
        seed = 20260923
    else:
        nsim = args.nsim if args.nsim is not None else int(cfg.get("validation_n", 1000))
        seed = args.seed if args.seed is not None else int(cfg.get("suggested_seed", 20260923))

    bin_path = Path(args.binary)
    if not bin_path.exists():
        raise FileNotFoundError(f"Simulator binary not found at {bin_path}. Run 'make -C tissue_scale' first.")

    cmd = [
        str(bin_path),
        str(cfg["mu_TP53"]),
        str(cfg["beta_BRCA1"]),
        str(cfg["delta_low"]),
        str(cfg["delta_high"]),
        str(cfg["theta_D1"]),
        str(cfg["theta_D2_mean"]),
        str(cfg["tau_T_years"]),
        str(cfg["sigma_thetaD2"]),
        str(nsim),
        str(seed),
    ]

    out = Path(args.output)
    out.parent.mkdir(parents=True, exist_ok=True)

    print(f"Running tissue_scale_sim (N={nsim}, seed={seed})...")
    with out.open("w") as fh:
        subprocess.run(cmd, stdout=fh, check=True)
    print(f"[SUCCESS] Wrote simulation output to {out}")


if __name__ == "__main__":
    main()
