#include "GenomeFactory.h"
#include "Gene.h"
#include "GeneConstants.h"
#include <unordered_map>

namespace domain::genome_factory {

// Default values for gene parameters
constexpr double DEFAULT_THRESHOLD = 0.1;
constexpr double DEFAULT_INSTABILITY_K = 0.0;


// Helper: safely retrieve value from map with default fallback
static double getMapValue(const std::unordered_map<std::string, double>& map,
                         const std::string& key,
                         double default_value) {
    auto it = map.find(key);
    return (it != map.end()) ? it->second : default_value;
}

Genome makeDefaultGenome(const std::unordered_map<std::string, double>& gene_thresholds,
                          const std::unordered_map<std::string, double>& gene_instability_k,
                          ports::ILoggerPtr logger) {
    // Retrieve gene parameters with sensible defaults
    double tp53_threshold = getMapValue(gene_thresholds, GeneNames::TP53, DEFAULT_THRESHOLD);
    double tp53_instability = getMapValue(gene_instability_k, GeneNames::TP53, DEFAULT_INSTABILITY_K);

    double brca1_threshold = getMapValue(gene_thresholds, GeneNames::BRCA1, DEFAULT_THRESHOLD);
    double brca1_instability = getMapValue(gene_instability_k, GeneNames::BRCA1, DEFAULT_INSTABILITY_K);

    // Create genes with their initial states
    Gene tp53(GeneNames::TP53, Gene::State::PlusPlus, tp53_threshold, tp53_instability, logger);
    Gene brca1(GeneNames::BRCA1, Gene::State::PlusMinus, brca1_threshold, brca1_instability, logger);

    // Build genome map and return
    std::unordered_map<std::string, Gene> genes{
        {tp53.name(), tp53},
        {brca1.name(), brca1}
    };
    return Genome(std::move(genes), logger);
}

} // namespace domain::genome_factory
