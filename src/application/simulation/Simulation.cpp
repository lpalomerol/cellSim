#include "Simulation.h"
#include <array>

namespace application {

    Simulation::Simulation(int max_t_years) {
        max_t_ = max_t_years;
        cell_state_counter_ = std::vector(max_t_, std::array<int, 3>{0, 0});

    }
    void Simulation::addCell(std::unique_ptr<domain::ICell> cell) {
        cells_.emplace_back(std::move(cell));
    }

    void Simulation::run() {
        int neoplastic_count = 0;
        for (int t = 0; t < max_t_; ++t) {
            neoplastic_count = 0;
            for (auto& c : cells_) {
                c->live();
                if (c->isNeoplastic()) {
                    neoplastic_count += 1;
                }
            }
            cell_state_counter_[t][2] = neoplastic_count;
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