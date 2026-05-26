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
using application::bootstrapping::computeWeightedSSE;
using application::bootstrapping::computeMahalanobisDistance;
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


// TEST SUITE 6: computeWeightedSSE
// Weighted SSE = Σᵢ (sim(ageᵢ) − target(ageᵢ))² / σᵢ²
// σᵢ = (ci_hi − ci_lo) / 3.92
// Kuchenbaecker σ values (pre-computed):
//   age 30: σ = (7.0−2.0)/3.92 = 1.2755   → weight = 0.6143
//   age 40: σ = (30.0−22.0)/3.92 = 2.0408  → weight = 0.2400
//   age 50: σ = (52.0−41.0)/3.92 = 2.8061  → weight = 0.1270
//   age 60: σ = (65.0−52.0)/3.92 = 3.3163  → weight = 0.09087
//   age 70: σ = (73.0−56.0)/3.92 = 4.3367  → weight = 0.05313
//   age 80: σ = (80.0−60.0)/3.92 = 5.1020  → weight = 0.03840

TEST(ComputeWeightedSSETest, EmptyResults) {
    EXPECT_DOUBLE_EQ(0.0, computeWeightedSSE({}));
}

TEST(ComputeWeightedSSETest, PerfectMatch) {
    // Simulated risk exactly equals Kuchenbaecker targets → SSE_w = 0
    // Kuchenbaecker targets: {30:4%, 40:26%, 50:46%, 60:58%, 70:65%, 80:70%}
    // With N=100 results and onset spread to hit exactly those percentages:
    //   4 onsets at year 30, 22 more at year 40, 20 more at 50, 12 more at 60,
    //   7 more at 70, 5 more at 80 → cumulative: 4/26/46/58/65/70 out of 100
    std::vector<RunResult> results;
    int seed = 0;
    auto add = [&](int year, int n) {
        for (int i = 0; i < n; ++i) results.push_back({seed++, year, 0.0});
    };
    add(30,  4);
    add(40, 22);
    add(50, 20);
    add(60, 12);
    add(70,  7);
    add(80,  5);
    for (int i = 0; i < 30; ++i) results.push_back({seed++, -1, 0.0}); // non-penetrant
    // total = 100; cumulative risk = 4/26/46/58/65/70 %
    EXPECT_NEAR(0.0, computeWeightedSSE(results), 1e-9);
}

TEST(ComputeWeightedSSETest, UnweightedVsWeightedDifferForHeteroscedasticErrors) {
    // If all errors are equal in absolute value, SSE_w != SSE (different weights).
    // Construct results with constant +10pp error at every age.
    // SSE (unweighted) = 6 * 100 = 600
    // SSE_w = Σ 100 * weightᵢ = 100 * (0.6143+0.2400+0.1270+0.09087+0.05313+0.03840)
    //       ≈ 100 * 1.1637 ≈ 116.37
    std::vector<RunResult> results;
    int seed = 0;
    auto add = [&](int year, int n) {
        for (int i = 0; i < n; ++i) results.push_back({seed++, year, 0.0});
    };
    // Target + 10pp at each age: 14/36/56/68/75/80%
    add(30, 14);
    add(40, 22);
    add(50, 20);
    add(60, 12);
    add(70,  7);
    add(80,  5);
    for (int i = 0; i < 20; ++i) results.push_back({seed++, -1, 0.0});
    // total = 100; cumulative = 14/36/56/68/75/80 → +10pp at every point

    double sse   = computeSSE(results);
    double sse_w = computeWeightedSSE(results);

    EXPECT_NEAR(600.0, sse,   0.5);  // 6 * 10² = 600
    EXPECT_NEAR(116.4, sse_w, 1.0);  // smaller: high-CI ages penalised less
    EXPECT_LT(sse_w, sse);           // always true: weights < 1 for all ages
}

