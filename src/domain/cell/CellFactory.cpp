//
// Created by luis on 31/10/25.
//

#include "CellFactory.h"

#include "AgenticCell.h"

namespace domain::cell_factory {

    std::unique_ptr<ICell> createSimpleCell(
        adapters::RandomNoise& noise,
        const domain::SimpleCellParams& params){
        return std::make_unique<domain::SimpleCell>(noise, params);

    }

    std::unique_ptr<ICell> createAgenticCell(
        unsigned seed,
        domain::Genome genome,
        double neoplasm_k){
        // Delegar en la nueva sobrecarga que acepta una fuente de ruido inyectada
        return createAgenticCell(std::make_unique<adapters::RandomNoise>(seed), std::move(genome), neoplasm_k);
    }

    // Sobrecarga: permite inyectar directamente la fuente de ruido
    std::unique_ptr<ICell> createAgenticCell(std::unique_ptr<INoiseSource> noise, domain::Genome genome, double neoplasm_k) {
        return std::make_unique<domain::AgenticCell>(std::move(noise), std::move(genome), neoplasm_k);
    }

} // namespace domain::cell_factory
