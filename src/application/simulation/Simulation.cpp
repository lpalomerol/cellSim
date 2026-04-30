#include "Simulation.h"
#include <array>
#include <algorithm>
#include <limits>
#include "../../domain/cell/AgenticCell.h"

namespace application {

    Simulation::Simulation(int max_t_years) {
        max_t_ = max_t_years;
        cell_state_counter_ = std::vector(max_t_, std::array<int, 3>{0, 0, 0});
    }

    void Simulation::addCell(std::unique_ptr<domain::ICell> cell) {
        if (cell) {
            total_cells_ever_++;
            tissue_.addCell(std::move(cell));
        }
    }

    int Simulation::executeCellCycle() {
        tissue_.live();
        cumulative_dead_ += tissue_.lastDeathCount();
        total_cells_ever_ += tissue_.lastBirthCount();

        int neoplastic_count = 0;
        for (auto* cell : tissue_.getLiveCells()) {
            if (cell->isNeoplastic()) neoplastic_count++;
        }
        return neoplastic_count;
    }

    void Simulation::captureAnnualSnapshot(int year) {
        auto live_cells = tissue_.getLiveCells();
        int alive_count = static_cast<int>(live_cells.size());
        int neoplastic_alive = 0;
        int protected_alive = 0;
        int neoplastic_apoptosis_susceptible = 0;
        int neoplastic_apoptosis_resistant = 0;

        double min_instability = std::numeric_limits<double>::max();
        double max_instability = std::numeric_limits<double>::lowest();

        int tp53_plus_plus = 0;
        int tp53_plus_minus = 0;
        int tp53_minus_minus = 0;

        for (auto* cell : live_cells) {
            std::string tp53_status = cell->getTP53Status();
            if (tp53_status == "+/+") tp53_plus_plus++;
            else if (tp53_status == "+/-") tp53_plus_minus++;
            else if (tp53_status == "-/-") tp53_minus_minus++;

            if (cell->isNeoplastic()) {
                neoplastic_alive++;
                auto* agentic_cell = dynamic_cast<domain::AgenticCell*>(cell);
                if (agentic_cell && agentic_cell->hasEvadedApoptosis()) {
                    neoplastic_apoptosis_resistant++;
                } else {
                    neoplastic_apoptosis_susceptible++;
                }
            } else {
                protected_alive++;
            }

            auto* agentic_cell = dynamic_cast<domain::AgenticCell*>(cell);
            if (agentic_cell) {
                double d1 = agentic_cell->getD1();
                min_instability = std::min(min_instability, d1);
                max_instability = std::max(max_instability, d1);
            }
        }

        if (alive_count == 0) {
            min_instability = 0.0;
            max_instability = 0.0;
        }

        int tp53_total = tp53_plus_plus + tp53_plus_minus + tp53_minus_minus;
        double tp53_pp_pct = (tp53_total > 0) ? static_cast<double>(tp53_plus_plus) / tp53_total : 0.0;
        double tp53_pm_pct = (tp53_total > 0) ? static_cast<double>(tp53_plus_minus) / tp53_total : 0.0;
        double tp53_mm_pct = (tp53_total > 0) ? static_cast<double>(tp53_minus_minus) / tp53_total : 0.0;

        YearlySnapshot snapshot{
            year,
            total_cells_ever_,
            alive_count,
            cumulative_dead_,
            neoplastic_alive,
            protected_alive,
            min_instability,
            max_instability,
            0.0, 0.0,  // min_d2, max_d2 (not tracked via ICell yet)
            tp53_pp_pct,
            tp53_pm_pct,
            tp53_mm_pct,
            neoplastic_apoptosis_susceptible,
            neoplastic_apoptosis_resistant
        };

        population_tracker_.addSnapshot(snapshot);
    }

    void Simulation::run() {
        captureAnnualSnapshot(0);

        for (int t = 0; t < max_t_; ++t) {
            int neoplastic_count = executeCellCycle();
            cell_state_counter_[t][2] = neoplastic_count;
            captureAnnualSnapshot(t + 1);
        }
    }

    int Simulation::firstTimeNeoplastic() {
        for (int t = 0; t < max_t_; ++t) {
            if (cell_state_counter_[t][2] > 0) {
                return t + 1;
            }
        }
        return -1;
    }

}