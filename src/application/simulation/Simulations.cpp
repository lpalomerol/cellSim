#include "Simulations.h"
#include <iostream>

namespace application {

Simulations::Simulations(const SimulationsConfig& cfg)
: cfg_(cfg), results_(cfg.num_simulations, { -1, -1 }) {}

void Simulations::runAll() {
    results_.assign(cfg_.num_simulations, { -1, -1 });

    for (int k = 0; k < cfg_.num_simulations; ++k) {
        // seed base por simulación
        unsigned sim_seed = static_cast<unsigned>(k);
        Simulation sim(cfg_.max_t);
        for (int i = 0; i < cfg_.n_cells; ++i) {
            domain::AgenticCellParams params; // defaults
            unsigned cell_seed = sim_seed * 100000u + static_cast<unsigned>(i);
            sim.addCell(factory_.createAgenticCell(cell_seed, params));
        }
        sim.run();
        results_[k][1] = sim.firstTimeTumoral();
    }
}

void Simulations::printSummary() const {
    int num_simulations = static_cast<int>(results_.size());
    int count_minus_one = 0;
    for (const auto& res : results_) {
        if (res[1] == -1) ++count_minus_one;
    }
    double ratio_minus_one = static_cast<double>(count_minus_one) / num_simulations;
    std::cout << "Ratio de -1 en First Tumoral Year: " << ratio_minus_one << "\n";

    std::vector<int> thresholds = {10, 20, 30, 40, 50, 60, 70, 80};
    std::vector<int> counts(thresholds.size(), 0);

    for (const auto& res : results_) {
        if (res[1] != -1) {
            for (size_t i = 0; i < thresholds.size(); ++i) {
                if (res[1] < thresholds[i]) ++counts[i];
            }
        }
    }

    for (size_t i = 0; i < thresholds.size(); ++i) {
        std::cout << "Proporción de tumor en <" << thresholds[i] << " años: "
                  << static_cast<double>(counts[i]) / num_simulations << "\n";
    }
}

} // namespace application
