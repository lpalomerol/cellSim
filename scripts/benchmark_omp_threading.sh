#!/usr/bin/env bash
# Benchmark script to compare OMP threading strategies
# Usage: bash scripts/benchmark_omp_threading.sh

set -euo pipefail

BINARY="./build/run_bootstrapping"
N_SIMS=10
N_RUNS=60


echo "=========================================="
echo "  OMP Threading Benchmark for ABC-SMC"
echo "=========================================="
echo "Running $N_SIMS parallel simulations (n_runs=$N_RUNS each)"
echo ""

# Test 1: Default OpenMP behavior (auto-detects cores)
echo "[1/3] Default OpenMP (auto threads)..."
unset OMP_NUM_THREADS
START=$(date +%s)
for i in $(seq 1 $N_SIMS); do
    $BINARY --n-runs $N_RUNS \
            --brca1-rate 0.012 --low-delta 0.3 --high-delta 0.8 \
            --neoplastic-div-rate 0.15 --weighted-sse \
            --output /dev/null > /dev/null 2>&1 &
done
wait
END=$(date +%s)
TIME_DEFAULT=$((END - START))
echo "  → Completed in ${TIME_DEFAULT}s"
echo ""

# Test 2: OMP_NUM_THREADS=1
echo "[2/3] OMP_NUM_THREADS=1..."
START=$(date +%s)
for i in $(seq 1 $N_SIMS); do
    OMP_NUM_THREADS=1 $BINARY --n-runs $N_RUNS \
            --brca1-rate 0.012 --low-delta 0.3 --high-delta 0.8 \
            --neoplastic-div-rate 0.15 --weighted-sse \
            --output /dev/null > /dev/null 2>&1 &
done
wait
END=$(date +%s)
TIME_OMP1=$((END - START))
echo "  → Completed in ${TIME_OMP1}s"
echo ""

# Test 3: Sequential (baseline)
echo "[3/3] Sequential baseline (1 sim at a time)..."
START=$(date +%s)
for i in $(seq 1 $N_SIMS); do
    $BINARY --n-runs $N_RUNS \
            --brca1-rate 0.012 --low-delta 0.3 --high-delta 0.8 \
            --neoplastic-div-rate 0.15 --weighted-sse \
            --output /dev/null > /dev/null 2>&1
done
END=$(date +%s)
TIME_SEQ=$((END - START))
echo "  → Completed in ${TIME_SEQ}s"
echo ""

# Results
echo "=========================================="
echo "  Results ($N_SIMS parallel simulations)"
echo "=========================================="
echo "Default OpenMP (auto):     ${TIME_DEFAULT}s"
echo "OMP_NUM_THREADS=1:         ${TIME_OMP1}s"
echo "Sequential (baseline):     ${TIME_SEQ}s"
echo ""

# Calculate speedup
SPEEDUP=$(echo "scale=2; $TIME_DEFAULT / $TIME_OMP1" | bc)
PARALLEL_EFF=$(echo "scale=1; 100 * $TIME_SEQ / ($TIME_OMP1 * $N_SIMS)" | bc)

if (( $(echo "$TIME_OMP1 < $TIME_DEFAULT" | bc -l) )); then
    echo "✅ OMP_NUM_THREADS=1 is ${SPEEDUP}x FASTER (${PARALLEL_EFF}% parallel efficiency)"
    echo "   Recommendation: Use OMP_NUM_THREADS=1 for ABC-SMC"
else
    echo "⚠️  Default OpenMP is faster for this workload"
    echo "   Consider tuning OMP_NUM_THREADS to match worker count"
fi
echo ""
