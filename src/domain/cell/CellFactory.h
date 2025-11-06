#pragma once
#include <memory>

#include "SimpleCell.h"
#include "AgenticCell.h"
#include "../adapters/RandomNoise.h"

namespace domain::cell_factory {

// Crea una SimpleCell a partir de un RandomNoise y parámetros.
std::unique_ptr<ICell> createSimpleCell(adapters::RandomNoise& noise, const domain::SimpleCellParams& params);

// Crea una AgenticCell recibiendo una seed para la fuente de ruido y un genoma movable
std::unique_ptr<ICell> createAgenticCell(unsigned seed, domain::Genome genome, double neoplasm_k, bool verbose = false);

// Sobrecarga: permite inyectar directamente la fuente de ruido
std::unique_ptr<ICell> createAgenticCell(std::unique_ptr<INoiseSource> noise, domain::Genome genome, double neoplasm_k, bool verbose = false);

} // namespace domain::cell_factory
