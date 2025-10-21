// tests/SimpleCellTests.cpp
#include <gtest/gtest.h>
#include "FakeNoise.h"
#include "../src/domain/ports/CellNoise.h"
#include "../src/domain/cell/SimpleCell.h"


using domain::CellNoise;
using domain::SimpleCell;

TEST(SimpleCell, StaysAliveWithHighHomeostasisAndLowNoise) {
    // u=0.1 < p=0.95 => vive
    FakeNoise noise({ CellNoise{0.1} });
    domain::SimpleCellParams cfg;
    cfg.p_homeostasis = 0.95;

    SimpleCell cell(noise, cfg);
    EXPECT_TRUE(cell.alive());
    cell.live();
    EXPECT_TRUE(cell.alive());
}

TEST(SimpleCell, DiesWhenNoiseExceedsHomeostasisProb) {
    // u=0.99 >= p=0.2 => muere (según la regla asumida)
    FakeNoise noise({ CellNoise{0.99} });
    domain::SimpleCellParams cfg;
    cfg.p_homeostasis = 0.2;

    SimpleCell cell(noise, cfg);
    EXPECT_TRUE(cell.alive());
    cell.live();
    EXPECT_FALSE(cell.alive());
}

TEST(SimpleCell, DeterministicAcrossStepsWithSequence) {
    // Primera vive (0.0 < 0.5), segunda muere (0.9 >= 0.5)
    FakeNoise noise({
        CellNoise{0.0},
        CellNoise{0.9}
    });
    domain::SimpleCellParams cfg;
    cfg.p_homeostasis = 0.5;

    SimpleCell cell(noise, cfg);
    EXPECT_TRUE(cell.alive());
    cell.live(); // u=0.0 -> vive
    EXPECT_TRUE(cell.alive());
    cell.live(); // u=0.9 -> muere
    EXPECT_FALSE(cell.alive());
}