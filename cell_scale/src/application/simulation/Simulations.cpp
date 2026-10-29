#include "Simulations.h"
#include <iostream>
#include <cstdint>
#include "../../domain/gene/GenomeFactory.h"

namespace application {

Simulations::Simulations(const SimulationConfig& cfg)
    : cfg_(cfg), num_simulations_(100), results_(100, { -1, -1 }) {}

void Simulations::runAll() {
    results_.assign(num_simulations_, { -1, -1 });

    for (int k = 0; k < num_simulations_; ++k) {
        // Use cfg_.seed >= 0 as base seed per simulation for reproducibility; fall back to k
        unsigned sim_seed = (cfg_.seed >= 0) ? static_cast<unsigned>(cfg_.seed) + static_cast<unsigned>(k)
                                             : static_cast<unsigned>(k);
        Simulation sim(cfg_.max_t);
        for (int i = 0; i < cfg_.n_cells; ++i) {
            // Spread 64-bit cell seed to avoid overflow
            std::uint64_t cell_seed64 = static_cast<std::uint64_t>(sim_seed) * 100000ull + static_cast<std::uint64_t>(i);
            auto cell_seed = static_cast<unsigned>(cell_seed64 & 0xFFFFFFFFull);
            domain::Genome genome = domain::genome_factory::makeDefaultGenome(
                cfg_.gene_thresholds,
                cfg_.gene_instability_k,
                cfg_.logger
            );
            auto noise = std::make_unique<domain::adapters::RandomNoise>();
            sim.addCell(
                domain::CellFactory::createCustomCell(
                    std::move(noise),
                    std::move(genome),
                    0.0001,  // low_delta_instability
                    0.0002,  // high_delta_instability
                    0.001,   // division_rate
                    0.001,   // neoplastic_division_rate (default)
                    false,   // enable_big_bang_mode (default false)
                    2.0,     // d1_primer_threshold
                    5.0,     // d2_apoptosis_threshold
                    cfg_.logger)
            );
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
    std::cout << "No-neoplasia ratio: " << ratio_minus_one << "\n";

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
        std::cout << "Neoplasia in <" << thresholds[i] << " years: "
                  << static_cast<double>(counts[i]) / num_simulations << "\n";
    }
}

} // namespace application


