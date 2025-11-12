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
        if (verbose_) {
            // Mostrar la semilla para trazabilidad; el simulador también lo verá via details() o getSeed()
            std::cout << "[Trace] AgenticCell seed: " << seed_ << "\n";

        }

    }

    void AgenticCell::live() {
        // Ejecutar fases en secuencia; las fases lanzarán CellDeathException si la célula muere
        try {
            if (verbose_) {
                details();

            }
            phase1_initialChecks();
            phase2_updateGenotype();
            phase3_updateInternalStatus();
            phase4_updatePhenotype();
        } catch (const CellDeathException& e) {
            if (verbose_) {
                std::cout << "[Trace] AgenticCell::live() - fase abortada por muerte de la célula: " << e.what() << "\n";
            }
            // Terminar el ciclo live() silenciosamente
            return;
        }
    }

    // Fase 1: comprobaciones iniciales (viva y no ya neoplásica)
    void AgenticCell::phase1_initialChecks() const {
        // Si la célula está muerta -> lanzar excepción para indicar terminación
        if (!alive()) {
            throw CellDeathException("dead@phase1");
        }
        // Si ya es neoplásica -> lanzar excepción para indicar terminación (comportamiento previo)
        if (is_neoplastic_) {
            throw CellDeathException("neoplastic@phase1");
        }
    }

    // Fase 2: actualizar genotipo (por ahora, avanzar genes)
    void AgenticCell::phase2_updateGenotype() {
        genome_.liveAllGenes();
        if (!alive()) {
            throw CellDeathException("dead@phase2");
        }
    }

    // Fase 3: actualizar estado interno (ajustar parámetros internos y aumentar edad)
    void AgenticCell::phase3_updateInternalStatus() {
        adjust_neoplasm_k();
        // Comprobar si la celula ha muerto tras actualizar estado interno
        if (!alive()) {
            throw CellDeathException("dead@phase3");
        }
        // Incrementar edad aquí como parte del estado interno
        increaseAge();
    }

    // Fase 4: actualizar fenotipo (evaluar si desarrolla tumor)
    void AgenticCell::phase4_updatePhenotype() {
        if (!isNeoplasticProtected()) {
            develop_neoplasm();
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
                "Seed: [" << seed_ << "]" << " | Age: [" << age_ << "]\n";
            // Mostrar el estado del genoma siempre (útil para depuración aunque la célula esté muerta)
            std::cout << "Genome details:\n";
            genome_.details();
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

    // Nueva: incrementar edad solo si la célula está viva
    void AgenticCell::increaseAge() {
        ++age_;
    }

} // domain
