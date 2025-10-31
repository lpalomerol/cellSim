#pragma once
#include <memory>

#include "SimpleCell.h"
#include "AgenticCell.h"
#include "../adapters/RandomNoise.h"

namespace domain {

    class CellFactory {
    public:
        virtual ~CellFactory() = default;
        virtual std::unique_ptr<ICell> createSimpleCell(adapters::RandomNoise& noise, const domain::SimpleCellParams& params);
        virtual std::unique_ptr<ICell> createAgenticCell(adapters::RandomNoise& noise, const domain::AgenticCellParams& params);
    };
}
