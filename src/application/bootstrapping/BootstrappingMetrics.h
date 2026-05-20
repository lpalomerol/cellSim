/// src/application/bootstrapping/BootstrappingMetrics.h
/// Shared types and metrics for the bootstrapping calibration tool.
///
/// Extracted from run_bootstrapping.cpp so that unit tests can exercise
/// the real implementation rather than a parallel copy.

#pragma once

#include <vector>

namespace application::bootstrapping {

// ---------------------------------------------------------------------------
// Kuchenbaecker et al. (JAMA 2017) — BRCA1 cumulative breast cancer risk
// ---------------------------------------------------------------------------
struct ClinicalPoint {
    int    age;
    double risk;    // %
    double ci_lo;
    double ci_hi;
};

extern const ClinicalPoint KUCHENBAECKER_BRCA1[6];
constexpr int N_CLINICAL = 6;

// ---------------------------------------------------------------------------
// Per-run result
// ---------------------------------------------------------------------------
struct RunResult {
    int    seed;
    int    onset_year;           // -1 = no tumour within max_t
    double final_neoplastic_pct;
};

// ---------------------------------------------------------------------------
// Cumulative risk at 'age': fraction of runs with onset_year in [0, age].
// Returns 0.0 for empty results.
// ---------------------------------------------------------------------------
double cumulativeRisk(const std::vector<RunResult>& results, int age);

// ---------------------------------------------------------------------------
// Sum of squared errors vs. Kuchenbaecker at the 6 anchor ages.
// Returns 0.0 for empty results.
// ---------------------------------------------------------------------------
double computeSSE(const std::vector<RunResult>& results);

// ---------------------------------------------------------------------------
// Unique seed per (run, cell) to ensure independence between runs.
// Formula: run_seed * 10000 + cell_index
// Precondition: cell_index < 10000
// ---------------------------------------------------------------------------
unsigned makeCellSeed(int run_seed, int cell_index);

} // namespace application::bootstrapping
