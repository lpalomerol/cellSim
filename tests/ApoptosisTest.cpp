#include <gtest/gtest.h>
#include <memory>
#include "TestNoise.h"
#include "../src/domain/cell/AgenticCell.h"
#include "../src/domain/signal/ApoptosisSignal.h"

// Test that ApenticCell receives and processes an apoptosis signal (broadcast)
TEST(ApoptosisSignalTest, CellReceivesApoptosisSignalBroadcast) {
    // Build a genome with BRCA1 in PlusMinus state so one mutation disables it
    domain::Gene tp53("TP53", domain::Gene::State::PlusPlus);
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusMinus);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);

    // Create cell with dummy noise
    domain::AgenticCell cell(std::make_unique<test::DummyNoise>(), genome, 0.002, 0.0001, 0.0002, 0.001, false);
    cell.setId(1);

    // Verify cell is alive before apoptosis signal
    ASSERT_TRUE(cell.alive());

    // Create apoptosis signal (broadcast, no specific targets)
    auto apoptosis_signal = std::make_unique<domain::ApoptosisSignal>(0, "cell_death_signal");

    // Send apoptosis signal to cell
    cell.receiveMessage(std::move(apoptosis_signal));

    // Run the cell lifecycle which should process the message and trigger apoptosis
    cell.live();

    // After apoptosis, cell should be dead
    EXPECT_FALSE(cell.alive());
}

// Test that ApoptosisSignal with directed target reaches intended cell
TEST(ApoptosisSignalTest, ApoptosisSignalWithDirectedTarget) {
    // Build a genome with BRCA1 in PlusMinus state so one mutation disables it
    domain::Gene tp53("TP53", domain::Gene::State::PlusPlus);
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusMinus);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);

    // Create cell
    domain::AgenticCell cell(std::make_unique<test::DummyNoise>(), genome, 0.002, 0.0001, 0.0002, 0.001, false);
    cell.setId(1);

    // Verify cell is alive before apoptosis signal
    ASSERT_TRUE(cell.alive());

    // Create apoptosis signal directed to cell 1
    auto apoptosis_signal = std::make_unique<domain::ApoptosisSignal>(0, "targeted_death", std::vector<std::uint64_t>{1});

    // Send apoptosis signal to cell
    cell.receiveMessage(std::move(apoptosis_signal));

    // Run the cell lifecycle which should process the message and trigger apoptosis
    cell.live();

    // After apoptosis, cell should be dead
    EXPECT_FALSE(cell.alive());
}

// Test that ApoptosisSignal directed to different cell is ignored
TEST(ApoptosisSignalTest, ApoptosisSignalNotDirectedToCell) {
    // Build a healthy genome
    domain::Gene tp53("TP53", domain::Gene::State::PlusPlus);
    domain::Gene brca1("BRCA1", domain::Gene::State::PlusPlus);
    std::unordered_map<std::string, domain::Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    domain::Genome genome(genes);

    // Create cell
    domain::AgenticCell cell(std::make_unique<test::DummyNoise>(), genome, 0.002, 0.0001, 0.0002, 0.001, false);
    cell.setId(1);

    // Verify cell is alive before apoptosis signal
    ASSERT_TRUE(cell.alive());

    // Create apoptosis signal directed to cell 2 (not our cell with id=1)
    auto apoptosis_signal = std::make_unique<domain::ApoptosisSignal>(0, "targeted_death", std::vector<std::uint64_t>{2});

    // Send apoptosis signal to cell
    cell.receiveMessage(std::move(apoptosis_signal));

    // Run the cell lifecycle which should NOT process the message since it's not for us
    cell.live();

    // Cell should still be alive
    EXPECT_TRUE(cell.alive());
}

