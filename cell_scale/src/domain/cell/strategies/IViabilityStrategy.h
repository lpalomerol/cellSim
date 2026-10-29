#pragma once

#include "../../ports/ICell.h"

namespace domain {

    /// Interface for cell viability strategies
    /// Allows injecting different survival criteria and testing various viability models
    ///
    /// This enables:
    /// - Testing with mock viability strategies (e.g., always alive, always dead)
    /// - Experimenting with different biological models (senescence, resource-based death)
    /// - Separating viability logic from AgenticCell (Single Responsibility)
    class IViabilityStrategy {
    public:
        virtual ~IViabilityStrategy() = default;

        /// Determine if the cell is alive based on specific criteria
        /// @param cell The cell to evaluate
        /// @return true if the cell is viable, false otherwise
        [[nodiscard]] virtual bool isAlive(const ICell& cell) const = 0;
    };

} // namespace domain

