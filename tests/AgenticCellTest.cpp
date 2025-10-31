#include <gtest/gtest.h>
#include "../src/domain/cell/AgenticCell.h"

using namespace domain;

class DummyNoise : public INoiseSource {
public:
    static double getNoise() { return 0.0; }
    domain::CellNoise next() override { return domain::CellNoise{0.0}; }
};

class HighNoise : public INoiseSource {
public:
    domain::CellNoise next() override { return domain::CellNoise{0.5}; }
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

TEST(AgenticCellTest, LiveMutatesGenes) {
    HighNoise noise;
    AgenticCellParams params;
    params.TP53 = Gene::State::PlusPlus;
    params.BRCA1 = Gene::State::PlusMinus;
    AgenticCell cell(noise, params);
    cell.live();
    EXPECT_EQ(cell.getTP53(), "+/-"); // TP53 muta de +/+ a +/-
    EXPECT_EQ(cell.getBRCA1(), "-/-"); // BRCA1 muta de +/- a -/-
}

TEST(AgenticCellTest, LiveMutatesGenesWithCustomThreshold) {
    HighNoise noise;
    AgenticCellParams params;
    params.TP53 = Gene::State::PlusPlus;
    params.BRCA1 = Gene::State::PlusMinus;
    // Umbral personalizado para mutación
    double threshold = 0.3;
    // Construcción manual de genes para test
    Gene tp53(&noise, params.TP53, threshold);
    Gene brca1(&noise, params.BRCA1, threshold);
    // Antes de live
    EXPECT_EQ(tp53.status(), "+/+");
    EXPECT_EQ(brca1.status(), "+/-");
    // Mutan porque noise=0.5 > threshold=0.3
    tp53.live();
    brca1.live();
    EXPECT_EQ(tp53.status(), "+/-");
    EXPECT_EQ(brca1.status(), "-/-");
    // El threshold se puede consultar
    EXPECT_DOUBLE_EQ(tp53.getMutationThreshold(), threshold);
    EXPECT_DOUBLE_EQ(brca1.getMutationThreshold(), threshold);
}
