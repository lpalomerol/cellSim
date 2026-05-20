/// tests/RunBootstrappingTest.cpp
/// Comprehensive tests for run_bootstrapping — edge cases and boundary conditions

#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <set>

#include "../src/application/bootstrapping/BootstrappingMetrics.h"

using application::bootstrapping::RunResult;
using application::bootstrapping::ClinicalPoint;
using application::bootstrapping::KUCHENBAECKER_BRCA1;
using application::bootstrapping::N_CLINICAL;
using application::bootstrapping::cumulativeRisk;
using application::bootstrapping::computeSSE;
using application::bootstrapping::makeCellSeed;

// ============================================================================
// TEST SUITE 1: cumulativeRisk — Edge Cases
// ============================================================================

TEST(CumulativeRiskTest, EmptyResults) {
    std::vector<RunResult> empty;
    EXPECT_DOUBLE_EQ(cumulativeRisk(empty, 50), 0.0);
    EXPECT_DOUBLE_EQ(cumulativeRisk(empty, 0), 0.0);
    EXPECT_DOUBLE_EQ(cumulativeRisk(empty, 100), 0.0);
}

TEST(CumulativeRiskTest, AllNegativeOnset) {
    // onset_year = -1 means no cancer; other negative values are also invalid and ignored
    std::vector<RunResult> results;
    for (int i = 0; i < 100; ++i)
        results.push_back({i, -1, 0.0});
    results.push_back({100, -2,   0.0});
    results.push_back({101, -999, 0.0});

    EXPECT_DOUBLE_EQ(cumulativeRisk(results, 30), 0.0);
    EXPECT_DOUBLE_EQ(cumulativeRisk(results, 50), 0.0);
    EXPECT_DOUBLE_EQ(cumulativeRisk(results, 80), 0.0);
}

TEST(CumulativeRiskTest, AllImmediateOnset) {
    // All runs have onset_year = 0 (immediate cancer)
    std::vector<RunResult> results;
    for (int i = 0; i < 100; ++i)
        results.push_back({i, 0, 100.0});

    EXPECT_DOUBLE_EQ(cumulativeRisk(results, 0), 100.0);
    EXPECT_DOUBLE_EQ(cumulativeRisk(results, 30), 100.0);
    EXPECT_DOUBLE_EQ(cumulativeRisk(results, 80), 100.0);
}

TEST(CumulativeRiskTest, AllLateCancer) {
    // All runs have onset_year = 90 (very late)
    std::vector<RunResult> results;
    for (int i = 0; i < 100; ++i)
        results.push_back({i, 90, 50.0});

    EXPECT_DOUBLE_EQ(cumulativeRisk(results, 80), 0.0);   // No events before 80
    EXPECT_DOUBLE_EQ(cumulativeRisk(results, 90), 100.0); // All events at 90
    EXPECT_DOUBLE_EQ(cumulativeRisk(results, 100), 100.0);
}

TEST(CumulativeRiskTest, MixedOnsets) {
    std::vector<RunResult> results = {
        {0, 25, 10.0},  // Before 30
        {1, 35, 15.0},  // Between 30-40
        {2, 45, 20.0},  // Between 40-50
        {3, 55, 25.0},  // Between 50-60
        {4, -1, 0.0},   // Never
    };

    EXPECT_DOUBLE_EQ(cumulativeRisk(results, 30), 20.0);  // {25}          → 1/5 = 20%
    EXPECT_DOUBLE_EQ(cumulativeRisk(results, 40), 40.0);  // {25,35}       → 2/5 = 40%
    EXPECT_DOUBLE_EQ(cumulativeRisk(results, 50), 60.0);  // {25,35,45}    → 3/5 = 60%
    EXPECT_DOUBLE_EQ(cumulativeRisk(results, 80), 80.0);  // {25,35,45,55} → 4/5 = 80%
}

TEST(CumulativeRiskTest, BoundaryAge) {
    std::vector<RunResult> results = {
        {0, 50, 10.0},
        {1, 50, 15.0},
        {2, 50, 20.0},
    };

    // Exactly at boundary
    EXPECT_DOUBLE_EQ(cumulativeRisk(results, 50), 100.0);
    EXPECT_DOUBLE_EQ(cumulativeRisk(results, 49), 0.0);   // Before boundary
    EXPECT_DOUBLE_EQ(cumulativeRisk(results, 51), 100.0); // After boundary
}

TEST(CumulativeRiskTest, SingleRun) {
    std::vector<RunResult> single = {{0, 40, 10.0}};

    EXPECT_DOUBLE_EQ(cumulativeRisk(single, 30), 0.0);
    EXPECT_DOUBLE_EQ(cumulativeRisk(single, 40), 100.0);
    EXPECT_DOUBLE_EQ(cumulativeRisk(single, 50), 100.0);
}

TEST(CumulativeRiskTest, NegativeAge) {
    std::vector<RunResult> results = {{0, 40, 10.0}};

    // Negative age should return 0 (no events before age 0)
    EXPECT_DOUBLE_EQ(cumulativeRisk(results, -10), 0.0);
}

