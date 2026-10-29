#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>

#include "../src/domain/cell/strategies/LognormalNoiseInstabilityStrategy.h"
#include "../src/domain/cell/strategies/IInstabilityDeltaStrategy.h"
#include "../src/domain/cell/model/InstabilityDeltas.h"
#include "../src/domain/ports/ICell.h"
#include "../src/domain/shared/CellLifeStage.h"
#include "../src/domain/adapters/RandomNoise.h"
#include "FakeNoise.h"

// ---- Stubs ----

class StubDeltaStrategy : public domain::IInstabilityDeltaStrategy {
public:
    StubDeltaStrategy(double d1, double d2) : d1_(d1), d2_(d2) {}
    [[nodiscard]] domain::InstabilityDeltas calculateDeltas(
        const domain::ICell&, domain::INoiseSource*) const override {
        return domain::InstabilityDeltas::create(d1_, d2_);
    }
private:
    double d1_, d2_;
};

class StubCell : public domain::ICell {
public:
    void live() override {}
    [[nodiscard]] bool alive() const override { return true; }
    [[nodiscard]] std::string getBRCA1Status() const override { return "+/-"; }
    [[nodiscard]] std::string getTP53Status() const override { return "+/+"; }
    [[nodiscard]] bool isNeoplastic() const override { return false; }
    [[nodiscard]] double getD1() const override { return 1.0; }
    [[nodiscard]] double getD2() const override { return 1.0; }
    [[nodiscard]] domain::CellLifeStage getCurrentCellLifeStage() const override {
        return domain::CellLifeStage::BASELINE;
    }
    [[nodiscard]] bool hasBRCA1Mutation() const override { return false; }
    [[nodiscard]] bool hasTP53Function() const override { return true; }
    [[nodiscard]] std::uint64_t getAge() const override { return 0; }
    [[nodiscard]] bool hasEvadedApoptosis() const override { return false; }
    std::unique_ptr<domain::ICell> takePendingDaughter() override { return nullptr; }
    void mutateGene(const std::string&) override {}
};

// ---- Tests ----

TEST(LognormalNoiseInstabilityStrategyTest, PassthroughWhenNoiseIsNull) {
    StubCell cell;
    domain::LognormalNoiseInstabilityStrategy strategy(
        std::make_unique<StubDeltaStrategy>(0.1, 0.2), 0.5);

    auto deltas = strategy.calculateDeltas(cell, nullptr);

    EXPECT_DOUBLE_EQ(deltas.d1(), 0.1);
    EXPECT_DOUBLE_EQ(deltas.d2(), 0.2);
}

TEST(LognormalNoiseInstabilityStrategyTest, PassthroughWhenCVIsZero) {
    StubCell cell;
    FakeNoise noise({domain::CellNoise{0.5}, domain::CellNoise{0.3},
                     domain::CellNoise{0.5}, domain::CellNoise{0.3}});
    domain::LognormalNoiseInstabilityStrategy strategy(
        std::make_unique<StubDeltaStrategy>(0.1, 0.2), 0.0);

    auto deltas = strategy.calculateDeltas(cell, &noise);

    EXPECT_DOUBLE_EQ(deltas.d1(), 0.1);
    EXPECT_DOUBLE_EQ(deltas.d2(), 0.2);
}

