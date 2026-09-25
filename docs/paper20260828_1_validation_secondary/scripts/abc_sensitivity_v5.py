#!/usr/bin/env python3
"""
abc_sensitivity_v5.py — Multi-level Sensitivity ABC-SMC for CellSim.

Evaluates fixed parameter sensitivity across 3 confidence interval levels:
  Level 1: ±10% variation around default fixed parameters
  Level 2: ±25% variation around default fixed parameters
  Level 3: ±50% variation around default fixed parameters

Parameters evaluated (9 total):
  Primary free parameters (4):
    brca1_rate, low_delta, high_delta, neoplastic_div_rate
  Secondary parameters (5):
    tp53_rate, d1_threshold, d2_threshold, division_rate, tumor_threshold
"""

from __future__ import annotations

import argparse
import csv
import json
import os
import platform
import re
import subprocess
import sys
import time
from concurrent.futures import FIRST_COMPLETED, ThreadPoolExecutor, wait
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

import numpy as np

# Force OpenMP to use 1 thread per C++ binary to avoid thread oversubscription.
os.environ["OMP_NUM_THREADS"] = "1"

REPO_ROOT = Path(__file__).resolve().parent.parent
BINARY = str(REPO_ROOT / "build" / "run_bootstrapping")

DEFAULTS_SECONDARY = {
    "tp53_rate": 0.003,
    "d1_threshold": 2.0,
    "d2_threshold": 5.0,
    "division_rate": 0.001,
    "tumor_threshold": 0.05,
}

CI_FACTORS = {
    1: 0.10,  # ±10%
    2: 0.25,  # ±25%
    3: 0.50,  # ±50%
}

PARAM_NAMES = [
    "brca1_rate",
    "low_delta",
    "high_delta",
    "neoplastic_div_rate",
    "tp53_rate",
    "d1_threshold",
    "d2_threshold",
    "division_rate",
    "tumor_threshold",
]

LOW_IDX = PARAM_NAMES.index("low_delta")
HIGH_IDX = PARAM_NAMES.index("high_delta")
D1_IDX = PARAM_NAMES.index("d1_threshold")
D2_IDX = PARAM_NAMES.index("d2_threshold")

_DIST_RE = re.compile(r"weighted SSE\s*=\s*([\d.eE+\-]+)")
_MEDIAN_ONS_RE = re.compile(r"median onset\s*=\s*([\d.eE+\-]+)")
_MEDIAN_S25_RE = re.compile(r"median sat25\s*=\s*([\d.eE+\-]+)")
_MEDIAN_S50_RE = re.compile(r"median sat50\s*=\s*([\d.eE+\-]+)")
_MEDIAN_S90_RE = re.compile(r"median sat90\s*=\s*([\d.eE+\-]+)")


def get_priors(ci_level: int) -> dict[str, tuple[float, float]]:
    if ci_level not in CI_FACTORS:
        raise ValueError(f"Invalid ci_level={ci_level}. Must be 1, 2, or 3.")
    factor = CI_FACTORS[ci_level]
    priors = {
        "brca1_rate": (0.025, 0.080),
        "low_delta": (0.06, 0.22),
        "high_delta": (0.12, 0.60),
        "neoplastic_div_rate": (0.10, 0.24),
    }
    for name, default in DEFAULTS_SECONDARY.items():
        priors[name] = (default * (1.0 - factor), default * (1.0 + factor))
    return priors


def _valid(theta: np.ndarray) -> bool:
    """Enforce biological and structural constraints:
    1) high_delta > low_delta
    2) d2_threshold > d1_threshold
    """
    if theta[HIGH_IDX] <= theta[LOW_IDX]:
        return False
    if theta[D2_IDX] <= theta[D1_IDX]:
        return False
    return True


def sample_prior(rng: np.random.Generator, priors: dict[str, tuple[float, float]], n: int = 1) -> np.ndarray:
    lo = np.array([priors[p][0] for p in PARAM_NAMES], dtype=float)
    hi = np.array([priors[p][1] for p in PARAM_NAMES], dtype=float)
    samples = []
    while len(samples) < n:
        candidate = rng.uniform(lo, hi, size=len(PARAM_NAMES))
        if _valid(candidate):
            samples.append(candidate)
    return np.array(samples)


