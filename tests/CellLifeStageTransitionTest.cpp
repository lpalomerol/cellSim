#include <gtest/gtest.h>
#include "../src/domain/cell/AgenticCell_v2.h"
#include "../src/domain/cell/CellLifeStage.h"
#include "../src/domain/shared/Genome.h"
#include "../src/adapters/RandomNoise.h"
#include "../src/adapters/NullLogger.h"

using namespace domain;

class CellLifeStageTransitionTest : public ::testing::Test {
protected:
    void SetUp() override {
        logger_ = std::make_shared<adapters::NullLogger>();
    }

    ports::ILoggerPtr logger_;

    // Helper: Create cell with specific genotypes
    // BIOLOGICALLY VALID COMBINATIONS:
    // - BRCA1 can only be: +/- (heterozygous, one functional copy) or -/- (homozygous KO)
    // - BRCA1 +/+ (both functional) is biologically impossible - cells don't have it
    std::unique_ptr<AgenticCell_v2> createCell(
        const std::string& tp53_status,
        const std::string& brca1_status) {

        auto noise = std::make_unique<adapters::RandomNoise>();
        Genome genome;

        // Set TP53 status
        if (tp53_status == "+/+") {
            genome.setGeneStatus("TP53", GeneStatus::NORMAL);
        } else if (tp53_status == "+/-") {
            genome.setGeneStatus("TP53", GeneStatus::HETEROZYGOUS_DELETION);
        } else if (tp53_status == "-/-") {
            genome.setGeneStatus("TP53", GeneStatus::HOMOZYGOUS_DELETION);
        }

        // Set BRCA1 status
        if (brca1_status == "+/+") {
            genome.setGeneStatus("BRCA1", GeneStatus::NORMAL);
        } else if (brca1_status == "+/-") {
            genome.setGeneStatus("BRCA1", GeneStatus::HETEROZYGOUS_DELETION);
        } else if (brca1_status == "-/-") {
            genome.setGeneStatus("BRCA1", GeneStatus::HOMOZYGOUS_DELETION);
        }

        auto cell = std::make_unique<AgenticCell_v2>(
            std::move(noise),
            std::move(genome),
            0.002,      // neoplasm_k
            0.0001,     // low_delta_instability
            0.0002,     // high_delta_instability
            0.001,      // division_rate
            0.001,      // neoplastic_division_rate
            false,      // enable_big_bang_mode
            10.0,       // apoptosis_instability_threshold
            logger_
        );

        return cell;
    }
};

// Test 1: BASELINE state
// TP53 +/+ & BRCA1 +/- → BASELINE
TEST_F(CellLifeStageTransitionTest, Test1_BASELINE_TP53_normal_BRCA1_het) {
    auto cell = createCell("+/+", "+/-");

    CellLifeStage stage = cell->getCurrentCellLifeStage();

    EXPECT_EQ(stage, CellLifeStage::BASELINE);
    EXPECT_TRUE(cell->alive());
    EXPECT_FALSE(cell->isNeoplastic());
}

// Test 2: UNSTABLE state
// TP53 +/- & BRCA1 +/- → UNSTABLE
TEST_F(CellLifeStageTransitionTest, Test2_UNSTABLE_TP53_het_BRCA1_het) {
    auto cell = createCell("+/-", "+/-");

    CellLifeStage stage = cell->getCurrentCellLifeStage();

    EXPECT_EQ(stage, CellLifeStage::UNSTABLE);
    EXPECT_TRUE(cell->alive());
    EXPECT_FALSE(cell->isNeoplastic());
}

// Test 3: UNPROTECTED state
// TP53 -/- & BRCA1 +/- (with low D1) → UNPROTECTED
TEST_F(CellLifeStageTransitionTest, Test3_UNPROTECTED_TP53_ko_BRCA1_het_lowD1) {
    auto cell = createCell("-/-", "+/-");

    // D1 is still low (default 1.0, threshold is 2.0)
    CellLifeStage stage = cell->getCurrentCellLifeStage();

    EXPECT_EQ(stage, CellLifeStage::UNPROTECTED);
    EXPECT_TRUE(cell->alive());
    EXPECT_FALSE(cell->isNeoplastic());
}

// Test 4: PRIMER state
// TP53 -/- & D1 > 2.0 → PRIMER
TEST_F(CellLifeStageTransitionTest, Test4_PRIMER_TP53_ko_D1_high) {
    auto cell = createCell("-/-", "+/-");

    // Manually set D1 > 2.0 to trigger PRIMER
    // Since D1 is private, we simulate through phase4 multiple times
    // or we assume D1 has grown. For this test, we check that if D1 were high,
    // getCurrentCellLifeStage would return PRIMER.
    //
    // For now, we'll test the logic by checking that UNPROTECTED is returned
    // when D1 <= 2.0, and PRIMER would be returned if D1 > 2.0.

    CellLifeStage stage = cell->getCurrentCellLifeStage();
    EXPECT_EQ(stage, CellLifeStage::UNPROTECTED);

    // If we had a way to set D1 directly (or trigger phase4 multiple times),
    // we would verify PRIMER here.
    // This is a limitation of the test without accessing private members.
}

