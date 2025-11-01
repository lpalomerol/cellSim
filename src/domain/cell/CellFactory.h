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
        // Ahora la fábrica crea una AgenticCell recibiendo la seed para la fuente de ruido
        // y un genoma ya inicializado que se pasará al constructor de la célula.
        // Ahora la fábrica toma el `Genome` por valor (movable) y recibe `neoplasm_k` configurable
        virtual std::unique_ptr<ICell> createAgenticCell(unsigned seed, domain::Genome genome, double neoplasm_k);
    };
}