TEST(CumulativeRiskTest, LargeN) {
    std::vector<RunResult> results;
    for (int i = 0; i < 10000; ++i) {
        // 50% have onset at 40, 50% never
        int onset = (i % 2 == 0) ? 40 : -1;
        results.push_back({i, onset, 10.0});
    }

    EXPECT_NEAR(cumulativeRisk(results, 40), 50.0, 0.01);
    EXPECT_NEAR(cumulativeRisk(results, 30), 0.0, 0.01);
}

TEST(CumulativeRiskTest, NeoplasticPctDoesNotAffectRisk) {
    // final_neoplastic_pct is metadata only — cumulativeRisk must ignore it
    std::vector<RunResult> results = {
        {0, 50,    0.0},
        {1, 50,  100.0},
        {2, 50, 1000.0},   // invalid value, but must not influence the result
    };

    EXPECT_DOUBLE_EQ(cumulativeRisk(results, 50), 100.0);
}

// ============================================================================
// TEST SUITE 2: computeSSE — Edge Cases
// ============================================================================

TEST(ComputeSSETest, EmptyResults) {
    std::vector<RunResult> empty;
    EXPECT_DOUBLE_EQ(computeSSE(empty), 0.0);
}

TEST(ComputeSSETest, PerfectMatch) {
    // Construct results that exactly match Kuchenbaecker
    // r30=4%, r40=26%, r50=46%, r60=58%, r70=65%, r80=70%
    std::vector<RunResult> results;

    // 100 runs total
    // 4 at age 30 (r30=4%)
    for (int i = 0; i < 4; ++i) results.push_back({i, 30, 10.0});
    // 22 more at age 40 (r40=26%)
    for (int i = 4; i < 26; ++i) results.push_back({i, 40, 10.0});
    // 20 more at age 50 (r50=46%)
    for (int i = 26; i < 46; ++i) results.push_back({i, 50, 10.0});
    // 12 more at age 60 (r60=58%)
    for (int i = 46; i < 58; ++i) results.push_back({i, 60, 10.0});
    // 7 more at age 70 (r70=65%)
    for (int i = 58; i < 65; ++i) results.push_back({i, 70, 10.0});
    // 5 more at age 80 (r80=70%)
    for (int i = 65; i < 70; ++i) results.push_back({i, 80, 10.0});
    // 30 never (non-penetrance)
    for (int i = 70; i < 100; ++i) results.push_back({i, -1, 0.0});

    double sse = computeSSE(results);
    EXPECT_NEAR(sse, 0.0, 0.1);  // Should be ~0 (perfect match)
}

TEST(ComputeSSETest, AllNoCancer) {
    std::vector<RunResult> results;
    for (int i = 0; i < 100; ++i)
        results.push_back({i, -1, 0.0});

    // All simulated risks = 0%
    // SSE = (0-4)^2 + (0-26)^2 + (0-46)^2 + (0-58)^2 + (0-65)^2 + (0-70)^2
    //     = 16 + 676 + 2116 + 3364 + 4225 + 4900 = 15297
    double expected_sse = 16.0 + 676.0 + 2116.0 + 3364.0 + 4225.0 + 4900.0;
    EXPECT_NEAR(computeSSE(results), expected_sse, 1.0);
}

TEST(ComputeSSETest, AllEarlyCancer) {
    // All onset at age 20 (100% penetrance by all ages)
    std::vector<RunResult> results;
    for (int i = 0; i < 100; ++i)
        results.push_back({i, 20, 100.0});

    // All simulated risks = 100%
    // SSE = (100-4)^2 + (100-26)^2 + (100-46)^2 + (100-58)^2 + (100-65)^2 + (100-70)^2
    //     = 9216 + 5476 + 2916 + 1764 + 1225 + 900 = 21497
    double expected_sse = 9216.0 + 5476.0 + 2916.0 + 1764.0 + 1225.0 + 900.0;
    EXPECT_NEAR(computeSSE(results), expected_sse, 1.0);
}

TEST(ComputeSSETest, OnsetAtAge50) {
    // All onset at age 50 exactly
    std::vector<RunResult> results;
    for (int i = 0; i < 100; ++i)
        results.push_back({i, 50, 10.0});

    // r30=0, r40=0, r50=100, r60=100, r70=100, r80=100
    // SSE = (0-4)^2 + (0-26)^2 + (100-46)^2 + (100-58)^2 + (100-65)^2 + (100-70)^2
    //     = 16 + 676 + 2916 + 1764 + 1225 + 900 = 7497
    EXPECT_NEAR(computeSSE(results), 7497.0, 1.0);
}

TEST(ComputeSSETest, SingleRun) {
    std::vector<RunResult> single = {{0, 50, 10.0}};

    // With single run, all risks are either 0% or 100%
    // r30=0, r40=0, r50=100, r60=100, r70=100, r80=100
    EXPECT_NEAR(computeSSE(single), 7497.0, 1.0);
}

