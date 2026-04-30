#include "LognormalNoiseInstabilityStrategy.h"
#include <cmath>
#include <numbers>
#include <limits>

namespace domain {

    InstabilityDeltas LognormalNoiseInstabilityStrategy::calculateDeltas(
        const ICell& cell, INoiseSource* noise) const {

        InstabilityDeltas det = inner_->calculateDeltas(cell, noise);

        if (cv_ <= 0.0 || noise == nullptr) {
            return det;
        }

        const double sigma_log = std::sqrt(std::log(1.0 + cv_ * cv_));

        auto lognormal_sample = [&](double mean) -> double {
            if (mean <= 0.0) return mean;
            double u1 = std::max(noise->next().u01, std::numeric_limits<double>::min());
            double u2 = noise->next().u01;
            double z = std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * std::numbers::pi_v<double> * u2);
            return mean * std::exp(sigma_log * z - 0.5 * sigma_log * sigma_log);
        };

        return InstabilityDeltas::create(lognormal_sample(det.d1()), lognormal_sample(det.d2()));
    }

} // namespace domain
