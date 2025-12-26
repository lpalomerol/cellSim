#include <gtest/gtest.h>
#include "../src/domain/cell/AgenticCell.h"
#include "../src/domain/cell/CellFactory.h"
#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/adapters/FixedNoise.h"
#include "../src/domain/adapters/Logger.h"

namespace domain {

/**
 * Test: Neoplastic cells propagate immortality to offspring
 *
 * When cells become neoplastic (by reaching PRIMER state with high D2),
 * they acquire immortality and pass this trait to their daughters.
 */
class NeoplasticImmortalityTest : public ::testing::Test {
protected:
    void SetUp() override {
        logger_ = std::make_shared<adapters::Logger>();
        logger_->setVerbose(true);
    }

    ports::ILoggerPtr logger_;
};

TEST_F(NeoplasticImmortalityTest, NeoplasticCellsPropagateImmortalityToOffspring) {
    // Test that neoplastic cells (immortal) propagate immunity to offspring
    // Cell becomes neoplastic automatically when D1 > 2.0 AND D2 > 5.0

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
    // When D1 > 2.0 (PRIMER) AND D2 > 5.0, the cell automatically becomes neoplastic
    // This is an internal transformation, not triggered by external signals
    auto parent = std::make_unique<AgenticCell>(
        std::move(fixed_noise),
        genome,
        0.9,           // neoplasm_k
        0.001,         // low_delta (not used, TP53 is -/-)
        5.0,           // high_delta - EXTREMELY HIGH: 1.0 + 5.0 = 6.0 in one cycle
        0.01,          // division_rate: allow division
        0.01,          // neoplastic_division_rate
        false,         // enable_big_bang_mode
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

    // With D1=6.0 and D2=6.0, cell should be neoplastic and alive
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

    // Daughter inherits neoplastic status (immortal), so BRCA1 -/- doesn't kill her
    EXPECT_TRUE(daughter->alive())
        << "Daughter should be immortal (inherited neoplastic status from parent)";
    EXPECT_EQ(daughter->getBRCA1Status(), "-/-") << "BRCA1 should be disabled";
    EXPECT_TRUE(daughter->isNeoplastic()) << "Daughter should remain neoplastic";
}
}