TEST(ComputeWeightedSSETest, EarlyAgeErrorPenalisedMoreThanLateAge) {
    // An equal absolute error at age 30 (tight CI) should produce a larger
    // contribution to SSE_w than the same error at age 80 (wide CI).
    //
    // Weight ratio: w30/w80 = (σ80/σ30)² = (5.1020/1.2755)² ≈ 15.99

    auto makeResults = [](int onset_age, double shift_pp) {
        // Build 100 results so that only the given age has a +shift_pp error.
        // All other ages hit the Kuchenbaecker target exactly.
        // Strategy: construct to hit targets exactly, then re-route 'shift' runs.
        std::vector<RunResult> r;
        int seed = 0;
        // Targets (incremental): 4, 22, 20, 12, 7, 5, 30 (non-pene) = 100
        struct { int year; int n; } buckets[] = {
            {30,4},{40,22},{50,20},{60,12},{70,7},{80,5}
        };
        // Adjust bucket for onset_age
        for (auto& b : buckets) {
            int n = b.n;
            if (b.year == onset_age) n += static_cast<int>(shift_pp);
            for (int i = 0; i < n; ++i) r.push_back({seed++, b.year, 0.0});
        }
        int total = static_cast<int>(r.size());
        int non_pene = 100 + static_cast<int>(shift_pp) - total;
        for (int i = 0; i < std::max(0, non_pene); ++i) r.push_back({seed++, -1, 0.0});
        return r;
    };

    // +5pp error only at age 30
    auto r30 = makeResults(30, 5.0);
    // +5pp error only at age 80 (need extra runs to keep total correct)
    auto r80 = makeResults(80, 5.0);

    double contrib30 = computeWeightedSSE(r30);
    double contrib80 = computeWeightedSSE(r80);

    EXPECT_GT(contrib30, contrib80)
        << "age-30 error (tight CI) should be penalised more than age-80 error (wide CI)";
}

TEST(ComputeWeightedSSETest, NonNegative) {
    std::vector<RunResult> results;
    for (int i = 0; i < 50; ++i) results.push_back({i, i * 2, 0.0});
    EXPECT_GE(computeWeightedSSE(results), 0.0);
}

TEST(ComputeWeightedSSETest, ChiSquaredInterpretation) {
    // Under perfect calibration SSE_w ~ chi2(6), so SSE_w = 0 for perfect fit.
    // For all-early-onset (100% at every age), verify SSE_w > chi2(6, 0.999) ≈ 22.5.
    // i.e., a clearly bad fit produces a value that would be rejected at any threshold.
    std::vector<RunResult> results;
    for (int i = 0; i < 100; ++i) results.push_back({i, 0, 0.0}); // all onset at t=0
    double sse_w = computeWeightedSSE(results);
    EXPECT_GT(sse_w, 22.5); // well above chi2(6, 0.999) = 22.46
}

// ============================================================================
// TEST SUITE 6: computeMilestones — saturation milestone tracking
// ============================================================================

using application::YearlySnapshot;
using application::bootstrapping::computeMilestones;
using application::bootstrapping::SaturationMilestones;

// Helper: build a minimal YearlySnapshot with only year/alive/neoplastic set.
static YearlySnapshot makeSnap(int year, int alive, int neoplastic) {
    YearlySnapshot s{};
    s.year             = year;
    s.alive_cells      = alive;
    s.neoplastic_alive = neoplastic;
    return s;
}

TEST(ComputeMilestonesTest, EmptySnapshots) {
    SaturationMilestones m = computeMilestones({});
    EXPECT_EQ(m.year_25, -1);
    EXPECT_EQ(m.year_50, -1);
    EXPECT_EQ(m.year_90, -1);
}

TEST(ComputeMilestonesTest, NeverReachesAnyThreshold) {
    // 10% neoplastic throughout — below every milestone
    std::vector<YearlySnapshot> snaps;
    for (int y = 0; y <= 80; ++y)
        snaps.push_back(makeSnap(y, 100, 10));  // 10% always

    SaturationMilestones m = computeMilestones(snaps);
    EXPECT_EQ(m.year_25, -1);
    EXPECT_EQ(m.year_50, -1);
    EXPECT_EQ(m.year_90, -1);
}

