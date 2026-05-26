#!/usr/bin/env python3
"""
abc_final_v4.py — ABC-SMC v4: 4 free params + saturation milestone tracking.

Changes vs v3:
  - Parses median_sat25 / median_sat50 / median_sat90 from binary stdout.
  - Stores them in gen_XX.csv for posterior characterisation.
  - Acceptance criterion unchanged: SSE_w only (Kuchenbaecker fit).
  - New plots: progression milestone distributions across gens + onset→sat speed.

Fixed parameters (biologically/empirically justified):
  d1_threshold = 2.0   (biological default)
  d2_threshold = 5.0   (biological default)
  tp53_rate    = 0.001 (collapsed in abc_explore + abc_calibrate)

Free parameters (priors — high_delta extended to [0.05, 1.20]):
  brca1_rate         ~ U[0.008, 0.017]
  low_delta          ~ U[0.15,  0.45]
  high_delta         ~ U[0.05,  1.20]   (high_delta > low_delta enforced)
  neoplastic_div_rate ~ U[0.10,  0.24]

Usage:
    python3 scripts/abc_final_v4.py [OPTIONS]

Options:
    --n-particles INT    Particles per generation (default: 200)
    --n-generations INT  Max SMC generations (default: 8)
    --epsilon-0 FLOAT    Initial tolerance (default: 15.0)
    --epsilon-final FLOAT  Stop threshold (default: 5.0)
    --alpha FLOAT        Quantile for adaptive ε (default: 0.4)
    --n-sim INT          CellSim runs per evaluation (default: 50)
    --max-attempts INT   Max attempts per particle (default: 30000)
    --output-dir PATH    Results directory (default: results/abc_final_v4/)
    --seed INT           RNG seed (default: 42)
    --workers INT        Parallel workers (default: 4)
    --resume             Resume from last saved generation
    --no-plots           Skip plots
    -v, --verbose        Print every evaluation
"""

import argparse
import csv
import os
import re
import subprocess
import sys
import time
from concurrent.futures import ThreadPoolExecutor, wait, FIRST_COMPLETED
from pathlib import Path

import numpy as np

REPO_ROOT = Path(__file__).resolve().parent.parent
os.chdir(REPO_ROOT)

BINARY = str(REPO_ROOT / "build" / "run_bootstrapping")

KUCH_AGES = [30, 40, 50, 60, 70, 80]
KUCH_MEAN = [4.0, 26.0, 46.0, 58.0, 65.0, 70.0]

# Fixed parameters (biological defaults + empirically collapsed)
D1_THRESHOLD = 2.0
D2_THRESHOLD = 5.0
TP53_RATE    = 0.001  # collapsed to this value across abc_explore + abc_calibrate

# ---------------------------------------------------------------------------
# Priors (informed by abc_final gen_02 posterior)
# ---------------------------------------------------------------------------
PRIORS = {
    "brca1_rate":          (0.008,  0.017),
    "low_delta":           (0.15,   0.45),
    "high_delta":          (0.05,   1.20),   # EXTENDED — was [0.08, 0.65], bimodal hit boundary
    "neoplastic_div_rate": (0.10,   0.24),
}
PARAM_NAMES = list(PRIORS.keys())
PRIOR_LO    = np.array([PRIORS[p][0] for p in PARAM_NAMES])
PRIOR_HI    = np.array([PRIORS[p][1] for p in PARAM_NAMES])

_SSE_W_RE      = re.compile(r"weighted SSE\s*=\s*([\d.]+)")
_MEDIAN_ONS_RE = re.compile(r"median onset\s*=\s*([\d.eE+\-]+)")
_MEDIAN_S25_RE = re.compile(r"median sat25\s*=\s*([\d.eE+\-]+)")
_MEDIAN_S50_RE = re.compile(r"median sat50\s*=\s*([\d.eE+\-]+)")
_MEDIAN_S90_RE = re.compile(r"median sat90\s*=\s*([\d.eE+\-]+)")


# ---------------------------------------------------------------------------
# Simulator
# ---------------------------------------------------------------------------

