#include "Simulation.h"
#include <iostream>
#include <array>
#include "../../domain/cell/CellState.h"
namespace application {

    Simulation::Simulation(int max_t_years) {
        max_t_ = max_t_years;
        cell_state_counter_ = std::vector(max_t_, std::array<int, 3>{0, 0});

    }
    void Simulation::addCell(std::unique_ptr<domain::ICell> cell) {
        cells_.emplace_back(std::move(cell));
    }

    void Simulation::run() {
        int tumors = 0;
        domain::OncoState state;
        for (int t = 0; t < max_t_; ++t) {
            tumors = 0;
            for (auto& c : cells_) {
                c->live();
                state = c->getOncoState();
                switch (state) {
                    case domain::OncoState::TP53_minus_minus:
                        tumors += 1;
                        break;
                    default:
                        break;

                }
            }
            cell_state_counter_[t][2] = tumors;
        }
    }


    int Simulation::firstTimeTumoral() {
        for (int t = 0; t < max_t_; ++t) {
            if (cell_state_counter_[t][2] > 0) {
                return t + 1;
            }
        }
        return -1;
    }




}