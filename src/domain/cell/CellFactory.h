#pragma once
#include <memory>

#include "SimpleCell.h"
#include "AgenticCell.h"
#include "../adapters/RandomNoise.h"

namespace domain::cell_factory {

// Create a SimpleCell from a RandomNoise and parameters.
std::unique_ptr<ICell> createSimpleCell(adapters::RandomNoise& noise, const domain::SimpleCellParams& params);

// Create an AgenticCell accepting a seed for the noise source and a movable genome
std::unique_ptr<ICell> createAgenticCell(unsigned seed, domain::Genome genome, double neoplasm_k,
    double low_delta_instability = 0.0001,
    double high_delta_instability = 0.0002,
    bool verbose = false);

// Overload: allows directly injecting the noise source
std::unique_ptr<ICell> createAgenticCell(std::unique_ptr<INoiseSource> noise, domain::Genome genome, double neoplasm_k,
    double low_delta_instability = 0.0001,
    double high_delta_instability = 0.0002,
    bool verbose = false);

} // namespace domain::cell_factory