def run_simulation(theta_arr: np.ndarray, n_sim: int) -> tuple[float, float, float, float, float] | tuple[None, None, None, None, None]:
    brca1_rate, low_delta, high_delta, neo_div = theta_arr
    cmd = [
        BINARY,
        "--n-runs",              str(n_sim),
        "--brca1-rate",          f"{brca1_rate:.6f}",
        "--low-delta",           f"{low_delta:.6f}",
        "--high-delta",          f"{high_delta:.6f}",
        "--tp53-rate",           f"{TP53_RATE:.6f}",
        "--d1-threshold",        f"{D1_THRESHOLD:.1f}",
        "--d2-threshold",        f"{D2_THRESHOLD:.1f}",
        "--neoplastic-div-rate", f"{neo_div:.6f}",
        "--big-bang",
        "--weighted-sse",
        "--output", "/dev/null",
    ]
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=120)
        m_sse = _SSE_W_RE.search(r.stdout)
        if m_sse:
            sse_w     = float(m_sse.group(1))
            onset_age = float(_MEDIAN_ONS_RE.search(r.stdout).group(1)) if _MEDIAN_ONS_RE.search(r.stdout) else float("nan")
            sat25     = float(_MEDIAN_S25_RE.search(r.stdout).group(1)) if _MEDIAN_S25_RE.search(r.stdout) else float("nan")
            sat50     = float(_MEDIAN_S50_RE.search(r.stdout).group(1)) if _MEDIAN_S50_RE.search(r.stdout) else float("nan")
            sat90     = float(_MEDIAN_S90_RE.search(r.stdout).group(1)) if _MEDIAN_S90_RE.search(r.stdout) else float("nan")
            return sse_w, onset_age, sat25, sat50, sat90
    except (subprocess.TimeoutExpired, OSError):
        pass
    return None, None, None, None, None


def _worker(args: tuple) -> tuple[np.ndarray, "float | None", "float | None", "float | None", "float | None", "float | None"]:
    theta_arr, n_sim = args
    sse_w, onset_age, sat25, sat50, sat90 = run_simulation(theta_arr, n_sim)
    return theta_arr, sse_w, onset_age, sat25, sat50, sat90


# ---------------------------------------------------------------------------
# Prior helpers
# ---------------------------------------------------------------------------

LOW_IDX  = PARAM_NAMES.index("low_delta")
HIGH_IDX = PARAM_NAMES.index("high_delta")


def _valid(theta: np.ndarray) -> bool:
    """Biological constraint: high_delta must exceed low_delta."""
    return bool(theta[HIGH_IDX] > theta[LOW_IDX])


def sample_prior(rng: np.random.Generator, n: int = 1) -> np.ndarray:
    samples = []
    while len(samples) < n:
        candidate = rng.uniform(PRIOR_LO, PRIOR_HI, size=len(PARAM_NAMES))
        if _valid(candidate):
            samples.append(candidate)
    return np.array(samples)


def log_prior(theta: np.ndarray) -> float:
    if np.any(theta < PRIOR_LO) or np.any(theta > PRIOR_HI):
        return -np.inf
    if not _valid(theta):
        return -np.inf
    return -np.sum(np.log(PRIOR_HI - PRIOR_LO))


# ---------------------------------------------------------------------------
# SMC weight update
# ---------------------------------------------------------------------------

def compute_weights(
    particles: np.ndarray,
    prev_particles: np.ndarray,
    prev_weights: np.ndarray,
    sigmas: np.ndarray,
) -> np.ndarray:
    N, D = particles.shape
    log_prior_vals = np.array([log_prior(particles[i]) for i in range(N)])

    log_kernel = np.zeros((N, N))
    for d in range(D):
        diff = particles[:, d:d+1] - prev_particles[:, d]
        log_kernel -= 0.5 * (diff / sigmas[d]) ** 2 + np.log(sigmas[d] * np.sqrt(2 * np.pi))

    log_denom = np.log(prev_weights) + log_kernel
    log_denom_sum = np.array([np.logaddexp.reduce(log_denom[i]) for i in range(N)])

    log_w = log_prior_vals - log_denom_sum
    log_w -= np.logaddexp.reduce(log_w)
    return np.exp(log_w)


