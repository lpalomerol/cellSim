#pragma once

#include "IInstabilityDeltaStrategy.h"
#include <memory>

namespace domain {

    /// Decorator: wraps any IInstabilityDeltaStrategy and applies lognormal noise to its output.
    ///
    /// Each delta is perturbed independently using Box-Muller transform:
    ///   noisy_delta = det_delta * exp(sigma_log * z - 0.5 * sigma_log^2)
    ///   where sigma_log = sqrt(log(1 + cv^2))
    ///
    /// If noise is nullptr or cv == 0, returns deterministic deltas unchanged.
    class LognormalNoiseInstabilityStrategy : public IInstabilityDeltaStrategy {
    public:
        LognormalNoiseInstabilityStrategy(std::unique_ptr<IInstabilityDeltaStrategy> inner, double cv)
            : inner_(std::move(inner)), cv_(cv) {}

        [[nodiscard]] InstabilityDeltas calculateDeltas(const ICell& cell, INoiseSource* noise) const override;

    private:
        std::unique_ptr<IInstabilityDeltaStrategy> inner_;
        double cv_;
    };

} // namespace domain