TEST(ComputeMilestonesTest, FullProgressionReachesAll) {
    // Crosses all three thresholds at years 20, 30, 40
    std::vector<YearlySnapshot> snaps = {
        makeSnap(10, 100,  10),   // 10%
        makeSnap(20, 100,  30),   // 30%  → crosses 25%
        makeSnap(30, 100,  60),   // 60%  → crosses 50%
        makeSnap(40, 100,  95),   // 95%  → crosses 90%
    };

    SaturationMilestones m = computeMilestones(snaps);
    EXPECT_EQ(m.year_25, 20);
    EXPECT_EQ(m.year_50, 30);
    EXPECT_EQ(m.year_90, 40);
}

TEST(ComputeMilestonesTest, SimStopsAtSaturation50Pct) {
    // Mirrors real behaviour: isSaturated() stops sim at ~50%
    // → year_25 set, year_50 set, year_90 never reached
    std::vector<YearlySnapshot> snaps = {
        makeSnap(10, 100,  20),   // 20%
        makeSnap(20, 100,  30),   // 30%  → crosses 25%
        makeSnap(30, 100,  50),   // 50%  → crosses 50% (sim stops here)
        // no further snapshots: saturation guard triggered
    };

    SaturationMilestones m = computeMilestones(snaps);
    EXPECT_EQ(m.year_25, 20);
    EXPECT_EQ(m.year_50, 30);
    EXPECT_EQ(m.year_90, -1);  // never reached after early stop
}

TEST(ComputeMilestonesTest, ExactlyAtThresholdCountsAsCrossed) {
    std::vector<YearlySnapshot> snaps = {
        makeSnap(5,  100, 25),   // exactly 25.0%
        makeSnap(10, 100, 50),   // exactly 50.0%
        makeSnap(15, 100, 90),   // exactly 90.0%
    };

    SaturationMilestones m = computeMilestones(snaps);
    EXPECT_EQ(m.year_25,  5);
    EXPECT_EQ(m.year_50, 10);
    EXPECT_EQ(m.year_90, 15);
}

TEST(ComputeMilestonesTest, SkipsSnapshotsWithNoAliveCells) {
    // alive_cells=0 means the tissue is dead/uninitialized — must be ignored
    std::vector<YearlySnapshot> snaps = {
        makeSnap(5,   0,  50),   // alive=0: skip even though 50/0 would be NaN/inf
        makeSnap(10, 100, 60),   // 60% → first valid snap, crosses 25% and 50%
        makeSnap(15, 100, 95),   // 95% → crosses 90%
    };

    SaturationMilestones m = computeMilestones(snaps);
    EXPECT_EQ(m.year_25, 10);   // year 5 was skipped
    EXPECT_EQ(m.year_50, 10);
    EXPECT_EQ(m.year_90, 15);
}

TEST(ComputeMilestonesTest, FirstCrossingIsRecorded_NotSubsequent) {
    // Even if pct goes back down (edge case), the first crossing year is kept
    std::vector<YearlySnapshot> snaps = {
        makeSnap(10, 100, 30),   // 30% → crosses 25% first time
        makeSnap(20, 100, 10),   // 10% (regression — unusual but should not overwrite)
        makeSnap(30, 100, 60),   // 60% → crosses 50% first time
    };

    SaturationMilestones m = computeMilestones(snaps);
    EXPECT_EQ(m.year_25, 10);   // first crossing at year 10
    EXPECT_EQ(m.year_50, 30);
    EXPECT_EQ(m.year_90, -1);
}

TEST(ComputeMilestonesTest, MonotonicityInvariant) {
    // year_25 <= year_50 <= year_90 whenever set
    std::vector<YearlySnapshot> snaps = {
        makeSnap( 5, 100,  5),
        makeSnap(15, 100, 25),
        makeSnap(25, 100, 51),
        makeSnap(35, 100, 91),
    };

    SaturationMilestones m = computeMilestones(snaps);
    ASSERT_NE(m.year_25, -1);
    ASSERT_NE(m.year_50, -1);
    ASSERT_NE(m.year_90, -1);
    EXPECT_LE(m.year_25, m.year_50);
    EXPECT_LE(m.year_50, m.year_90);
}

// ============================================================================
// TEST SUITE: computeMahalanobisDistance — Publication-Grade ABC-SMC Metric
// ============================================================================

