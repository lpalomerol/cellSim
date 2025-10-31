#include <gtest/gtest.h>
#include "../src/domain/gene/Gene.h"

using domain::Gene;
using domain::INoiseSource;

class DummyNoise : public INoiseSource {
public:
    double value;
    DummyNoise(double v) : value(v) {}
    domain::CellNoise next() override { return domain::CellNoise{value}; }
};

TEST(GeneTest, DefaultConstructorInitializesPlusPlus) {
    DummyNoise noise(0.0);
    Gene g(&noise);
    EXPECT_TRUE(g.enabled());
}

TEST(GeneTest, ConstructorInitializesState) {
    DummyNoise noise(0.0);
    Gene g1(&noise, Gene::State::PlusPlus);
    Gene g2(&noise, Gene::State::PlusMinus);
    Gene g3(&noise, Gene::State::MinusMinus);
    EXPECT_TRUE(g1.enabled());
    EXPECT_TRUE(g2.enabled());
    EXPECT_FALSE(g3.enabled());
}

TEST(GeneTest, MutateCyclesStates) {
    DummyNoise noise(0.0);
    Gene g(&noise, Gene::State::PlusPlus);
    g.mutate();
    EXPECT_TRUE(g.enabled()); // PlusMinus
    g.mutate();
    EXPECT_FALSE(g.enabled()); // MinusMinus
    g.mutate();
    EXPECT_TRUE(g.enabled()); // PlusPlus
}

TEST(GeneTest, EnabledReturnsCorrectValue) {
    DummyNoise noise(0.0);
    Gene g1(&noise, Gene::State::PlusPlus);
    Gene g2(&noise, Gene::State::PlusMinus);
    Gene g3(&noise, Gene::State::MinusMinus);
    EXPECT_TRUE(g1.enabled());
    EXPECT_TRUE(g2.enabled());
    EXPECT_FALSE(g3.enabled());
}

TEST(GeneTest, LiveDoesNotMutateIfNoiseBelowThreshold) {
    DummyNoise noise(0.05); // por debajo de 0.1
    Gene g(&noise, Gene::State::PlusPlus);
    g.live();
    EXPECT_EQ(g.status(), "+/+");
}

TEST(GeneTest, LiveMutatesIfNoiseAboveThreshold) {
    DummyNoise noise(0.5); // por encima de 0.1
    Gene g(&noise, Gene::State::PlusPlus);
    g.live();
    EXPECT_EQ(g.status(), "+/-");
}
