#pragma once
#include <memory>

#include "AgenticCell.h"
#include "../adapters/RandomNoise.h"
#include "../ports/ILogger.h"

namespace domain::cell_factory {

// Create an AgenticCell accepting a seed for the noise source and a movable genome
std::unique_ptr<ICell> createAgenticCell(unsigned seed, domain::Genome genome, double neoplasm_k,
    double low_delta_instability = 0.0001,
    double high_delta_instability = 0.0002,
    double division_rate = 0.001,
    bool verbose = false,
    ports::ILoggerPtr logger = nullptr);

// Overload: allows directly injecting the noise source
std::unique_ptr<ICell> createAgenticCell(std::unique_ptr<INoiseSource> noise, domain::Genome genome, double neoplasm_k,
    double low_delta_instability = 0.0001,
    double high_delta_instability = 0.0002,
    double division_rate = 0.001,
    bool verbose = false,
    ports::ILoggerPtr logger = nullptr);

} // namespace domain::cell_factory
