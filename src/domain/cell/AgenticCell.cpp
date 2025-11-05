//
// Created by luis on 31/10/25.
//

#include "AgenticCell.h"
#include <iostream>

namespace domain {
    AgenticCell::AgenticCell(std::unique_ptr<INoiseSource> noise, Genome genome, double neoplasm_k)
        : noise_(std::move(noise)), genome_(std::move(genome)), neoplasm_k_(neoplasm_k), is_neoplastic_(false) {
        // Inyectar la fuente de ruido en todos los genes del genoma usando la API de Genome
        genome_.setNoiseSourceForAll(noise_.get());
    }

    void AgenticCell::live() {
        // Avanzar todos los genes (mutaciones/efectos) sin traza
        genome_.liveAllGenes();

        // Si la célula queda muerta después de avanzar genes, no hacer nada más
        if (!alive()) {
            return;
        }

        // Si ya es neoplásica, nada que muestrear
        if (is_neoplastic_) {
            return;
        }

        // Determinar probabilidad p para neoplasia: protegida si TP53 está activado
        const Gene *tp53 = genome_.getGene("TP53");
        double p = (tp53 && tp53->enabled()) ? 0.0 : neoplasm_k_;

        // Muestrear ruido y decidir neoplasia
        double sample = noise_->next().u01;
        if (sample < p) {
            is_neoplastic_ = true;
        }
    }

    bool AgenticCell::alive() const {
        const Gene *brca1 = genome_.getGene("BRCA1");
        return brca1 && brca1->enabled();
    }

    bool AgenticCell::isNeoplastic() const {
        return is_neoplastic_;
    }

    std::string AgenticCell::getTP53() const {
        const Gene *tp53 = genome_.getGene("TP53");
        return tp53 ? tp53->status() : "?";
    }

    std::string AgenticCell::getBRCA1() const {
        const Gene *brca1 = genome_.getGene("BRCA1");
        return brca1 ? brca1->status() : "?";
    }

    void AgenticCell::details() const {
        std::string cell_is_alive = (alive() ? "yes" : "no");
        std::string cell_is_neoplastic = (isNeoplastic() ? "yes" : "no");
        std::string cell_is_neoplastic_protected = (isNeoplasticProtected() ? "yes" : "no");
        std::cout << "[Cell details] "<<
            "Alive: [" <<cell_is_alive << "] |  "<<
            "Neoplastic protected ["<< cell_is_neoplastic_protected<< "] | "<<
            "Neoplastic: [" << cell_is_neoplastic << "]\n";
        if (alive()) {
            std::cout << "Genome details:\n";
            genome_.details();
        }

    }

    // Implementación de mutateGene: delega en Genome::mutate
    void AgenticCell::mutateGene(const std::string& name) {
        genome_.mutate(name);
    }

    bool AgenticCell::isNeoplasticProtected() const {
        const Gene *tp53 = genome_.getGene("TP53");
        return (tp53 && tp53->enabled());
    }

} // domain
