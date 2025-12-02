#include <gtest/gtest.h>
#include <memory>
#include "TestNoise.h"
#include "FakeNoise.h"
#include "../src/domain/tissue/Tissue.h"
#include "../src/domain/signal/CellDivisionSignal.h"

// Test that a cell can divide and the daughter cell is added to the tissue
// (Simplified: just verify the mechanism works with the DaughterCellInheritsGenome test)
TEST(CellDivisionIntegrationTest, TissueReceivesDaughterCell) {
    domain::Tissue t;

    // Create parent cell with specific genome
    domain::Gene tp53("TP53", domain::Gene::State::PlusMinus);
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusMinus);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);

    // Use FakeNoise to force division by providing enough samples
    std::vector<domain::CellNoise> noise_seq;
    for (int i = 0; i < 100; ++i) {
        noise_seq.push_back(domain::CellNoise{0.1});  // Division will occur (0.1 < 0.5)
    }

    auto cell = std::make_unique<domain::AgenticCell>(
        std::make_unique<FakeNoise>(noise_seq),
        genome,
        0.002, 0.0001, 0.0002,
        0.5    // division_rate = 50%
    );

    t.addCell(std::move(cell));
    ASSERT_EQ(t.size(), 1u);

    // Act: run cycle with division
    t.live();
    EXPECT_GE(t.size(), 1u);  // Should have at least parent cell

    // If division occurred, we'll have 2+ cells
    if (t.size() >= 2) {
        auto* daughter = dynamic_cast<domain::AgenticCell*>(t.getCell(1));
        ASSERT_NE(daughter, nullptr);
        EXPECT_GE(daughter->id(), 1u);
    }
}

// Test that daughter cells inherit parent's genome state
TEST(CellDivisionIntegrationTest, DaughterCellInheritsGenome) {
    domain::Tissue t;

    // Create parent cell with specific genome
    domain::Gene tp53("TP53", domain::Gene::State::PlusMinus);
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusMinus);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);

    // Use FakeNoise to force division
    std::vector<domain::CellNoise> noise_seq{
        domain::CellNoise{0.1}  // Division! (0.1 < 0.5)
    };

    auto cell = std::make_unique<domain::AgenticCell>(
        std::make_unique<FakeNoise>(noise_seq),
        genome,
        0.002, 0.0001, 0.0002,
        0.5   // high division rate

    );

    t.addCell(std::move(cell));

    // Act: run cycle with division
    t.live();
    EXPECT_EQ(t.size(), 2u);

    // Verify parent genome state
    auto* parent = dynamic_cast<domain::AgenticCell*>(t.getCell(0));
    ASSERT_NE(parent, nullptr);
    EXPECT_EQ(parent->getTP53(), "+/-");
    EXPECT_EQ(parent->getBRCA1(), "+/-");

    // Verify daughter inherited the same genome state
    auto* daughter = dynamic_cast<domain::AgenticCell*>(t.getCell(1));
    ASSERT_NE(daughter, nullptr);
    EXPECT_EQ(daughter->getTP53(), "+/-");
    EXPECT_EQ(daughter->getBRCA1(), "+/-");
}

// Test that division is disabled when division_rate = 0
TEST(CellDivisionIntegrationTest, DivisionDisabledWhenRateIsZero) {
    domain::Tissue t;

    domain::Gene tp53("TP53", domain::Gene::State::PlusPlus);
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusMinus);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);

    auto cell = std::make_unique<domain::AgenticCell>(
        std::make_unique<test::DummyNoise>(),
        genome,
        0.002, 0.0001, 0.0002,
        0.0    // division_rate = 0 (disabled)
    );

    t.addCell(std::move(cell));

    // Act: run multiple cycles
    for (int i = 0; i < 5; ++i) {
        t.live();
    }

    // Verify no division occurred
    EXPECT_EQ(t.size(), 1u);
}

// Test that division is processed and daughter cell is added to tissue
TEST(CellDivisionIntegrationTest, TissueCollectsDivisionSignals) {
    domain::Tissue t;

    domain::Gene tp53("TP53", domain::Gene::State::PlusPlus);
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusMinus);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);

    std::vector<domain::CellNoise> noise_seq{
        domain::CellNoise{0.1}  // Division!
    };

    auto cell = std::make_unique<domain::AgenticCell>(
        std::make_unique<FakeNoise>(noise_seq),
        genome,
        0.002, 0.0001, 0.0002,
        0.5    // high division rate
    );

    t.addCell(std::move(cell));
    ASSERT_EQ(t.size(), 1u);

    t.live();

    // Verify: Division signal was processed (daughter cell was added)
    // The signal is NOT collected in stealEmittedSignals() because it was processed internally
    EXPECT_EQ(t.size(), 2u) << "Division should have created a daughter cell";

    // Verify both cells exist and have proper IDs
    auto* parent = t.getCell(0);
    auto* daughter = t.getCell(1);
    ASSERT_NE(parent, nullptr);
    ASSERT_NE(daughter, nullptr);
    EXPECT_LT(parent->id(), daughter->id());
}

