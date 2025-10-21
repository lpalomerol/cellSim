#include "Simulation.h"
#include <iostream>

namespace application {

    void Simulation::addCell(std::unique_ptr<domain::ICell> cell) {
        cells_.emplace_back(std::move(cell));
    }

    void Simulation::run() {
        std::cout << "Starting simulation (dt=1 year, max_t=" << max_t_
                  << ", cells=" << cells_.size() << ")\n";
        for (int t = 0; t < max_t_; ++t) {
            std::cout << "--- Tick year " << (t+1) << " ---\n";
            for (auto& c : cells_) {
                c->live();
                std::cout << "\n";
            }
        }
        std::cout << "Simulation finished.\n";
    }

}