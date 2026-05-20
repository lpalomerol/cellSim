#!/usr/bin/env bash
# =============================================================================
# run_experiments.sh — CellSim calibration experiment pipeline
#
# Reproduces all phases of the BRCA1 calibration against Kuchenbaecker 2017.
# Run from the repository root:
#   bash scripts/run_experiments.sh
#
# Prerequisites: build/run_bootstrapping must exist (make rebuild)
# =============================================================================

set -euo pipefail

# Always run relative to repo root regardless of where the script is called from
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$REPO_ROOT"

BIN=./build/run_bootstrapping
RESULTS=.

echo "=== CellSim Calibration Pipeline ==="
echo "Binary: $BIN"
echo ""

# ---------------------------------------------------------------------------
# Phase 1 — Baseline: default params, no big bang
# ---------------------------------------------------------------------------
echo "[Phase 1] Baseline — default params, N=1000, big_bang=false"
$BIN \
  --n-runs 1000 \
  --brca1-rate 0.008 --low-delta 0.150 --high-delta 0.400 \
  --output "$RESULTS/results_baseline.csv" \
  2>&1 | grep -E "^\s+(30|40|50|60|70|80|SSE)"
echo ""

# ---------------------------------------------------------------------------
# Phase 2 — Coarse grid, no big bang
# ---------------------------------------------------------------------------
echo "[Phase 2] Coarse sweep — no big bang, N=200/combo"
$BIN \
  --sweep --n-runs 200 \
  --output "$RESULTS/sweep_results_nobbang.csv" \
  2>&1 | tail -5
echo ""

# ---------------------------------------------------------------------------
# Phase 3 — Coarse grid WITH big bang
# ---------------------------------------------------------------------------
echo "[Phase 3] Coarse sweep — big bang ON, N=200/combo"
$BIN \
  --sweep --n-runs 200 \
  --big-bang \
  --output "$RESULTS/sweep_results_bigbang.csv" \
  2>&1 | tail -5
echo ""

# ---------------------------------------------------------------------------
# Phase 4 — Fine grid, big bang ON
# ---------------------------------------------------------------------------
echo "[Phase 4] Fine sweep — big bang ON, N=500/combo"
$BIN \
  --fine-sweep --n-runs 500 \
  --big-bang \
  --output "$RESULTS/sweep_results_fine.csv" \
  2>&1 | tail -5
echo ""

# ---------------------------------------------------------------------------
# Phase 5 — TP53 fine sweep (best combo fixed)
# ---------------------------------------------------------------------------
echo "[Phase 5] TP53 fine sweep — tp53_rate 0.0025..0.0050, N=500"
for TP53 in 0.0025 0.0030 0.0035 0.0040 0.0045 0.0050; do
  printf "  tp53=%-7s  " "$TP53"
  $BIN \
    --n-runs 500 \
    --brca1-rate 0.045 --low-delta 0.120 --high-delta 0.240 \
    --big-bang --tp53-rate "$TP53" \
    --output /dev/null \
    2>&1 | grep -E "^\s+SSE"
done
echo ""

# ---------------------------------------------------------------------------
# Convergence analysis — N=500 / 1000 / 2000 on best combo
# ---------------------------------------------------------------------------
echo "[Convergence] N sensitivity — best combo (brca1=0.045, low_d=0.120)"
for N in 500 1000 2000; do
  printf "  N=%-5s  " "$N"
  $BIN \
    --n-runs "$N" \
    --brca1-rate 0.045 --low-delta 0.120 --high-delta 0.240 \
    --big-bang \
    --output "$RESULTS/results_convergence_n${N}.csv" \
    2>&1 | grep -E "^\s+SSE"
done
echo ""

# ---------------------------------------------------------------------------
# Final runs — N=1000, best combo, BB ON and OFF
# ---------------------------------------------------------------------------
echo "[Final] N=1000, best combo, big_bang=true"
$BIN \
  --n-runs 1000 \
  --brca1-rate 0.045 --low-delta 0.120 --high-delta 0.240 \
  --big-bang \
  --output "$RESULTS/results_final_n1000.csv" \
  2>&1 | grep -E "^\s+(30|40|50|60|70|80|SSE)"
echo ""

echo "[Final] N=1000, best combo, big_bang=false"
$BIN \
  --n-runs 1000 \
  --brca1-rate 0.045 --low-delta 0.120 --high-delta 0.240 \
  --output "$RESULTS/results_final_nobb_n1000.csv" \
  2>&1 | grep -E "^\s+(30|40|50|60|70|80|SSE)"
echo ""

echo "[Final] N=1000, initial params, big_bang=true (for figure comparison)"
$BIN \
  --n-runs 1000 \
  --brca1-rate 0.008 --low-delta 0.150 --high-delta 0.400 \
  --big-bang \
  --output "$RESULTS/results_initial_params.csv" \
  2>&1 | grep -E "^\s+(30|40|50|60|70|80|SSE)"
echo ""

echo "=== All experiments done. Run scripts/generate_figures.py to generate figures. ==="