# ---------------------------------------------------------------------------
# Generation runner
# ---------------------------------------------------------------------------

def run_generation(
    gen: int,
    n_particles: int,
    epsilon: float,
    n_sim: int,
    max_attempts: int,
    n_workers: int,
    rng: np.random.Generator,
    prev_particles: np.ndarray | None,
    prev_weights: np.ndarray | None,
    sigmas: np.ndarray | None,
    verbose: bool,
) -> tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray]:
    accepted_theta:     list[np.ndarray] = []
    accepted_sse:       list[float]      = []
    accepted_onset_age: list[float]      = []
    accepted_sat25:     list[float]      = []
    accepted_sat50:     list[float]      = []
    accepted_sat90:     list[float]      = []
    n_attempts = 0
    t_start    = time.time()

    def _propose() -> np.ndarray:
        if gen == 0:
            return sample_prior(rng, 1)[0]
        idx   = rng.choice(len(prev_particles), p=prev_weights)
        theta = prev_particles[idx] + rng.normal(0, sigmas)
        return np.clip(theta, PRIOR_LO, PRIOR_HI)

    with ThreadPoolExecutor(max_workers=n_workers) as executor:
        pending: dict = {}

        def _submit_one():
            nonlocal n_attempts
            if n_attempts >= max_attempts:
                return
            theta = _propose()
            fut   = executor.submit(_worker, (theta, n_sim))
            pending[fut] = theta
            n_attempts += 1

        for _ in range(n_workers):
            _submit_one()

        while pending and len(accepted_theta) < n_particles:
            done, _ = wait(pending.keys(), return_when=FIRST_COMPLETED)
            for fut in done:
                theta, sse_w, onset_age, sat25, sat50, sat90 = fut.result()
                pending.pop(fut)

                if sse_w is not None and sse_w <= epsilon:
                    accepted_theta.append(theta)
                    accepted_sse.append(sse_w)
                    accepted_onset_age.append(onset_age if onset_age is not None else float("nan"))
                    accepted_sat25.append(sat25 if sat25 is not None else float("nan"))
                    accepted_sat50.append(sat50 if sat50 is not None else float("nan"))
                    accepted_sat90.append(sat90 if sat90 is not None else float("nan"))

                if verbose or (n_attempts % 50 == 0):
                    rate    = len(accepted_theta) / max(n_attempts, 1) * 100
                    elapsed = time.time() - t_start
                    print(
                        f"  gen={gen} | attempts={n_attempts:5d} | "
                        f"accepted={len(accepted_theta):4d}/{n_particles} ({rate:.1f}%) | "
                        f"ε={epsilon:.2f} | {elapsed:.0f}s",
                        end="\r", flush=True,
                    )

                if len(accepted_theta) < n_particles:
                    _submit_one()

        for fut in list(pending):
            fut.cancel()

    print()
    particles  = np.array(accepted_theta)
    sse_vals   = np.array(accepted_sse)
    onset_ages = np.array(accepted_onset_age)
    sat25_arr  = np.array(accepted_sat25)
    sat50_arr  = np.array(accepted_sat50)
    sat90_arr  = np.array(accepted_sat90)

    if gen == 0:
        weights = np.ones(len(particles)) / len(particles)
    else:
        weights = compute_weights(particles, prev_particles, prev_weights, sigmas)

    return particles, weights, sse_vals, onset_ages, sat25_arr, sat50_arr, sat90_arr


# ---------------------------------------------------------------------------
# Persistence
# ---------------------------------------------------------------------------

