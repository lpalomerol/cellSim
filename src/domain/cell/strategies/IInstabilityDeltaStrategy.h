#pragma once

#include "../model/InstabilityDeltas.h"
#include "../../ports/ICell.h"
#include "../../ports/INoiseSource.h"

namespace domain {

    /// Interface for instability delta calculation strategies
    /// Allows different mathematical models for D1 (DNA damage) and D2 (immunosuppression) progression
    /// This enables testing with different biological assumptions and parameter calibration
    class IInstabilityDeltaStrategy {
    public:
        virtual ~IInstabilityDeltaStrategy() = default;

        /// Calculate the increments for D1 and D2 based on cell state
        /// @param cell The cell to evaluate
        /// @param noise Optional noise source for stochastic strategies (may be nullptr)
        /// @return InstabilityDeltas value object with DNA damage (d1) and immunosuppression (d2) increments
        [[nodiscard]] virtual InstabilityDeltas calculateDeltas(const ICell& cell, INoiseSource* noise) const = 0;
    };

} // namespace domain

