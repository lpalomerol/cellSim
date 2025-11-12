// filepath: src/application/simulation/InteractiveSimulation.cpp
#include "InteractiveSimulation.h"
#include <iostream>


namespace application {

InteractiveSimulation::InteractiveSimulation(int max_t_years)
: max_t_(max_t_years), current_year_(0), current_neoplastic_count_(0), first_time_neoplastic_(-1) {}

void InteractiveSimulation::addCell(std::unique_ptr<domain::ICell> cell) {
    std::cout << "------------------------" << std::endl;
    std::cout << "Creando nueva célula:" << std::endl;
    cell->details();
    std::cout << "Célula añadida a la simulación." << std::endl;
    std::cout << "------------------------" << std::endl;

    cells_.emplace_back(std::move(cell));
}

// Nueva sobrecarga: realiza un año de simulación aplicando la opción proporcionada
bool InteractiveSimulation::step(MenuOption option) {
    if (current_year_ >= max_t_) return false;

    // Si el usuario pidió salir, detener la simulación sin avanzar el año
    if (option == MenuOption::Quit) {
        std::cout << "Simulación finalizada por usuario (opción 'q')." << std::endl;
        return false;
    }

    ++current_year_;
    int neoplastic_count = 0;
    std::cout << "--- Year " << current_year_ << " ---" << std::endl;
    for (auto& c : cells_) {
        switch (option) {
            case MenuOption::MutarBRCA:
                c->mutateGene("BRCA1");
                std::cout << "Mutación BRCA1 aplicada a la célula." << std::endl;
                break;
            case MenuOption::MutarTP53:
                c->mutateGene("TP53");
                std::cout << "Mutación TP53 aplicada a la célula." << std::endl;
                break;
            case MenuOption::Nada:
            case MenuOption::Quit:
                // No hacer nada
                break;
        }
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
