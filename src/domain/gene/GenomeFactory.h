#pragma once

#include "Genome.h"

namespace domain {
namespace genome_factory {

// Devuelve un genoma por defecto con los genes principales (TP53, BRCA1).
Genome makeDefaultGenome();

} // namespace genome_factory
} // namespace domain

