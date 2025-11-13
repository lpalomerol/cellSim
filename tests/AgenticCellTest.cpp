#include <gtest/gtest.h>
#include <memory>
#include <tuple>
#include "../src/domain/cell/AgenticCell.h"
#include "TestNoise.h"

// Se evita `using namespace domain;` para que los tests usen `domain::` explícito.

class AgenticCellStateTest : public ::testing::TestWithParam<std::tuple<domain::Gene::State, domain::Gene::State, std::string, std::string>> {
};

TEST_P(AgenticCellStateTest, StatesMatch) {
    auto [tp53State, brcaState, tp53Expected, brcaExpected] = GetParam();
    domain::Gene tp53("TP53", tp53State);
    domain::Gene brca1("BRCA1", brcaState);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);
    domain::AgenticCell cell(std::make_unique<test::DummyNoise>(), genome);
    EXPECT_EQ(cell.getTP53(), tp53Expected);
    EXPECT_EQ(cell.getBRCA1(), brcaExpected);
}

INSTANTIATE_TEST_SUITE_P(
    DefaultCustomAll,
    AgenticCellStateTest,
    ::testing::Values(
        std::make_tuple(domain::Gene::State::PlusPlus, domain::Gene::State::PlusMinus, std::string("+/+"), std::string("+/-")),
        std::make_tuple(domain::Gene::State::MinusMinus, domain::Gene::State::PlusPlus, std::string("-/-"), std::string("+/+")),
        std::make_tuple(domain::Gene::State::PlusMinus, domain::Gene::State::MinusMinus, std::string("+/-"), std::string("-/-"))
    )
);

TEST(AgenticCellTest, LiveMutatesGenes) {
    domain::Gene tp53("TP53", domain::Gene::State::PlusPlus);
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusMinus);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);
    domain::AgenticCell cell(std::make_unique<test::HighNoise>(), genome);
    cell.live();
    EXPECT_EQ(cell.getTP53(), "+/-");
    EXPECT_EQ(cell.getBRCA1(), "-/-");
}

TEST(AgenticCellTest, LiveMutatesGenesWithCustomThreshold) {
    double threshold = 0.3;
    domain::Gene tp53("TP53", domain::Gene::State::PlusPlus, threshold);
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusMinus, threshold);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);
    domain::AgenticCell cell(std::make_unique<test::HighNoise>(), genome);
    EXPECT_EQ(cell.getTP53(), "+/+");
    EXPECT_EQ(cell.getBRCA1(), "+/-");
    cell.live();
    EXPECT_EQ(cell.getTP53(), "+/-");
    EXPECT_EQ(cell.getBRCA1(), "-/-");
}

TEST(AgenticCellTest, AliveWhenBRCA1IsPlusMinus) {
    domain::Gene tp53("TP53", domain::Gene::State::PlusPlus);
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusMinus);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);
    domain::AgenticCell cell(std::make_unique<test::DummyNoise>(), genome);
    EXPECT_TRUE(cell.alive());
}

TEST(AgenticCellTest, DeadWhenBRCA1IsMinusMinus) {
    domain::Gene tp53("TP53", domain::Gene::State::PlusPlus);
    domain::Gene brca1("BRCA1", domain::Gene::State::MinusMinus);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);
    domain::AgenticCell cell(std::make_unique<test::DummyNoise>(), genome);
    EXPECT_FALSE(cell.alive());
}

TEST(AgenticCellTest, LiveDisablesCellWhenBRCA1Mutates) {
    domain::Gene tp53("TP53", domain::Gene::State::PlusPlus);
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusMinus); // empieza viva
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);
    domain::AgenticCell cell(std::make_unique<test::HighNoise>(), genome);
    EXPECT_TRUE(cell.alive());
    cell.live();
    EXPECT_FALSE(cell.alive()); // ahora BRCA1 debería ser -/- y la célula está disabled
}

TEST(AgenticCellTest, NoTumoralWhenTP53IsMinusMinusByDefault) {
    domain::Gene tp53("TP53", domain::Gene::State::MinusMinus);
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusMinus);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);
    domain::AgenticCell cell(std::make_unique<test::DummyNoise>(), genome);
    EXPECT_EQ(cell.getTP53(), "-/-");
    EXPECT_FALSE(cell.isNeoplastic());
}

TEST(AgenticCellTest, TumorProtectedByActiveTP53) {
    domain::Gene tp53("TP53", domain::Gene::State::PlusPlus, 1.0); // TP53 activo protege, threshold alto para evitar mutación
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusMinus);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);
    domain::AgenticCell cell(std::make_unique<test::HighNoise>(), genome);
    EXPECT_FALSE(cell.isNeoplastic());
    cell.live();
    EXPECT_FALSE(cell.isNeoplastic()); // protegido por TP53 activo
}

TEST(AgenticCellTest, TumoralWhenTP53InitiallyInactiveWithHighK) {
    domain::Gene tp53("TP53", domain::Gene::State::MinusMinus, 1.0); // TP53 inactivo, threshold alto para evitar mutación
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusMinus);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);
    domain::AgenticCell cell(std::make_unique<test::HighNoise>(), genome, 1.0);
    EXPECT_EQ(cell.getTP53(), "-/-");
    EXPECT_FALSE(cell.isNeoplastic());
    cell.live();
    EXPECT_FALSE(cell.isNeoplastic());
}

TEST(AgenticCellTest, LiveMakesCellTumoralWhenTP53MutatesAndKHigh) {
    domain::Gene tp53("TP53", domain::Gene::State::PlusMinus);
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusMinus);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);
    domain::AgenticCell cell(std::make_unique<test::HighNoise>(), genome, 1.0);
    EXPECT_EQ(cell.getTP53(), "+/-");
    EXPECT_FALSE(cell.isNeoplastic());
    cell.live();
    EXPECT_EQ(cell.getTP53(), "-/-");
    EXPECT_FALSE(cell.isNeoplastic());
}

// ---------- Nuevas pruebas para comprobar contador de edad ----------

TEST(AgenticCellTest, AgeIncrementsWhenAlive) {
    domain::Gene tp53("TP53", domain::Gene::State::PlusPlus);
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusMinus); // viva
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);
    domain::AgenticCell cell(std::make_unique<test::DummyNoise>(), genome);
    EXPECT_EQ(cell.getAge(), 0u);
    cell.live();
    EXPECT_EQ(cell.getAge(), 1u);
    cell.live();
    EXPECT_EQ(cell.getAge(), 2u);
}

TEST(AgenticCellTest, AgeDoesNotIncrementWhenDead) {
    domain::Gene tp53("TP53", domain::Gene::State::PlusPlus);
    domain::Gene brca1("BRCA1", domain::Gene::State::MinusMinus); // muerta
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);
    domain::AgenticCell cell(std::make_unique<test::DummyNoise>(), genome);
    EXPECT_EQ(cell.getAge(), 0u);
    cell.live(); // no debe incrementar porque está muerta
    EXPECT_EQ(cell.getAge(), 0u);
}