TEST(ComputeMahalanobisDistanceTest, EmptyResults) {
    // Empty input should return 0 (no deviation from prior reference)
    std::vector<RunResult> empty;
    double dist = computeMahalanobisDistance(empty);
    EXPECT_DOUBLE_EQ(dist, 0.0);
}

TEST(ComputeMahalanobisDistanceTest, SingleRunNoCancer) {
    // Single run with no cancer (all ages > onset_year means not observed)
    // cumulative risk at all ages should be 0%
    std::vector<RunResult> results = {{0, -1, 0.0}};
    double dist = computeMahalanobisDistance(results);
    
    // Expect non-negative distance (typically large since clinical has 10-40%)
    EXPECT_GE(dist, 0.0);
    // Distance should be bounded (not NaN/inf)
    EXPECT_TRUE(std::isfinite(dist));
}

TEST(ComputeMahalanobisDistanceTest, AllImmediateOnset) {
    // All runs have early cancer (onset at year 0)
    // cumulative risk at all ages should be 100%
    std::vector<RunResult> results;
    for (int i = 0; i < 50; ++i)
        results.push_back({i, 0, 100.0});
    
    double dist = computeMahalanobisDistance(results);
    
    // Expect finite distance (not NaN/inf)
    EXPECT_TRUE(std::isfinite(dist));
    // Should be large since clinical expectations are 10-40%, not 100%
    EXPECT_GT(dist, 0.0);
}

TEST(ComputeMahalanobisDistanceTest, IsNonNegative) {
    // Mahalanobis distance should always be >= 0
    std::vector<RunResult> scenarios[] = {
        {{0, -1, 0.0}},                          // no cancer
        {{0, 30, 50.0}, {1, 40, 60.0}},         // mixed
        {{0, 0, 100.0}, {1, 0, 100.0}},         // early
        {{0, 80, 10.0}, {1, 90, 20.0}},         // late
    };
    
    for (const auto& scenario : scenarios) {
        double dist = computeMahalanobisDistance(scenario);
        EXPECT_GE(dist, 0.0) << "Distance should be non-negative for all scenarios";
        EXPECT_TRUE(std::isfinite(dist)) << "Distance should be finite";
    }
}

TEST(ComputeMahalanobisDistanceTest, RepeatedResultsConsistent) {
    // Running the same input twice should yield the same distance
    std::vector<RunResult> results = {
        {0, 30, 100.0},
        {1, 35, 90.0},
        {2, -1, 0.0},
    };
    
    double dist1 = computeMahalanobisDistance(results);
    double dist2 = computeMahalanobisDistance(results);
    
    EXPECT_DOUBLE_EQ(dist1, dist2);
}

TEST(ComputeMahalanobisDistanceTest, LargerSampleConsistency) {
    // Larger sample should produce smoother/more stable estimates
    // (order-independent: permutations of same data should give same distance)
    std::vector<RunResult> original = {
        {0, 25, 100.0},
        {1, 30, 95.0},
        {2, 35, 85.0},
        {3, -1, 0.0},
        {4, 50, 100.0},
    };
    
    double dist1 = computeMahalanobisDistance(original);
    
    // Permute
    std::vector<RunResult> permuted = original;
    std::rotate(permuted.begin(), permuted.begin() + 2, permuted.end());
    
    double dist2 = computeMahalanobisDistance(permuted);
    
    // Should be identical (input order should not matter)
    EXPECT_DOUBLE_EQ(dist1, dist2);
}

