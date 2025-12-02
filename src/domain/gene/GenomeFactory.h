#pragma once

#include "Genome.h"
#include "../ports/ILogger.h"
#include <unordered_map>
#include <memory>

namespace domain::genome_factory {

// Factory function: creates a default genome with TP53 and BRCA1 genes.
//
// Parameters:
//   gene_thresholds: optional map of gene name -> mutation threshold probability
//                    (default: 0.1 for all genes if not provided)
//   gene_instability_k: optional map of gene name -> instability penalty
//                       (default: 0.0 for all genes if not provided)
//   logger: optional logger for debug output (uses NullLogger if nullptr)
//
// Gene states:
//   TP53: initialized as +/+ (wild-type, protected)
//   BRCA1: initialized as +/- (heterozygous)
//
// Returns: Genome object containing both genes with specified parameters
Genome makeDefaultGenome(const std::unordered_map<std::string, double>& gene_thresholds = {},
                          const std::unordered_map<std::string, double>& gene_instability_k = {},
                          ports::ILoggerPtr logger = nullptr);

} // namespace domain::genome_factory