def save_generation(out_dir: Path, gen: int, particles: np.ndarray, weights: np.ndarray, sse_vals: np.ndarray, onset_ages: np.ndarray, sat25: np.ndarray, sat50: np.ndarray, sat90: np.ndarray):
    path = out_dir / f"gen_{gen:02d}.csv"
    with open(path, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(PARAM_NAMES + ["weight", "sse_weighted", "onset_age", "sat25", "sat50", "sat90"])
        for theta, wt, sse, onset, s25, s50, s90 in zip(particles, weights, sse_vals, onset_ages, sat25, sat50, sat90):
            w.writerow([f"{v:.6f}" for v in theta] + [f"{wt:.8f}", f"{sse:.4f}", f"{onset:.2f}", f"{s25:.2f}", f"{s50:.2f}", f"{s90:.2f}"])
    return path


def load_generation(out_dir: Path, gen: int) -> "tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray] | None":
    path = out_dir / f"gen_{gen:02d}.csv"
    if not path.exists():
        return None
    with open(path) as f:
        rows = list(csv.DictReader(f))
    if not rows:
        return None
    particles  = np.array([[float(r[p]) for p in PARAM_NAMES] for r in rows])
    weights    = np.array([float(r["weight"])      for r in rows])
    sse_vals   = np.array([float(r["sse_weighted"]) for r in rows])
    onset_ages = np.array([float(r.get("onset_age", "nan")) for r in rows])
    sat25      = np.array([float(r.get("sat25", "nan")) for r in rows])
    sat50      = np.array([float(r.get("sat50", "nan")) for r in rows])
    sat90      = np.array([float(r.get("sat90", "nan")) for r in rows])
    weights   /= weights.sum()
    return particles, weights, sse_vals, onset_ages, sat25, sat50, sat90


def find_last_saved_gen(out_dir: Path) -> int:
    gen = -1
    while (out_dir / f"gen_{gen+1:02d}.csv").exists():
        gen += 1
    return gen


def _save_eps_history(out_dir: Path, eps_history: list) -> None:
    with open(out_dir / "epsilon_schedule.csv", "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["generation", "epsilon"])
        for i, e in enumerate(eps_history):
            w.writerow([i, f"{e:.6f}"])


# ---------------------------------------------------------------------------
# Main ABC-SMC loop
# ---------------------------------------------------------------------------

def run_abc_final(
    n_particles: int,
    n_generations: int,
    epsilon_0: float,
    epsilon_final: float,
    alpha: float,
    n_sim: int,
    max_attempts: int,
    out_dir: Path,
    seed: int,
    n_workers: int,
    verbose: bool,
    resume: bool = False,
):
    rng = np.random.default_rng(seed)
    out_dir.mkdir(parents=True, exist_ok=True)

    print(f"ABC-SMC Final v4 — 4 free parameters + saturation milestones  (workers={n_workers})")
    print(f"  Free params   : {', '.join(PARAM_NAMES)}")
    print(f"  Fixed         : d1_threshold={D1_THRESHOLD}  d2_threshold={D2_THRESHOLD}  tp53_rate={TP53_RATE}")
    print(f"  Particles/gen : {n_particles}")
    print(f"  Generations   : {n_generations}")
    print(f"  ε schedule    : {epsilon_0:.1f} → adaptive (α={alpha}) → stop at {epsilon_final:.2f}")
    print(f"  N_sim/eval    : {n_sim}")
    print()

    particles = prev_particles = None
    weights   = prev_weights   = None
    sse_vals  = None
    sigmas    = None
    epsilon   = epsilon_0
    eps_history: list[float] = []
    start_gen   = 0

    if resume:
        last_gen = find_last_saved_gen(out_dir)
        if last_gen < 0:
            print("[resume] No saved generations found — starting from scratch.")
        else:
            loaded = load_generation(out_dir, last_gen)
            if loaded is None:
                print(f"[resume] Failed to load gen_{last_gen:02d}.csv — starting from scratch.")
            else:
                particles, weights, sse_vals, _, _, _, _ = loaded
                n_particles = len(particles)
                sch = out_dir / "epsilon_schedule.csv"
                if sch.exists():
                    with open(sch) as f:
                        for row in csv.DictReader(f):
                            eps_history.append(float(row["epsilon"]))
                epsilon = float(np.quantile(sse_vals, alpha))
                sigmas = 2.0 * np.sqrt(np.average(
                    (particles - np.average(particles, weights=weights, axis=0)) ** 2,
                    weights=weights, axis=0,
                ))
                sigmas = np.maximum(sigmas, 1e-6)
                prev_particles = particles.copy()
                prev_weights   = weights.copy()
                start_gen      = last_gen + 1
                print(f"[resume] Loaded gen_{last_gen:02d}  ({n_particles} particles) "
                      f"— continuing from gen {start_gen}  ε={epsilon:.4f}")
                print()

    for gen in range(start_gen, n_generations):
        print(f"{'='*65}")
        print(f"  Generation {gen}   ε = {epsilon:.4f}")
        print(f"{'='*65}")
        t0 = time.time()

        particles, weights, sse_vals, onset_ages, sat25_arr, sat50_arr, sat90_arr = run_generation(
            gen=gen,
            n_particles=n_particles,
            epsilon=epsilon,
            n_sim=n_sim,
            max_attempts=max_attempts,
            n_workers=n_workers,
            rng=rng,
            prev_particles=prev_particles,
            prev_weights=prev_weights,
            sigmas=sigmas,
            verbose=verbose,
        )

        actual_n = len(particles)
        if actual_n == 0:
            print(f"  [warn] No particles accepted at ε={epsilon:.2f}. Stopping.")
            break

        if actual_n < n_particles:
            print(f"  [warn] Only {actual_n}/{n_particles} particles — padding by resampling.")
            idx        = rng.choice(actual_n, size=n_particles - actual_n, replace=True)
            particles  = np.vstack([particles, particles[idx]])
            weights    = np.concatenate([weights, weights[idx]])
            sse_vals   = np.concatenate([sse_vals, sse_vals[idx]])
            onset_ages = np.concatenate([onset_ages, onset_ages[idx]])
            sat25_arr  = np.concatenate([sat25_arr, sat25_arr[idx]])
            sat50_arr  = np.concatenate([sat50_arr, sat50_arr[idx]])
            sat90_arr  = np.concatenate([sat90_arr, sat90_arr[idx]])
            weights   /= weights.sum()

        path    = save_generation(out_dir, gen, particles, weights, sse_vals, onset_ages, sat25_arr, sat50_arr, sat90_arr)
        elapsed = time.time() - t0
        ess     = 1.0 / np.sum(weights ** 2)
        valid_onset = onset_ages[~np.isnan(onset_ages)]
        onset_str   = f"onset mean={valid_onset.mean():.1f}y" if len(valid_onset) > 0 else "onset=n/a"
        valid_sat50 = sat50_arr[~np.isnan(sat50_arr) & (sat50_arr > 0)]
        sat50_str   = f"sat50 mean={valid_sat50.mean():.1f}y" if len(valid_sat50) > 0 else "sat50=n/a"
        print(f"  Accepted: {actual_n}  |  ESS: {ess:.1f}/{n_particles}  |  "
              f"SSE_w mean={sse_vals.mean():.2f} min={sse_vals.min():.2f}  |  "
              f"{onset_str}  |  {sat50_str}  |  {elapsed:.1f}s")
        print(f"  Saved → {path}")

        print(f"  Posterior (weighted mean ± std):")
        wmean = np.average(particles, weights=weights, axis=0)
        wstd  = np.sqrt(np.average((particles - wmean)**2, weights=weights, axis=0))
        for name, lo, hi, mu, sd in zip(PARAM_NAMES, PRIOR_LO, PRIOR_HI, wmean, wstd):
            print(f"    {name:<22s}: {mu:.5f} ± {sd:.5f}  [prior: {lo:.4f}–{hi:.4f}]")

        eps_history.append(epsilon)
        next_epsilon = float(np.quantile(sse_vals, alpha))
        print(f"  Next ε = {next_epsilon:.4f}  (α={alpha} quantile)")

        if next_epsilon <= epsilon_final:
            print(f"  ε below final threshold ({epsilon_final}) — stopping.")
            eps_history.append(next_epsilon)
            break

        epsilon        = next_epsilon
        prev_particles = particles.copy()
        prev_weights   = weights.copy()
        sigmas = 2.0 * np.sqrt(np.average(
            (particles - np.average(particles, weights=weights, axis=0)) ** 2,
            weights=weights, axis=0
        ))
        sigmas = np.maximum(sigmas, 1e-6)

    print(f"\n{'='*65}")
    print(f"  ABC-SMC Final complete  |  ε: {' → '.join(f'{e:.2f}' for e in eps_history)}")
    print(f"  Results in: {out_dir}")
    print(f"{'='*65}")
    _save_eps_history(out_dir, eps_history)
    return particles, weights, sse_vals


# ---------------------------------------------------------------------------
# Plots
# ---------------------------------------------------------------------------

def plot_final(out_dir: Path, n_generations: int, epsilon_final: float) -> None:
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print("[warn] matplotlib not available — skipping plots")
        return

    n_params = len(PARAM_NAMES)
    colors   = plt.cm.viridis(np.linspace(0.15, 0.9, n_generations))

    # --- 1. Marginal posteriors ---
    fig, axes = plt.subplots(1, n_params, figsize=(3.5 * n_params, 4))
    if n_params == 1:
        axes = [axes]

    all_gens_data = []
    for gen in range(n_generations):
        path = out_dir / f"gen_{gen:02d}.csv"
        if not path.exists():
            break
        with open(path) as f:
            rows = list(csv.DictReader(f))
        if not rows:
            continue
        data    = {p: np.array([float(r[p]) for r in rows]) for p in PARAM_NAMES}
        weights = np.array([float(r["weight"]) for r in rows])
        weights /= weights.sum()
        data["onset_age"] = np.array([float(r.get("onset_age", "nan")) for r in rows])
        data["sat25"]     = np.array([float(r.get("sat25", "nan")) for r in rows])
        data["sat50"]     = np.array([float(r.get("sat50", "nan")) for r in rows])
        data["sat90"]     = np.array([float(r.get("sat90", "nan")) for r in rows])
        data["_weights"]  = weights
        data["_gen"]      = gen
        all_gens_data.append(data)

        for ax, p in zip(axes, PARAM_NAMES):
            lo, hi = PRIORS[p]
            bins   = np.linspace(lo, hi, 25)
            ax.hist(data[p], bins=bins, weights=weights, density=True,
                    alpha=0.5, color=colors[gen], label=f"gen {gen}")

    for ax, p in zip(axes, PARAM_NAMES):
        lo, hi = PRIORS[p]
        ax.axhline(1 / (hi - lo), color="red", ls="--", lw=1.2, label="Prior")
        ax.set_xlabel(p, fontsize=9)
        ax.set_ylabel("Density", fontsize=9)
        ax.set_title(p, fontsize=9)
        ax.legend(fontsize=7)
        ax.grid(True, alpha=0.3)

    fig.suptitle(
        f"ABC-SMC v4 — 4 params  (d1={D1_THRESHOLD}, d2={D2_THRESHOLD}, tp53={TP53_RATE}, ε_final={epsilon_final})",
        fontsize=11, fontweight="bold"
    )
    plt.tight_layout()
    for ext in ("png", "pdf"):
        p = out_dir / f"abc_final_v4_marginals.{ext}"
        fig.savefig(p, dpi=150, bbox_inches="tight")
        print(f"  Saved {p}")
    plt.close(fig)

    if not all_gens_data:
        return

    # --- 2. Milestone progression plot (last gen) ---
    last = all_gens_data[-1]
    w    = last["_weights"]
    gen  = last["_gen"]

    milestone_fields = ["onset_age", "sat25", "sat50", "sat90"]
    milestone_labels = ["Onset (5%)", "Sat 25%", "Sat 50%", "Sat 90%"]
    milestone_colors = ["#2196F3", "#4CAF50", "#FF9800", "#F44336"]

    fig, axes = plt.subplots(1, 2, figsize=(12, 4))

    # 2a. Weighted distributions of each milestone
    ax = axes[0]
    for field, label, color in zip(milestone_fields, milestone_labels, milestone_colors):
        vals = last[field]
        valid = vals[(~np.isnan(vals)) & (vals > 0)]
        wt_valid = w[(~np.isnan(vals)) & (vals > 0)]
        if len(valid) == 0:
            continue
        wt_valid = wt_valid / wt_valid.sum()
        bins = np.linspace(0, 85, 30)
        ax.hist(valid, bins=bins, weights=wt_valid, density=True,
                alpha=0.55, color=color, label=label)
    ax.set_xlabel("Age (years)", fontsize=10)
    ax.set_ylabel("Weighted density", fontsize=10)
    ax.set_title(f"Milestone age distributions — gen {gen}", fontsize=10)
    ax.legend(fontsize=9)
    ax.grid(True, alpha=0.3)

    # 2b. Speed: onset → sat50 scatter (coloured by neoplastic_div_rate)
    ax = axes[1]
    onset = last["onset_age"]
    sat50 = last["sat50"]
    neo   = last["neoplastic_div_rate"]
    mask  = (~np.isnan(onset)) & (~np.isnan(sat50)) & (onset > 0) & (sat50 > 0)
    speed = sat50[mask] - onset[mask]
    sc = ax.scatter(onset[mask], speed, c=neo[mask], cmap="plasma",
                    s=20 + 80 * w[mask] / w[mask].max(), alpha=0.7)
    plt.colorbar(sc, ax=ax, label="neoplastic_div_rate")
    ax.set_xlabel("Onset age (years)", fontsize=10)
    ax.set_ylabel("Progression speed (onset→sat50, years)", fontsize=10)
    ax.set_title("Tumor aggressiveness proxy — gen " + str(gen), fontsize=10)
    ax.grid(True, alpha=0.3)

    fig.suptitle("ABC-SMC v4 — Tumor progression milestones", fontsize=11, fontweight="bold")
    plt.tight_layout()
    for ext in ("png", "pdf"):
        p = out_dir / f"abc_final_v4_milestones.{ext}"
        fig.savefig(p, dpi=150, bbox_inches="tight")
        print(f"  Saved {p}")
    plt.close(fig)


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--n-particles",   type=int,   default=200,   metavar="INT")
    parser.add_argument("--n-generations", type=int,   default=8,     metavar="INT")
    parser.add_argument("--epsilon-0",     type=float, default=15.0,  metavar="FLOAT")
    parser.add_argument("--epsilon-final", type=float, default=5.0,   metavar="FLOAT")
    parser.add_argument("--alpha",         type=float, default=0.4,   metavar="FLOAT")
    parser.add_argument("--n-sim",         type=int,   default=50,    metavar="INT")
    parser.add_argument("--max-attempts",  type=int,   default=30000, metavar="INT")
    parser.add_argument("--output-dir",    type=Path,  default=Path("results/abc_final_v4/"), metavar="PATH")
    parser.add_argument("--seed",          type=int,   default=42,    metavar="INT")
    parser.add_argument("--workers",       type=int,   default=4,     metavar="INT")
    parser.add_argument("--resume",        action="store_true", help="Resume from last saved generation")
    parser.add_argument("--no-plots",      action="store_true")
    parser.add_argument("-v", "--verbose", action="store_true")
    args = parser.parse_args()

    if not Path(BINARY).exists():
        print(f"[error] Binary not found: {BINARY}\n  Run: make rebuild")
        sys.exit(1)

    result = run_abc_final(
        n_particles=args.n_particles,
        n_generations=args.n_generations,
        epsilon_0=args.epsilon_0,
        epsilon_final=args.epsilon_final,
        alpha=args.alpha,
        n_sim=args.n_sim,
        max_attempts=args.max_attempts,
        out_dir=args.output_dir,
        seed=args.seed,
        n_workers=args.workers,
        verbose=args.verbose,
        resume=args.resume,
    )

    if not args.no_plots and result is not None:
        print("\n[plots] Generating final plots...")
        plot_final(args.output_dir, args.n_generations, args.epsilon_final)


if __name__ == "__main__":
    main()
