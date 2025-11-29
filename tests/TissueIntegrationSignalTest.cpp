// filepath: /home/luis/CLionProjects/cellSim/tests/TissueIntegrationSignalTest.cpp
#include <gtest/gtest.h>
#include "TestNoise.h"
#include "../src/domain/tissue/Tissue.h"
#include "../src/domain/signal/NeoplasmSignal.h"

TEST(TissueIntegrationSignalTest, TissueCollectsNeoplasmSignals) {
    domain::Tissue t;
    // create a cell that will become neoplastic
    domain::Gene tp53("TP53", domain::Gene::State::MinusMinus);
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusPlus);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);
    // division_rate=0.0 to disable division and focus on neoplasm signal only
    auto cell = std::make_unique<domain::AgenticCell>(std::make_unique<test::HighNoise>(), genome, 1.0, 0.0001, 0.0002, 0.0);

    t.addCell(std::move(cell));
    ASSERT_EQ(t.size(), 1u);

    // Act: run tissue live which will call live() on the cell and cell should emit signal
    t.live();

    // Collect emitted signals
    auto signals = t.stealEmittedSignals();
    ASSERT_EQ(signals.size(), 1u);
    auto* s = dynamic_cast<domain::NeoplasmSignal*>(signals[0].get());
    ASSERT_NE(s, nullptr);
    // source id should be 0 because Tissue assigned first id as 0
    EXPECT_EQ(s->sourceId(), 0u);
}
