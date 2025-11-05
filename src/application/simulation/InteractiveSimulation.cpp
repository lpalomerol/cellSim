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

bool InteractiveSimulation::step() {
    if (current_year_ >= max_t_) return false;
    MenuOption option = menu();
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
                // No hacer nada
                break;
        }
        c->live();
        // Mostrar detalles de la célula tras avanzar (la implementación del cell decide si imprime)
        c->details();
        if (c->isNeoplastic()) ++neoplastic_count;
    }
    current_neoplastic_count_ = neoplastic_count;
    if (current_neoplastic_count_ > 0 && first_time_neoplastic_ == -1) {
        first_time_neoplastic_ = current_year_;
    }

    // Devuelve true si aún quedan años por ejecutar después de este step
    return current_year_ < max_t_;
}

// Implementación del menú interactivo: imprime opciones y lee una línea de stdin
 MenuOption InteractiveSimulation::menu() {
     std::cout << "------------------------\n";
     std::cout << "Menú de mutaciones:\n";
     std::cout << "1 -> Mutar BRCA; 2 -> Mutar TP53; otra tecla -> No mutar\n";
     std::cout << "------------------------\n";
     std::string input;
     std::getline(std::cin, input);
     if (input.empty()) {
         return MenuOption::Nada;
     }

     char opt = input[0];
     switch (opt) {
         case '1':
             std::cout << "Seleccionada opción 1: marcar mutación BRCA1 (no aplicada aquí)\n";
             return MenuOption::MutarBRCA;
             break;
         case '2':
             std::cout << "Seleccionada opción 2: marcar mutación TP53 (no aplicada aquí)\n";
             return MenuOption::MutarTP53;
             break;
         default:
             std::cout << "No se seleccionó ninguna mutación.\n";
             return MenuOption::Nada;
             break;
     }

 }


} // namespace application
