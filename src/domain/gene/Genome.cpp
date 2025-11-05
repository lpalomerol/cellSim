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
    // Aplicar el componente de inestabilidad solo si el genoma está inestable (según TP53)
    bool unstable = isUnstable();
    for (auto& kv : genes_) {
        kv.second.live(unstable);
    }
}

// Imprime los detalles de todos los genes (una línea por gen)
void Genome::details() const {
    bool unstable = isUnstable();
    for (const auto& kv : genes_) {
        // Usar Gene::details() que devuelve "NAME[status]"
        std::cout << kv.second.details(unstable) << std::endl;
    }
}

// Nueva implementación: aplica mutación al gen identificado por `name`.
void Genome::mutate(const std::string& name) {
    auto it = genes_.find(name);
    if (it != genes_.end()) {
        it->second.mutate();
    }
}

// Nueva: devuelve true si TP53 indica inestabilidad ("+/-" o "-/-").
bool Genome::isUnstable() const {
    const Gene* tp53 = getGene("TP53");
    if (!tp53) return false;
    std::string s = tp53->status();
    return (s == "+/-" || s == "-/-");
}

} // namespace domain
