#pragma once
#include <memory>

#include "SimpleCell.h"
#include "../adapters/RandomNoise.h"

namespace domain {

    class CellFactory {
    public:
        virtual ~CellFactory() = default;
        virtual std::unique_ptr<ICell> createSimpleCell(adapters::RandomNoise& noise, const domain::SimpleCellParams& params);
    };
}
