//
// Created by luis on 31/10/25.
//

#pragma once
#include "../ports/ICell.h"
#include "../ports/INoiseSource.h"
#include "../gene/Genome.h"
#include <memory>
#include <unordered_map>

namespace domain {

    class AgenticCell final : public domain::ICell {
    public:
        // Toma el Genome por valor para clarificar ownership (se copia/mueve internamente)
        AgenticCell(std::unique_ptr<INoiseSource> noise, Genome genome, double neoplasm_k = 0.002);

        void live() override;
        bool alive() override;
        bool isNeoplastic() override;
        [[nodiscard]] std::string getTP53() const;
        [[nodiscard]] std::string getBRCA1() const;
        // Estructura de trazado por tick
        struct TickTrace {
            int tick = 0;
            bool alive_before = true;
            bool alive_after = true;
            bool is_neoplastic_before = false;
            bool is_neoplastic_after = false;
            double neoplasm_sample = -1.0;
            double neoplasm_threshold = 0.0;
            bool neoplasm_happened = false;
            std::unordered_map<std::string, domain::Gene::LiveTrace> gene_traces;
        };

        // Ejecuta un tick y devuelve la traza detallada para auditoría
        TickTrace liveWithTrace(int tickIndex = 0);

    private:
        std::unique_ptr<INoiseSource> noise_;
        Genome genome_;
        double neoplasm_k_;
        bool is_neoplastic_;
    };
} // domain