def log_prior(theta: np.ndarray, priors: dict[str, tuple[float, float]]) -> float:
    lo = np.array([priors[p][0] for p in PARAM_NAMES], dtype=float)
    hi = np.array([priors[p][1] for p in PARAM_NAMES], dtype=float)
    if np.any(theta < lo) or np.any(theta > hi):
        return -np.inf
    if not _valid(theta):
        return -np.inf
    return -float(np.sum(np.log(hi - lo)))


def compute_ess(weights: np.ndarray) -> float:
    if len(weights) == 0:
        return 0.0
    return float(1.0 / np.sum(weights ** 2))


def _to_float(match: re.Match[str] | None) -> float:
    return float(match.group(1)) if match else float("nan")


def run_simulation(
    theta_arr: np.ndarray, n_sim: int, n_reps: int, seed_base: int
) -> tuple[float, float, float, float, float] | tuple[None, None, None, None, None]:
    (
        brca1_rate,
        low_delta,
        high_delta,
        neo_div,
        tp53_rate,
        d1_thresh,
        d2_thresh,
        div_rate,
        tumor_thresh,
    ) = theta_arr

    distances: list[float] = []
    onset_vals: list[float] = []
    sat25_vals: list[float] = []
    sat50_vals: list[float] = []
    sat90_vals: list[float] = []

    for rep in range(n_reps):
        cmd = [
            BINARY,
            "--n-runs", str(n_sim),
            "--brca1-rate", f"{brca1_rate:.6f}",
            "--low-delta", f"{low_delta:.6f}",
            "--high-delta", f"{high_delta:.6f}",
            "--neoplastic-div-rate", f"{neo_div:.6f}",
            "--tp53-rate", f"{tp53_rate:.6f}",
            "--d1-threshold", f"{d1_thresh:.6f}",
            "--d2-threshold", f"{d2_thresh:.6f}",
            "--division-rate", f"{div_rate:.6f}",
            "--tumor-threshold", f"{tumor_thresh:.6f}",
            "--big-bang",
            "--weighted-sse",
            "--seed-offset", str(seed_base + rep * 100_003),
            "--output", "/dev/null",
        ]
        try:
            r = subprocess.run(cmd, capture_output=True, text=True, timeout=180)
        except (subprocess.TimeoutExpired, OSError):
            return None, None, None, None, None
        if r.returncode != 0:
            return None, None, None, None, None

        m_dist = _DIST_RE.search(r.stdout)
        if not m_dist:
            return None, None, None, None, None

        distances.append(float(m_dist.group(1)))
        onset_vals.append(_to_float(_MEDIAN_ONS_RE.search(r.stdout)))
        sat25_vals.append(_to_float(_MEDIAN_S25_RE.search(r.stdout)))
        sat50_vals.append(_to_float(_MEDIAN_S50_RE.search(r.stdout)))
        sat90_vals.append(_to_float(_MEDIAN_S90_RE.search(r.stdout)))

    if not distances:
        return None, None, None, None, None

    return (
        float(np.mean(distances)),
        float(np.nanmedian(onset_vals)),
        float(np.nanmedian(sat25_vals)),
        float(np.nanmedian(sat50_vals)),
        float(np.nanmedian(sat90_vals)),
    )


def _worker(
    args: tuple[np.ndarray, int, int, int]
) -> tuple[np.ndarray, float | None, float | None, float | None, float | None, float | None]:
    theta_arr, n_sim, n_reps, seed_base = args
    dist, onset, s25, s50, s90 = run_simulation(theta_arr, n_sim, n_reps, seed_base)
    return theta_arr, dist, onset, s25, s50, s90


