#pragma once

#include "../../domain/cell/CellState.h"
#include "../cell/OncoState.h"

namespace domain {

    struct ICell {
        virtual ~ICell() = default;
        virtual void live() = 0;
        virtual bool alive() = 0;
        // Indica si la célula presenta neoplasia (estado neoplásico)
        virtual bool isNeoplastic() = 0;

    };
}
