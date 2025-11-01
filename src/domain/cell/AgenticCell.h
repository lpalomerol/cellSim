//
// Created by luis on 31/10/25.
//

#pragma once
#include "../ports/ICell.h"
#include "../ports/INoiseSource.h"
#include "../gene/Genome.h"
#include <memory>
#include "AgenticTickTrace.h"

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

        // Usamos la clase ligera definida en AgenticTickTrace.h para evitar un struct largo aquí
        using TickTrace = AgenticTickTrace;

        // Ejecuta un tick y devuelve la traza detallada para auditoría
        TickTrace liveWithTrace(int tickIndex = 0);

    private:
        std::unique_ptr<INoiseSource> noise_;
        Genome genome_;
        double neoplasm_k_;
        bool is_neoplastic_;
    };
} // domain
