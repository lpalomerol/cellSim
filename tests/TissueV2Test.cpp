#include <gtest/gtest.h>
#include "../src/domain/tissue/TissueV2.h"
#include "../src/domain/cell/AgenticCell_v2.h"
#include "../src/domain/cell/CellFactory_v2.h"
#include "../src/domain/gene/Genome.h"
#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/adapters/NullLogger.h"
#include "../src/domain/exception/CellDeathException.h"

using namespace domain;

class TissueV2Test : public ::testing::Test {
protected:
    void SetUp() override {
        logger_ = std::make_shared<adapters::NullLogger>();
        tissue_ = std::make_unique<TissueV2>(logger_);
    }

    ports::ILoggerPtr logger_;
    std::unique_ptr<TissueV2> tissue_;

    // Helper: Create a normal cell with standard genotype
    std::unique_ptr<AgenticCell_v2> createNormalCell() {
        auto genome = genome_factory::makeDefaultGenome({}, {}, logger_);
        return CellFactory_v2::createNormalCell(genome, logger_);
    }

    // Helper: Create cell with specific genotypes
    std::unique_ptr<AgenticCell_v2> createCellWithGenotype(
        const std::string& tp53_status,
        const std::string& brca1_status) {

        // Create genes manually with desired states
        std::unordered_map<std::string, Gene> genes;

        // Create TP53 with desired state
        Gene::State tp53_state = Gene::State::PlusPlus;
        if (tp53_status == "+/-") {
            tp53_state = Gene::State::PlusMinus;
        } else if (tp53_status == "-/-") {
            tp53_state = Gene::State::MinusMinus;
        }
        genes.emplace("TP53", Gene("TP53", tp53_state, 0.1, 0.0, logger_));

        // Create BRCA1 with desired state
        Gene::State brca1_state = Gene::State::PlusMinus;  // default
        if (brca1_status == "-/-") {
            brca1_state = Gene::State::MinusMinus;
        }
        genes.emplace("BRCA1", Gene("BRCA1", brca1_state, 0.1, 0.0, logger_));

        auto genome = Genome(std::move(genes), logger_);

        return CellFactory_v2::createNormalCell(genome, logger_);
    }
};

// Test 1: Create TissueV2 and verify basic properties
TEST_F(TissueV2Test, Test1_CreateTissueV2) {
    EXPECT_EQ(tissue_->size(), 0);
    EXPECT_EQ(tissue_->id(), 0);

    tissue_->setId(42);
    EXPECT_EQ(tissue_->id(), 42);
}

// Test 2: Add cell to tissue
TEST_F(TissueV2Test, Test2_AddCell) {
    auto cell = createNormalCell();
    tissue_->addCell(std::move(cell));

    EXPECT_EQ(tissue_->size(), 1);
    EXPECT_NE(tissue_->getCell(0), nullptr);
}

// Test 3: Add multiple cells
TEST_F(TissueV2Test, Test3_AddMultipleCells) {
    for (int i = 0; i < 5; ++i) {
        auto cell = createNormalCell();
        tissue_->addCell(std::move(cell));
    }

    EXPECT_EQ(tissue_->size(), 5);
}

// Test 4: Execute live() on healthy cells
TEST_F(TissueV2Test, Test4_LiveOnHealthyCells) {
    auto cell = createNormalCell();
    tissue_->addCell(std::move(cell));

    // Should not throw
    EXPECT_NO_THROW(tissue_->live());
    EXPECT_EQ(tissue_->size(), 1);
}

// Test 5: Get live cells
TEST_F(TissueV2Test, Test5_GetLiveCells) {
    for (int i = 0; i < 3; ++i) {
        auto cell = createNormalCell();
        tissue_->addCell(std::move(cell));
    }

    auto live_cells = tissue_->getLiveCells();
    EXPECT_EQ(live_cells.size(), 3);
}

