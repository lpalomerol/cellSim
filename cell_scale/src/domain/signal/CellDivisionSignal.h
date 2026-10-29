#pragma once

#include "BaseSignal.h"
#include "../cell/AgenticCell.h"
#include <memory>
#include <string>
#include <vector>

namespace domain {

    class CellDivisionSignal : public BaseSignal {
    public:
        /**
         * Construct a CellDivisionSignal.
         * @param source_id ID de la célula madre
         * @param daughter_cell Célula hija (almacenada como miembro privado)
         * @param message Mensaje (default "cell_division")
         * @param targets IDs de destino (vacío = broadcast)
         */
        explicit CellDivisionSignal(std::uint64_t source_id,
                                   std::unique_ptr<AgenticCell> daughter_cell,
                                   std::string message = "cell_division",
                                   std::vector<std::uint64_t> targets = {})
            : BaseSignal(source_id, std::move(message), std::move(targets)),
              daughter_cell_(std::move(daughter_cell)) {}

        Type type() const noexcept override { return Type::CellDivision; }

        // Take ownership of the daughter cell
        // Note: After calling this, the signal no longer holds the cell
        std::unique_ptr<AgenticCell> takeDaughterCell() noexcept {
            return std::move(daughter_cell_);
        }

    private:
        std::unique_ptr<AgenticCell> daughter_cell_;
    };

} // namespace domain


