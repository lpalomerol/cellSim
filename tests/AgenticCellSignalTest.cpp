// filepath: /home/luis/CLionProjects/cellSim/tests/AgenticCellSignalTest.cpp
#include <gtest/gtest.h>
#include <memory>
#include "TestNoise.h"
#include "../src/domain/signal/ISignal.h"
#include "../src/domain/tissue/Tissue.h"
#include "../src/domain/signal/NeoplasmSignal.h"

// Test that AgenticCell emits a NeoplasmSignal via the injected emitter when
// it transitions to neoplastic state.
TEST(AgenticCellSignalTest, EmitsNeoplasmSignalOnTransition) {
    // Build a genome where TP53 is inactive so protection does not prevent neoplasm
    domain::Gene tp53("TP53", domain::Gene::State::MinusMinus);
    // Ensure BRCA1 starts enabled so the cell doesn't die before neoplasm phase
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusPlus);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);

    // Create cell with HighNoise and guarantee neoplasm by setting neoplasm_k=1.0
    // division_rate=0.0 to disable division and focus on neoplasm signal
    domain::AgenticCell cell(std::make_unique<test::HighNoise>(), genome, 1.0, 0.0001, 0.0002, 0.0);

    // assign an id so the signal will carry a known sourceId
    cell.setId(42);

    std::unique_ptr<domain::ISignal> captured;
    cell.setSignalEmitter([&captured](std::unique_ptr<domain::ISignal> s){ captured = std::move(s); });

    // Act: run one lifecycle
    cell.live();

    // Assert
    ASSERT_NE(captured, nullptr);
    EXPECT_EQ(captured->type(), domain::ISignal::Type::Neoplasm);
    EXPECT_EQ(captured->sourceId(), 42u);
}
