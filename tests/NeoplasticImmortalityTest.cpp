#include <gtest/gtest.h>
#include "../src/domain/cell/AgenticCell.h"
#include "../src/domain/cell/CellFactory.h"
#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/adapters/FixedNoise.h"
#include "../src/domain/adapters/Logger.h"
#include "../src/domain/signal/ApoptosisSignal.h"

namespace domain {

/**
 * Test: Neoplastic cells that evade apoptosis become immortal
 */
class NeoplasticImmortalityTest : public ::testing::Test {
protected:
    void SetUp() override {
        logger_ = std::make_shared<adapters::Logger>();
        logger_->setVerbose(true);
    }

    ports::ILoggerPtr logger_;
};

TEST_F(NeoplasticImmortalityTest, NeoplasticCellBecomesImmortalAfterEvadingApoptosis) {
    // V2: Cell reaches PRIMER state (D1 > 2.0) WITH D2 > 5.0 to automatically become neoplastic
    auto fixed_noise = std::make_unique<domain::adapters::FixedNoise>(CellNoise{0.01});

    std::unordered_map<std::string, double> thresholds = {
        {"BRCA1", 0.001},
        {"TP53", 0.001}
    };
    std::unordered_map<std::string, double> instability_k = {
        {"BRCA1", 0.0},  // No instability growth to avoid unwanted mutations
        {"TP53", 0.0}    // No instability growth
    };

    auto genome = genome_factory::makeDefaultGenome(thresholds, instability_k, logger_);

    // Force TP53 to -/- for high instability
    genome.mutate("TP53");
    genome.mutate("TP53");

    auto cell = std::make_unique<AgenticCell>(
        std::move(fixed_noise),
        genome,
        0.9,           // neoplasm_k: high (not used with automatic transformation)
        0.001,         // low_delta_instability (not used, TP53 is -/-)
        5.0,           // high_delta_instability: VERY HIGH (1.0 + 5.0 = 6.0 in one cycle)
        0.0,           // division_rate: no division
        0.0,           // neoplastic_division_rate
        false,         // enable_big_bang_mode
        100.0,         // apoptosis_instability_threshold (high to avoid other deaths)
        nullptr,       // logger
        2.0,           // d1_primer_threshold (standard)
        5.0            // d2_apoptosis_threshold (standard)
    );

    cell->setId(0);

    std::cout << "\n=== Step 1: Cell before becoming neoplastic ===" << std::endl;
    std::cout << "Is alive: " << (cell->alive() ? "yes" : "no") << std::endl;
    std::cout << "Is neoplastic: " << (cell->isNeoplastic() ? "yes" : "no") << std::endl;
    std::cout << "D1 (DNA damage): " << cell->getD1() << std::endl;
    std::cout << "D2 (Immunosuppression): " << cell->getD2() << std::endl;

    // Execute ONE cycle: D1 and D2 jump from 1.0 to 6.0
    // Cell enters PRIMER (D1 > 2.0) WITH D2 > 5.0, transforms automatically to neoplastic
    std::cout << "\n=== Executing ONE cycle to reach PRIMER with high D2 ===" << std::endl;
    cell->live();

    std::cout << "\n=== Step 2: Cell after ONE cycle (automatic transformation) ===" << std::endl;
    std::cout << "Is alive: " << (cell->alive() ? "yes" : "no") << std::endl;
    std::cout << "Is neoplastic: " << (cell->isNeoplastic() ? "yes" : "no") << std::endl;
    std::cout << "D1 (DNA damage): " << cell->getD1() << std::endl;
    std::cout << "D2 (Immunosuppression): " << cell->getD2() << std::endl;

    std::cout << "\n=== Step 2: Cell after accumulating instability ===" << std::endl;
    std::cout << "Is neoplastic: " << (cell->isNeoplastic() ? "yes" : "no") << std::endl;
    std::cout << "D1 (DNA damage): " << cell->getD1() << std::endl;
    std::cout << "D2 (Immunosuppression): " << cell->getD2() << std::endl;

    // Cell should have D1 > 2.0 and D2 > 5.0
    EXPECT_GT(cell->getD1(), 2.0) << "D1 should be > 2.0 (PRIMER state)";
    EXPECT_GT(cell->getD2(), 5.0) << "D2 should be > 5.0 (can evade apoptosis)";

    // Send apoptosis signal - cell should evade because D2 > 5.0 and enter PRIMER → become neoplastic
    std::cout << "\n=== Step 3: Sending apoptosis signal ===" << std::endl;
    auto apoptosis_signal = std::make_unique<ApoptosisSignal>(
        1,  // source: from tissue
        "test_apoptosis",
        std::vector<std::uint64_t>{0}  // targeted to cell 0
    );
    cell->receiveMessage(std::move(apoptosis_signal));

    // Process the apoptosis signal - should NOT throw exception due to D2 > 5.0
    cell->live();

    std::cout << "\n=== Step 4: Cell after receiving apoptosis signal ===" << std::endl;
    std::cout << "Is alive: " << (cell->alive() ? "yes" : "no") << std::endl;
    std::cout << "Is neoplastic: " << (cell->isNeoplastic() ? "yes" : "no") << std::endl;
    std::cout << "D1 (DNA damage): " << cell->getD1() << std::endl;
    std::cout << "D2 (Immunosuppression): " << cell->getD2() << std::endl;

    // Verify cell evaded apoptosis and became neoplastic (immortal)
    EXPECT_TRUE(cell->alive()) << "Cell should still be alive after evading apoptosis";
    EXPECT_TRUE(cell->isNeoplastic()) << "Cell should be neoplastic (immortal) after evading in PRIMER state";

    // Now mutate BRCA1 multiple times to simulate continued damage
    std::cout << "\n=== Step 5: Attempting to kill cell by mutating BRCA1 ===" << std::endl;
    std::cout << "BRCA1 status before mutations: " << cell->getBRCA1Status() << std::endl;

    cell->mutateGene("BRCA1");
    std::cout << "BRCA1 status after 1st mutation: " << cell->getBRCA1Status() << std::endl;

    cell->mutateGene("BRCA1");
    std::cout << "BRCA1 status after 2nd mutation (should be disabled): " << cell->getBRCA1Status() << std::endl;

    // Even with BRCA1 disabled, the cell should remain alive because it evaded apoptosis
    std::cout << "\n=== Step 6: Cell status after BRCA1 disabled ===" << std::endl;
    std::cout << "Is alive: " << (cell->alive() ? "yes" : "no") << std::endl;

    EXPECT_TRUE(cell->alive()) << "Cell should be IMMORTAL and cannot die even with BRCA1 disabled";
    EXPECT_FALSE(cell->getBRCA1Status() == "+/-" || cell->getBRCA1Status() == "+/+")
        << "BRCA1 should be disabled";
}

TEST_F(NeoplasticImmortalityTest, ImmortalCellsPropagatImmunityToOffspring) {
    // V2: Test that neoplastic cells (already immortal) propagate immunity to offspring
    // We create a cell that will become neoplastic by having high D2 from the start

    auto fixed_noise = std::make_unique<domain::adapters::FixedNoise>(CellNoise{0.01});

    std::unordered_map<std::string, double> thresholds = {
        {"BRCA1", 0.001},
        {"TP53", 0.001}
    };
    std::unordered_map<std::string, double> instability_k = {
        {"BRCA1", 0.0},  // No instability growth
        {"TP53", 0.0}    // No instability growth
    };

    auto genome = genome_factory::makeDefaultGenome(thresholds, instability_k, logger_);
    genome.mutate("TP53");
    genome.mutate("TP53");

    // Strategy: Use VERY HIGH delta (5.0) so that in ONE cycle, both D1 and D2 jump to 6.0
    // This way, when the cell enters PRIMER (D1=6.0 > 2.0), D2 is also 6.0 (> 5.0)
    // So the cell transforms to neoplastic instead of dying
    auto parent = std::make_unique<AgenticCell>(
        std::move(fixed_noise),
        genome,
        0.9,           // neoplasm_k
        0.001,         // low_delta (not used, TP53 is -/-)
        5.0,           // high_delta - EXTREMELY HIGH: 1.0 + 5.0 = 6.0 in one cycle
        0.01,          // division_rate: allow division
        0.01,          // neoplastic_division_rate
        false,         // enable_big_bang_mode
        100.0,         // apoptosis_instability_threshold (high to avoid other deaths)
        nullptr,       // logger
        2.0,           // d1_primer_threshold (standard)
        5.0            // d2_apoptosis_threshold (standard)
    );

    parent->setId(0);

    // Execute ONE cycle: D1 and D2 both jump to 6.0
    // Cell enters PRIMER (D1 > 2.0) WITH D2 > 5.0, so transforms to neoplastic
    std::cout << "\n=== Executing ONE cycle to reach PRIMER with high D2 ===" << std::endl;
    std::cout << "Initial: D1=" << parent->getD1() << ", D2=" << parent->getD2() << std::endl;

    parent->live();

    std::cout << "After 1 cycle: D1=" << parent->getD1()
             << ", D2=" << parent->getD2()
             << ", isNeoplastic=" << parent->isNeoplastic()
             << ", alive=" << parent->alive() << std::endl;

    // V2: With D1=6.0 and D2=6.0, cell should be neoplastic
    std::cout << "\n=== Parent status after transformation ===" << std::endl;
    std::cout << "Parent is alive: " << (parent->alive() ? "yes" : "no") << std::endl;
    std::cout << "Parent is neoplastic: " << (parent->isNeoplastic() ? "yes" : "no") << std::endl;

    EXPECT_TRUE(parent->alive()) << "Parent should be alive after D2 > 5.0";
    EXPECT_TRUE(parent->isNeoplastic()) << "Parent should be neoplastic (automatic transformation)";
    EXPECT_GT(parent->getD2(), 5.0) << "Parent D2 should be > 5.0";
    EXPECT_GT(parent->getD1(), 2.0) << "Parent D1 should be > 2.0";

    // Clone the parent - daughter should inherit neoplastic status
    std::cout << "\n=== Cloning neoplastic parent ===" << std::endl;
    auto daughter = parent->clone();

    std::cout << "Daughter is alive: " << (daughter->alive() ? "yes" : "no") << std::endl;
    std::cout << "Daughter is neoplastic: " << (daughter->isNeoplastic() ? "yes" : "no") << std::endl;

    EXPECT_TRUE(daughter->alive()) << "Daughter should be alive after cloning";
    EXPECT_TRUE(daughter->isNeoplastic()) << "Daughter should inherit neoplastic status from parent";

    // Mutate daughter's BRCA1 to test immortality
    std::cout << "\n=== Testing daughter's immortality (BRCA1 mutation) ===" << std::endl;
    daughter->mutateGene("BRCA1");
    daughter->mutateGene("BRCA1");

    std::cout << "Daughter BRCA1 status: " << daughter->getBRCA1Status() << std::endl;
    std::cout << "Daughter is still alive: " << (daughter->alive() ? "yes" : "no") << std::endl;

    // V2: Daughter inherits has_evaded_apoptosis_=true (neoplastic), so BRCA1 -/- doesn't kill her
    EXPECT_TRUE(daughter->alive())
        << "Daughter should be immortal (inherited neoplastic status from parent)";
    EXPECT_EQ(daughter->getBRCA1Status(), "-/-") << "BRCA1 should be disabled";
    EXPECT_TRUE(daughter->isNeoplastic()) << "Daughter should remain neoplastic";
}
}


