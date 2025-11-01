#include "GenomeFactory.h"
#include "Gene.h"
#include <unordered_map>

namespace domain {
namespace genome_factory {

Genome makeDefaultGenome() {
    Gene tp53("TP53", Gene::State::PlusPlus);
    Gene brca1("BRCA1", Gene::State::PlusMinus);
    std::unordered_map<std::string, Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    return Genome(std::move(genes));
}

// New overload: create default genome but use provided per-gene mutation thresholds
Genome makeDefaultGenome(const std::unordered_map<std::string, double>& gene_thresholds) {
    double tp53_th = 0.1;
    double brca1_th = 0.1;
    auto it_tp = gene_thresholds.find("TP53");
    if (it_tp != gene_thresholds.end()) tp53_th = it_tp->second;
    auto it_br = gene_thresholds.find("BRCA1");
    if (it_br != gene_thresholds.end()) brca1_th = it_br->second;

    Gene tp53("TP53", Gene::State::PlusPlus, tp53_th);
    Gene brca1("BRCA1", Gene::State::PlusMinus, brca1_th);
    std::unordered_map<std::string, Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    return Genome(std::move(genes));
}

// New overload: create default genome using thresholds and instability_k per gene
Genome makeDefaultGenome(const std::unordered_map<std::string, double>& gene_thresholds,
                          const std::unordered_map<std::string, double>& gene_instability_k) {
    double tp53_th = 0.1;
    double brca1_th = 0.1;
    double tp53_k = 0.0;
    double brca1_k = 0.0;

    auto it_tp = gene_thresholds.find("TP53");
    if (it_tp != gene_thresholds.end()) tp53_th = it_tp->second;
    auto it_br = gene_thresholds.find("BRCA1");
    if (it_br != gene_thresholds.end()) brca1_th = it_br->second;

    auto itk_tp = gene_instability_k.find("TP53");
    if (itk_tp != gene_instability_k.end()) tp53_k = itk_tp->second;
    auto itk_br = gene_instability_k.find("BRCA1");
    if (itk_br != gene_instability_k.end()) brca1_k = itk_br->second;

    Gene tp53("TP53", Gene::State::PlusPlus, tp53_th, tp53_k);
    Gene brca1("BRCA1", Gene::State::PlusMinus, brca1_th, brca1_k);
    std::unordered_map<std::string, Gene> genes{{tp53.name(), tp53}, {brca1.name(), brca1}};
    return Genome(std::move(genes));
}

} // namespace genome_factory
} // namespace domain
