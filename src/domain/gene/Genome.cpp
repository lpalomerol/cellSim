#include "Genome.h"
#include "GenomeFactory.h"
#include "GeneConstants.h"
#include "../adapters/NullLogger.h"
#include <cassert>

namespace domain {

// Constructor: takes ownership of genes map via move semantics.
// Logger is optional; if nullptr, a NullLogger is used by default.
Genome::Genome(std::unordered_map<std::string, Gene> genes, ports::ILoggerPtr logger)
    : genes_(std::move(genes)), logger_(logger ? logger : std::make_shared<adapters::NullLogger>()) {}

bool Genome::hasGene(const std::string& name) const {
    return genes_.find(name) != genes_.end();
}

const Gene* Genome::getGene(const std::string& name) const {
    auto it = genes_.find(name);
    if (it == genes_.end()) return nullptr;
    return &it->second;
}

const std::unordered_map<std::string, Gene>& Genome::genes() const {
    return genes_;
}

// Factory: return the default genome
Genome Genome::makeDefaultGenome() {
    return genome_factory::makeDefaultGenome();
}

// Return a shallow copy of the genome.
// Copies the genes map, but both parent and child share the same logger reference.
Genome Genome::clone() const {
    return Genome(genes_, logger_);
}

// Inject a noise source into every gene in the genome
void Genome::setNoiseSourceForAll(INoiseSource* noise) {
    for (auto& kv : genes_) {
        kv.second.setNoiseSource(noise);
    }
}

// Advance (live) all genes in the genome.
// If TP53 indicates instability (+/- or -/-), adds instability penalty to each gene's mutation threshold.
// genomic_instability: multiplicative factor applied to gene mutation thresholds (>1 increases mutation probability).
void Genome::liveAllGenes(double genomic_instability) {
    assert(genomic_instability > 0.0);
    bool unstable = isUnstable();
    for (auto& kv : genes_) {
        // Propagate genomic_instability to each gene
        kv.second.live(unstable, genomic_instability);
    }
}

// Print details of all genes (one line per gene) with their mutation probabilities.
// If genome is unstable (TP53 mutation), probabilities include instability penalties.
void Genome::details() const {
    bool unstable = isUnstable();
    for (const auto& kv : genes_) {
        logger_->logGenome(kv.second.details(unstable));
    }
}


// Apply mutation to the gene identified by `name`.
void Genome::mutate(const std::string& name) {
    auto it = genes_.find(name);
    if (it != genes_.end()) {
        it->second.mutate();
    }
}

// Return true if TP53 indicates genomic instability
// Only TP53 +/+ (wildtype/enabled) is considered stable
bool Genome::isUnstable() const {
    return hasTP53Instability();
}

// ===== Genomic Queries (Information Provider) =====

bool Genome::hasBRCA1Mutation() const {
    const Gene* brca1 = getGene(GeneNames::BRCA1);
    if (!brca1) {
        return false;  // Gene missing ≠ mutation
    }
    return brca1->getStatus().isDisabled();  // BRCA1 -/-
}

bool Genome::hasTP53Function() const {
    const Gene* tp53 = getGene(GeneNames::TP53);
    if (!tp53) {
        return false;  // No gene = no function
    }
    // Functional = enabled or partially_enabled (NOT disabled)
    return !tp53->getStatus().isDisabled();
}

bool Genome::hasTP53Loss() const {
    const Gene* tp53 = getGene(GeneNames::TP53);
    if (!tp53) {
        return true;  // Missing = complete loss
    }
    return tp53->getStatus().isDisabled();  // TP53 -/-
}

bool Genome::hasTP53Instability() const {
    const Gene* tp53 = getGene(GeneNames::TP53);
    if (!tp53) {
        return true;  // Missing = instability
    }
    // Unstable if NOT wildtype (+/+)
    return !tp53->getStatus().isEnabled();
}

bool Genome::hasNeoplasticProtection() const {
    // Neoplastic protection = TP53 function
    return hasTP53Function();
}

} // namespace domain
