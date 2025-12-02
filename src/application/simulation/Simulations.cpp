#include "Simulations.h"
#include <iostream>
#include <cstdint>
#include "../../domain/gene/GenomeFactory.h"

namespace application {

Simulations::Simulations(const SimulationsConfig& cfg)
: cfg_(cfg), results_(cfg.num_simulations, { -1, -1 }) {}

void Simulations::runAll() {
    results_.assign(cfg_.num_simulations, { -1, -1 });

    for (int k = 0; k < cfg_.num_simulations; ++k) {
        // seed base por simulación: si cfg_.seed >= 0 usar cfg_.seed + k para que cada
        // simulación sea reproducible pero distinta; si cfg_.seed < 0 usar k.
        unsigned sim_seed = (cfg_.seed >= 0) ? static_cast<unsigned>(cfg_.seed) + static_cast<unsigned>(k)
                                             : static_cast<unsigned>(k);
        Simulation sim(cfg_.max_t);
        for (int i = 0; i < cfg_.n_cells; ++i) {
            // construir la seed de la célula usando 64 bits para evitar overflow
            std::uint64_t cell_seed64 = static_cast<std::uint64_t>(sim_seed) * 100000ull + static_cast<std::uint64_t>(i);
            auto cell_seed = static_cast<unsigned>(cell_seed64 & 0xFFFFFFFFull);
             // Obtener un genoma por defecto usando thresholds e instability_k proporcionados por la configuración
             domain::Genome genome = domain::genome_factory::makeDefaultGenome(
                 cfg_.gene_mutation_thresholds,
                 cfg_.gene_mutation_instability_k,
                 cfg_.logger
                 );
             sim.addCell(
                 domain::cell_factory::createAgenticCell(
                     cell_seed,
                     std::move(genome),
                     cfg_.neoplasm_k,
                     cfg_.low_delta_instability,
                     cfg_.high_delta_instability,
                     0.001,
                     10.0,
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
