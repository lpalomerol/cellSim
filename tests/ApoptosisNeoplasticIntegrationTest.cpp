#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../src/domain/cell/AgenticCell.h"
#include "../src/domain/cell/CellFactory.h"
#include "../src/domain/tissue/Tissue.h"
#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/adapters/RandomNoise.h"
#include "../src/domain/adapters/FixedNoise.h"
#include "../src/domain/adapters/Logger.h"
#include "FakeNoise.h"

namespace domain {

/**
 * Integration test: Neoplastic cell receives apoptosis signal and dies
 *
 * Scenario:
 * 1. Create a cell with controlled genomic instability (low)
 * 2. Force it to become neoplastic using high neoplasm probability
 * 3. Tissue detects neoplasia and sends apoptosis signal
 * 4. Cell receives signal and dies (because genomic_instability <= threshold)
 */
class ApoptosisNeoplasticDeathTest : public ::testing::Test {
protected:
    void SetUp() override {
        logger_ = std::make_shared<adapters::Logger>();
        logger_->setVerbose(true);  // VERBOSE MODE for debugging
    }

    ports::ILoggerPtr logger_;
};

TEST_F(ApoptosisNeoplasticDeathTest, NeoplasticCellDiesWhenGenomicInstabilityBelowThreshold) {
    // Setup: Create a cell with very high neoplasm probability (0.9 to force mutation quickly)
    // Use FixedNoise with 0.01 to avoid gene mutations, but neoplasm can trigger

    auto fixed_noise = std::make_unique<domain::adapters::FixedNoise>(CellNoise{0.01});

    // Create genome with LOW thresholds so mutations are unlikely
    // With FixedNoise(0.01), we need threshold < 0.01 to avoid mutations (sample < threshold rule)
    std::unordered_map<std::string, double> thresholds = {
        {"BRCA1", 0.001},  // Very low: won't mutate (0.01 >= 0.001)
        {"TP53", 0.001}    // Very low: won't mutate (0.01 >= 0.001)
    };
    std::unordered_map<std::string, double> instability_k = {
        {"BRCA1", 0.001},
        {"TP53", 0.001}
    };

    auto genome = genome_factory::makeDefaultGenome(thresholds, instability_k, logger_);

    // Force TP53 to -/- state so the cell is NOT protected from neoplasia
    genome.mutate("TP53");
    genome.mutate("TP53");  // Need to mutate twice to reach -/- from +/+

    // Create cell with:
    // - High neoplasm probability (0.9) - will trigger with noise 0.01
    // - LOW apoptosis_instability_threshold (5.0) so apoptosis IS effective
    auto cell = std::make_unique<AgenticCell>(
        std::move(fixed_noise),
        genome,
        0.9,           // neoplasm_k: high to force neoplasm (0.01 < 0.9)
        0.0001,        // low_delta_instability
        0.0002,        // high_delta_instability
        0.0,           // division_rate: no division
        5.0,           // apoptosis_instability_threshold: LOW (apoptosis works)
        logger_
    );

    cell->setId(1);

    // Create tissue to manage the cell and send signals
    Tissue tissue(logger_);
    tissue.setId(0);
    tissue.addCell(std::move(cell));

    ASSERT_EQ(tissue.size(), 1);

    // Tick 1: Cell should become neoplastic
    std::cout << "\n=== TICK 1: Attempting to trigger neoplasm ===" << std::endl;
    std::cout << "Before tissue.live():" << std::endl;
    std::cout << "  - Tissue size: " << tissue.size() << std::endl;

    tissue.live();

    std::cout << "After tissue.live():" << std::endl;
    std::cout << "  - Tissue size: " << tissue.size() << std::endl;

    // Verify the cell is still alive after first tick
    // Note: getCell(idx) uses array index (0), not cell ID
    auto cell_ptr = tissue.getCell(0);
    if (cell_ptr) {
        std::cout << "  - Cell found at index 0" << std::endl;
    } else {
        std::cout << "  - Cell NOT found at index 0 (returned nullptr)" << std::endl;
    }

    ASSERT_TRUE(cell_ptr != nullptr) << "Cell died before neoplasm check";

    auto agenticCell = dynamic_cast<AgenticCell*>(cell_ptr);
    ASSERT_TRUE(agenticCell != nullptr);

    std::cout << "After TICK 1:" << std::endl;
    std::cout << "  - Is Neoplastic: " << (agenticCell->isNeoplastic() ? "true" : "false") << std::endl;
    std::cout << "  - Genomic Instability: " << agenticCell->getGenomicInstability() << std::endl;

    EXPECT_TRUE(agenticCell->isNeoplastic());

    // At this point, tissue should have detected neoplasia and queued apoptosis signal
    // Tick 2: Cell processes apoptosis signal and dies (because genomic_instability ~1.0 < 5.0)
    std::cout << "\n=== TICK 2: Processing apoptosis signal ===" << std::endl;
    tissue.live();

    std::cout << "After TICK 2:" << std::endl;
    std::cout << "  - Tissue size: " << tissue.size() << std::endl;

    // Verify the cell is now dead (tissue should have removed it)
    EXPECT_EQ(tissue.size(), 0);
    EXPECT_FALSE(tissue.getCell(0));
}

/**
 * Integration test: Neoplastic cell receives apoptosis signal but SURVIVES
 *
 * Scenario:
 * 1. Create a cell that will accumulate high genomic instability
 * 2. Force it to become neoplastic
 * 3. Tissue sends apoptosis signal
 * 4. Cell receives signal but SURVIVES (because genomic_instability > threshold)
 */
class ApoptosisNeoplasticSurvivalTest : public ::testing::Test {
protected:
    void SetUp() override {
        logger_ = std::make_shared<adapters::Logger>();
        logger_->setVerbose(true);  // VERBOSE MODE for debugging
    }

