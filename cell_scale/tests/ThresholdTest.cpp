#include <gtest/gtest.h>
#include <stdexcept>

#include "../src/domain/shared/Threshold.h"

using domain::shared::Threshold;

TEST(ThresholdTest, ConstructorClampsToUpper) {
    Threshold t(1.5); // default limits [0,1]
    EXPECT_DOUBLE_EQ(t.value(), 1.0);
}

TEST(ThresholdTest, ConstructorClampsToLower) {
    Threshold t(-0.5);
    EXPECT_DOUBLE_EQ(t.value(), 0.0);
}

TEST(ThresholdTest, InvalidLimitsThrow) {
    EXPECT_THROW(Threshold(0.5, 1.0, 0.0), std::invalid_argument);
}

TEST(ThresholdTest, ArithmeticOperatorsClamp) {
    Threshold t(0.5);
    t += 0.6; // becomes 1.1 -> clamped to 1.0
    EXPECT_DOUBLE_EQ(t.value(), 1.0);
    t -= 0.3; // 0.7
    EXPECT_DOUBLE_EQ(t.value(), 0.7);
    t -= 1.0; // -0.3 -> clamped to 0.0
    EXPECT_DOUBLE_EQ(t.value(), 0.0);
}

TEST(ThresholdTest, Comparisons) {
    Threshold a(0.2);
    Threshold b(0.7);
    Threshold c(0.2);

    EXPECT_TRUE(a < b);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(a <= c);
    EXPECT_TRUE(a == c);
    EXPECT_TRUE(a != b);
}

TEST(ThresholdTest, MultiplicativeOperatorsClamp) {
    Threshold t(0.6);
    t *= 2.0; // 1.2 -> clamped to 1.0
    EXPECT_DOUBLE_EQ(t.value(), 1.0);

    Threshold u(0.5);
    Threshold v = u * 0.5; // 0.25
    EXPECT_DOUBLE_EQ(v.value(), 0.25);

    Threshold w = 3.0 * u; // 1.5 -> clamped to 1.0
    EXPECT_DOUBLE_EQ(w.value(), 1.0);
}


// EOF
