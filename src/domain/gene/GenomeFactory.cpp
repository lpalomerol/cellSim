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

} // namespace genome_factory
} // namespace domain

