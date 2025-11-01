#pragma once

#include <string>
#include <unordered_map>
#include "Gene.h"

namespace domain {

class Genome {
public:
    // Constructor recibe un mapa (clave -> Gene) y lo almacena internamente.
    explicit Genome(std::unordered_map<std::string, Gene> genes = {});

    // Comprueba si existe un gen con la clave dada
    bool hasGene(const std::string& name) const;

    // Devuelve puntero const al gen; nullptr si no existe
    const Gene* getGene(const std::string& name) const;

    // Acceso al mapa completo
    const std::unordered_map<std::string, Gene>& genes() const;

    // Fábrica: devuelve un genoma por defecto (TP53 y BRCA1 con estados por defecto)
    static Genome makeDefaultGenome();

private:
    std::unordered_map<std::string, Gene> genes_;
};

} // namespace domain
