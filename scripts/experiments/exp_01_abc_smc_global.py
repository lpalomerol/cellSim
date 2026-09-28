#!/usr/bin/env python3
"""
exp_01_abc_smc_global.py — Pillar 1: Unified Bayesian Calibration for BRCA1-ONSET.

Estimates free parameters (beta / brca1_rate, delta_low, delta_high, neoplastic_div_rate, etc.)
against the Kuchenbaecker 2017 clinical penetrance curve using adaptive ABC-SMC.

Treats neoplastic division rate (nu) as a continuous variable, demonstrating that the
posterior probability mass concentrates at accelerated proliferation (nu >> d) and
robustly rejects basal tissue division.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import sys

from abc_smc_framework import ABCConfig, ABCSMCEngine

# Comprehensive priors for all 9 free parameters in the model
GLOBAL_PRIORS = {
    "brca1_rate":          (0.010, 0.080),  # Beta: baseline BRCA1 mutation rate per tick
    "low_delta":           (0.050, 0.250),  # Delta_low: genomic instability increment (+/-)
    "high_delta":          (0.120, 0.600),  # Delta_high: genomic instability increment (-/-)
    "neoplastic_div_rate": (0.001, 0.250),  # Nu: continuous expansion rate spanning basal (0.001) to accelerated
    "tp53_rate":           (0.001, 0.010),  # Mu_TP53: TP53 somatic mutation rate
    "d1_threshold":        (1.0,   4.0),    # Theta_D1: PRIMER transformation threshold
    "d2_threshold":        (3.0,   8.0),    # Theta_D2: Immune evasion / TUMORAL threshold
    "division_rate":       (0.0005, 0.005), # d: Normal tissue division rate per tick
    "kappa_brca1":         (1.0,   4.0),    # Kappa: BRCA1 D2 accumulation structural multiplier
}


def parse_args():
    parser = argparse.ArgumentParser(
        description="Run unified ABC-SMC Bayesian parameter calibration for BRCA1-ONSET."
    )
    parser.add_argument(
        "--output-dir",
        type=str,
        default="results/20260928_draft_map/exp_01_abc_smc_global",
        help="Target outcome directory",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Fast pipeline smoke test (2 particles, 2 generations, 10 sims)",
    )
    parser.add_argument(
        "--particles",
        type=int,
        default=50,
        help="Number of particles per generation (ignored in --dry-run)",
    )
    parser.add_argument(
        "--generations",
        type=int,
        default=5,
        help="Number of SMC generations (ignored in --dry-run)",
    )
    parser.add_argument(
        "--workers",
        type=int,
        default=4,
        help="Concurrent execution workers",
    )
    parser.add_argument(
        "--seed",
        type=int,
        default=42,
        help="Random seed",
    )
    return parser.parse_args()


def main():
    args = parse_args()
    output_dir = Path(args.output_dir)

    if args.dry_run:
        print("[DRY-RUN] Running fast smoke test for exp_01_abc_smc_global...")
        n_particles = 2
        n_generations = 2
        n_sim = 10
        n_reps = 1
    else:
        n_particles = args.particles
        n_generations = args.generations
        n_sim = 200
        n_reps = 2

    config = ABCConfig(
        priors=GLOBAL_PRIORS,
        n_particles=n_particles,
        n_generations=n_generations,
        n_sim=n_sim,
        n_reps=n_reps,
        alpha_quantile=0.75,
        enable_big_bang=True,
        workers=args.workers,
        seed=args.seed,
    )

    engine = ABCSMCEngine(config, output_dir)
    results = engine.run()
    print(f"\n[DONE] Global calibration finished. Results saved to: {output_dir}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