TEST(ComputeSSETest, TwoRunsOpposite) {
    std::vector<RunResult> results = {
        {0, 30, 10.0},  // Early
        {1, -1, 0.0},   // Never
    };

    // r30=50%, r40=50%, r50=50%, r60=50%, r70=50%, r80=50%
    // SSE = (50-4)^2 + (50-26)^2 + (50-46)^2 + (50-58)^2 + (50-65)^2 + (50-70)^2
    //     = 2116 + 576 + 16 + 64 + 225 + 400 = 3397
    EXPECT_NEAR(computeSSE(results), 3397.0, 1.0);
}

// ============================================================================
// TEST SUITE 3: Seed Generation — No Collisions
// ============================================================================

TEST(SeedGenerationTest, NoCollisionsStandard) {
    std::set<unsigned> seeds;
    int n_runs = 1000;
    int n_cells = 500;

    for (int seed = 0; seed < n_runs; ++seed) {
        for (int i = 0; i < n_cells; ++i) {
            unsigned cell_seed = makeCellSeed(seed, i);
            EXPECT_EQ(seeds.count(cell_seed), 0)
                << "Collision: seed=" << seed << ", i=" << i;
            seeds.insert(cell_seed);
        }
    }

    EXPECT_EQ(seeds.size(), static_cast<size_t>(n_runs * n_cells));
}

TEST(SeedGenerationTest, NoCollisionsSingleCell) {
    std::set<unsigned> seeds;
    int n_runs = 10000;

    for (int seed = 0; seed < n_runs; ++seed) {
        unsigned cell_seed = makeCellSeed(seed, 0);
        EXPECT_EQ(seeds.count(cell_seed), 0);
        seeds.insert(cell_seed);
    }

    EXPECT_EQ(seeds.size(), static_cast<size_t>(n_runs));
}

TEST(SeedGenerationTest, BoundarySeeds) {
    std::set<unsigned> seeds;

    std::vector<int> test_seeds = {0, 1, 999, 1000, 9999, 10000};
    std::vector<int> test_cells = {0, 1, 499, 500, 999};

    for (int seed : test_seeds)
        for (int i : test_cells)
            seeds.insert(makeCellSeed(seed, i));

    EXPECT_EQ(seeds.size(), test_seeds.size() * test_cells.size());
}

TEST(SeedGenerationTest, OverflowCheck) {
    // makeCellSeed(100000, 999) = 1_000_000_999 < UINT_MAX (4_294_967_295)
    unsigned cell_seed = makeCellSeed(100000, 999);
    EXPECT_LT(cell_seed, UINT_MAX);
}

// ============================================================================
// TEST SUITE 4: Numerical Stability
// ============================================================================

TEST(NumericalStabilityTest, VeryLargeN) {
    std::vector<RunResult> results;
    for (int i = 0; i < 100000; ++i) {
        results.push_back({i, 50, 10.0});
    }

    double risk = cumulativeRisk(results, 50);
    EXPECT_NEAR(risk, 100.0, 0.001);  // Should be exactly 100%
}

TEST(NumericalStabilityTest, TinyProbability) {
    // 1 event out of 100000
    std::vector<RunResult> results;
    results.push_back({0, 50, 10.0});
    for (int i = 1; i < 100000; ++i) {
        results.push_back({i, -1, 0.0});
    }

    double risk = cumulativeRisk(results, 50);
    EXPECT_NEAR(risk, 0.001, 0.0001);  // 1/100000 * 100 = 0.001%
}

TEST(NumericalStabilityTest, SSEWithLargeErrors) {
    // All onset at 0 → simulated risk = 100% at every anchor age
    // SSE = (100-4)^2 + (100-26)^2 + (100-46)^2 + (100-58)^2 + (100-65)^2 + (100-70)^2
    //     = 9216 + 5476 + 2916 + 1764 + 1225 + 900 = 21497
    std::vector<RunResult> results;
    for (int i = 0; i < 1000; ++i)
        results.push_back({i, 0, 100.0});

    EXPECT_NEAR(computeSSE(results), 21497.0, 1.0);
}

// ============================================================================
// TEST SUITE 5: Consistency Checks
// ============================================================================

TEST(ConsistencyTest, MonotonicRisk) {
    // Risk should be monotonic increasing with age
    std::vector<RunResult> results;
    for (int i = 0; i < 100; ++i) {
        int onset = 10 + (i % 70);  // Spread from 10 to 80
        results.push_back({i, onset, 10.0});
    }

    std::vector<int> ages = {30, 40, 50, 60, 70, 80};
    for (size_t i = 1; i < ages.size(); ++i) {
        double risk_prev = cumulativeRisk(results, ages[i-1]);
        double risk_curr = cumulativeRisk(results, ages[i]);
        EXPECT_GE(risk_curr, risk_prev)
            << "Risk should be monotonic: age " << ages[i-1] << " vs " << ages[i];
    }
}

TEST(ConsistencyTest, RiskBounds) {
    // Risk should always be in [0, 100]
    std::vector<RunResult> results;
    for (int i = 0; i < 100; ++i)
        results.push_back({i, i, 10.0});

    for (int age = 0; age <= 100; ++age) {
        double risk = cumulativeRisk(results, age);
        EXPECT_GE(risk, 0.0) << "Risk at age " << age;
        EXPECT_LE(risk, 100.0) << "Risk at age " << age;
    }
}

