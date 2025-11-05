#include "Genome.h"
#include "GenomeFactory.h"
#include <iostream>

namespace domain {

Genome::Genome(std::unordered_map<std::string, Gene> genes)
    : genes_(std::move(genes)) {}

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

// Fábrica para genoma por defecto
Genome Genome::makeDefaultGenome() {
    return genome_factory::makeDefaultGenome();
}

// Devuelve una copia profunda del genoma
Genome Genome::clone() const {
    return Genome(genes_);
}

// Inyecta una fuente de ruido en todos los genes del genoma
void Genome::setNoiseSourceForAll(INoiseSource* noise) {
    for (auto& kv : genes_) {
        kv.second.setNoiseSource(noise);
    }
}

// Avanza (live) todos los genes del genoma
void Genome::liveAllGenes() {
    for (auto& kv : genes_) {
        kv.second.live();
    }
}

// Imprime los detalles de todos los genes (una línea por gen)
void Genome::details() const {
    for (const auto& kv : genes_) {
        // Usar Gene::details() que devuelve "NAME[status]"
        std::cout << kv.second.details() << std::endl;
    }
}

} // namespace domain
