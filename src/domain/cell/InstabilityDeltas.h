#pragma once

#include <string>
#include <utility>

namespace domain {

    /// InstabilityDeltas: Encapsulates the mutation rate deltas (delta_d1, delta_d2)
    /// based on the genetic status (BRCA1 and TP53).
    ///
    /// The matrix follows the model:
    /// - BRCA1 status: {+/+, +/-, -/-}
    /// - TP53 status: {+/+, +/-, -/-}
    ///
    /// Each combination yields different growth rates for D1 (DNA damage) and D2 (immunosuppression).
    class InstabilityDeltas {
    public:
        /// Delta values (configurable, with reasonable defaults)
        static constexpr double DELTA_LOW = 0.001;
        static constexpr double DELTA_MEDIUM = 0.002;
        static constexpr double DELTA_HIGH = 0.003;
        static constexpr double DELTA_VERY_HIGH = 0.004;

    public:
        /// Compute (delta_d1, delta_d2) based on TP53 and BRCA1 genetic status.
        ///
        /// Matrix:
        /// - BRCA1 +/- & TP53 +/+ → (LOW, LOW)
        /// - BRCA1 +/- & TP53 +/- → (MEDIUM, MEDIUM)
        /// - BRCA1 +/- & TP53 -/- → (HIGH, HIGH)
        /// - BRCA1 -/- & TP53 -/- → (VERY_HIGH, VERY_HIGH)
        ///
        /// @param tp53_status String representation ("+/+", "+/-", "-/-")
        /// @param brca1_status String representation ("+/+", "+/-", "-/-")
        /// @param low_delta Low delta value to use (default DELTA_LOW)
        /// @param high_delta High delta value to use (default DELTA_HIGH)
        /// @return pair<double, double> (delta_d1, delta_d2)
        static std::pair<double, double> getDeltas(const std::string& tp53_status,
                                                     const std::string& brca1_status,
                                                     double low_delta = DELTA_LOW,
                                                     double high_delta = DELTA_HIGH) {
            // BRCA1 +/- & TP53 +/+ → LOW
            if (brca1_status == "+/-" && tp53_status == "+/+") {
                return {low_delta, low_delta};
            }
            // BRCA1 +/- & TP53 +/- → MEDIUM
            if (brca1_status == "+/-" && tp53_status == "+/-") {
                return {low_delta * 2, low_delta * 2};
            }
            // BRCA1 +/- & TP53 -/- → HIGH
            if (brca1_status == "+/-" && tp53_status == "-/-") {
                return {high_delta, high_delta};
            }
            // BRCA1 -/- & TP53 -/- → VERY_HIGH
            if (brca1_status == "-/-" && tp53_status == "-/-") {
                return {high_delta * 1.33, high_delta * 1.33};
            }
            // Default: no instability (should not reach here in normal operation)
            return {0.0, 0.0};
        }
    };

} // namespace domain

