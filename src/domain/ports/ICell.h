
#pragma once

#include "../../domain/cell/CellState.h"
#include "../cell/OncoState.h"

namespace domain {

    struct ICell {
        virtual ~ICell() = default;
        virtual void live() = 0;
        virtual bool alive() = 0;
        virtual CellState state() = 0;
        virtual OncoState getOncoState() = 0;
    };
}

