#include <gtest/gtest.h>
#include "../src/domain/cell/AgenticCell.h"

using namespace domain;

class DummyNoise : public INoiseSource {
public:
    static double getNoise() { return 0.0; }
    domain::CellNoise next() override { return domain::CellNoise{0.0}; }
};

TEST(AgenticCellTest, DefaultStates) {
    DummyNoise noise;
    AgenticCell cell(noise);
    EXPECT_EQ(cell.getTP53(), "+/+");
    EXPECT_EQ(cell.getBRCA1(), "+/-");
}

TEST(AgenticCellTest, CustomStates) {
    DummyNoise noise;
    AgenticCellParams params;
    params.TP53 = Gene::State::MinusMinus;
    params.BRCA1 = Gene::State::PlusPlus;
    AgenticCell cell(noise, params);
    EXPECT_EQ(cell.getTP53(), "-/-");
    EXPECT_EQ(cell.getBRCA1(), "+/+");
}

TEST(AgenticCellTest, AllStates) {
    DummyNoise noise;
    AgenticCellParams params;
    params.TP53 = Gene::State::PlusMinus;
    params.BRCA1 = Gene::State::MinusMinus;
    AgenticCell cell(noise, params);
    EXPECT_EQ(cell.getTP53(), "+/-");
    EXPECT_EQ(cell.getBRCA1(), "-/-");
}

