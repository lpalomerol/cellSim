#include <iostream>
#include "../src/application/simulation/Simulation.h"
#include "../src/domain/cell/SimpleCell.h"
#include "../src/domain/adapters/RandomNoise.h"

int main() {
    int n_cells = 5;
    int num_simulations = 10;
    std::vector<std::vector<int>> results(num_simulations, std::vector<int>(2, 0));

    for (int k = 0; k < num_simulations; ++k) {
        std::cout << "Running simulation #" << (k+1) << "\n";

        adapters::RandomNoise noise(k);
        constexpr int max_t = 80; // 1 año por tick
        application::Simulation sim(max_t);
        domain::SimpleCellParams the_params = domain::SimpleCellParams();

        for (int i = 0; i < n_cells; ++i) {
            sim.addCell(std::make_unique<domain::SimpleCell>(
                noise, the_params));
        }

        sim.run();
        results[k][0] = sim.firstTimeDead();
        results[k][1] = sim.firstTimeTumoral();

    }

    std::cout << "Simulation Results over " << num_simulations << " runs:\n";
    std::cout << "Run\tFirst Dead Year\tFirst Tumoral Year\n";
    for (int k = 0; k < num_simulations; ++k) {
        std::cout << k << "\t" << results[k][0] << "\t" << results[k][1] << "\n";
    }
    return 0;
}