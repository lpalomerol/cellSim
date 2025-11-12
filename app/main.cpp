#include <iostream>
#include <unordered_map>
#include <string>
#include "../src/application/simulation/Simulations.h"

int main() {
    // Parámetros de prueba - definidos aquí para pruebas mínimas y centralizar cambios
    // Cambia estos valores según necesites para experimentar rápidamente.
    int n_cells = 100;             // Número de células por simulación (default 100)
    int num_simulations = 1;     // Número de simulaciones a ejecutar (default 200)
    int max_t = 80;                // Tiempo máximo por simulación (años)
    int seed = -1;                 // Semilla global (-1 = variar por simulación)
    double neoplasm_k = 0.3;    // Probabilidad base de neoplasia
    bool verbose = false;          // Activar trazas verbose (true/false)

    // Mutational parameters: thresholds y componentes de inestabilidad por gen
    // Define aquí todos los genes que usa el genoma por defecto
    std::unordered_map<std::string, double> gene_thresholds{
            {"TP53", 0.010},
            {"BRCA1", 0.015}
    };
    std::unordered_map<std::string, double> gene_instability_k{
            {"TP53", 0.010},
            {"BRCA1", 0.015}
    };

    // --- Imprimir descripción inicial de la simulación ---
    std::cout << "Iniciando simulaciones:\n";
    std::cout << "  Número de escenarios (simulaciones): " << num_simulations << "\n";
    std::cout << "  Células por escenario: " << n_cells << "\n";
    std::cout << "  Tiempo máximo analizado (años): " << max_t << "\n";
    std::cout << "  Semilla (seed): " << seed << "\n";
    std::cout << "  Thresholds por gen:\n";
    for (const auto& p : gene_thresholds) {
        std::cout << "    - " << p.first << ": " << p.second << "\n";
    }
    std::cout << "  Componentes de inestabilidad (k) por gen:\n";
    for (const auto& p : gene_instability_k) {
        std::cout << "    - " << p.first << ": " << p.second << "\n";
    }
    std::cout << "----------------------------------------\n";

    application::SimulationsConfig cfg;
    cfg.n_cells = n_cells;
    cfg.num_simulations = num_simulations;
    cfg.max_t = max_t;
    cfg.seed = seed;
    cfg.neoplasm_k = neoplasm_k;
    cfg.verbose = verbose;
    // Asignar thresholds e instability k al config para que las simulaciones los usen
    cfg.gene_mutation_thresholds = std::move(gene_thresholds);
    cfg.gene_mutation_instability_k = std::move(gene_instability_k);

    application::Simulations sims(cfg);
    sims.runAll();
    sims.printSummary();
    return 0;
}
