#include <iostream>
#include "../src/application/simulation/Simulations.h"
#include "../src/application/config/SimulationConfig.h"

int main() {
    // Cargar configuración por defecto
    auto cfg = application::SimulationConfig::loadDefault(/* verbose = */ false);

    // Permitir customización rápida descomentando líneas
    // cfg.n_cells = 100;
    // cfg.max_t = 50;

    std::cout << "Iniciando simulaciones:\n";
    std::cout << "  Número de escenarios (simulaciones): 100\n";
    std::cout << "  Células por escenario: " << cfg.n_cells << "\n";
    std::cout << "  Tiempo máximo analizado (años): " << cfg.max_t << "\n";
    std::cout << "  Semilla (seed): " << cfg.seed << "\n";
    std::cout << "  Thresholds por gen:\n";
    for (const auto& [gene, threshold] : cfg.gene_thresholds) {
        std::cout << "    - " << gene << ": " << threshold << "\n";
    }
    std::cout << "----------------------------------------\n";


    application::Simulations sims(cfg);
    sims.runAll();
    sims.printSummary();
    return 0;
}
