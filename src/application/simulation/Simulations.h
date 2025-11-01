#pragma once

#include <vector>
#include <array>
#include <cstddef>

#include "Simulation.h"
#include "../../domain/cell/CellFactory.h"
#include "../../domain/adapters/RandomNoise.h"

namespace application {

struct SimulationsConfig {
    int n_cells = 100;
    int num_simulations = 100;
    int max_t = 80;
    // Probabilidad base de neoplasia usada cuando TP53 está inactivo
    double neoplasm_k = 0.002;
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
    domain::CellFactory factory_;
    std::vector<std::array<int,2>> results_;
};

} // namespace application
