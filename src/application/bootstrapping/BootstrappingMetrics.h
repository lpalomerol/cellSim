/// src/application/bootstrapping/BootstrappingMetrics.h
/// Shared types and metrics for the bootstrapping calibration tool.
///
/// Extracted from run_bootstrapping.cpp so that unit tests can exercise
/// the real implementation rather than a parallel copy.

#pragma once

#include <vector>
#include "../simulation/PopulationTracker.h"

namespace application::bootstrapping {

// ---------------------------------------------------------------------------
// Saturation milestones: first year the tumoral fraction crossed 25/50/90%.
// -1 means the threshold was never reached within the simulation.
// ---------------------------------------------------------------------------
struct SaturationMilestones {
    int year_25 = -1;
    int year_50 = -1;
    int year_90 = -1;
};

// ---------------------------------------------------------------------------
// Compute saturation milestones from a series of yearly snapshots.
// Skips snapshots with alive_cells <= 0.
// ---------------------------------------------------------------------------
SaturationMilestones computeMilestones(const std::vector<application::YearlySnapshot>& snapshots);

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
    int    year_25pct = -1;      // first year ≥25% tumoral (-1 = never)
    int    year_50pct = -1;      // first year ≥50% tumoral (-1 = never)
    int    year_90pct = -1;      // first year ≥90% tumoral (-1 = never)
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
// Weighted SSE: Σᵢ (sim(ageᵢ) − target(ageᵢ))² / σᵢ²
// where σᵢ = (ci_hi − ci_lo) / 3.92  (95% CI → σ, assuming normality).
//
// This is proportional to −2·log L under a Gaussian likelihood with
// heteroscedastic variance derived from Kuchenbaecker's published CIs.
// Under the null (perfect fit), SSE_w ~ χ²(6).
// Returns 0.0 for empty results.
// ---------------------------------------------------------------------------
double computeWeightedSSE(const std::vector<RunResult>& results);

// ---------------------------------------------------------------------------
// Mahalanobis distance using full covariance on cumulative-risk anchors.
// Σ = Σ_clinical(diagonal from reported CIs) + Σ_simulation(mean estimator).
// Returns diffᵀ Σ⁻¹ diff, with a small ridge regularization for stability.
// Returns 0.0 for empty results.
// ---------------------------------------------------------------------------
double computeMahalanobisDistance(const std::vector<RunResult>& results);

// ---------------------------------------------------------------------------
// Median saturation year computed from a vector of per-run milestone years.
// Values of -1 (never reached) are ignored.
// Returns -1.0 if no run reached the milestone.
// ---------------------------------------------------------------------------
double medianSaturation(const std::vector<int>& milestone_years);

// ---------------------------------------------------------------------------
// Median onset year across runs that developed a tumour (onset_year >= 0).
// Returns -1.0 if no run developed a tumour.
// ---------------------------------------------------------------------------
double medianOnset(const std::vector<RunResult>& results);

// ---------------------------------------------------------------------------
// Unique seed per (run, cell) to ensure independence between runs.
// Formula: run_seed * 10000 + cell_index
// Precondition: cell_index < 10000
// ---------------------------------------------------------------------------
unsigned makeCellSeed(int run_seed, int cell_index);

} // namespace application::bootstrapping
