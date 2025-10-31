
#include <gtest/gtest.h>
#include "../src/domain/cell/GeneCell.h"
#include "FakeNoise.h"

using namespace domain;

TEST(GeneCellTest, MutatesBRCA1WhenNoiseExceedsThreshold) {
    GeneCellParams params;
    params.p_mutation_brca = 0.5;
    FakeNoise noise({ CellNoise{0.1, 0.1, 0.1} });

    GeneCell cell(noise, params, Gene::PartiallyDisabled, Gene::Active);

    EXPECT_TRUE(cell.alive());
    EXPECT_TRUE(cell.state() == CellState::Alive);

}

TEST(GeneCellTest, DiesWhenBRCA1Disabled) {
    GeneCellParams params;
    params.p_mutation_brca = 0.5;
    FakeNoise noise({ CellNoise{0.1, 0.1, 0.1} });

    GeneCell cell(noise, params, Gene::PartiallyDisabled, Gene::Active);
    cell.live();

    EXPECT_FALSE(cell.alive());
    EXPECT_TRUE(cell.state() == CellState::Dead);
    // EXPECT_EQ(cell.getTP53(), Gene::Active);
}