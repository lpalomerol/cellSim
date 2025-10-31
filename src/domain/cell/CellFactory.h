#pragma once
#include <memory>

#include "GeneCell.h"
#include "../adapters/RandomNoise.h"

namespace domain {

    class CellFactory {
    public:
        virtual ~CellFactory() = default;
        virtual std::unique_ptr<ICell> createGeneCell(adapters::RandomNoise& noise, const domain::GeneCellParams& params);
    };
}
