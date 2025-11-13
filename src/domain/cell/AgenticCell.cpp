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
        // Ejecutar fases en secuencia; las fases lanzarán excepciones si la célula muere o es neoplásica
        try {
            phase0_BaselineAssessment();

            phase1_G1IntegrityCheckpoint();

            phase2_Endocytosis();

            phase3_NuclearDynamics();

            phase4_CytoplasmicRemodeling();

            phase5_Exocytosis();

        } catch (const NeoplasticException& e) {
            if (verbose_) {
                std::cout << "[Trace] neoplastic@live: " << e.what() << "\n";
            }
            return;
        } catch (const CellDeathException& e) {
            if (verbose_) {
                std::cout << "[Trace] dead@live: " << e.what() << "\n";
            }
            // Terminar el ciclo live() silenciosamente
            return;
        }
    }

    // phase0: mostrar detalles si verbose
    void AgenticCell::phase0_BaselineAssessment() const {
        if (verbose_) {
            details();
        }
    }

    // phase1: comprobaciones de integridad
    void AgenticCell::phase1_G1IntegrityCheckpoint() const {
        if (!alive()) {
            throw CellDeathException("dead@phase1");
        }
        if (is_neoplastic_) {
            throw NeoplasticException("neoplastic@phase1");
        }
    }

    // phase2: endocytosis (por ahora vacío)
    void AgenticCell::phase2_Endocytosis() {

    }

    // phase3: procesos nucleares (genes + ajustes internos + aumentar edad)
    void AgenticCell::phase3_NuclearDynamics() {
        // por ahora usamos esto para avanzar todos los genes
        genome_.liveAllGenes();
        adjust_neoplasm_k();
        if (!alive()) {
            throw CellDeathException("dead@phase3");
        }
        increaseAge();
    }

    // phase4: procesos citoplasmáticos / evaluación fenotípica
    void AgenticCell::phase4_CytoplasmicRemodeling() {
        if (!isNeoplasticProtected()) {
            develop_neoplasm();
        }
    }

    // phase5: exocitosis (stub por ahora)
    void AgenticCell::phase5_Exocytosis() {
        // Intencionalmente vacío por ahora
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
