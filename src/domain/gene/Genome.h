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

    // Devuelve una copia profunda del genoma (clone explícito)
    Genome clone() const;

    // Inyecta una fuente de ruido a todos los genes del genoma
    void setNoiseSourceForAll(INoiseSource* noise);

    // Avanza (live) todos los genes del genoma
    void liveAllGenes();

    // Imprime los detalles de todos los genes (una línea por gen), usando Gene::details()
    void details() const;

    // Nueva: aplica la mutación al gen identificado por `name` llamando a Gene::mutate().
    // Si el gen no existe, no hace nada.
    void mutate(const std::string& name);

    // Nueva: devuelve true si el gen TP53 indica inestabilidad (TP53 == +/- o -/-)
    bool isUnstable() const;

private:
    std::unordered_map<std::string, Gene> genes_;
};

} // namespace domain
