//
// Created by luis on 31/10/25.
//

#include "CellFactory.h"

namespace domain {

    std::unique_ptr<ICell> CellFactory::createGeneCell(
        adapters::RandomNoise& noise,
        const domain::GeneCellParams& params){
        return std::make_unique<domain::GeneCell>(noise, params);

    }




}