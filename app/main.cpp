#include <iostream>
#include "../application/simulation/Simulation.h"
#include "../domain/cell/SimpleCell.h"

int main() {
    constexpr int max_t = 80; // 1 año por tick
    application::Simulation sim(max_t);

    // Por ahora creamos algunas células de ejemplo:
    sim.addCell(std::make_unique<domain::SimpleCell>());
    sim.addCell(std::make_unique<domain::SimpleCell>());

    sim.run();
    return 0;
}