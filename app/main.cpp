#include <iostream>
#include "../src/application/simulation/Simulations.h"
#include "../src/application/config/SimulationConfig.h"

int main() {
    // Cargar configuración por defecto
    auto sim_cfg = application::SimulationConfig::loadDefault(/* verbose = */ false);

    // Crear SimulationsConfig adaptado
    application::SimulationsConfig cfg;
    cfg.n_cells = sim_cfg.n_cells;
    cfg.num_simulations = 100;  // Para simulaciones batch
    cfg.max_t = sim_cfg.max_t;
    cfg.seed = sim_cfg.seed;
    cfg.neoplasm_k = sim_cfg.neoplasm_k;
    cfg.gene_mutation_thresholds = sim_cfg.gene_thresholds;
    cfg.gene_mutation_instability_k = sim_cfg.gene_instability_k;

    std::cout << "Iniciando simulaciones:\n";
    std::cout << "  Número de escenarios (simulaciones): " << cfg.num_simulations << "\n";
    std::cout << "  Células por escenario: " << cfg.n_cells << "\n";
    std::cout << "  Tiempo máximo analizado (años): " << cfg.max_t << "\n";
    std::cout << "  Semilla (seed): " << cfg.seed << "\n";
    std::cout << "  Thresholds por gen:\n";
    for (const auto& [gene, threshold] : cfg.gene_mutation_thresholds) {
        std::cout << "    - " << gene << ": " << threshold << "\n";
    }
    std::cout << "----------------------------------------\n";


    application::Simulations sims(cfg);
    sims.runAll();
    sims.printSummary();
    return 0;
}
