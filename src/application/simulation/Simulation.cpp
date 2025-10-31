#include "Simulation.h"
#include <iostream>
#include <array>

namespace application {

    Simulation::Simulation(int max_t_years) {
        max_t_ = max_t_years;
        cell_state_counter_ = std::vector<std::array<int, 2>>(max_t_, std::array<int, 2>{0, 0});

    }
    void Simulation::addCell(std::unique_ptr<domain::ICell> cell) {
        cells_.emplace_back(std::move(cell));
    }

    void Simulation::run() {
        int alives = 0;
        int deads = 0;

        std::cout << "Starting simulation (dt=1 year, max_t=" << max_t_
                  << ", cells=" << cells_.size() << ")\n";
        for (int t = 0; t < max_t_; ++t) {
            alives = 0;
            deads = 0;
            for (auto& c : cells_) {
                c->live();
                if (c-> alive()) {
                    alives += 1;
                } else {
                    deads += 1;
                }
                std::cout << "\n";
            }
            cell_state_counter_[t][0] = alives;
            cell_state_counter_[t][1] = deads;
        }
        std::cout << "Simulation finished.\n";
    }

    void Simulation::printResults() {
        std::cout << "Year\tAlive\tDead\n";
        for (int t = 0; t < max_t_; ++t) {
            std::cout << (t+1) << "\t" << cell_state_counter_[t][0]
                      << "\t" << cell_state_counter_[t][1] << "\n";
        }
    }

}