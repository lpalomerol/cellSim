#pragma once
#include <vector>
#include <array>
#include <memory>
#include "../../domain/ports/ICell.h"
#include "../../domain/tissue/Tissue.h"
#include "PopulationTracker.h"

namespace application {
    class Simulation {
    public:
        explicit Simulation(int max_t_years);
        void addCell(std::unique_ptr<domain::ICell> cell);
        void run(); // dt=1 year, up to max_t_

        [[nodiscard]] int firstTimeNeoplastic();

        // Executes one year cell cycle; returns neoplastic count
        [[nodiscard]] int executeCellCycle();

        // Access population tracker
        [[nodiscard]] const PopulationTracker& populationTracker() const {
            return population_tracker_;
        }

    protected:
        int max_t_ = 0;
        domain::Tissue tissue_;
        int total_cells_ever_ = 0;
        int cumulative_dead_ = 0;
        std::vector<std::array<int, 3>> cell_state_counter_;
        PopulationTracker population_tracker_;

        // Capture annual population snapshot
        void captureAnnualSnapshot(int year);
    };
}