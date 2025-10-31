
#include <gtest/gtest.h>
#include "../src/domain/cell/GeneCell.h"
#include "FakeNoise.h"

using namespace domain;

TEST(GeneCellTest, DoesNotMutateBRCA1WhenNoiseExceedsThreshold) {
    GeneCellParams params;
    FakeNoise noise({ CellNoise{0.1, 0.1, 0.9, .9} });

    GeneCell cell(noise, params, Gene::PartiallyDisabled, Gene::Active);
    cell.live();
    EXPECT_TRUE(cell.alive());
    EXPECT_TRUE(cell.state() == CellState::Alive);
    EXPECT_TRUE(cell.getBRCA1().getState() == Gene::PartiallyDisabled);
    EXPECT_TRUE(cell.getTP53().getState() == Gene::Active);

}

TEST(GeneCellTest, MutatesAndDiesBRCA1WhenNoiseExceedsThreshold) {
    GeneCellParams params;
    params.p_mutation_brca = 0.3;
    FakeNoise noise({ CellNoise{0.1, 0.1, 0.0001, .99} });

    GeneCell cell(noise, params, Gene::PartiallyDisabled, Gene::Active);
    cell.live();
    EXPECT_FALSE(cell.alive());
    EXPECT_TRUE(cell.state() == CellState::Apoptotic);
    EXPECT_TRUE(cell.getBRCA1().getState() == Gene::Disabled);
    EXPECT_TRUE(cell.getTP53().getState() == Gene::Active);

}


TEST(GeneCell, AliveTransitionsToDamagedWithUBelowThreshold) {
    // Matriz: Alive->Damaged = 0.5
    domain::TransitionMatrix P = {{
        {{0.5, 0.5, 0}},
        {{0.0, 1.0, 0}},
        {{0.0, 0, 1.0}}
    }};
    GeneCellParams cfg;
    cfg.base_matrix = P;

    FakeNoise noise({ CellNoise{0.6,0,0,0} }); // u=0.4 < 0.5 → Damaged
    domain::GeneCell cell(noise, cfg);
    cell.live();
    EXPECT_EQ(cell.state(), domain::CellState::Damaged);
}
