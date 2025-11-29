#pragma once

#include "Genome.h"
#include "../ports/ILogger.h"
#include <unordered_map>
#include <memory>

namespace domain::genome_factory {

// Devuelve un genoma por defecto con los genes principales (TP53, BRCA1).
// Se unifican las sobrecargas: ambos parámetros son opcionales (mapas vacíos por defecto).
Genome makeDefaultGenome(const std::unordered_map<std::string, double>& gene_thresholds = {},
                          const std::unordered_map<std::string, double>& gene_instability_k = {},
                          ports::ILoggerPtr logger = nullptr);

} // namespace domain::genome_factory
