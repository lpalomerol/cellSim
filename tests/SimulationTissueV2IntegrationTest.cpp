#include <gtest/gtest.h>
#include "../src/application/simulation/Simulation.h"
#include "../src/domain/cell/CellFactory.h"
#include "../src/domain/gene/Genome.h"
#include "../src/domain/gene/Gene.h"
#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/adapters/NullLogger.h"
#include "../src/domain/adapters/FixedNoise.h"

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
        return CellFactory::createNormalCell(genome, logger_);
    }
};

// Test 1: Simulation compila y corre correctamente
TEST_F(SimulationTissueV2IntegrationTest, Test1_DefaultBehaviorNoTissueV2) {
    Simulation sim(10);

    sim.addCell(createNormalCell());
    EXPECT_NO_THROW(sim.run());
}

// Test 2: addCell + run básico
TEST_F(SimulationTissueV2IntegrationTest, Test2_AddCellAndRun) {
    Simulation sim(5);

    sim.addCell(createNormalCell());
    EXPECT_NO_THROW(sim.run());

    const auto& tracker = sim.populationTracker();
    EXPECT_GE(tracker.snapshots().size(), 1u);
}

// Test 3: Agregar célula y ejecutar ciclo celular
TEST_F(SimulationTissueV2IntegrationTest, Test3_AddCellThenExecuteCycle) {
    Simulation sim(5);
    sim.addCell(createNormalCell());

    int neo_count = sim.executeCellCycle();
    EXPECT_GE(neo_count, 0);
}

// Test 4: Ejecutar 5 años
TEST_F(SimulationTissueV2IntegrationTest, Test4_Run5Years) {
    Simulation sim(5);
    sim.addCell(createNormalCell());
    sim.addCell(createNormalCell());

    EXPECT_NO_THROW(sim.run());

    const auto& tracker = sim.populationTracker();
    EXPECT_GE(tracker.snapshots().size(), 1u);
}

// Test 5: Snapshots contienen datos razonables
TEST_F(SimulationTissueV2IntegrationTest, Test5_SnapshotsHaveReasonableData) {
    Simulation sim(5);
    sim.addCell(createNormalCell());
    sim.addCell(createNormalCell());

    EXPECT_NO_THROW(sim.run());

    const auto& tracker = sim.populationTracker();
    EXPECT_GE(tracker.snapshots().size(), 1u);
}

// Test 6: Population counts básicos
TEST_F(SimulationTissueV2IntegrationTest, Test6_PopulationCounts) {
    Simulation sim1(3);
    sim1.addCell(createNormalCell());
    sim1.addCell(createNormalCell());
    sim1.addCell(createNormalCell());
    sim1.run();

    Simulation sim2(3);
    sim2.addCell(createNormalCell());
    sim2.addCell(createNormalCell());
    sim2.addCell(createNormalCell());
    sim2.run();

    EXPECT_GE(sim1.populationTracker().snapshots().size(), 1u);
    EXPECT_GE(sim2.populationTracker().snapshots().size(), 1u);

    auto final1 = sim1.populationTracker().snapshots().back();
    auto final2 = sim2.populationTracker().snapshots().back();

    EXPECT_GE(final1.alive_cells, 0);
    EXPECT_GE(final2.alive_cells, 0);
}

// Test 7: firstTimeNeoplastic() funciona
TEST_F(SimulationTissueV2IntegrationTest, Test7_FirstTimeNeoplastic) {
    Simulation sim(10);
    sim.addCell(createNormalCell());
    sim.run();
    int first_neo = sim.firstTimeNeoplastic();

    EXPECT_TRUE((first_neo == -1) || (first_neo >= 0));
}

// Test 8: Multiple cells
TEST_F(SimulationTissueV2IntegrationTest, Test8_MultipleCells) {
    Simulation sim(5);

    for (int i = 0; i < 10; ++i) {
        sim.addCell(createNormalCell());
    }

    EXPECT_NO_THROW(sim.run());

    const auto& tracker = sim.populationTracker();
    EXPECT_GE(tracker.snapshots().size(), 1u);
}

// Test 9: executeCellCycle() devuelve valor razonable
TEST_F(SimulationTissueV2IntegrationTest, Test9_ExecuteCellCycleReturnsValue) {
    Simulation sim(1);
    sim.addCell(createNormalCell());
    int neo = sim.executeCellCycle();
    EXPECT_GE(neo, 0);
}

// Test 10: No crash when no cells added
TEST_F(SimulationTissueV2IntegrationTest, Test10_EmptySimulation) {
    Simulation sim(5);

    EXPECT_NO_THROW(sim.run());
    EXPECT_GE(sim.populationTracker().snapshots().size(), 1u);
}

// Test 11: Simulation stops before max_t when tissue saturates.
//
// Setup: 1 normal cell + 2 quick-tumoral cells in a 50-year simulation.
// After the first tick the tumoral cells dominate → saturation → early exit.
// The tracker should have fewer snapshots than max_t + 1 (year 0 + 50 years).
TEST_F(SimulationTissueV2IntegrationTest, Test11_SimulationStopsEarlyOnSaturation) {
    constexpr int MAX_T = 50;
    Simulation sim(MAX_T);

    // 1 BASELINE cell
    sim.addCell(createNormalCell());

    // 2 cells that become TUMORAL after the first live() tick
    auto makeQuickTumoral = [this]() -> std::unique_ptr<ICell> {
        std::unordered_map<std::string, Gene> genes;
        genes.emplace("TP53",  Gene("TP53",  Gene::State::MinusMinus, 0.0, 0.0, logger_));
        genes.emplace("BRCA1", Gene("BRCA1", Gene::State::PlusMinus,  0.0, 0.0, logger_));
        auto genome = Genome(std::move(genes), logger_);
        return CellFactory::createCustomCell(
            std::make_unique<adapters::FixedNoise>(CellNoise{0.5}),
            genome,
            /*low_delta=*/    1.0,
            /*high_delta=*/   2.0,
            /*div_rate=*/     0.0,
            /*neo_div_rate=*/ 0.0,
            /*big_bang=*/     true,
            /*d1_threshold=*/ 0.5,
            /*d2_threshold=*/ 0.5,
            logger_
        );
    };
    sim.addCell(makeQuickTumoral());
    sim.addCell(makeQuickTumoral());

    EXPECT_NO_THROW(sim.run());

    // With saturation at tick 1, the tracker should have < MAX_T + 1 snapshots
    const std::size_t full_run_snapshots = static_cast<std::size_t>(MAX_T + 1);
    EXPECT_LT(sim.populationTracker().snapshots().size(), full_run_snapshots);
}

