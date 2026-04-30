#include "GenomicInstabilityDeltaStrategy.h"

namespace domain {

    InstabilityDeltas GenomicInstabilityDeltaStrategy::calculateDeltas(
        const ICell& cell) const {

        const std::string tp53  = cell.getTP53Status();
        const std::string brca1 = cell.getBRCA1Status();
        const std::uint64_t age = cell.getAge();

        double delta_tp53  = deltaForStatus(tp53);

        double delta_brca1 = 0.0;
        if (brca1 == "+/-") {
            delta_brca1 = low_delta_;
        } else if (brca1 == "-/-") {
            delta_brca1 = 2 * high_delta_;
        }

        // D1 (DNA damage)      = Δ(TP53) + small age factor
        // D2 (Immunosuppression) = Δ(TP53) + Δ(BRCA1) + small age factor
        double age_factor = age * 0.00001;
        double delta_d1 = delta_tp53  + age_factor;
        double delta_d2 = delta_tp53 + delta_brca1 + age_factor;

        return InstabilityDeltas::create(delta_d1, delta_d2);
    }

    double GenomicInstabilityDeltaStrategy::deltaForStatus(const std::string& status) const {
        if (status == "+/-") return low_delta_;
        if (status == "-/-") return high_delta_;
        return 0.0;  // "+/+" or unknown
    }

} // namespace domain