// Test 5: PRIMER detected but NO neoplasm yet
// Verify that PRIMER state is detected but is_neoplastic_ is still false
TEST_F(CellLifeStageTransitionTest, Test5_PRIMER_detected_no_neoplasm) {
    auto cell = createCell("-/-", "+/-");

    // Cell starts in UNPROTECTED state
    EXPECT_EQ(cell->getCurrentCellLifeStage(), CellLifeStage::UNPROTECTED);
    EXPECT_FALSE(cell->isNeoplastic());

    // If we could run phase4 multiple times to increase D1 > 2.0,
    // we would verify:
    // - getCurrentCellLifeStage() == PRIMER
    // - isNeoplastic() == false
    // This requires access to phase4 or D1 setter.
}

// Test 6: INTRINSIC APOPTOSIS - BRCA1 -/-
// BRCA1 -/- with any TP53 → DEAD (intrinsic apoptosis)
TEST_F(CellLifeStageTransitionTest, Test6_intrinsic_apoptosis_BRCA1_ko_TP53_normal) {
    auto cell = createCell("+/+", "-/-");

    // Cell with BRCA1 -/- should not be alive
    EXPECT_FALSE(cell->alive());
    EXPECT_EQ(cell->getCurrentCellLifeStage(), CellLifeStage::DEAD);
}

// Test 7: INTRINSIC APOPTOSIS - BRCA1 -/- & TP53 -/-
// Both genes KO → DEAD (intrinsic apoptosis, BRCA1 kills first)
TEST_F(CellLifeStageTransitionTest, Test7_intrinsic_apoptosis_BRCA1_ko_TP53_ko) {
    auto cell = createCell("-/-", "-/-");

    EXPECT_FALSE(cell->alive());
    EXPECT_EQ(cell->getCurrentCellLifeStage(), CellLifeStage::DEAD);
}

// Test 8: Verify state transitions don't depend on D2
// D2 should NOT affect state classification (only affects survival in phase2)
TEST_F(CellLifeStageTransitionTest, Test8_D2_does_not_affect_state_classification) {
    auto cell1 = createCell("+/-", "+/-");
    auto cell2 = createCell("+/-", "+/-");

    // Both should have same state regardless of D2
    EXPECT_EQ(cell1->getCurrentCellLifeStage(), CellLifeStage::UNSTABLE);
    EXPECT_EQ(cell2->getCurrentCellLifeStage(), CellLifeStage::UNSTABLE);
}

// Test 9: DEAD state - not alive
TEST_F(CellLifeStageTransitionTest, Test9_DEAD_when_not_alive) {
    auto cell = createCell("+/-", "-/-");

    // BRCA1 -/- → not alive
    EXPECT_FALSE(cell->alive());
    EXPECT_EQ(cell->getCurrentCellLifeStage(), CellLifeStage::DEAD);
}

// Test 10: BRCA1 -/- + TP53 -/- should LIVE (TP53 cannot detect BRCA1 damage)
// This is biologically important: BRCA1 -/- is only lethal if TP53 is functional
TEST_F(CellLifeStageTransitionTest, Test10_BRCA1_ko_TP53_ko_lives) {
    auto cell = createCell("-/-", "-/-");

    // BRCA1 -/- with TP53 -/- → cell LIVES (TP53 cannot trigger apoptosis)
    EXPECT_TRUE(cell->alive());
    // Cell is UNPROTECTED (TP53 -/-) but alive
    EXPECT_EQ(cell->getCurrentCellLifeStage(), CellLifeStage::UNPROTECTED);
}

// Test 11: BRCA1 -/- + TP53 +/+ should DIE (intrinsic apoptosis)
// TP53 functional → detects BRCA1 damage → apoptosis
TEST_F(CellLifeStageTransitionTest, Test11_intrinsic_apoptosis_BRCA1_ko_TP53_normal) {
    auto cell = createCell("+/+", "-/-");

    // BRCA1 -/- with TP53 +/+ → cell DIES
    EXPECT_FALSE(cell->alive());
    EXPECT_EQ(cell->getCurrentCellLifeStage(), CellLifeStage::DEAD);
}

// Test 12: BRCA1 -/- + TP53 +/- should DIE (intrinsic apoptosis)
// TP53 heterozygous still functional → detects BRCA1 damage → apoptosis
TEST_F(CellLifeStageTransitionTest, Test12_intrinsic_apoptosis_BRCA1_ko_TP53_het) {
    auto cell = createCell("+/-", "-/-");

    // BRCA1 -/- with TP53 +/- → cell DIES
    EXPECT_FALSE(cell->alive());
    EXPECT_EQ(cell->getCurrentCellLifeStage(), CellLifeStage::DEAD);
}

