/**
 * @file TissueScaleSimulatorTest.cpp
 * @brief Unit and regression tests for tissue_scale::simulate_tissue.
 */

#include <gtest/gtest.h>
#include "../tissue_scale/src/TissueScaleSimulator.h"

using namespace tissue_scale;

class TissueScaleSimulatorTest : public ::testing::Test {
protected:
    TissueSimulationParameters baseline_params {
        2.0075832417423e-05, // mu_tp53
        0.00275,              // beta_brca1
        0.226663913638,       // delta_low
        1.4927594013876,      // delta_high
        13.64168215564,       // theta_d1
        31.78627057176,       // theta_d2_mean
        0.65,                 // tau_expansion
        0.38                  // sigma_theta_d2
    };
};

TEST_F(TissueScaleSimulatorTest, ExactNumericalEquivalenceAcrossSeeds) {
    // Seed 20260923 + i * 1000003ULL
    uint64_t base_seed = 20260923;

    // Tissue 0: Expect no tumor by age 80
    auto res0 = simulate_tissue(baseline_params, base_seed);
    EXPECT_EQ(res0.age_at_detection, -1);
    EXPECT_EQ(res0.total_founders, 0ULL);
    EXPECT_EQ(res0.null_founders, 0ULL);

    // Tissue 1: Detected at age 29 with 1 founder
    auto res1 = simulate_tissue(baseline_params, base_seed + 1000003ULL);
    EXPECT_EQ(res1.age_at_detection, 29);
    EXPECT_EQ(res1.total_founders, 1ULL);
    EXPECT_EQ(res1.null_founders, 1ULL);
    EXPECT_NEAR(res1.effective_founders, 1.0, 1e-6);

    // Tissue 2: Detected early at age 32 with 6 founders (polyclonal)
    auto res2 = simulate_tissue(baseline_params, base_seed + 2000006ULL);
    EXPECT_EQ(res2.age_at_detection, 32);
    EXPECT_EQ(res2.total_founders, 6ULL);
    EXPECT_EQ(res2.null_founders, 1ULL);
    EXPECT_EQ(res2.major_lineages_1pct, 3);
    EXPECT_EQ(res2.major_lineages_5pct, 2);
    EXPECT_NEAR(res2.effective_founders, 1.3781315, 1e-6);

    // Tissue 4: Detected at age 55 with 1 founder (monoclonal)
    auto res4 = simulate_tissue(baseline_params, base_seed + 4000012ULL);
    EXPECT_EQ(res4.age_at_detection, 55);
    EXPECT_EQ(res4.total_founders, 1ULL);
    EXPECT_EQ(res4.null_founders, 1ULL);
    EXPECT_NEAR(res4.effective_founders, 1.0, 1e-9);
}

TEST_F(TissueScaleSimulatorTest, HighRestraintThresholdPreventsTransformation) {
    // Setting theta_d1 and theta_d2 infinitely high should prevent any detection
    auto safe_params = baseline_params;
    safe_params.theta_d1 = 1e9;
    safe_params.theta_d2_mean = 1e9;

    auto res = simulate_tissue(safe_params, 12345, 100000ULL, 50);
    EXPECT_EQ(res.age_at_detection, -1);
    EXPECT_EQ(res.final_unmutated_cells + res.final_null_cells, 0.0L);
}

TEST_F(TissueScaleSimulatorTest, DamageAccumulationFormulaInvariant) {
    CellCohortKey key;
    key.d1_low_hits = 2;
    key.d1_high_hits = 3;
    key.d1_age_accum = 50;

    double expected_d1 = 1.0 + 2 * baseline_params.delta_low + 3 * baseline_params.delta_high + 1e-5 * 50;
    double computed_d1 = compute_d1(key, baseline_params, 1e-5);
    EXPECT_DOUBLE_EQ(computed_d1, expected_d1);

    key.d2_low_hits = 4;
    key.d2_high_hits = 1;
    key.d2_age_accum = 100;
    double expected_d2 = 1.0 + 4 * baseline_params.delta_low + 1 * baseline_params.delta_high + 1e-5 * 100;
    double computed_d2 = compute_d2(key, baseline_params, 1e-5);
    EXPECT_DOUBLE_EQ(computed_d2, expected_d2);
}
