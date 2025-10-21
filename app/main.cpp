#include <iostream>
#include "../application/simulation/Simulation.h"
#include "../domain/cell/SimpleCell.h"
#include "../domain/adapters/RandomNoise.h"

int main() {
    adapters::RandomNoise noise(42);
    constexpr int max_t = 80; // 1 año por tick
    application::Simulation sim(max_t);
    domain::SimpleCellParams the_params = domain::SimpleCellParams();
    // Por ahora creamos algunas células de ejemplo:
    sim.addCell(std::make_unique<domain::SimpleCell>(
        noise, the_params));
    sim.addCell(std::make_unique<domain::SimpleCell>(
        noise, the_params));

    sim.run();
    return 0;
}