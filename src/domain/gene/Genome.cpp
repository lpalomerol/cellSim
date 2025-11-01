#include "Genome.h"

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

} // namespace domain