// Test 6: Cell with BRCA1 -/- + TP53 +/+ should die (intrinsic apoptosis)
TEST_F(TissueV2Test, Test6_IntrinsicApoptosisRemovesDeadCell) {
    // Normal cell
    {
        auto cell = createNormalCell();
        tissue_->addCell(std::move(cell));
    }

    // BRCA1 -/- + TP53 +/+ → should die
    {
        auto cell = createCellWithGenotype("+/+", "-/-");
        tissue_->addCell(std::move(cell));
    }

    EXPECT_EQ(tissue_->size(), 2);

    // Execute live() - BRCA1 -/- cell should die in phase1
    tissue_->live();

    // Should have 1 cell left (the normal one)
    EXPECT_EQ(tissue_->size(), 1);
}

// Test 7: Get cells by stage - BASELINE
TEST_F(TissueV2Test, Test7_GetCellsByStage_BASELINE) {
    // Create normal cells (BASELINE: TP53 +/+, BRCA1 +/-)
    for (int i = 0; i < 3; ++i) {
        auto cell = createNormalCell();
        tissue_->addCell(std::move(cell));
    }

    auto baseline_cells = tissue_->getCellsByStage(CellLifeStage::BASELINE);
    EXPECT_EQ(baseline_cells.size(), 3);
}

// Test 8: Get cells by stage - UNPROTECTED
TEST_F(TissueV2Test, Test8_GetCellsByStage_UNPROTECTED) {
    // Normal cell (BASELINE)
    {
        auto cell = createNormalCell();
        tissue_->addCell(std::move(cell));
    }

    // TP53 -/- + BRCA1 +/- → UNPROTECTED (low D1)
    {
        auto cell = createCellWithGenotype("-/-", "+/-");
        tissue_->addCell(std::move(cell));
    }

    auto unprotected_cells = tissue_->getCellsByStage(CellLifeStage::UNPROTECTED);
    EXPECT_EQ(unprotected_cells.size(), 1);

    auto baseline_cells = tissue_->getCellsByStage(CellLifeStage::BASELINE);
    EXPECT_EQ(baseline_cells.size(), 1);
}

// Test 9: Clear tissue
TEST_F(TissueV2Test, Test9_ClearTissue) {
    for (int i = 0; i < 5; ++i) {
        auto cell = createNormalCell();
        tissue_->addCell(std::move(cell));
    }

    EXPECT_EQ(tissue_->size(), 5);
    tissue_->clear();
    EXPECT_EQ(tissue_->size(), 0);
}

// Test 10: Get cell out of range
TEST_F(TissueV2Test, Test10_GetCellOutOfRange) {
    auto cell = createNormalCell();
    tissue_->addCell(std::move(cell));

    EXPECT_NE(tissue_->getCell(0), nullptr);
    EXPECT_EQ(tissue_->getCell(1), nullptr);
    EXPECT_EQ(tissue_->getCell(100), nullptr);
}

// Test 11: Multiple live() cycles
TEST_F(TissueV2Test, Test11_MultipleLiveCycles) {
    auto cell = createNormalCell();
    tissue_->addCell(std::move(cell));

    // Run multiple cycles
    for (int i = 0; i < 10; ++i) {
        EXPECT_NO_THROW(tissue_->live());
        EXPECT_GE(tissue_->size(), 1);  // Cell should still be alive
    }
}

// Test 12: Mixed genotypes
TEST_F(TissueV2Test, Test12_MixedGenotypes) {
    // BASELINE (TP53 +/+, BRCA1 +/-)
    tissue_->addCell(createNormalCell());

    // UNSTABLE (TP53 +/-, BRCA1 +/-)
    tissue_->addCell(createCellWithGenotype("+/-", "+/-"));

    // UNPROTECTED (TP53 -/-, BRCA1 +/-)
    tissue_->addCell(createCellWithGenotype("-/-", "+/-"));

    EXPECT_EQ(tissue_->size(), 3);

    tissue_->live();  // Should not crash
    EXPECT_EQ(tissue_->size(), 3);  // All alive

    auto baseline = tissue_->getCellsByStage(CellLifeStage::BASELINE);
    auto unstable = tissue_->getCellsByStage(CellLifeStage::UNSTABLE);
    auto unprotected = tissue_->getCellsByStage(CellLifeStage::UNPROTECTED);

    EXPECT_EQ(baseline.size(), 1);
    EXPECT_EQ(unstable.size(), 1);
    EXPECT_EQ(unprotected.size(), 1);
}

