//
// Created by luis on 31/10/25.
//

#include "CellFactory.h"

namespace domain {

    std::unique_ptr<ICell> CellFactory::createSimpleCell(
        adapters::RandomNoise& noise,
        const domain::SimpleCellParams& params) {
        return std::make_unique<domain::SimpleCell>(noise, params);
    }


    std::unique_ptr<ICell> CellFactory::createGeneCell(
        adapters::RandomNoise& noise,
        const domain::GeneCellParams& params){
        return std::make_unique<domain::GeneCell>(noise, params);

    }




}