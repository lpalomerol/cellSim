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
    // Create a neoplastic cell with high genomic instability that will evade apoptosis
    auto fixed_noise = std::make_unique<::adapters::FixedNoise>(CellNoise{0.01});

    std::unordered_map<std::string, double> thresholds = {
        {"BRCA1", 0.001},
        {"TP53", 0.001}
    };
    std::unordered_map<std::string, double> instability_k = {
        {"BRCA1", 0.001},
        {"TP53", 0.001}
    };

    auto genome = genome_factory::makeDefaultGenome(thresholds, instability_k, logger_);

    // Force TP53 to -/- for neoplasia
    genome.mutate("TP53");
    genome.mutate("TP53");

    auto cell = std::make_unique<AgenticCell>(
        std::move(fixed_noise),
        genome,
        0.9,           // neoplasm_k: high to force neoplasm
        0.0001,        // low_delta_instability
        1.5,           // high_delta_instability: HIGH to reach 2.5 instability
        0.0,           // division_rate: no division
        2.0,           // apoptosis_instability_threshold: low so cell will evade
        logger_
    );

    cell->setId(0);

    std::cout << "\n=== Step 1: Cell before becoming neoplastic ===" << std::endl;
    std::cout << "Is alive: " << (cell->alive() ? "yes" : "no") << std::endl;
    std::cout << "Is neoplastic: " << (cell->isNeoplastic() ? "yes" : "no") << std::endl;
    std::cout << "Genomic instability: " << cell->getGenomicInstability() << std::endl;

    // Run one cycle to become neoplastic and accumulate instability
    cell->live();

    std::cout << "\n=== Step 2: Cell after becoming neoplastic ===" << std::endl;
    std::cout << "Is neoplastic: " << (cell->isNeoplastic() ? "yes" : "no") << std::endl;
    std::cout << "Genomic instability: " << cell->getGenomicInstability() << std::endl;
    EXPECT_TRUE(cell->isNeoplastic());

    // Send apoptosis signal
    std::cout << "\n=== Step 3: Sending apoptosis signal ===" << std::endl;
    auto apoptosis_signal = std::make_unique<ApoptosisSignal>(
        1,  // source: from tissue
        "test_apoptosis",
        std::vector<std::uint64_t>{0}  // targeted to cell 0
    );
    cell->receiveMessage(std::move(apoptosis_signal));

    // Process the apoptosis signal
    cell->live();

    std::cout << "\n=== Step 4: Cell after receiving apoptosis signal ===" << std::endl;
    std::cout << "Is alive: " << (cell->alive() ? "yes" : "no") << std::endl;
    std::cout << "Is neoplastic: " << (cell->isNeoplastic() ? "yes" : "no") << std::endl;
    std::cout << "Genomic instability: " << cell->getGenomicInstability() << std::endl;

    // Verify cell is still alive (evaded apoptosis)
    EXPECT_TRUE(cell->alive()) << "Cell should still be alive after evading apoptosis";
    EXPECT_TRUE(cell->isNeoplastic()) << "Cell should still be neoplastic";

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
    // Create a neoplastic cell that will evade apoptosis and become immortal
    auto fixed_noise = std::make_unique<::adapters::FixedNoise>(CellNoise{0.01});

    std::unordered_map<std::string, double> thresholds = {
        {"BRCA1", 0.001},
        {"TP53", 0.001}
    };
    std::unordered_map<std::string, double> instability_k = {
        {"BRCA1", 0.001},
        {"TP53", 0.001}
    };

    auto genome = genome_factory::makeDefaultGenome(thresholds, instability_k, logger_);
    genome.mutate("TP53");
    genome.mutate("TP53");

    auto parent = std::make_unique<AgenticCell>(
        std::move(fixed_noise),
        genome,
        0.9,
        0.0001,
        1.5,
        0.01,  // division_rate: allow division
        2.0,
        logger_
    );

    parent->setId(0);

    // Make parent neoplastic and accumulate instability
    parent->live();
    EXPECT_TRUE(parent->isNeoplastic());

    // Send apoptosis so parent evades and becomes immortal
    std::cout << "\n=== Making parent immortal ===" << std::endl;
    auto apoptosis_signal = std::make_unique<ApoptosisSignal>(1, "test", std::vector<std::uint64_t>{0});
    parent->receiveMessage(std::move(apoptosis_signal));
    parent->live();

    EXPECT_TRUE(parent->alive()) << "Parent should be immortal";

    // Clone the parent
    std::cout << "\n=== Cloning immortal parent ===" << std::endl;
    auto daughter = parent->clone();

    std::cout << "Daughter is alive: " << (daughter->alive() ? "yes" : "no") << std::endl;
    std::cout << "Daughter is neoplastic: " << (daughter->isNeoplastic() ? "yes" : "no") << std::endl;
    std::cout << "Daughter genomic instability: " << daughter->getGenomicInstability() << std::endl;

    // Mutate daughter's BRCA1 to test immortality
    std::cout << "\n=== Testing daughter's immortality ===" << std::endl;
    daughter->mutateGene("BRCA1");
    daughter->mutateGene("BRCA1");

    std::cout << "Daughter BRCA1 status: " << daughter->getBRCA1Status() << std::endl;
    std::cout << "Daughter is still alive: " << (daughter->alive() ? "yes" : "no") << std::endl;

    // Verify daughter inherited immortality
    EXPECT_TRUE(daughter->alive())
        << "Daughter should also be immortal (inherited from parent)";
}

} // namespace domain

