#include <gtest/gtest.h>
#include <memory>
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
    Gene tp53("TP53", Gene::State::PlusPlus);
    Gene brca1("BRCA1", Gene::State::PlusMinus);
    std::unordered_map<std::string, Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    Genome genome(genes);
    AgenticCell cell(std::make_unique<DummyNoise>(), genome);
    EXPECT_EQ(cell.getTP53(), "+/+");
    EXPECT_EQ(cell.getBRCA1(), "+/-");
}

TEST(AgenticCellTest, CustomStates) {
    Gene tp53("TP53", Gene::State::MinusMinus);
    Gene brca1("BRCA1", Gene::State::PlusPlus);
    std::unordered_map<std::string, Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    Genome genome(genes);
    AgenticCell cell(std::make_unique<DummyNoise>(), genome);
    EXPECT_EQ(cell.getTP53(), "-/-");
    EXPECT_EQ(cell.getBRCA1(), "+/+");
}

TEST(AgenticCellTest, AllStates) {
    Gene tp53("TP53", Gene::State::PlusMinus);
    Gene brca1("BRCA1", Gene::State::MinusMinus);
    std::unordered_map<std::string, Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    Genome genome(genes);
    AgenticCell cell(std::make_unique<DummyNoise>(), genome);
    EXPECT_EQ(cell.getTP53(), "+/-");
    EXPECT_EQ(cell.getBRCA1(), "-/-");
}

TEST(AgenticCellTest, LiveMutatesGenes) {
    Gene tp53("TP53", Gene::State::PlusPlus);
    Gene brca1("BRCA1", Gene::State::PlusMinus);
    std::unordered_map<std::string, Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    Genome genome(genes);
    AgenticCell cell(std::make_unique<HighNoise>(), genome);
    cell.live();
    EXPECT_EQ(cell.getTP53(), "+/-");
    EXPECT_EQ(cell.getBRCA1(), "-/-");
}

TEST(AgenticCellTest, LiveMutatesGenesWithCustomThreshold) {
    double threshold = 0.3;
    Gene tp53("TP53", Gene::State::PlusPlus, threshold);
    Gene brca1("BRCA1", Gene::State::PlusMinus, threshold);
    std::unordered_map<std::string, Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    Genome genome(genes);
    AgenticCell cell(std::make_unique<HighNoise>(), genome);
    EXPECT_EQ(cell.getTP53(), "+/+");
    EXPECT_EQ(cell.getBRCA1(), "+/-");
    cell.live();
    EXPECT_EQ(cell.getTP53(), "+/-");
    EXPECT_EQ(cell.getBRCA1(), "-/-");
}

TEST(AgenticCellTest, AliveWhenBRCA1IsPlusMinus) {
    Gene tp53("TP53", Gene::State::PlusPlus);
    Gene brca1("BRCA1", Gene::State::PlusMinus);
    std::unordered_map<std::string, Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    Genome genome(genes);
    AgenticCell cell(std::make_unique<DummyNoise>(), genome);
    EXPECT_TRUE(cell.alive());
}

TEST(AgenticCellTest, DeadWhenBRCA1IsMinusMinus) {
    Gene tp53("TP53", Gene::State::PlusPlus);
    Gene brca1("BRCA1", Gene::State::MinusMinus);
    std::unordered_map<std::string, Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    Genome genome(genes);
    AgenticCell cell(std::make_unique<DummyNoise>(), genome);
    EXPECT_FALSE(cell.alive());
}

TEST(AgenticCellTest, LiveDisablesCellWhenBRCA1Mutates) {
    Gene tp53("TP53", Gene::State::PlusPlus);
    Gene brca1("BRCA1", Gene::State::PlusMinus); // empieza viva
    std::unordered_map<std::string, Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    Genome genome(genes);
    AgenticCell cell(std::make_unique<HighNoise>(), genome);
    EXPECT_TRUE(cell.alive());
    cell.live();
    EXPECT_FALSE(cell.alive()); // ahora BRCA1 debería ser -/- y la célula está disabled
}

TEST(AgenticCellTest, NoTumoralWhenTP53IsMinusMinusByDefault) {
    Gene tp53("TP53", Gene::State::MinusMinus);
    Gene brca1("BRCA1", Gene::State::PlusMinus);
    std::unordered_map<std::string, Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    Genome genome(genes);
    AgenticCell cell(std::make_unique<DummyNoise>(), genome);
    EXPECT_EQ(cell.getTP53(), "-/-");
    EXPECT_FALSE(cell.isNeoplastic());
}

TEST(AgenticCellTest, TumorProtectedByActiveTP53) {
    Gene tp53("TP53", Gene::State::PlusPlus, 1.0); // TP53 activo protege, threshold alto para evitar mutación
    Gene brca1("BRCA1", Gene::State::PlusMinus);
    std::unordered_map<std::string, Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    Genome genome(genes);
    AgenticCell cell(std::make_unique<HighNoise>(), genome);
    EXPECT_FALSE(cell.isNeoplastic());
    cell.live();
    EXPECT_FALSE(cell.isNeoplastic()); // protegido por TP53 activo
}

TEST(AgenticCellTest, TumoralWhenTP53InitiallyInactiveWithHighK) {
    Gene tp53("TP53", Gene::State::MinusMinus, 1.0); // TP53 inactivo, threshold alto para evitar mutación
    Gene brca1("BRCA1", Gene::State::PlusMinus);
    std::unordered_map<std::string, Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    Genome genome(genes);
    AgenticCell cell(std::make_unique<HighNoise>(), genome, 1.0);
    EXPECT_EQ(cell.getTP53(), "-/-");
    EXPECT_FALSE(cell.isNeoplastic());
    cell.live();
    EXPECT_TRUE(cell.isNeoplastic());
}

TEST(AgenticCellTest, LiveMakesCellTumoralWhenTP53MutatesAndKHigh) {
    Gene tp53("TP53", Gene::State::PlusMinus);
    Gene brca1("BRCA1", Gene::State::PlusMinus);
    std::unordered_map<std::string, Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    Genome genome(genes);
    AgenticCell cell(std::make_unique<HighNoise>(), genome, 1.0);
    EXPECT_EQ(cell.getTP53(), "+/-");
    EXPECT_FALSE(cell.isNeoplastic());
    cell.live();
    EXPECT_EQ(cell.getTP53(), "-/-");
    EXPECT_TRUE(cell.isNeoplastic());
}
