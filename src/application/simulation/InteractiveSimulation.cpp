// filepath: src/application/simulation/InteractiveSimulation.cpp
#include "InteractiveSimulation.h"

namespace application {

InteractiveSimulation::InteractiveSimulation(int max_t_years)
: max_t_(max_t_years), current_year_(0), current_neoplastic_count_(0), first_time_neoplastic_(-1) {}

void InteractiveSimulation::addCell(std::unique_ptr<domain::ICell> cell) {
    cells_.emplace_back(std::move(cell));
}

bool InteractiveSimulation::step() {
    if (current_year_ >= max_t_) return false;

    ++current_year_;
    int neoplastic_count = 0;
    for (auto& c : cells_) {
        c->live();
        if (c->isNeoplastic()) ++neoplastic_count;
    }
    current_neoplastic_count_ = neoplastic_count;
    if (current_neoplastic_count_ > 0 && first_time_neoplastic_ == -1) {
        first_time_neoplastic_ = current_year_;
    }

    // Devuelve true si aún quedan años por ejecutar después de este step
    return current_year_ < max_t_;
}

} // namespace application

