#include <gtest/gtest.h>
#include "../src/domain/cell/InstabilityDeltas.h"
#include "../src/domain/cell/AgenticCell.h"
#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/adapters/FixedNoise.h"

namespace domain {

/**
 * Test: Demonstrate custom delta calculation strategies using service injection
 */
class CustomDeltaCalculatorTest : public ::testing::Test {
protected:
    void TearDown() override {
        // Always reset calculator after each test
        InstabilityDeltas::resetCalculator();
    }
};

/// Custom calculator: D2 grows 10x faster than D1
class FastD2Calculator : public IInstabilityDeltaCalculator {
public:
    std::pair<double, double> calculate(
        const std::string& tp53_status,
        const std::string& brca1_status) const override {

        // D1 grows slowly
        double d1 = 0.0;
        if (tp53_status == "+/-") d1 = 0.1;
        if (tp53_status == "-/-") d1 = 0.2;

        // D2 grows 10x faster
        double d2 = 0.0;
        if (tp53_status == "+/-") d2 = 1.0;
        if (tp53_status == "-/-") d2 = 2.0;
        if (brca1_status == "+/-") d2 += 1.0;
        if (brca1_status == "-/-") d2 += 2.0;

        return {d1, d2};
    }
};

/// Custom calculator: Exponential growth based on combined mutations
class ExponentialCalculator : public IInstabilityDeltaCalculator {
public:
    std::pair<double, double> calculate(
        const std::string& tp53_status,
        const std::string& brca1_status) const override {

        int tp53_level = (tp53_status == "+/+") ? 0 : (tp53_status == "+/-") ? 1 : 2;
        int brca1_level = (brca1_status == "+/+") ? 0 : (brca1_status == "+/-") ? 1 : 2;

        // Exponential growth: 2^level
        double d1 = (tp53_level > 0) ? (0.1 * (1 << tp53_level)) : 0.0;  // 0, 0.2, 0.4
        double d2 = (tp53_level + brca1_level > 0) ? (0.1 * (1 << (tp53_level + brca1_level))) : 0.0;

        return {d1, d2};
    }
};

TEST_F(CustomDeltaCalculatorTest, LegacyAPIStillWorks) {
    // Test backward compatibility: calling with explicit coefficients (2 params)
    auto [d1, d2] = InstabilityDeltas::getDeltas(
        "-/-", "+/-",  // TP53 -/-, BRCA1 +/-
        0.001, 0.003   // low_delta=0.001, high_delta=0.003
    );

    // TP53 -/- → d1 = 0.003 (high)
    // TP53 -/- + BRCA1 +/- → d2 = 0.003 + 0.001 = 0.004
    EXPECT_DOUBLE_EQ(d1, 0.003);
    EXPECT_DOUBLE_EQ(d2, 0.004);
}

TEST_F(CustomDeltaCalculatorTest, ServiceBasedAPIWithCustomCalculator) {
    // Inject custom calculator
    InstabilityDeltas::setCalculator(std::make_unique<FastD2Calculator>());

    // Call service-based API (no coefficients needed)
    auto [d1, d2] = InstabilityDeltas::getDeltas("-/-", "+/-");

    // FastD2Calculator returns: d1=0.2, d2=2.0+1.0=3.0
    EXPECT_DOUBLE_EQ(d1, 0.2);
    EXPECT_DOUBLE_EQ(d2, 3.0);
}

TEST_F(CustomDeltaCalculatorTest, ExponentialGrowthCalculator) {
    // Inject exponential calculator
    InstabilityDeltas::setCalculator(std::make_unique<ExponentialCalculator>());

    // Test various combinations
    auto [d1_1, d2_1] = InstabilityDeltas::getDeltas("+/-", "+/+");
    EXPECT_DOUBLE_EQ(d1_1, 0.2);  // 2^1 * 0.1
    EXPECT_DOUBLE_EQ(d2_1, 0.2);  // 2^1 * 0.1

    auto [d1_2, d2_2] = InstabilityDeltas::getDeltas("-/-", "+/-");
    EXPECT_DOUBLE_EQ(d1_2, 0.4);  // 2^2 * 0.1
    EXPECT_DOUBLE_EQ(d2_2, 0.8);  // 2^3 * 0.1

    auto [d1_3, d2_3] = InstabilityDeltas::getDeltas("-/-", "-/-");
    EXPECT_DOUBLE_EQ(d1_3, 0.4);  // 2^2 * 0.1
    EXPECT_DOUBLE_EQ(d2_3, 1.6);  // 2^4 * 0.1
}

TEST_F(CustomDeltaCalculatorTest, ResetCalculatorRestoresDefaultBehavior) {
    // Set custom calculator
    InstabilityDeltas::setCalculator(std::make_unique<FastD2Calculator>());

    auto [d1_custom, d2_custom] = InstabilityDeltas::getDeltas("-/-", "+/-");
    EXPECT_DOUBLE_EQ(d1_custom, 0.2);
    EXPECT_DOUBLE_EQ(d2_custom, 3.0);

    // Reset to default
    InstabilityDeltas::resetCalculator();

    // Now falls back to default coefficients (DELTA_LOW=0.001, DELTA_HIGH=0.003)
    auto [d1_default, d2_default] = InstabilityDeltas::getDeltas("-/-", "+/-");
    EXPECT_DOUBLE_EQ(d1_default, 0.003);  // TP53 -/- with high=0.003
    EXPECT_DOUBLE_EQ(d2_default, 0.004);  // TP53 -/- (0.003) + BRCA1 +/- (0.001)
}

TEST_F(CustomDeltaCalculatorTest, CoefficientBasedCalculatorWithFineTuning) {
    // Create a coefficient-based calculator with specific values per state
    using GeneCoeffs = CoefficientBasedCalculator::GeneCoefficients;

    // TP53 affects D1: +/+ → 0, +/- → 0.5, -/- → 1.0
    GeneCoeffs tp53_for_d1{0.0, 0.5, 1.0};

    // TP53 affects D2: +/+ → 0, +/- → 0.2, -/- → 0.4
    GeneCoeffs tp53_for_d2{0.0, 0.2, 0.4};

    // BRCA1 affects D2: +/+ → 0, +/- → 0.3, -/- → 0.9
    GeneCoeffs brca1_for_d2{0.0, 0.3, 0.9};

    InstabilityDeltas::setCalculator(
        std::make_unique<CoefficientBasedCalculator>(
            tp53_for_d1, tp53_for_d2, brca1_for_d2
        )
    );

    // Test: TP53 +/-, BRCA1 +/-
    auto [d1_1, d2_1] = InstabilityDeltas::getDeltas("+/-", "+/-");
    EXPECT_DOUBLE_EQ(d1_1, 0.5);   // TP53 +/- → 0.5
    EXPECT_DOUBLE_EQ(d2_1, 0.5);   // TP53 +/- (0.2) + BRCA1 +/- (0.3)

    // Test: TP53 -/-, BRCA1 -/-
    auto [d1_2, d2_2] = InstabilityDeltas::getDeltas("-/-", "-/-");
    EXPECT_DOUBLE_EQ(d1_2, 1.0);   // TP53 -/- → 1.0
    EXPECT_DOUBLE_EQ(d2_2, 1.3);   // TP53 -/- (0.4) + BRCA1 -/- (0.9)
}

} // namespace domain

