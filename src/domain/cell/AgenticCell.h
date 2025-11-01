//
// Created by luis on 31/10/25.
//

#pragma once
#include "../ports/ICell.h"
#include "../ports/INoiseSource.h"
#include "../gene/Genome.h"
#include <memory>

namespace domain {

    class AgenticCell final : public domain::ICell {
    public:
        AgenticCell(std::unique_ptr<INoiseSource> noise, const Genome& genome, double tumor_k = 0.002);

        void live() override;
        bool alive() override;
        bool tumoral() override;
        [[nodiscard]] std::string getTP53() const;
        [[nodiscard]] std::string getBRCA1() const;

    private:
        std::unique_ptr<INoiseSource> noise_;
        Genome genome_;
        double tumor_k_;
        bool is_tumoral_;
    };
} // domain