def compute_weights(
    particles: np.ndarray,
    prev_particles: np.ndarray,
    prev_weights: np.ndarray,
    sigmas: np.ndarray,
    priors: dict[str, tuple[float, float]],
) -> np.ndarray:
    n, d = particles.shape
    log_prior_vals = np.array([log_prior(particles[i], priors) for i in range(n)])

    if np.any(np.isinf(log_prior_vals)):
        raise RuntimeError("compute_weights: some particles have log_prior = -inf")

    log_kernel = np.zeros((n, n))
    for dim in range(d):
        diff = particles[:, dim : dim + 1] - prev_particles[:, dim]
        log_kernel -= 0.5 * (diff / sigmas[dim]) ** 2 + np.log(sigmas[dim] * np.sqrt(2 * np.pi))

    sum_log_kernel = np.sum(log_kernel, axis=2) if log_kernel.ndim == 3 else np.sum(log_kernel, axis=1, keepdims=True)
    # Using log-space sum
    log_prev_weights = np.log(prev_weights + 1e-300)
    denom_terms = log_prev_weights + log_kernel
    max_denom = np.max(denom_terms, axis=1, keepdims=True)
    log_denom = max_denom.squeeze() + np.log(np.sum(np.exp(denom_terms - max_denom), axis=1))

    log_unnorm = log_prior_vals - log_denom
    max_log = np.max(log_unnorm)
    unnorm = np.exp(log_unnorm - max_log)
    w_sum = np.sum(unnorm)
    if w_sum == 0 or not np.isfinite(w_sum):
        return np.full(n, 1.0 / n)
    return unnorm / w_sum


