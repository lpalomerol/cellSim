#!/usr/bin/env python3
"""
exp_01_abc_smc_locked_division.py — Pillar 1 Support: ABC-SMC calibration without accelerated tumoral expansion.

Evaluates the model under the constraint that neoplastic cells divide at normal basal tissue
rate (nu = d = 0.001) without the Big Bang switch.
Used as methodological proof/support for reviewers showing that unaccelerated tumoral
division fails to replicate clinical penetrance.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import sys

from abc_smc_framework import ABCConfig, ABCSMCEngine

# Priors for all free parameters under unaccelerated proliferation (neoplastic division = basal division)
LOCKED_PRIORS = {
    "brca1_rate":    (0.010, 0.080),  # Beta: baseline BRCA1 mutation rate per tick
    "low_delta":     (0.050, 0.250),  # Delta_low: genomic instability increment (+/-)
    "high_delta":    (0.120, 0.600),  # Delta_high: genomic instability increment (-/-)
    "tp53_rate":     (0.001, 0.010),  # Mu_TP53: TP53 somatic mutation rate
    "d1_threshold":  (1.0,   4.0),    # Theta_D1: PRIMER transformation threshold
    "d2_threshold":  (3.0,   8.0),    # Theta_D2: Immune evasion / TUMORAL threshold
    "division_rate": (0.0005, 0.005), # d: Normal tissue division rate per tick
    "kappa_brca1":   (1.0,   4.0),    # Kappa: BRCA1 D2 accumulation structural multiplier
}


def parse_args():
    parser = argparse.ArgumentParser(
        description="Run ABC-SMC calibration without accelerated tumoral expansion (locked division)."
    )
    parser.add_argument(
        "--output-dir",
        type=str,
        default="results/20260928_draft_map/exp_01_abc_smc_locked_division",
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
        "--n-sim",
        type=int,
        default=200,
        help="Simulated tissues per particle evaluation (default: 200)",
    )
    parser.add_argument(
        "--n-reps",
        type=int,
        default=2,
        help="Stochastic replicates per particle (default: 2)",
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
        print("[DRY-RUN] Running fast smoke test for exp_01_abc_smc_locked_division...")
        n_particles = 2
        n_generations = 2
        n_sim = 10
        n_reps = 1
    else:
        n_particles = args.particles
        n_generations = args.generations
        n_sim = args.n_sim
        n_reps = args.n_reps

    # enable_big_bang=False forces neoplastic cells to divide at basal division_rate
    config = ABCConfig(
        priors=LOCKED_PRIORS,
        n_particles=n_particles,
        n_generations=n_generations,
        n_sim=n_sim,
        n_reps=n_reps,
        alpha_quantile=0.75,
        enable_big_bang=False,
        workers=args.workers,
        seed=args.seed,
    )

    engine = ABCSMCEngine(config, output_dir)
    results = engine.run()
    print(f"\n[DONE] Locked division calibration finished. Results saved to: {output_dir}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
