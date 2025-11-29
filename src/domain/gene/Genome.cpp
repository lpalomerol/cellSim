#include "Genome.h"
#include "GenomeFactory.h"
#include <iostream>
#include "../adapters/NullLogger.h"

namespace domain {

Genome::Genome(std::unordered_map<std::string, Gene> genes, bool verbose, ports::ILoggerPtr logger)
    : genes_(std::move(genes)), verbose_(verbose), logger_(logger ? logger : std::make_shared<adapters::NullLogger>()) {
    logger_->setVerbose(verbose_);
}

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

// Return a deep copy of the genome
Genome Genome::clone() const {
    return Genome(genes_, verbose_, logger_);
}

// Inject a noise source into every gene in the genome
void Genome::setNoiseSourceForAll(INoiseSource* noise) {
    for (auto& kv : genes_) {
        kv.second.setNoiseSource(noise);
    }
}

// Advance (live) all genes in the genome
void Genome::liveAllGenes(double genomic_instability) {
    // Apply instability component only if the genome is unstable (TP53)
    bool unstable = isUnstable();
    for (auto& kv : genes_) {
        // Propagate genomic_instability to each gene (default 1.0 = no effect)
        kv.second.live(unstable, genomic_instability);
    }
}

// Print details of all genes (one line per gene)
void Genome::details() const {
    bool unstable = isUnstable();
    for (const auto& kv : genes_) {
        // Use Gene::details() which returns "NAME[status]"
        logger_->logGenome(kv.second.details(unstable));
    }
}

void Genome::setVerbose(bool v) {
    verbose_ = v;
    // Propagate verbose setting to all genes
    for (auto& kv : genes_) {
        kv.second.setVerbose(v);
    }
}

// Apply mutation to the gene identified by `name`.
void Genome::mutate(const std::string& name) {
    auto it = genes_.find(name);
    if (it != genes_.end()) {
        it->second.mutate();
    }
}

// Return true if TP53 indicates instability (TP53 == +/- or -/-)
bool Genome::isUnstable() const {
    const Gene* tp53 = getGene("TP53");
    if (!tp53) return false;
    std::string s = tp53->status();
    return (s == "+/-" || s == "-/-");
}

} // namespace domain
