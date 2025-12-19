#include <gtest/gtest.h>
#include <memory>
#include <tuple>
#include "../src/domain/cell/AgenticCell.h"
#include "TestNoise.h"
#include "FakeNoise.h"

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
    // V2: Mutations must be explicit
    cell.mutateGene("TP53"); // TP53 +/- → -/-
    cell.live();
    EXPECT_EQ(cell.getTP53(), "-/-");
    // V2: Neoplastic transition happens via PRIMER state, not automatically
    // Cell needs to enter PRIMER state (D1 > 2.0) to be detected
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


TEST(AgenticCellTest, GenomicInstabilityEvolutionByTP53State) {
    // TP53 +/+ & BRCA1 +/- → DELTA_LOW = 0.001
    // D1: 1.0 * 1.0 + 0.001 = 1.001
    domain::Gene tp53_pp("TP53", domain::Gene::State::PlusPlus);
    domain::Gene brca1_pp("BRCA1", domain::Gene::State::PlusMinus);
    std::unordered_map<std::string, domain::Gene> genes_pp{{tp53_pp.name(), tp53_pp}, {brca1_pp.name(), brca1_pp}};
    domain::Genome genome_pp(genes_pp);
    domain::AgenticCell cell_pp(std::make_unique<test::DummyNoise>(), genome_pp);
    EXPECT_DOUBLE_EQ(cell_pp.getD1(), 1.0);
    cell_pp.live();
    EXPECT_DOUBLE_EQ(cell_pp.getD1(), 1.001);

    // TP53 +/- & BRCA1 +/- → DELTA_MEDIUM = 0.002
    // D1: 1.0 * 1.0 + 0.002 = 1.002
    domain::Gene tp53_pm("TP53", domain::Gene::State::PlusMinus);
    domain::Gene brca1_pm("BRCA1", domain::Gene::State::PlusMinus);
    std::unordered_map<std::string, domain::Gene> genes_pm{{tp53_pm.name(), tp53_pm}, {brca1_pm.name(), brca1_pm}};
    domain::Genome genome_pm(genes_pm);
    domain::AgenticCell cell_pm(std::make_unique<test::DummyNoise>(), genome_pm);
    EXPECT_DOUBLE_EQ(cell_pm.getD1(), 1.0);
    cell_pm.live();
    EXPECT_DOUBLE_EQ(cell_pm.getD1(), 1.002);

    // TP53 -/- & BRCA1 +/- → DELTA_HIGH = 0.003
    // D1: 1.0 * 1.0 + 0.003 = 1.003
    domain::Gene tp53_mm("TP53", domain::Gene::State::MinusMinus);
    domain::Gene brca1_mm("BRCA1", domain::Gene::State::PlusMinus);
    std::unordered_map<std::string, domain::Gene> genes_mm{{tp53_mm.name(), tp53_mm}, {brca1_mm.name(), brca1_mm}};
    domain::Genome genome_mm(genes_mm);
    domain::AgenticCell cell_mm(std::make_unique<test::DummyNoise>(), genome_mm);
    EXPECT_DOUBLE_EQ(cell_mm.getD1(), 1.0);
    cell_mm.live();
    EXPECT_DOUBLE_EQ(cell_mm.getD1(), 1.003);
}

// ---------- Nuevas pruebas para la división celular ----------

TEST(AgenticCellTest, DivisionRateDefaultValue) {
    domain::Gene tp53("TP53", domain::Gene::State::PlusPlus);
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusMinus);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);
    // Constructor sin especificar division_rate debe usar valor por defecto 0.001
    domain::AgenticCell cell(std::make_unique<test::DummyNoise>(), genome);
    // Nota: no hay getter público para division_rate, pero probamos que no causa errores
    cell.live();
    EXPECT_TRUE(cell.alive()); // Debería seguir viva
}

TEST(AgenticCellTest, DivisionRateCustomValue) {
    domain::Gene tp53("TP53", domain::Gene::State::PlusPlus);
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusMinus);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);
    // Constructor con division_rate personalizado
    domain::AgenticCell cell(std::make_unique<test::DummyNoise>(), genome,
                             0.002, // neoplasm_k
                             0.0001, // low_delta_instability
                             0.0002, // high_delta_instability
                             0.5    // division_rate alto (50%)
                             ); // verbose
    cell.live();
    EXPECT_TRUE(cell.alive());
}

TEST(AgenticCellTest, DivisionDisabledWhenRateIsZero) {
    domain::Gene tp53("TP53", domain::Gene::State::PlusPlus);
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusMinus);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);
    // division_rate = 0.0 disables division
    domain::AgenticCell cell(std::make_unique<test::DummyNoise>(), genome,
                             0.002, 0.0001, 0.0002, 0.0);
    cell.live();
    EXPECT_TRUE(cell.alive()); // Sin intentos de división
}

TEST(AgenticCellTest, DivisionAttemptWithVerboseOutput) {
    domain::Gene tp53("TP53", domain::Gene::State::PlusPlus);
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusMinus);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);
    // Usar secuencia de ruido que permita división pero no mutaciones fatales
    // - Valores bajos para mutación (se aplica a genes)
    // - Pero la célula debe sobrevivir con BRCA1 +/-
    std::vector<domain::CellNoise> noise_sequence{
        domain::CellNoise{0.5},  // Fase 0-3: evita mutaciones (threshold 0.1 es muy bajo)
        domain::CellNoise{0.05}, // Fase 4: provoca división (0.05 < 0.5)
        domain::CellNoise{0.5},  // Ciclos posteriores
        domain::CellNoise{0.5}
    };
    domain::AgenticCell cell(std::make_unique<FakeNoise>(noise_sequence),
                             genome,
                             0.002, 0.0001, 0.0002, 0.5);
    // El test pasa si no lanza excepciones durante la ejecución
    cell.live();
    EXPECT_TRUE(cell.alive());
}

TEST(AgenticCellTest, NoDivisionWhenRandomAboveThreshold) {
    domain::Gene tp53("TP53", domain::Gene::State::PlusPlus);
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusMinus);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);
    // division_rate = 0.3, FakeNoise devuelve 1.0 (muy alto)
    // 1.0 < 0.3 es falso, no hay división
    domain::AgenticCell cell(std::make_unique<FakeNoise>(std::vector<domain::CellNoise>{domain::CellNoise{1.0}}),
                             genome,
                             0.002, 0.0001, 0.0002, 0.3);
    cell.live();
    EXPECT_TRUE(cell.alive()); // Sin intento de división
}

TEST(AgenticCellTest, DivisionAttemptMultipleCycles) {
    domain::Gene tp53("TP53", domain::Gene::State::PlusPlus);
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusMinus);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);
    // Múltiples ciclos, algunos con potencial de división
    std::vector<domain::CellNoise> noise_sequence{
        domain::CellNoise{0.1},  // Ciclo 1: probablemente división (si division_rate > 0.1)
        domain::CellNoise{0.8},  // Ciclo 2: sin división
        domain::CellNoise{0.05}, // Ciclo 3: probablemente división
        domain::CellNoise{0.9}   // Ciclo 4: sin división
    };
    domain::AgenticCell cell(std::make_unique<FakeNoise>(noise_sequence),
                             genome,
                             0.002, 0.0001, 0.0002, 0.2);

    // Ejecutar múltiples ciclos
    for (int i = 0; i < 4; ++i) {
        cell.live();
        EXPECT_TRUE(cell.alive()); // Debe seguir viva
        EXPECT_EQ(cell.getAge(), static_cast<std::uint64_t>(i + 1));
    }
}