TEST(ComputeMahalanobisDistanceTest, MonotonicBehavior) {
    // As simulated risk diverges from clinical, distance should increase
    // Clinical baseline expectations: age 50 → ~25%, age 70 → ~45%
    
    // Scenario 1: realistic (closer to clinical)
    std::vector<RunResult> realistic;
    for (int i = 0; i < 100; ++i) {
        if (i < 25) realistic.push_back({i, 50 + (i % 20), 100.0});  // ~25% by 50
        else        realistic.push_back({i, -1, 0.0});
    }
    
    // Scenario 2: unrealistic (very early)
    std::vector<RunResult> tooEarly;
    for (int i = 0; i < 100; ++i)
        tooEarly.push_back({i, 30, 100.0});  // 100% by 30 (way too early)
    
    // Scenario 3: unrealistic (very late)
    std::vector<RunResult> tooLate;
    for (int i = 0; i < 100; ++i)
        tooLate.push_back({i, -1, 0.0});  // 0% always (way too late)
    
    double dist_realistic = computeMahalanobisDistance(realistic);
    double dist_early = computeMahalanobisDistance(tooEarly);
    double dist_late = computeMahalanobisDistance(tooLate);
    
    // All should be finite and comparable
    EXPECT_TRUE(std::isfinite(dist_realistic));
    EXPECT_TRUE(std::isfinite(dist_early));
    EXPECT_TRUE(std::isfinite(dist_late));
    
    // Extreme scenarios should have larger distances
    EXPECT_GT(dist_early, dist_realistic);
    EXPECT_GT(dist_late, dist_realistic);
}

TEST(ComputeMahalanobisDistanceTest, NumericalStability) {
    // Test that ridge regularization (1e-6) handles potential singularity
    // even with very uniform results (all runs identical)
    
    std::vector<RunResult> uniform;
    for (int i = 0; i < 100; ++i)
        uniform.push_back({i, 40, 100.0});
    
    double dist = computeMahalanobisDistance(uniform);
    
    // Should not be NaN/inf even with uniform input
    EXPECT_TRUE(std::isfinite(dist));
    EXPECT_GE(dist, 0.0);
}

TEST(ComputeMahalanobisDistanceTest, SmallVsLargeSamples) {
    // Distance should be stable across sample sizes
    // (empirical covariance has n*(n-1) in denominator, so smaller n → less empirical covariance)
    
    std::vector<RunResult> small, large;
    
    // Generate identical-in-composition samples
    for (int i = 0; i < 3; ++i) {
        small.push_back({i, 30 + (i * 10), 100.0});
    }
    
    for (int i = 0; i < 3; ++i) {
        large.push_back({i, 30 + (i * 10), 100.0});
    }
    for (int i = 3; i < 30; ++i) {
        // Repeat same pattern to make larger sample with same composition
        large.push_back({i, 30 + ((i % 3) * 10), 100.0});
    }
    
    double dist_small = computeMahalanobisDistance(small);
    double dist_large = computeMahalanobisDistance(large);
    
    // Both should be finite
    EXPECT_TRUE(std::isfinite(dist_small));
    EXPECT_TRUE(std::isfinite(dist_large));
    
    // Distances should be in same ballpark (not wildly different)
    // They won't be identical due to empirical covariance, but should be comparable
    double ratio = dist_large / (dist_small + 1e-10);  // +eps to avoid div-by-zero
    EXPECT_GT(ratio, 0.5);  // At least 50% of small distance
    EXPECT_LT(ratio, 2.0);  // At most 2x the small distance
}

TEST(ComputeMahalanobisDistanceTest, DistanceComparisonConcordance) {
    // Two different parameter sets: one should have smaller Mahalanobis distance
    // if it's closer to the clinical observation
    
    // Scenario A: matches clinical (25% by 50, 45% by 70)
    std::vector<RunResult> scenarioA;
    for (int i = 0; i < 100; ++i) {
        if (i < 25) {
            scenarioA.push_back({i, 50 + (i % 15), 100.0});  // 25% by 50
        } else if (i < 45) {
            scenarioA.push_back({i, 70 + ((i - 25) % 15), 100.0});  // additional 20% by 70
        } else {
            scenarioA.push_back({i, -1, 0.0});
        }
    }
    
    // Scenario B: way off (100% by 30, 0% by 70)
    std::vector<RunResult> scenarioB;
    for (int i = 0; i < 100; ++i)
        scenarioB.push_back({i, 30, 100.0});
    
    double distA = computeMahalanobisDistance(scenarioA);
    double distB = computeMahalanobisDistance(scenarioB);
    
    // Scenario A (closer to clinical) should have smaller distance
    EXPECT_LT(distA, distB);
}
