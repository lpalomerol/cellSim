#pragma once

#include <vector>
#include <array>
#include <cstddef>
#include <string>
#include <memory>

#include "Simulation.h"
#include "../config/SimulationConfig.h"
#include "../../domain/cell/CellFactory.h"
#include "../../domain/adapters/RandomNoise.h"
#include "../../domain/ports/ILogger.h"

namespace application {

class Simulations {
public:
    explicit Simulations(const SimulationConfig& cfg);

    // Ejecuta todas las simulaciones (con seeds 0..num_simulations-1)
    void runAll();

    // Devuelve los resultados (vector de pares, index 1 = firstTimeNeoplastic)
    [[nodiscard]] const std::vector<std::array<int,2>>& results() const { return results_; }

    // Imprime un resumen de estadísticas por consola
    void printSummary() const;

private:
    SimulationConfig cfg_;
    int num_simulations_ = 100;  // Número de simulaciones a ejecutar
    std::vector<std::array<int,2>> results_;
};

} // namespace application