    ports::ILoggerPtr logger_;
};

TEST_F(ApoptosisNeoplasticSurvivalTest, NeoplasticCellSurvivesWhenGenomicInstabilityAboveThreshold) {
    // Setup: Create a cell that will accumulate high genomic instability
    // We'll use FixedNoise with 0.01 to avoid initial gene mutations

    auto fixed_noise = std::make_unique<domain::adapters::FixedNoise>(CellNoise{0.01});

    // Create genome where genes won't mutate with FixedNoise(0.01)
    // With FixedNoise(0.01), we need threshold < 0.01 to avoid mutations
    std::unordered_map<std::string, double> thresholds = {
        {"BRCA1", 0.001},  // Very low: won't mutate (0.01 >= 0.001)
        {"TP53", 0.001}    // Very low: won't mutate (0.01 >= 0.001)
    };
    std::unordered_map<std::string, double> instability_k = {
        {"BRCA1", 0.001},
        {"TP53", 0.001}     // LOW instability_k to prevent mutation with 0.01 noise
    };

    auto genome = genome_factory::makeDefaultGenome(thresholds, instability_k, logger_);

    // Force TP53 to -/- state so the cell is NOT protected from neoplasia
    genome.mutate("TP53");
    genome.mutate("TP53");  // Need to mutate twice to reach -/- from +/+

    // Create cell with:
    // - High neoplasm probability (0.9) - will trigger with noise 0.01
    // - LOW apoptosis_instability_threshold (2.0) so it will be EXCEEDED by accumulated instability
    auto cell = std::make_unique<AgenticCell>(
        std::move(fixed_noise),
        genome,
        0.9,           // neoplasm_k: high to force neoplasm (0.01 < 0.9)
        0.0001,        // low_delta_instability
        1.5,           // high_delta_instability: HIGH to increase instability faster (will exceed 2.0 threshold)
        0.0,           // division_rate: no division
        2.0,           // apoptosis_instability_threshold: threshold the cell will exceed
        logger_
    );

    cell->setId(2);

    // Create tissue to manage the cell and send signals
    Tissue tissue(logger_);
    tissue.setId(1);
    tissue.addCell(std::move(cell));

    ASSERT_EQ(tissue.size(), 1);

    // Tick 1: Cell should become neoplastic (0.5 < 0.9 threshold)
    tissue.live();

    // Verify the cell is still alive and neoplastic
    auto cell_ptr = tissue.getCell(0);
    ASSERT_TRUE(cell_ptr != nullptr) << "Cell died before neoplasm check";
    EXPECT_TRUE(cell_ptr->isNeoplastic());

    // Check genomic instability
    auto agenticCell = dynamic_cast<AgenticCell*>(cell_ptr);
    ASSERT_TRUE(agenticCell != nullptr);
    double instability_before = agenticCell->getGenomicInstability();

    // Tick 2: Cell processes apoptosis signal but should SURVIVE
    // because genomic_instability will exceed the threshold (2.0)
    tissue.live();

    double instability_after = agenticCell->getGenomicInstability();

    // Verify the cell is STILL ALIVE (tissue still contains it)
    EXPECT_EQ(tissue.size(), 1) << "Cell should survive because genomic_instability > threshold";
    EXPECT_TRUE(tissue.getCell(0) != nullptr);
    EXPECT_TRUE(tissue.getCell(0)->isNeoplastic());

    // Verify genomic instability is above the apoptosis threshold (allowing evasion)
    EXPECT_GT(instability_after, 2.0) << "Genomic instability should exceed threshold to evade apoptosis";
}

} // namespace domain

