#include <gtest/gtest.h>
#include "FakeNoise.h"

#include "../src/domain/ports/CellNoise.h"
#include "../src/domain/cell/GeneCell.h"

TEST(GeneCell, S0_to_S1_then_to_Tumoral_with_two_tp53_hits) {
    // p_tp53 = 0.4, p_brca = 0.1
    domain::GeneCellParams params;
    params.p_mutation_tp53 = 0.1;
    params.p_mutation_brca = 0.1;
    // 0 -> 0.8 S0, 0.8->0.9 S1, 0.9->1.0 Apoptotic

    std::vector<domain::CellNoise> seq = {
        {0.85}, // tick 1: u=0.05 → S0->S1
        {0.85}, // tick 2: u=0.05 → S1->Tumoral
    };

    FakeNoise noise(seq);

    domain::GeneCell cell(noise, params);

    // Tick 1 => sigue viva (S1 mapea a Alive)
    cell.live();

    EXPECT_EQ(cell.state(), domain::CellState::Alive);
    EXPECT_TRUE(cell.alive());

    // Tick 2 => Tumoral (absorbente)
    cell.live();
    EXPECT_EQ(cell.state(), domain::CellState::Alive);
    EXPECT_EQ(cell.getOncoState(), domain::OncoState::Tumoral);
    EXPECT_TRUE(cell.alive()); // si Tumoral se considera "viva"
}
TEST(GeneCell, Becomes_Apoptotic_with_BRCA_mutation) {
    domain::GeneCellParams params;
    params.p_mutation_tp53 = 0.0;
    params.p_mutation_brca = 1.0; // Fuerza mutación BRCA

    std::vector<domain::CellNoise> seq = {
        {0.5}, // tick 1: cualquier valor, BRCA muta seguro
    };

    FakeNoise noise(seq);
    domain::GeneCell cell(noise, params);

    cell.live();

    EXPECT_EQ(cell.state(), domain::CellState::Apoptotic);
    EXPECT_FALSE(cell.alive());
}