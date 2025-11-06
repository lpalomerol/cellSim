#pragma once

#include <vector>
#include <array>
#include <cstddef>
#include <string>
#include <unordered_map>

#include "Simulation.h"
#include "../../domain/cell/CellFactory.h"
#include "../../domain/adapters/RandomNoise.h"

namespace application {

struct SimulationsConfig {
    int n_cells = 100;
    int num_simulations = 200;
    int max_t = 80;
    // Probabilidad base de neoplasia usada cuando TP53 está inactivo
    double neoplasm_k = 0.003;

    // Semilla fija para reproducibilidad; -1 = aleatoria por simulación
    int seed = -1;

    // Umbrales de mutación por gen (probabilidad básica)
    std::unordered_map<std::string, double> gene_mutation_thresholds{
        {"TP53", 0.994},
        {"BRCA1", 0.995}
    };

    // Componentes de inestabilidad (k) que se suman al threshold en cada tick
    std::unordered_map<std::string, double> gene_mutation_instability_k{
        {"TP53", -0.010},
        {"BRCA1", -0.005}
    };

    // Nuevo flag: activar trazas verbose para genoma/células/genes (por defecto false)
    bool verbose = false;
};

class Simulations {
public:
    explicit Simulations(const SimulationsConfig& cfg = SimulationsConfig());

    // Ejecuta todas las simulaciones (con seeds 0..num_simulations-1)
    void runAll();

    // Devuelve los resultados (vector de pares, index 1 = firstTimeNeoplastic)
    const std::vector<std::array<int,2>>& results() const { return results_; }

    // Imprime un resumen de estadísticas por consola
    void printSummary() const;

private:
    SimulationsConfig cfg_;
    std::vector<std::array<int,2>> results_;
};

} // namespace application
