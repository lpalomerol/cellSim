#pragma once

#include <utility>

namespace domain {

    class AgenticCell;

    /// Interface for instability delta calculation strategies
    /// Allows different mathematical models for D1 (DNA damage) and D2 (immunosuppression) progression
    /// This enables testing with different biological assumptions and parameter calibration
    class IInstabilityDeltaStrategy {
    public:
        virtual ~IInstabilityDeltaStrategy() = default;

        /// Calculate the increments for D1 and D2 based on cell state
        /// @param cell The cell to evaluate
        /// @return pair<delta_d1, delta_d2> where first is DNA damage increment, second is immunosuppression increment
        virtual std::pair<double, double> calculateDeltas(const AgenticCell& cell) const = 0;
    };

} // namespace domain

