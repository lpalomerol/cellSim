//
// Created by luis on 31/10/25.
//

#pragma once
#include "../ports/ICell.h"
#include "../ports/INoiseSource.h"
#include "../gene/Genome.h"
#include <memory>
#include <cstdint>
#include <exception>
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

        // Nueva: contador de edad de la célula (incrementa 1 por iteración si está viva)
        std::uint64_t age_ = 0;

        // Nueva: encapsula la lógica de desarrollar neoplasia (muestra ruido y aplica neoplasm_k_)
        void develop_neoplasm();

        // Nueva: incrementa age (la fase que la llama debe decidir si la célula sigue viva)
        void increaseAge();

        // Excepción usada para señalizar que la célula ha muerto durante una fase
        struct CellDeathException : public std::exception {
            explicit CellDeathException(std::string m) : msg_(std::move(m)) {}
            const char* what() const noexcept override { return msg_.c_str(); }
        private:
            std::string msg_;
        };

        // Nuevas: cuatro fases de la evolución de la célula, encapsuladas
        // phase1: initial checks
        // phase2: update genotype (actualiza genes)
        // phase3: update internal status (ajusta neoplasm_k_ y sube edad)
        // phase4: update phenotype (evalúa si desarrolla tumor)
        // Las fases lanzan `CellDeathException` si detectan que la célula ha muerto.
        void phase1_initialChecks() const;
        void phase2_updateGenotype();
        void phase3_updateInternalStatus();
        void phase4_updatePhenotype();

        static void adjust_neoplasm_k();
    };
} // domain
