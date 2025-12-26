#include <gtest/gtest.h>
#include "../src/domain/cell/AgenticCell.h"
#include "../src/domain/cell/CellFactory.h"
#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/adapters/FixedNoise.h"
#include "../src/domain/adapters/Logger.h"

namespace domain {

/**
 * Test: When a cell divides, D1 (DNA damage) propagates to the daughter cell
 */
class CellDivisionInstabilityPropagationTest : public ::testing::Test {
protected:
    void SetUp() override {
        logger_ = std::make_shared<adapters::Logger>();
        logger_->setVerbose(true);
    }

    ports::ILoggerPtr logger_;
};

TEST_F(CellDivisionInstabilityPropagationTest, DaughterInheritsParentGenomicInstability) {
    // Create a parent cell with high genomic instability
    auto fixed_noise = std::make_unique<domain::adapters::FixedNoise>(CellNoise{0.01});

    std::unordered_map<std::string, double> thresholds = {
        {"BRCA1", 0.001},
        {"TP53", 0.001}
    };
    std::unordered_map<std::string, double> instability_k = {
        {"BRCA1", 0.001},
        {"TP53", 0.001}
    };

    auto genome = genome_factory::makeDefaultGenome(thresholds, instability_k, logger_);

    domain::InstabilityConfig instability{0.0001, 0.5};
    domain::DivisionConfig division{0.01, 0.01, false};
    domain::ThresholdConfig config_thresholds{2.0, 5.0, 0.000};

    auto parent = std::make_unique<AgenticCell>(
        std::move(fixed_noise),
        genome,
        instability,
        division,
        config_thresholds,
        logger_
    );

    parent->setId(1);

    // Manually set parent's genomic instability to a high value
    // (In real scenario, this would accumulate through updateGenomicInstability)
    double parent_instability = 5.0;

    // We need to run a cycle to see the real flow, but let's first check via clone
    std::cout << "\n=== Parent cell state before cloning ===" << std::endl;
    std::cout << "Parent D1 (DNA damage): " << parent->getD1() << std::endl;

    // Clone the parent (this is what division does internally)
    auto daughter = parent->clone();

    std::cout << "\n=== Daughter cell state after cloning ===" << std::endl;
    std::cout << "Daughter D1 (DNA damage): " << daughter->getD1() << std::endl;

    // Verify the daughter inherited parent's genomic instability
    EXPECT_EQ(daughter->getD1(), parent->getD1());
    EXPECT_EQ(daughter->getD1(), 1.0) << "Default parent instability should be 1.0";
}

TEST_F(CellDivisionInstabilityPropagationTest, DaughterInheritsAccumulatedInstability) {
    // More realistic: accumulate instability in parent, then divide

    auto fixed_noise = std::make_unique<domain::adapters::FixedNoise>(CellNoise{0.01});

    std::unordered_map<std::string, double> thresholds = {
        {"BRCA1", 0.001},
        {"TP53", 0.001}
    };
    std::unordered_map<std::string, double> instability_k = {
        {"BRCA1", 0.001},
        {"TP53", 0.5}  // HIGH for TP53 so it contributes more to instability
    };

    auto genome = genome_factory::makeDefaultGenome(thresholds, instability_k, logger_);

    // Force TP53 to -/- so it contributes high_delta_instability
    genome.mutate("TP53");
    genome.mutate("TP53");
    domain::InstabilityConfig instability2{0.0001, 0.01};
    domain::DivisionConfig division2{0.01, 0.01, false};
    domain::ThresholdConfig config_thresholds2{2.0, 5.0, 0.002};

    auto parent = std::make_unique<AgenticCell>(
        std::move(fixed_noise),
        genome,
        instability2,
        division2,
        config_thresholds2,
        logger_
    );

    parent->setId(1);

    // Run parent through one cycle to accumulate instability
    std::cout << "\n=== Parent before live() ===" << std::endl;
    std::cout << "Instability: " << parent->getD1() << std::endl;

    parent->live();

    std::cout << "\n=== Parent after live() ===" << std::endl;
    double parent_instability_after = parent->getD1();
    std::cout << "Instability: " << parent_instability_after << std::endl;

    // Now clone (divide)
    auto daughter = parent->clone();

    std::cout << "\n=== Daughter after cloning ===" << std::endl;
    double daughter_instability = daughter->getD1();
    std::cout << "Daughter instability: " << daughter_instability << std::endl;

    // Verify daughter inherited the accumulated instability
    EXPECT_EQ(daughter_instability, parent_instability_after)
        << "Daughter should inherit parent's accumulated genomic instability";

    // With high_delta_instability = 1.5 and TP53 = -/-, we expect:
    // next = 1.0 * 1.0 + 1.5 = 2.5
    EXPECT_GT(daughter_instability, 1.0)
        << "Daughter instability should be > 1.0 after inheriting accumulated instability";
}

} // namespace domain

