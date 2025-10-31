#include <iostream>
#include "../src/application/simulation/Simulations.h"

int main() {
    application::SimulationsConfig cfg;
    // por defecto: n_cells=100, num_simulations=100, max_t=80
    application::Simulations sims(cfg);
    sims.runAll();
    sims.printSummary();
    return 0;
}