TEST(LognormalNoiseInstabilityStrategyTest, OutputDiffersFromDeterministicWithNoise) {
    StubCell cell;
    // Box-Muller with u1=0.5, u2=0.5 → z = -sqrt(2*ln2) ≈ -1.1774
    // cv=0.5 → sigma_log≈0.4724 → noisy ≈ 0.513 * deterministic
    FakeNoise noise({domain::CellNoise{0.5}, domain::CellNoise{0.5},
                     domain::CellNoise{0.5}, domain::CellNoise{0.5}});
    domain::LognormalNoiseInstabilityStrategy strategy(
        std::make_unique<StubDeltaStrategy>(0.1, 0.2), 0.5);

    auto deltas = strategy.calculateDeltas(cell, &noise);

    EXPECT_NE(deltas.d1(), 0.1);
    EXPECT_NE(deltas.d2(), 0.2);
    EXPECT_GT(deltas.d1(), 0.0);
    EXPECT_GT(deltas.d2(), 0.0);
    // verify exact Box-Muller result
    const double sigma_log = std::sqrt(std::log(1.0 + 0.5 * 0.5));
    const double z = std::sqrt(-2.0 * std::log(0.5)) * std::cos(2.0 * M_PI * 0.5);
    const double expected_d1 = 0.1 * std::exp(sigma_log * z - 0.5 * sigma_log * sigma_log);
    EXPECT_NEAR(deltas.d1(), expected_d1, 1e-12);
}

TEST(LognormalNoiseInstabilityStrategyTest, ZeroDeltaRemainsZeroAfterNoise) {
    StubCell cell;
    FakeNoise noise({domain::CellNoise{0.5}, domain::CellNoise{0.5},
                     domain::CellNoise{0.5}, domain::CellNoise{0.5}});
    domain::LognormalNoiseInstabilityStrategy strategy(
        std::make_unique<StubDeltaStrategy>(0.0, 0.2), 0.5);

    auto deltas = strategy.calculateDeltas(cell, &noise);

    EXPECT_DOUBLE_EQ(deltas.d1(), 0.0);
    EXPECT_GT(deltas.d2(), 0.0);
}

TEST(LognormalNoiseInstabilityStrategyTest, OutputAlwaysPositive) {
    StubCell cell;
    domain::adapters::RandomNoise noise(42);
    domain::LognormalNoiseInstabilityStrategy strategy(
        std::make_unique<StubDeltaStrategy>(0.1, 0.2), 0.5);

    for (int i = 0; i < 100; ++i) {
        auto deltas = strategy.calculateDeltas(cell, &noise);
        EXPECT_GT(deltas.d1(), 0.0) << "iteration " << i;
        EXPECT_GT(deltas.d2(), 0.0) << "iteration " << i;
    }
}

TEST(LognormalNoiseInstabilityStrategyTest, EmpiricalMeanApproxDeterministic) {
    StubCell cell;
    constexpr double det_d1 = 0.1;
    constexpr double det_d2 = 0.2;
    constexpr int n = 1000;
    domain::adapters::RandomNoise noise(12345);
    domain::LognormalNoiseInstabilityStrategy strategy(
        std::make_unique<StubDeltaStrategy>(det_d1, det_d2), 0.5);

    double sum_d1 = 0.0, sum_d2 = 0.0;
    for (int i = 0; i < n; ++i) {
        auto deltas = strategy.calculateDeltas(cell, &noise);
        sum_d1 += deltas.d1();
        sum_d2 += deltas.d2();
    }

    EXPECT_NEAR(sum_d1 / n, det_d1, det_d1 * 0.2);
    EXPECT_NEAR(sum_d2 / n, det_d2, det_d2 * 0.2);
}

TEST(LognormalNoiseInstabilityStrategyTest, HigherCVProducesHigherVariance) {
    StubCell cell;
    constexpr int n = 200;

    auto stddev = [&](double cv, std::uint64_t seed) {
        domain::adapters::RandomNoise noise(seed);
        domain::LognormalNoiseInstabilityStrategy strategy(
            std::make_unique<StubDeltaStrategy>(0.1, 0.2), cv);
        std::vector<double> vals;
        vals.reserve(n);
        for (int i = 0; i < n; ++i) {
            vals.push_back(strategy.calculateDeltas(cell, &noise).d1());
        }
        double mean = std::accumulate(vals.begin(), vals.end(), 0.0) / n;
        double var = 0.0;
        for (double v : vals) var += (v - mean) * (v - mean);
        return std::sqrt(var / n);
    };

    EXPECT_GT(stddev(0.5, 42), stddev(0.1, 42));
}
