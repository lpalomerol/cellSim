//
// Created by luis on 31/10/25.
//

#pragma once
#include "../ports/ICell.h"
#include "../ports/INoiseSource.h"
#include "../gene/Genome.h"
#include <memory>
#include <string>


namespace domain {

    class AgenticCell final : public domain::ICell {
    public:
        // Toma el Genome por valor para clarificar ownership (se copia/mueve internamente)
        AgenticCell(std::unique_ptr<INoiseSource> noise, Genome genome, double neoplasm_k = 0.002, bool verbose = false);

        void live() override;
        bool alive() const override;
        bool isNeoplastic() const override;
        [[nodiscard]] std::string getTP53() const;
        [[nodiscard]] std::string getBRCA1() const;
        // Imprime detalles de la célula y su genoma (solo si está viva). No modifica estado.
        void details() const override;

        // Permite forzar la mutación de un gen en el genoma interno
        void mutateGene(const std::string& name) override;

        bool isNeoplasticProtected() const;

        // Exponer la semilla usada por la fuente de ruido
        std::uint64_t getSeed() const { return seed_; }

        // Nuevo: obtener la edad (necesario para tests y trazabilidad)
        std::uint64_t getAge() const { return age_; }

    private:
        std::unique_ptr<INoiseSource> noise_;
        Genome genome_;
        double neoplasm_k_;
        bool is_neoplastic_;
        bool verbose_ = false;
        std::uint64_t seed_ = 0; // guarda la semilla usada

        // Nueva: contador de edad de la célula (incrementa 1 por iteración)
        std::uint64_t age_ = 0;

        // Nueva: encapsula la lógica de desarrollar neoplasia (muestra ruido y aplica neoplasm_k_)
        void develop_neoplasm();

        // Nueva: incrementar edad (la fase que la llama decide cuando hacerlo)
        void increaseAge();

        // Excepciones están definidas en domain/exception y se usan como `domain::CellDeathException` y `domain::NeoplasticException`.

        // Nuevas: fases con nombres biológicamente más descriptivos
        void phase0_BaselineAssessment() const;
        void phase1_G1IntegrityCheckpoint() const;
        void phase2_Endocytosis();
        void phase3_NuclearDynamics();
        void phase4_CytoplasmicRemodeling();
        void phase5_Exocytosis();

        static void adjust_neoplasm_k();
    };
} // domain
