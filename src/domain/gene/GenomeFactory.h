#pragma once

#include "Genome.h"

namespace domain {
namespace genome_factory {

// Devuelve un genoma por defecto con los genes principales (TP53, BRCA1).
Genome makeDefaultGenome();

// Devuelve un genoma por defecto pero usando thresholds de mutación por gen
Genome makeDefaultGenome(const std::unordered_map<std::string, double>& gene_thresholds);

// Devuelve un genoma por defecto usando thresholds e instability_k por gen
Genome makeDefaultGenome(const std::unordered_map<std::string, double>& gene_thresholds,
                          const std::unordered_map<std::string, double>& gene_instability_k);

} // namespace genome_factory
} // namespace domain
