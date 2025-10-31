//
// Created by luis on 31/10/25.
//

#pragma once
#include "../ports/ICell.h"
#include "../ports/INoiseSource.h"

namespace domain {

    struct AgenticCellParams {
        // Parámetros específicos para AgenticCell (si los hay)
    };

    class AgenticCell final : public ICell {
    public:
        AgenticCell(INoiseSource& noise, const AgenticCellParams& params = AgenticCellParams())
            : params_(params), noise_(noise) {}

        void live() override;
        bool alive() override;
        bool tumoral() override;

    private:
        INoiseSource& noise_;
        AgenticCellParams params_;
    };
} // domain