def run_abc_smc(
    ci_level: int,
    n_particles: int = 200,
    n_generations: int = 8,
    n_sim: int = 150,
    n_reps: int = 1,
    alpha: float = 0.6,
    workers: int = 4,
    seed: int = 42,
    outdir: Path | None = None,
) -> Path:
    priors = get_priors(ci_level)
    rng = np.random.Generator(np.random.PCG64(seed))

    if outdir is None:
        outdir = REPO_ROOT / "results" / "abc_sensitivity" / f"level_{ci_level}"
    outdir.mkdir(parents=True, exist_ok=True)

    print(f"=== Starting ABC-SMC Sensitivity Run (CI Level {ci_level}: ±{int(CI_FACTORS[ci_level]*100)}%) ===")
    print(f"  Particles={n_particles}, Generations={n_generations}, Workers={workers}, Seed={seed}")
    print(f"  Output directory: {outdir}")

    # Generation 0
    print("\n--- Generation 0 (Prior Sampling) ---")
    accepted_theta: list[np.ndarray] = []
    accepted_dist: list[float] = []
    accepted_onset: list[float] = []
    accepted_s25: list[float] = []
    accepted_s50: list[float] = []
    accepted_s90: list[float] = []

    attempts = 0
    failures = 0
    seed_counter = seed

    with ThreadPoolExecutor(max_workers=workers) as executor:
        futures = set()
        while len(accepted_theta) < n_particles:
            while len(futures) < workers * 2:
                cand = sample_prior(rng, priors, n=1)[0]
                seed_counter += 1
                fut = executor.submit(_worker, (cand, n_sim, n_reps, seed_counter))
                futures.add(fut)

            done, futures = wait(futures, return_when=FIRST_COMPLETED)
            for fut in done:
                attempts += 1
                cand_theta, dist, onset, s25, s50, s90 = fut.result()
                if dist is None:
                    failures += 1
                    continue
                if len(accepted_theta) < n_particles:
                    accepted_theta.append(cand_theta)
                    accepted_dist.append(dist)
                    accepted_onset.append(onset)
                    accepted_s25.append(s25)
                    accepted_s50.append(s50)
                    accepted_s90.append(s90)

    particles = np.array(accepted_theta)
    distances = np.array(accepted_dist)
    weights = np.full(n_particles, 1.0 / n_particles)
    epsilon = float(np.percentile(distances, alpha * 100))

    gen0_csv = outdir / "gen_00.csv"
    with open(gen0_csv, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(PARAM_NAMES + ["distance", "weight", "onset_age", "sat25", "sat50", "sat90"])
        for i in range(n_particles):
            writer.writerow(
                list(particles[i]) + [distances[i], weights[i], accepted_onset[i], accepted_s25[i], accepted_s50[i], accepted_s90[i]]
            )

    print(f"Gen 0 complete: dist_mean={np.mean(distances):.2f}, dist_min={np.min(distances):.2f}, eps_next={epsilon:.2f}")

    # Subsequent SMC Generations
    for gen in range(1, n_generations):
        print(f"\n--- Generation {gen} (eps_target <= {epsilon:.2f}) ---")
        prev_particles = particles.copy()
        prev_weights = weights.copy()
        sigmas = 2.0 * np.std(prev_particles, axis=0)

        gen_theta: list[np.ndarray] = []
        gen_dist: list[float] = []
        gen_onset: list[float] = []
        gen_s25: list[float] = []
        gen_s50: list[float] = []
        gen_s90: list[float] = []

        gen_attempts = 0
        gen_failures = 0

        with ThreadPoolExecutor(max_workers=workers) as executor:
            futures = set()
            while len(gen_theta) < n_particles:
                while len(futures) < workers * 2:
                    idx = rng.choice(n_particles, p=prev_weights)
                    parent = prev_particles[idx]
                    proposal = parent + rng.normal(0, sigmas)

                    if log_prior(proposal, priors) == -np.inf:
                        gen_attempts += 1
                        continue

                    seed_counter += 1
                    fut = executor.submit(_worker, (proposal, n_sim, n_reps, seed_counter))
                    futures.add(fut)

                done, futures = wait(futures, return_when=FIRST_COMPLETED)
                for fut in done:
                    gen_attempts += 1
                    prop_theta, dist, onset, s25, s50, s90 = fut.result()
                    if dist is None or dist > epsilon:
                        if dist is None:
                            gen_failures += 1
                        continue
                    if len(gen_theta) < n_particles:
                        gen_theta.append(prop_theta)
                        gen_dist.append(dist)
                        gen_onset.append(onset)
                        gen_s25.append(s25)
                        gen_s50.append(s50)
                        gen_s90.append(s90)

        particles = np.array(gen_theta)
        distances = np.array(gen_dist)
        weights = compute_weights(particles, prev_particles, prev_weights, sigmas, priors)

        epsilon = float(np.percentile(distances, alpha * 100))
        ess = compute_ess(weights)

        gen_csv = outdir / f"gen_{gen:02d}.csv"
        with open(gen_csv, "w", newline="") as f:
            writer = csv.writer(f)
            writer.writerow(PARAM_NAMES + ["distance", "weight", "onset_age", "sat25", "sat50", "sat90"])
            for i in range(n_particles):
                writer.writerow(
                    list(particles[i]) + [distances[i], weights[i], gen_onset[i], gen_s25[i], gen_s50[i], gen_s90[i]]
                )

        print(
            f"Gen {gen} complete: dist_mean={np.mean(distances):.2f}, dist_min={np.min(distances):.2f}, "
            f"ESS={ess:.1f}/{n_particles}, eps_next={epsilon:.2f}"
        )

    # Save run manifest
    manifest = {
        "ci_level": ci_level,
        "ci_factor": CI_FACTORS[ci_level],
        "n_particles": n_particles,
        "n_generations": n_generations,
        "priors": {k: list(v) for k, v in priors.items()},
        "completed_at": datetime.now(timezone.utc).isoformat(),
        "platform": platform.platform(),
    }
    with open(outdir / "run_manifest.json", "w") as f:
        json.dump(manifest, f, indent=2)

    print(f"\nSensitivity ABC-SMC Level {ci_level} completed successfully.")
    return outdir


def main():
    parser = argparse.ArgumentParser(description="Multi-level Sensitivity ABC-SMC for CellSim")
    parser.add_argument("--ci-level", type=int, choices=[1, 2, 3], required=True, help="CI level: 1 (±10%), 2 (±25%), 3 (±50%)")
    parser.add_argument("--n-particles", type=int, default=200, help="Particles per generation (default: 200)")
    parser.add_argument("--n-generations", type=int, default=8, help="SMC generations (default: 8)")
    parser.add_argument("--n-sim", type=int, default=150, help="Simulations per evaluation (default: 150)")
    parser.add_argument("--n-reps", type=int, default=1, help="Replicates per particle (default: 1)")
    parser.add_argument("--workers", type=int, default=4, help="Parallel worker threads (default: 4)")
    parser.add_argument("--seed", type=int, default=42, help="RNG seed (default: 42)")
    parser.add_argument("--outdir", type=str, default=None, help="Output directory")

    args = parser.parse_args()
    outdir_path = Path(args.outdir) if args.outdir else None
    run_abc_smc(
        ci_level=args.ci_level,
        n_particles=args.n_particles,
        n_generations=args.n_generations,
        n_sim=args.n_sim,
        n_reps=args.n_reps,
        workers=args.workers,
        seed=args.seed,
        outdir=outdir_path,
    )


if __name__ == "__main__":
    main()
