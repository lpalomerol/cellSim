//
// Created by luis on 31/10/25.
//

#include "AgenticCell.h"
#include <iostream>

namespace domain {
    AgenticCell::AgenticCell(std::unique_ptr<INoiseSource> noise, Genome genome, double neoplasm_k, bool verbose)
        : noise_(std::move(noise)), genome_(std::move(genome)), neoplasm_k_(neoplasm_k), is_neoplastic_(false), verbose_(verbose) {
        // Inyectar la fuente de ruido en todos los genes del genoma usando la API de Genome
        genome_.setNoiseSourceForAll(noise_.get());
        // Asegurar que el genoma y sus genes respeten el flag verbose
        genome_.setVerbose(verbose_);

        // Guardar y mostrar la semilla usada (requiere INoiseSource::getSeed())
        if (noise_) {
            seed_ = noise_->getSeed();
        } else {
            seed_ = 0;
        }
        if (verbose) {
            // Mostrar la semilla para trazabilidad; el simulador también lo verá via details() o getSeed()
            std::cout << "[Trace] AgenticCell seed: " << seed_ << "\n";

        }

    }

    void AgenticCell::live() {
        // FASE 1: Revisar estado inicial (muerta o ya neoplásica)
        if (verbose_) {
            std::cout << "[Trace] AgenticCell::live() - fase 1: estado inicial\n";
            details();
            std::cout << "               \n";
        }

        // Si la célula está muerta, no hacer nada
        if (!alive()) {
            if (verbose_) std::cout << "[Trace] AgenticCell::live() - célula muerta, terminando fase.\n";
            return;
        }

        // Si ya es neoplásica, no hacer nada
        if (is_neoplastic_) {
            if (verbose_) std::cout << "[Trace] AgenticCell::live() - ya neoplásica, terminando fase.\n";
            return;
        }

        // FASE 2: Actualizar genes
        if (verbose_) std::cout << "[Trace] AgenticCell::live() - fase 2: actualizar genes\n";
        genome_.liveAllGenes();

        // Si la célula queda muerta después de avanzar genes, no hacer nada más
        if (!alive()) {
            if (verbose_) std::cout << "[Trace] AgenticCell::live() - tras genes, la célula ha muerto.\n";
            return;
        }

        // FASE 3: Posible ajuste de k (neoplasm_k_)
        if (verbose_) std::cout << "[Trace] AgenticCell::live() - fase 3: posible ajuste de k\n";
        adjust_neoplasm_k();

        // FASE 4: Desarrollar neoplasia si corresponde
        if (verbose_) std::cout << "[Trace] AgenticCell::live() - fase 4: evaluar desarrollo de neoplasia\n";

        // Sólo desarrollar neoplasia si no está protegida por TP53
        if (!isNeoplasticProtected()) {
            develop_neoplasm();
        } else {
            // Trazabilidad: célula protegida por TP53, no puede volverse neoplásica
            if (verbose_) std::cout << "[Trace] Célula protegida " << std::endl;
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
        if (verbose_) {
            std::cout << "[Cell details] "<<
                "Alive: [" <<cell_is_alive << "] |  "<<
                "Neoplastic protected ["<< cell_is_neoplastic_protected<< "] | "<<
                "Neoplastic: [" << cell_is_neoplastic << "] | " <<
                "Seed: [" << seed_ << "]\n";
            if (alive()) {
                std::cout << "Genome details:\n";
                genome_.details();
            }
        }

    }

    // Implementación de mutateGene: delega en Genome::mutate
    void AgenticCell::mutateGene(const std::string& name) {
        genome_.mutate(name);
    }

    // Nueva función: encapsula el muestreo de ruido y la decisión de neoplasia
    void AgenticCell::develop_neoplasm() {
        if (!noise_) return; // seguridad
        double sample = noise_->next().u01;
        // Trazabilidad: mostrar sample y umbral
        if (verbose_) std::cout << "[Trace] muestreo para neoplasia: sample=" << sample << " threshold=" << neoplasm_k_ << "\n";
        if (sample < neoplasm_k_) {
            is_neoplastic_ = true;
            if (verbose_) std::cout << "[Trace] Resultado: la célula se vuelve NEOPLÁSICA\n";
        } else {
            if (verbose_) std::cout << "[Trace] Resultado: no se desarrolla neoplasia (sample >= threshold)\n";
        }
    }

    bool AgenticCell::isNeoplasticProtected() const {
        const Gene *tp53 = genome_.getGene("TP53");
        return (tp53 && tp53->enabled());
    }

    void AgenticCell::adjust_neoplasm_k() {
    }

} // domain
