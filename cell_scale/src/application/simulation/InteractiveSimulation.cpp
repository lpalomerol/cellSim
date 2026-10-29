// filepath: src/application/simulation/InteractiveSimulation.cpp
#include "InteractiveSimulation.h"
#include <iostream>

namespace application {

InteractiveSimulation::InteractiveSimulation(int max_t_years)
    : Simulation(max_t_years), current_year_(0), current_neoplastic_count_(0), first_time_neoplastic_(-1) {}

void InteractiveSimulation::addCell(std::unique_ptr<domain::ICell> cell) {
    std::cout << "------------------------" << std::endl;
    std::cout << "New cell:" << std::endl;
    cell->details();
    std::cout << "Cell added to simulation." << std::endl;
    std::cout << "------------------------" << std::endl;

    total_cells_ever_++;
    tissue_.addCell(std::move(cell));
}

bool InteractiveSimulation::step(MenuOption option) {
    if (current_year_ >= max_t_) return false;

    if (option == MenuOption::Quit) {
        std::cout << "Simulation ended by user." << std::endl;
        return false;
    }

    ++current_year_;
    std::cout << "--- Year " << current_year_ << " ---" << std::endl;

    for (auto* c : tissue_.getLiveCells()) {
        switch (option) {
            case MenuOption::MutateBRCA1:
                c->mutateGene("BRCA1");
                std::cout << "BRCA1 mutation applied." << std::endl;
                break;
            case MenuOption::MutateTP53:
                c->mutateGene("TP53");
                std::cout << "TP53 mutation applied." << std::endl;
                break;
            case MenuOption::None:
            case MenuOption::Quit:
                break;
        }
    }

    current_neoplastic_count_ = executeCellCycle();

    if (current_neoplastic_count_ > 0 && first_time_neoplastic_ == -1) {
        first_time_neoplastic_ = current_year_;
    }

    return current_year_ < max_t_;
}


} // namespace application
