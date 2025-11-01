#include "Simulations.h"
#include <iostream>
#include "../../domain/gene/GenomeFactory.h"

namespace application {

Simulations::Simulations(const SimulationsConfig& cfg)
: cfg_(cfg), results_(cfg.num_simulations, { -1, -1 }) {}

void Simulations::runAll() {
    results_.assign(cfg_.num_simulations, { -1, -1 });

    for (int k = 0; k < cfg_.num_simulations; ++k) {
        // seed base por simulación: usa cfg_.seed si está fijada, si no usa k
        unsigned sim_seed = (cfg_.seed >= 0) ? static_cast<unsigned>(cfg_.seed) : static_cast<unsigned>(k);
        Simulation sim(cfg_.max_t);
        for (int i = 0; i < cfg_.n_cells; ++i) {
            unsigned cell_seed = sim_seed * 100000u + static_cast<unsigned>(i);
            // Obtener un genoma por defecto usando thresholds e instability_k proporcionados por la configuración
            domain::Genome genome = domain::genome_factory::makeDefaultGenome(cfg_.gene_mutation_thresholds, cfg_.gene_mutation_instability_k);
            sim.addCell(domain::cell_factory::createAgenticCell(cell_seed, std::move(genome), cfg_.neoplasm_k));
        }
        sim.run();
        results_[k][1] = sim.firstTimeNeoplastic();
    }
}


void Simulations::printSummary() const {
    int num_simulations = static_cast<int>(results_.size());
    int count_minus_one = 0;
    for (const auto& res : results_) {
        if (res[1] == -1) ++count_minus_one;
    }
    double ratio_minus_one = static_cast<double>(count_minus_one) / num_simulations;
    std::cout << "Proporción de casos sin neoplasia: " << ratio_minus_one << "\n";

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
        std::cout << "Proporción de neoplasia en <" << thresholds[i] << " años: "
                  << static_cast<double>(counts[i]) / num_simulations << "\n";
    }
}

} // namespace application
