#!/usr/bin/env python3
"""
abc_smc_framework.py — Publication-grade adaptive ABC-SMC calibration engine for CellSim.

Features:
  - Adaptive tolerance thresholding: epsilon_{t} = Q_alpha(distances_{t-1})
  - Multivariate Gaussian perturbation kernel with optimal Beaumont/Toni weighted empirical covariance
  - Sequential Importance Sampling (SIS) weight updates with log-sum-exp numerical safeguards
  - Prior evaluation and boundary constraint checking (e.g. high_delta > low_delta)
  - ThreadPoolExecutor parallel orchestration controlling OMP_NUM_THREADS to avoid thread thrashing
  - Multi-replicate evaluation per particle to tame Monte Carlo stochasticity
  - Automated structured reporting: generation CSVs, diagnostics, parameter estimation summary JSON, run manifest, and markdown reports.
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
from dataclasses import dataclass, field
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Callable, Dict, List, Optional, Tuple

import numpy as np

# Prevent OpenMP from fighting Python thread pools
os.environ["OMP_NUM_THREADS"] = "1"

# Regex extractors for run_bootstrapping stdout
_DIST_RE = re.compile(r"weighted SSE\s*=\s*([\d.eE+\-]+)")
_UNWEIGHTED_DIST_RE = re.compile(r"distance\s*\(SSE\)\s*=\s*([\d.eE+\-]+)")
_MEDIAN_ONS_RE = re.compile(r"median onset\s*=\s*([\d.eE+\-]+)")
_MEDIAN_S25_RE = re.compile(r"median sat25\s*=\s*([\d.eE+\-]+)")
_MEDIAN_S50_RE = re.compile(r"median sat50\s*=\s*([\d.eE+\-]+)")
_MEDIAN_S90_RE = re.compile(r"median sat90\s*=\s*([\d.eE+\-]+)")


def _to_float(match: Optional[re.Match[str]]) -> float:
    return float(match.group(1)) if match else float("nan")


@dataclass
class ABCDiagnostics:
    generation: int
    epsilon: float
    particles_accepted: int
    proposals_attempted: int
    acceptance_rate: float
    ess: float
    min_distance: float
    median_distance: float
    elapsed_seconds: float


@dataclass
class ABCConfig:
    priors: Dict[str, Tuple[float, float]]
    n_particles: int = 50
    n_generations: int = 4
    n_sim: int = 200
    n_reps: int = 2
    alpha_quantile: float = 0.75
    enable_big_bang: bool = True
    binary_path: str = "./build/run_bootstrapping"
    workers: int = 4
    seed: int = 42
    max_proposals_per_gen: int = 25000
    fixed_params: Dict[str, float] = field(default_factory=dict)
    custom_constraints: Optional[Callable[[Dict[str, float]], bool]] = None


class ABCSMCEngine:
    def __init__(self, config: ABCConfig, output_dir: Path):
        self.config = config
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)
        self.param_names = list(config.priors.keys())
        self.prior_lo = np.array([config.priors[p][0] for p in self.param_names], dtype=float)
        self.prior_hi = np.array([config.priors[p][1] for p in self.param_names], dtype=float)
        self.rng = np.random.default_rng(config.seed)
        self.diagnostics_log: List[ABCDiagnostics] = []

    def validate_params(self, param_dict: Dict[str, float]) -> bool:
        for name, (lo, hi) in self.config.priors.items():
            val = param_dict[name]
            if val < lo or val > hi:
                return False

        # Domain biological constraints
        if "low_delta" in param_dict and "high_delta" in param_dict:
            if param_dict["high_delta"] <= param_dict["low_delta"]:
                return False

        if self.config.custom_constraints and not self.config.custom_constraints(param_dict):
            return False

        return True

    def array_to_dict(self, theta_arr: np.ndarray) -> Dict[str, float]:
        d = {name: float(val) for name, val in zip(self.param_names, theta_arr)}
        d.update(self.config.fixed_params)
        return d

    def sample_prior_particle(self) -> np.ndarray:
        while True:
            candidate = self.rng.uniform(self.prior_lo, self.prior_hi)
            param_dict = self.array_to_dict(candidate)
            if self.validate_params(param_dict):
                return candidate

    def log_prior(self, theta_arr: np.ndarray) -> float:
        param_dict = self.array_to_dict(theta_arr)
        if not self.validate_params(param_dict):
            return -np.inf
        # Uniform prior density
        return -float(np.sum(np.log(self.prior_hi - self.prior_lo)))

    def run_simulation(
        self, theta_arr: np.ndarray, seed_base: int
    ) -> Tuple[Optional[float], Optional[float], Optional[float], Optional[float], Optional[float]]:
        params = self.array_to_dict(theta_arr)
        distances: List[float] = []
        onset_vals: List[float] = []
        sat25_vals: List[float] = []
        sat50_vals: List[float] = []
        sat90_vals: List[float] = []

        for rep in range(self.config.n_reps):
            cmd = [
                self.config.binary_path,
                "--n-runs", str(self.config.n_sim),
                "--seed-offset", str(seed_base + rep * 100_003),
                "--output", "/dev/null",
                "--weighted-sse",
            ]

            if self.config.enable_big_bang:
                cmd.append("--big-bang")

            # Map parameter values to CLI args
            cli_arg_map = {
                "brca1_rate": "--brca1-rate",
                "tp53_rate": "--tp53-rate",
                "low_delta": "--low-delta",
                "high_delta": "--high-delta",
                "d1_threshold": "--d1-threshold",
                "d2_threshold": "--d2-threshold",
                "neoplastic_div_rate": "--neoplastic-div-rate",
                "division_rate": "--division-rate",
                "kappa_brca1": "--kappa-brca1",
                "age_drift": "--age-drift",
            }

            for p_key, arg_flag in cli_arg_map.items():
                if p_key in params:
                    cmd.extend([arg_flag, f"{params[p_key]:.6f}"])

            try:
                res = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
            except (subprocess.TimeoutExpired, OSError):
                return None, None, None, None, None

            if res.returncode != 0:
                return None, None, None, None, None

            m_dist = _DIST_RE.search(res.stdout) or _UNWEIGHTED_DIST_RE.search(res.stdout)
            if not m_dist:
                return None, None, None, None, None

            distances.append(float(m_dist.group(1)))
            onset_vals.append(_to_float(_MEDIAN_ONS_RE.search(res.stdout)))
            sat25_vals.append(_to_float(_MEDIAN_S25_RE.search(res.stdout)))
            sat50_vals.append(_to_float(_MEDIAN_S50_RE.search(res.stdout)))
            sat90_vals.append(_to_float(_MEDIAN_S90_RE.search(res.stdout)))

        if not distances:
            return None, None, None, None, None

        return (
            float(np.mean(distances)),
            float(np.nanmedian(onset_vals)),
            float(np.nanmedian(sat25_vals)),
            float(np.nanmedian(sat50_vals)),
            float(np.nanmedian(sat90_vals)),
        )

    def compute_ess(self, weights: np.ndarray) -> float:
        if len(weights) == 0:
            return 0.0
        sum_sq = float(np.sum(weights ** 2))
        return 1.0 / sum_sq if sum_sq > 0 else 0.0

    def calculate_weights(
        self,
        current_particles: np.ndarray,
        prev_particles: np.ndarray,
        prev_weights: np.ndarray,
        cov_matrix: np.ndarray,
    ) -> np.ndarray:
        n = len(current_particles)
        log_prior_vals = np.array([self.log_prior(p) for p in current_particles])

        # Guard against zero or singular covariance
        inv_cov = np.linalg.pinv(cov_matrix)
        det_cov = np.linalg.det(cov_matrix)
        if det_cov <= 0 or not np.isfinite(det_cov):
            det_cov = 1e-12
        dim = len(self.param_names)
        norm_const = np.sqrt(((2 * np.pi) ** dim) * det_cov)

        log_weights = np.zeros(n)
        for i in range(n):
            theta_i = current_particles[i]
            # Kernel denominator: sum_j prev_w_j * K(theta_i | prev_theta_j)
            diffs = theta_i - prev_particles  # (M, dim)
            # Mahalanobis terms: diff @ inv_cov @ diff.T
            quad = np.einsum("ki,ij,kj->k", diffs, inv_cov, diffs)
            log_densities = -0.5 * quad - np.log(norm_const)
            max_log = np.max(log_densities)
            denom = np.sum(prev_weights * np.exp(log_densities - max_log))
            log_denom = max_log + np.log(max(denom, 1e-300))
            log_weights[i] = log_prior_vals[i] - log_denom

        # Normalize with log-sum-exp
        max_lw = np.max(log_weights)
        unnorm_w = np.exp(log_weights - max_lw)
        return unnorm_w / np.sum(unnorm_w)

    def run(self) -> Dict[str, Any]:
        print(f"\n=================================================================")
        print(f"Starting ABC-SMC Calibration: {len(self.param_names)} parameters, {self.config.n_generations} gens")
        print(f"Priors: {self.config.priors}")
        print(f"Fixed:  {self.config.fixed_params}")
        print(f"Target: {self.config.n_particles} particles, {self.config.n_sim} sims/rep, {self.config.n_reps} reps")
        print(f"Big Bang: {self.config.enable_big_bang}, Workers: {self.config.workers}")
        print(f"=================================================================\n")

        start_time = time.time()
        prev_particles: Optional[np.ndarray] = None
        prev_weights: Optional[np.ndarray] = None
        cov_matrix: Optional[np.ndarray] = None
        epsilon = float("inf")

        for gen in range(self.config.n_generations):
            gen_start = time.time()
            accepted_particles: List[np.ndarray] = []
            accepted_distances: List[float] = []
            accepted_metrics: List[Tuple[float, float, float, float]] = []
            proposals_count = 0

            print(f"[Generation {gen}] Target epsilon: {epsilon:.4f}")

            with ThreadPoolExecutor(max_workers=self.config.workers) as executor:
                pending_futures = {}

                def submit_task(task_seed: int):
                    if gen == 0 or prev_particles is None or prev_weights is None or cov_matrix is None:
                        cand = self.sample_prior_particle()
                    else:
                        # Perturb from previous population
                        idx = self.rng.choice(len(prev_particles), p=prev_weights)
                        cand = self.rng.multivariate_normal(prev_particles[idx], cov_matrix)
                        # Keep perturbing until prior constraints are satisfied
                        tries = 0
                        while not self.validate_params(self.array_to_dict(cand)) and tries < 100:
                            cand = self.rng.multivariate_normal(prev_particles[idx], cov_matrix)
                            tries += 1
                        if tries >= 100:
                            cand = self.sample_prior_particle()

                    fut = executor.submit(self.run_simulation, cand, task_seed)
                    pending_futures[fut] = cand

                # Prime pool
                initial_batch = min(self.config.workers * 2, self.config.n_particles * 3)
                for _ in range(initial_batch):
                    proposals_count += 1
                    submit_task(self.rng.integers(1, 10_000_000))

                while len(accepted_particles) < self.config.n_particles and pending_futures:
                    done, _ = wait(pending_futures.keys(), return_when=FIRST_COMPLETED)
                    for fut in done:
                        cand_theta = pending_futures.pop(fut)
                        dist, ons, s25, s50, s90 = fut.result()

                        if dist is not None and dist <= epsilon:
                            accepted_particles.append(cand_theta)
                            accepted_distances.append(dist)
                            accepted_metrics.append((ons if ons else float('nan'),
                                                     s25 if s25 else float('nan'),
                                                     s50 if s50 else float('nan'),
                                                     s90 if s90 else float('nan')))
                            print(f"  -> Accepted {len(accepted_particles)}/{self.config.n_particles} "
                                  f"(dist={dist:.4f}, props={proposals_count})")

                        # Spawn more if needed
                        if (
                            len(accepted_particles) + len(pending_futures) < self.config.n_particles
                            and proposals_count < self.config.max_proposals_per_gen
                        ):
                            proposals_count += 1
                            submit_task(self.rng.integers(1, 10_000_000))

                    if (
                        proposals_count >= self.config.max_proposals_per_gen
                        and not pending_futures
                        and len(accepted_particles) < self.config.n_particles
                    ):
                        print(
                            f"\n[WARNING] Generation {gen} reached max proposals ({self.config.max_proposals_per_gen}). "
                            f"Accepted {len(accepted_particles)}/{self.config.n_particles}. Stopping SMC early."
                        )
                        break

            if len(accepted_particles) < 2:
                print(f"[ERROR] Insufficient particles accepted in generation {gen}. Finalizing with current state.")
                break

            particles_arr = np.array(accepted_particles)
            distances_arr = np.array(accepted_distances)

            # Compute weights
            if gen == 0:
                weights = np.ones(len(particles_arr)) / len(particles_arr)
            else:
                assert prev_particles is not None and prev_weights is not None and cov_matrix is not None
                weights = self.calculate_weights(particles_arr, prev_particles, prev_weights, cov_matrix)

            # Empirical 2 * weighted covariance for Beaumont perturbation kernel
            weighted_mean = np.sum(particles_arr * weights[:, None], axis=0)
            diffs = particles_arr - weighted_mean
            cov = 2.0 * np.sum(weights[:, None, None] * (diffs[:, :, None] @ diffs[:, None, :]), axis=0)
            # Add small regularizer to maintain positive definiteness
            cov += np.eye(len(self.param_names)) * 1e-8
            cov_matrix = cov

            # Adaptive epsilon for next generation: Q_alpha of accepted distances
            epsilon = float(np.quantile(distances_arr, self.config.alpha_quantile))

            ess = self.compute_ess(weights)
            elapsed_gen = time.time() - gen_start
            acceptance_rate = len(particles_arr) / max(proposals_count, 1)

            diag = ABCDiagnostics(
                generation=gen,
                epsilon=epsilon,
                particles_accepted=len(particles_arr),
                proposals_attempted=proposals_count,
                acceptance_rate=acceptance_rate,
                ess=ess,
                min_distance=float(np.min(distances_arr)),
                median_distance=float(np.median(distances_arr)),
                elapsed_seconds=elapsed_gen,
            )
            self.diagnostics_log.append(diag)

            # Save generation results
            self._save_generation_csv(gen, particles_arr, weights, distances_arr, accepted_metrics)

            prev_particles = particles_arr
            prev_weights = weights

        total_elapsed = time.time() - start_time
        summary = self._finalize_run(particles_arr, weights, distances_arr, total_elapsed)
        return summary

    def _save_generation_csv(
        self,
        gen: int,
        particles: np.ndarray,
        weights: np.ndarray,
        distances: np.ndarray,
        metrics: List[Tuple[float, float, float, float]],
    ):
        csv_path = self.output_dir / f"generation_{gen:02d}.csv"
        fieldnames = ["particle_id", "weight", "distance"] + self.param_names + [
            "median_onset", "sat25", "sat50", "sat90"
        ]
        with open(csv_path, "w", newline="") as f:
            writer = csv.DictWriter(f, fieldnames=fieldnames)
            writer.writeheader()
            for i in range(len(particles)):
                row = {
                    "particle_id": i,
                    "weight": f"{weights[i]:.8e}",
                    "distance": f"{distances[i]:.6f}",
                }
                for p_idx, p_name in enumerate(self.param_names):
                    row[p_name] = f"{particles[i, p_idx]:.6f}"
                ons, s25, s50, s90 = metrics[i]
                row["median_onset"] = f"{ons:.2f}"
                row["sat25"] = f"{s25:.2f}"
                row["sat50"] = f"{s50:.2f}"
                row["sat90"] = f"{s90:.2f}"
                writer.writerow(row)

    def _finalize_run(
        self, final_particles: np.ndarray, final_weights: np.ndarray, final_distances: np.ndarray, total_time: float
    ) -> Dict[str, Any]:
        # Save diagnostics.csv
        diag_path = self.output_dir / "diagnostics.csv"
        with open(diag_path, "w", newline="") as f:
            writer = csv.DictWriter(
                f,
                fieldnames=[
                    "generation", "epsilon", "particles_accepted", "proposals_attempted",
                    "acceptance_rate", "ess", "min_distance", "median_distance", "elapsed_seconds"
                ]
            )
            writer.writeheader()
            for d in self.diagnostics_log:
                writer.writerow({
                    "generation": d.generation,
                    "epsilon": f"{d.epsilon:.6f}",
                    "particles_accepted": d.particles_accepted,
                    "proposals_attempted": d.proposals_attempted,
                    "acceptance_rate": f"{d.acceptance_rate:.4f}",
                    "ess": f"{d.ess:.2f}",
                    "min_distance": f"{d.min_distance:.6f}",
                    "median_distance": f"{d.median_distance:.6f}",
                    "elapsed_seconds": f"{d.elapsed_seconds:.2f}",
                })

        # Calculate parameter statistics (weighted percentiles)
        param_summary: Dict[str, Any] = {}
        for p_idx, p_name in enumerate(self.param_names):
            vals = final_particles[:, p_idx]
            # Weighted quantiles
            sort_idx = np.argsort(vals)
            sorted_vals = vals[sort_idx]
            sorted_weights = final_weights[sort_idx]
            cum_w = np.cumsum(sorted_weights)
            cum_w /= cum_w[-1]

            q025 = float(np.interp(0.025, cum_w, sorted_vals))
            q50 = float(np.interp(0.50, cum_w, sorted_vals))
            q975 = float(np.interp(0.975, cum_w, sorted_vals))
            mean_val = float(np.sum(vals * final_weights))

            param_summary[p_name] = {
                "mean": mean_val,
                "median": q50,
                "ci_95_low": q025,
                "ci_95_high": q975,
            }

        # Parameter summary JSON
        with open(self.output_dir / "parameter_estimation_summary.json", "w") as f:
            json.dump(param_summary, f, indent=2)

        # Run manifest JSON
        git_commit = "unknown"
        try:
            r = subprocess.run(["git", "rev-parse", "HEAD"], capture_output=True, text=True, check=True)
            git_commit = r.stdout.strip()
        except Exception:
            pass

        manifest = {
            "timestamp_utc": datetime.now(timezone.utc).isoformat(),
            "git_commit": git_commit,
            "platform": platform.platform(),
            "config": {
                "n_particles": self.config.n_particles,
                "n_generations": self.config.n_generations,
                "n_sim": self.config.n_sim,
                "n_reps": self.config.n_reps,
                "alpha_quantile": self.config.alpha_quantile,
                "enable_big_bang": self.config.enable_big_bang,
                "seed": self.config.seed,
                "priors": self.config.priors,
                "fixed_params": self.config.fixed_params,
            },
            "total_elapsed_seconds": total_time,
            "best_particle_distance": float(np.min(final_distances)),
        }
        with open(self.output_dir / "run_manifest.json", "w") as f:
            json.dump(manifest, f, indent=2)

        # Markdown report
        self._generate_markdown_report(param_summary, total_time, float(np.min(final_distances)))

        return {
            "parameter_summary": param_summary,
            "manifest": manifest,
            "diagnostics": [d.__dict__ for d in self.diagnostics_log],
        }

    def _generate_markdown_report(self, param_summary: Dict[str, Any], total_time: float, best_distance: float):
        md_lines = [
            "# ABC-SMC Parameter Estimation Summary",
            "",
            f"- **Execution Time:** {total_time:.2f} seconds",
            f"- **Generations Completed:** {len(self.diagnostics_log)}",
            f"- **Particles per Generation:** {self.config.n_particles}",
            f"- **Best Distance ($SSE_w$):** {best_distance:.6f}",
            f"- **Big Bang Mode:** `{'Enabled' if self.config.enable_big_bang else 'Disabled'}`",
            "",
            "## Parameter Estimates (Posterior Distribution)",
            "",
            "| Parameter | Mean | Median (MAP) | 95% Credible Interval |",
            "|---|---|---|---|",
        ]
        for p_name, stats in param_summary.items():
            md_lines.append(
                f"| `{p_name}` | {stats['mean']:.5f} | **{stats['median']:.5f}** | "
                f"[{stats['ci_95_low']:.5f}, {stats['ci_95_high']:.5f}] |"
            )

        md_lines.extend([
            "",
            "## Generation Convergence Diagnostics",
            "",
            "| Gen | Target $\\epsilon$ | Accepted | Attempted | Acc. Rate | ESS | Min Dist | Med Dist | Time (s) |",
            "|---|---|---|---|---|---|---|---|---|",
        ])
        for d in self.diagnostics_log:
            md_lines.append(
                f"| {d.generation} | {d.epsilon:.4f} | {d.particles_accepted} | {d.proposals_attempted} | "
                f"{d.acceptance_rate * 100:.1f}% | {d.ess:.1f} | {d.min_distance:.4f} | {d.median_distance:.4f} | {d.elapsed_seconds:.1f}s |"
            )

        report_path = self.output_dir / "summary_report.md"
        with open(report_path, "w") as f:
            f.write("\n".join(md_lines) + "\n")
