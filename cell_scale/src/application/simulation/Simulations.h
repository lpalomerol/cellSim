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

    void runAll();

    [[nodiscard]] const std::vector<std::array<int,2>>& results() const { return results_; }

    void printSummary() const;

private:
    SimulationConfig cfg_;
    int num_simulations_ = 100;
    std::vector<std::array<int,2>> results_;
};

} // namespace application


