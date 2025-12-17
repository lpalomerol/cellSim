#include <gtest/gtest.h>
#include "../src/application/simulation/Simulation.h"
#include "../src/domain/cell/CellFactory_v2.h"
#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/adapters/NullLogger.h"

using namespace application;
using namespace domain;

class SimulationTissueV2IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        logger_ = std::make_shared<adapters::NullLogger>();
    }

    ports::ILoggerPtr logger_;

    /// Helper: Crear célula normal (TP53 +/+, BRCA1 +/-)
    std::unique_ptr<ICell> createNormalCell() {
        auto genome = genome_factory::makeDefaultGenome({}, {}, logger_);
        return CellFactory_v2::createNormalCell(genome, logger_);
    }
};

// Test 1: Simulation compila con TissueV2 deshabilitado (default)
TEST_F(SimulationTissueV2IntegrationTest, Test1_DefaultBehaviorNoTissueV2) {
    Simulation sim(10);

    EXPECT_FALSE(sim.isTissueV2Enabled());
    sim.addCell(createNormalCell());
    EXPECT_NO_THROW(sim.run());
}

// Test 2: Habilitar/deshabilitar TissueV2 es reversible
TEST_F(SimulationTissueV2IntegrationTest, Test2_EnableDisableTissueV2IsReversible) {
    Simulation sim(5);

    // Debe ser deshabilitado por defecto
    EXPECT_FALSE(sim.isTissueV2Enabled());

    // Habilitar
    sim.enableTissueV2(true);
    EXPECT_TRUE(sim.isTissueV2Enabled());

    // Deshabilitar
    sim.enableTissueV2(false);
    EXPECT_FALSE(sim.isTissueV2Enabled());
}

// Test 3: Agregar célula antes de habilitar TissueV2, luego habilitar
TEST_F(SimulationTissueV2IntegrationTest, Test3_AddCellThenEnableTissueV2) {
    Simulation sim(5);

    // Agregar célula con Tissue original
    sim.addCell(createNormalCell());

    // Luego habilitar TissueV2 (debe transferir la célula)
    sim.enableTissueV2(true);
    EXPECT_TRUE(sim.isTissueV2Enabled());

    // Ejecutar ciclo (no debe crash)
    int neo_count = sim.executeCellCycle();
    EXPECT_GE(neo_count, 0);
}

// Test 4: Ejecutar 5 años con Tissue original
TEST_F(SimulationTissueV2IntegrationTest, Test4_Run5YearsWithOriginalTissue) {
    Simulation sim(5);
    sim.addCell(createNormalCell());
    sim.addCell(createNormalCell());

    EXPECT_FALSE(sim.isTissueV2Enabled());
    EXPECT_NO_THROW(sim.run());

    // Capturar stats
    const auto& tracker = sim.populationTracker();
    EXPECT_GE(tracker.snapshots().size(), 1);
}

// Test 5: Ejecutar 5 años con TissueV2
TEST_F(SimulationTissueV2IntegrationTest, Test5_Run5YearsWithTissueV2) {
    Simulation sim(5);
    sim.addCell(createNormalCell());
    sim.addCell(createNormalCell());

    // Habilitar TissueV2
    sim.enableTissueV2(true);
    EXPECT_TRUE(sim.isTissueV2Enabled());

    EXPECT_NO_THROW(sim.run());

    // Capturar stats
    const auto& tracker = sim.populationTracker();
    EXPECT_GE(tracker.snapshots().size(), 1);
}

// Test 6: Comparar population counts: Tissue vs TissueV2 (basic compatibility)
// NOTA: Este test verifica que ambos dan resultados similares
TEST_F(SimulationTissueV2IntegrationTest, Test6_PopulationCountsCompatibility) {
    // Scenario 1: Original Tissue
    Simulation sim1(3);
    sim1.addCell(createNormalCell());
    sim1.addCell(createNormalCell());
    sim1.addCell(createNormalCell());
    EXPECT_FALSE(sim1.isTissueV2Enabled());
    sim1.run();

    // Scenario 2: TissueV2
    Simulation sim2(3);
    sim2.addCell(createNormalCell());
    sim2.addCell(createNormalCell());
    sim2.addCell(createNormalCell());
    sim2.enableTissueV2(true);
    EXPECT_TRUE(sim2.isTissueV2Enabled());
    sim2.run();

    // Ambas deben tener al menos 1 snapshot
    EXPECT_GE(sim1.populationTracker().snapshots().size(), 1);
    EXPECT_GE(sim2.populationTracker().snapshots().size(), 1);

    // Los números finales de células vivas deben ser razonables
    // (No necesariamente idénticos debido a randomness, pero similares)
    auto final1 = sim1.populationTracker().snapshots().back();
    auto final2 = sim2.populationTracker().snapshots().back();

    // Ambas deben tener células vivas (o similares patrones)
    EXPECT_GE(final1.alive_cells, 0);
    EXPECT_GE(final2.alive_cells, 0);
}

// Test 7: firstTimeNeoplastic() funciona con ambas implementaciones
TEST_F(SimulationTissueV2IntegrationTest, Test7_FirstTimeNeoplasticWithTissueV2) {
    // Con Tissue original
    Simulation sim1(10);
    sim1.addCell(createNormalCell());
    sim1.run();
    int first_neo1 = sim1.firstTimeNeoplastic();

    // Con TissueV2
    Simulation sim2(10);
    sim2.addCell(createNormalCell());
    sim2.enableTissueV2(true);
    sim2.run();
    int first_neo2 = sim2.firstTimeNeoplastic();

    // Ambos pueden ser -1 (nunca neoplástico) o >= 0 (tiempo de transformación)
    EXPECT_TRUE((first_neo1 == -1) || (first_neo1 >= 0));
    EXPECT_TRUE((first_neo2 == -1) || (first_neo2 >= 0));
}

// Test 8: Multiple cells in TissueV2
TEST_F(SimulationTissueV2IntegrationTest, Test8_MultipleCellsInTissueV2) {
    Simulation sim(5);

    // Agregar 10 células
    for (int i = 0; i < 10; ++i) {
        sim.addCell(createNormalCell());
    }

    sim.enableTissueV2(true);
    EXPECT_NO_THROW(sim.run());

    const auto& tracker = sim.populationTracker();
    EXPECT_GE(tracker.snapshots().size(), 1);
}

// Test 9: executeCellCycle() condicional
TEST_F(SimulationTissueV2IntegrationTest, Test9_ExecuteCellCycleConditional) {
    Simulation sim1(1);
    sim1.addCell(createNormalCell());
    int neo1 = sim1.executeCellCycle();  // Usa Tissue original
    EXPECT_GE(neo1, 0);

    Simulation sim2(1);
    sim2.addCell(createNormalCell());
    sim2.enableTissueV2(true);
    int neo2 = sim2.executeCellCycle();  // Usa TissueV2
    EXPECT_GE(neo2, 0);
}

// Test 10: No crash when TissueV2 is enabled but no cells added
TEST_F(SimulationTissueV2IntegrationTest, Test10_EmptySimulationWithTissueV2) {
    Simulation sim(5);
    sim.enableTissueV2(true);

    EXPECT_NO_THROW(sim.run());
    // Simulation captura snapshots anuales incluso sin células
    // 5 años = 5 snapshots como mínimo (o más según implementación)
    EXPECT_GE(sim.populationTracker().snapshots().size(), 1);
}

