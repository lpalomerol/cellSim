//
// Created by luis on 31/10/25.
//

#include "CellFactory.h"

#include "AgenticCell.h"

namespace domain {

    std::unique_ptr<ICell> CellFactory::createSimpleCell(
        adapters::RandomNoise& noise,
        const domain::SimpleCellParams& params){
        return std::make_unique<domain::SimpleCell>(noise, params);

    }

    std::unique_ptr<ICell> CellFactory::createAgenticCell(
        unsigned seed,
        domain::Genome genome,
        double neoplasm_k){
        return std::make_unique<domain::AgenticCell>(
            std::make_unique<adapters::RandomNoise>(seed),
            std::move(genome),
            neoplasm_k
        );
    }


}
