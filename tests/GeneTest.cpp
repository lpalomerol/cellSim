#include "../src/domain/gene/Gene.h"
#include "FakeNoise.h"
#include <gtest/gtest.h>

using domain::Gene;

TEST(GeneTest, ConstructorSetsInstabilityK) {
    FakeNoise noise({domain::CellNoise{0.5}}); // Valor arbitrario
    Gene gene(&noise, Gene::State::PlusPlus, 0.1, 0.2);
    EXPECT_DOUBLE_EQ(gene.getMutationInstabilityK(), 0.2);
    EXPECT_DOUBLE_EQ(gene.getMutationThreshold(), 0.1);
}

TEST(GeneTest, LiveMutatesWhenNoiseExceedsThresholdPlusK) {
    FakeNoise noise({domain::CellNoise{0.31}}); // 0.31 > 0.3 (0.1 + 0.2)
    Gene gene(&noise, Gene::State::PlusPlus, 0.1, 0.2);
    gene.live();
    EXPECT_EQ(gene.status(), "+/-");
}

TEST(GeneTest, LiveDoesNotMutateWhenNoiseBelowThresholdPlusK) {
    FakeNoise noise({domain::CellNoise{0.29}}); // 0.29 < 0.3 (0.1 + 0.2)
    Gene gene(&noise, Gene::State::PlusPlus, 0.1, 0.2);
    gene.live();
    EXPECT_EQ(gene.status(), "+/+");
}

TEST(GeneTest, MutateCyclesStates) {
    FakeNoise noise({domain::CellNoise{1.0}, domain::CellNoise{1.0}, domain::CellNoise{1.0}}); // Siempre muta
    Gene gene(&noise, Gene::State::PlusPlus, 0.0, 0.0);
    gene.live();
    EXPECT_EQ(gene.status(), "+/-");
    gene.live();
    EXPECT_EQ(gene.status(), "-/-");
    gene.live();
    EXPECT_EQ(gene.status(), "+/+");
}

TEST(GeneTest, EnabledReturnsCorrectValue) {
    FakeNoise noise({domain::CellNoise{1.0}});
    Gene gene(&noise, Gene::State::PlusPlus, 0.0, 0.0);
    EXPECT_TRUE(gene.enabled());
    gene.mutate();
    EXPECT_TRUE(gene.enabled());
    gene.mutate();
    EXPECT_FALSE(gene.enabled());
}
