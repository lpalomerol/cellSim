#!/usr/bin/env python3
"""
abc_smc.py — Canonical 4-Parameter ABC-SMC Calibration Engine for BRCA1-ONSET.

Scientific Context:
-------------------
Implements the canonical Bayesian Approximate Bayesian Computation Sequential
Monte Carlo (ABC-SMC) calibration pipeline described in Methods of Palomero et al. (2026).

Estimates the 4 unconstrained mechanistic parameters of the cell-scale model:
  1. beta (brca1_rate): Somatic loss of remaining functional BRCA1 allele [0.025, 0.080]
  2. delta_low (low_delta): Latent DNA damage accumulation in heterozygous cells [0.060, 0.220]
  3. delta_high (high_delta): Biallelic damage accumulation in homozygous cells [0.120, 0.600]
  4. nu (neoplastic_div_rate): Transformed neoplastic cell division rate [0.100, 0.240]

Fixed biological background parameters (validated by global sensitivity invariance):
  - mu_TP53 = 0.003
  - theta_D1 = 2.0
  - theta_D2 = 5.0
  - d (basal division) = 0.001
  - kappa_BRCA1 = 2.0
  - A_it (age drift) = 1e-5
  - tumor_threshold = 0.05 (5% neoplastic threshold)

Algorithm Specifications:
  - Population 0: Direct prior sample (uniform within bounds satisfying delta_high > delta_low).
  - Adaptive tolerance: epsilon_{t} = Q_0.75(distances_{t-1}).
  - Perturbation kernel: Multivariate Gaussian with Beaumont-Toni optimal empirical covariance:
      Sigma_t = 2 * Cov_w(particles_{t-1}) + diag_regularization.
  - Sequential Importance Sampling (SIS) weight updates with log-sum-exp numerical safeguards.
  - Multi-threaded execution across random seeds with OMP_NUM_THREADS=1 to prevent thread thrashing.
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
from typing import Dict, List, Optional, Tuple

import numpy as np
import pandas as pd
from scipy.stats import multivariate_normal

# Canonical 4 parameters and priors
PARAM_NAMES = ["brca1_rate", "low_delta", "high_delta", "neoplastic_div_rate"]
PRIORS: Dict[str, Tuple[float, float]] = {
    "brca1_rate":          (0.025, 0.080),
    "low_delta":           (0.060, 0.220),
    "high_delta":          (0.120, 0.600),
    "neoplastic_div_rate": (0.100, 0.240),
}

# Regex to parse stdout from run_bootstrapping
_DIST_RE = re.compile(r"weighted SSE\s*=\s*([\d.eE+\-]+)")
_AGE_ROW_RE = re.compile(r"^\s*(\d{2})\s+([\d.]+)\s+", re.MULTILINE)
AGES = [30, 40, 50, 60, 70, 80]


def is_valid_particle(theta: np.ndarray) -> bool:
    """Check prior bounds and biological constraint delta_high > delta_low."""
    for i, name in enumerate(PARAM_NAMES):
        lo, hi = PRIORS[name]
        if theta[i] < lo or theta[i] > hi:
            return False
    # delta_high must strictly exceed delta_low
    low_delta = theta[PARAM_NAMES.index("low_delta")]
    high_delta = theta[PARAM_NAMES.index("high_delta")]
    return high_delta > low_delta


def sample_prior(rng: np.random.Generator) -> np.ndarray:
    """Rejection-sample a single parameter vector from the constrained prior."""
    lo_bounds = np.array([PRIORS[p][0] for p in PARAM_NAMES])
    hi_bounds = np.array([PRIORS[p][1] for p in PARAM_NAMES])
    while True:
        candidate = rng.uniform(lo_bounds, hi_bounds)
        if is_valid_particle(candidate):
            return candidate


def evaluate_simulation(
    binary_path: str,
    theta: np.ndarray,
    n_sim: int,
    seed_offset: int,
    timeout_sec: int = 180,
) -> Tuple[float, Dict[str, float]]:
    """Runs run_bootstrapping for a single particle and returns (distance, risks)."""
    p_dict = {name: float(theta[i]) for i, name in enumerate(PARAM_NAMES)}
    cmd = [
        binary_path,
        "--n-runs", str(n_sim),
        "--seed-offset", str(seed_offset),
        "--output", "/dev/null",
        "--weighted-sse",
        "--big-bang",
        "--brca1-rate", f"{p_dict['brca1_rate']:.8g}",
        "--low-delta", f"{p_dict['low_delta']:.8g}",
        "--high-delta", f"{p_dict['high_delta']:.8g}",
        "--neoplastic-div-rate", f"{p_dict['neoplastic_div_rate']:.8g}",
        "--tp53-rate", "0.003",
        "--d1-threshold", "2.0",
        "--d2-threshold", "5.0",
        "--division-rate", "0.001",
        "--kappa-brca1", "2.0",
        "--age-drift", "1e-5",
    ]

    try:
        env = dict(os.environ)
        env["OMP_NUM_THREADS"] = "1"
        res = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout_sec, env=env)
    except (subprocess.TimeoutExpired, OSError):
        return float("inf"), {}

    if res.returncode != 0:
        return float("inf"), {}

    m_dist = _DIST_RE.search(res.stdout)
    if not m_dist:
        return float("inf"), {}
    dist = float(m_dist.group(1))

    risks = {}
    for m in _AGE_ROW_RE.finditer(res.stdout):
        age_val = int(m.group(1))
        if age_val in AGES:
            risks[f"risk_{age_val}"] = float(m.group(2))

    return dist, risks


def weighted_covariance(particles: np.ndarray, weights: np.ndarray) -> np.ndarray:
    """Compute Beaumont-Toni weighted empirical covariance matrix."""
    w = weights / np.sum(weights)
    mean = np.sum(particles * w[:, None], axis=0)
    diff = particles - mean
    denom = 1.0 - np.sum(w ** 2)
    if denom <= 1e-12:
        denom = 1.0
    cov = (diff * w[:, None]).T @ diff / denom
    # Perturbation covariance is 2 * empirical covariance + diagonal regularization
    cov_reg = 2.0 * cov + 1e-6 * np.eye(len(PARAM_NAMES))
    return cov_reg


def run_abc_smc(
    n_particles: int,
    n_generations: int,
    n_sim: int,
    workers: int,
    binary_path: str,
    output_dir: Path,
    seed: int = 42,
) -> None:
    """Executes the full multi-generation adaptive ABC-SMC calibration."""
    output_dir.mkdir(parents=True, exist_ok=True)
    rng = np.random.default_rng(seed)

    print("=========================================================================")
    print(f"Starting ABC-SMC Calibration: {len(PARAM_NAMES)} parameters, {n_generations} generations")
    print(f"Target: {n_particles} particles, {n_sim} tissues per particle")
    print(f"Binary: {binary_path} | Workers: {workers}")
    print("=========================================================================\n")

    current_particles = np.zeros((n_particles, len(PARAM_NAMES)))
    current_weights = np.ones(n_particles) / n_particles
    current_distances = np.zeros(n_particles)
    current_risks: List[Dict[str, float]] = [{}] * n_particles

    diag_rows = []
    prev_cov = np.eye(len(PARAM_NAMES))

    for gen in range(n_generations):
        t0 = time.time()
        print(f"\n[Generation {gen}/{n_generations - 1}]")

        if gen == 0:
            target_epsilon = float("inf")
            print("  Sampling initial population directly from prior...")
            for i in range(n_particles):
                current_particles[i] = sample_prior(rng)

            # Evaluate initial particles
            def eval_p0(idx: int) -> Tuple[int, float, Dict[str, float]]:
                dist, risks = evaluate_simulation(
                    binary_path, current_particles[idx], n_sim, seed_offset=10_000_000 + idx * 100_003
                )
                return idx, dist, risks

            with ThreadPoolExecutor(max_workers=workers) as executor:
                for idx, dist, risks in executor.map(eval_p0, range(n_particles)):
                    current_distances[idx] = dist
                    current_risks[idx] = risks

            attempts = n_particles
            accepted = n_particles
            prior_reject = 0
            current_weights = np.ones(n_particles) / n_particles
        else:
            # Adaptive epsilon: 75th percentile of previous generation distances
            target_epsilon = float(np.percentile(current_distances, 75))
            prev_particles = np.copy(current_particles)
            prev_weights = np.copy(current_weights)
            prev_cov = weighted_covariance(prev_particles, prev_weights)

            print(f"  Target tolerance threshold: epsilon_{gen} = {target_epsilon:.2f}")

            new_particles = []
            new_distances = []
            new_risks = []
            new_weights = []

            attempts = 0
            prior_reject = 0

            while len(new_particles) < n_particles:
                # Sample parent particle according to weights
                parent_idx = rng.choice(n_particles, p=prev_weights)
                parent_theta = prev_particles[parent_idx]

                # Perturb parent particle with Gaussian kernel
                candidate = rng.multivariate_normal(parent_theta, prev_cov)
                attempts += 1

                if not is_valid_particle(candidate):
                    prior_reject += 1
                    continue

                # Evaluate candidate simulation
                seed_offset = 20_000_000 + gen * 1_000_000 + attempts * 101
                dist, risks = evaluate_simulation(binary_path, candidate, n_sim, seed_offset)

                if dist <= target_epsilon:
                    new_particles.append(candidate)
                    new_distances.append(dist)
                    new_risks.append(risks)

                    # Compute Sequential Importance Sampling weight
                    # w_i = Prior(theta_i) / sum_j [ w_{j, prev} * Kernel(theta_i | theta_j) ]
                    # Constrained uniform prior is constant
                    prior_dens = 1.0
                    kernel_dens = multivariate_normal.pdf(prev_particles, mean=candidate, cov=prev_cov)
                    denom = np.sum(prev_weights * kernel_dens)
                    w = prior_dens / denom if denom > 0 else 1.0
                    new_weights.append(w)

                    if len(new_particles) % 50 == 0 or len(new_particles) == n_particles:
                        acc_rate = len(new_particles) / attempts
                        print(f"    -> Accepted {len(new_particles)}/{n_particles} | "
                              f"Attempts: {attempts} | Acc. rate: {acc_rate * 100:.1f}%")

            current_particles = np.array(new_particles)
            current_distances = np.array(new_distances)
            current_risks = new_risks
            current_weights = np.array(new_weights) / np.sum(new_weights)
            accepted = len(new_particles)

        elapsed = time.time() - t0
        ess = 1.0 / np.sum(current_weights ** 2)
        ess_ratio = ess / n_particles
        acc_rate = accepted / max(attempts, 1)

        med_dist = float(np.median(current_distances))
        q25_dist = float(np.percentile(current_distances, 25))
        q75_dist = float(np.percentile(current_distances, 75))

        print(f"  [Gen {gen} Summary] Med SSE: {med_dist:.2f} | Acc Rate: {acc_rate*100:.1f}% | "
              f"ESS: {ess:.1f} ({ess_ratio*100:.1f}%) | Time: {elapsed:.1f}s")

        # Save generation particle CSV
        gen_df = pd.DataFrame(current_particles, columns=PARAM_NAMES)
        gen_df["weight"] = current_weights
        gen_df["distance_weighted_sse"] = current_distances
        for age in AGES:
            gen_df[f"risk_{age}"] = [r.get(f"risk_{age}", np.nan) for r in current_risks]
        gen_df.to_csv(output_dir / f"generation_{gen:02d}.csv", index=False)

        diag_rows.append({
            "generation": gen,
            "epsilon": target_epsilon if gen > 0 else np.nan,
            "attempts": attempts,
            "prior_reject": prior_reject,
            "accepted": accepted,
            "acceptance_rate": acc_rate,
            "ess": ess,
            "ess_ratio": ess_ratio,
            "weighted_sse_median": med_dist,
            "weighted_sse_q25": q25_dist,
            "weighted_sse_q75": q75_dist,
            "elapsed_s": elapsed,
        })

    # Save full convergence diagnostics
    diag_df = pd.DataFrame(diag_rows)
    diag_df.to_csv(output_dir / "smc_diagnostics_20gen.csv", index=False)
    print(f"\n[DONE] ABC-SMC calibration finished successfully! Outputs saved to: {output_dir}")


def main() -> int:
    parser = argparse.ArgumentParser(description="Canonical 4-Parameter ABC-SMC Calibration Engine.")
    parser.add_argument("--particles", type=int, default=250, help="Number of particles per generation (default: 250)")
    parser.add_argument("--generations", type=int, default=20, help="Number of generations (default: 20)")
    parser.add_argument("--n-sim", type=int, default=300, help="Simulations per evaluation (default: 300)")
    parser.add_argument("--workers", type=int, default=8, help="Worker threads (default: 8)")
    parser.add_argument("--binary", type=str, default="./build/run_bootstrapping", help="Path to run_bootstrapping")
    parser.add_argument("--output-dir", type=str, default=str(Path(__file__).parent / "data"), help="Output directory")
    parser.add_argument("--smoke-test", action="store_true", help="Fast smoke test (2 particles, 2 gens, 5 sims)")
    parser.add_argument("--seed", type=int, default=42, help="RNG seed")

    args = parser.parse_args()
    out_dir = Path(args.output_dir)

    if args.smoke_test:
        print("[SMOKE TEST] Running fast verification of ABC-SMC pipeline...")
        t0 = time.time()
        run_abc_smc(
            n_particles=2,
            n_generations=2,
            n_sim=5,
            workers=2,
            binary_path=args.binary,
            output_dir=out_dir / "smoke_test_run",
            seed=args.seed,
        )
        elapsed = time.time() - t0
        print(f"[OK] Smoke test completed successfully in {elapsed:.2f}s!")
        return 0

    run_abc_smc(
        n_particles=args.particles,
        n_generations=args.generations,
        n_sim=args.n_sim,
        workers=args.workers,
        binary_path=args.binary,
        output_dir=out_dir,
        seed=args.seed,
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
