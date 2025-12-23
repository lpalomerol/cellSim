#pragma once

#include <string>
#include <utility>
#include <memory>

namespace domain {

    /// Interface for custom instability delta calculation strategies
    /// Allows injecting different calculation models without modifying AgenticCell
    class IInstabilityDeltaCalculator {
    public:
        virtual ~IInstabilityDeltaCalculator() = default;

        /// Calculate (delta_d1, delta_d2) based on gene statuses
        /// @param tp53_status String representation ("+/+", "+/-", "-/-")
        /// @param brca1_status String representation ("+/+", "+/-", "-/-")
        /// @return pair<double, double> (delta_d1, delta_d2)
        virtual std::pair<double, double> calculate(
            const std::string& tp53_status,
            const std::string& brca1_status) const = 0;
    };

    /// Standard coefficient-based calculator
    /// Uses per-gene, per-state coefficients for maximum flexibility
    class CoefficientBasedCalculator : public IInstabilityDeltaCalculator {
    public:
        // Coefficients: for each gene state (+/+, +/-, -/-), store the contribution
        struct GeneCoefficients {
            double plus_plus = 0.0;
            double plus_minus = 0.001;
            double minus_minus = 0.003;
        };

    private:
        GeneCoefficients tp53_for_d1_;   // TP53 contribution to D1
        GeneCoefficients tp53_for_d2_;   // TP53 contribution to D2
        GeneCoefficients brca1_for_d2_;  // BRCA1 contribution to D2

        double getCoeff(const std::string& status, const GeneCoefficients& coeffs) const {
            if (status == "+/+") return coeffs.plus_plus;
            if (status == "+/-") return coeffs.plus_minus;
            if (status == "-/-") return coeffs.minus_minus;
            return 0.0;
        }

    public:
        CoefficientBasedCalculator(
            GeneCoefficients tp53_for_d1,
            GeneCoefficients tp53_for_d2,
            GeneCoefficients brca1_for_d2)
            : tp53_for_d1_(tp53_for_d1),
              tp53_for_d2_(tp53_for_d2),
              brca1_for_d2_(brca1_for_d2) {}

        std::pair<double, double> calculate(
            const std::string& tp53_status,
            const std::string& brca1_status) const override {

            // D1 depends only on TP53
            double delta_d1 = getCoeff(tp53_status, tp53_for_d1_);

            // D2 depends on both TP53 and BRCA1 (additive model)
            double delta_d2 = getCoeff(tp53_status, tp53_for_d2_)
                            + getCoeff(brca1_status, brca1_for_d2_);

            return {delta_d1, delta_d2};
        }
    };

    /// Simple calculator using just low/high deltas (backward compatible with 2 params)
    class SimpleDeltaCalculator : public IInstabilityDeltaCalculator {
    private:
        double low_delta_;
        double high_delta_;

    public:
        SimpleDeltaCalculator(double low_delta, double high_delta)
            : low_delta_(low_delta), high_delta_(high_delta) {}

        std::pair<double, double> calculate(
            const std::string& tp53_status,
            const std::string& brca1_status) const override {

            // Calculate delta per gene individually
            double delta_tp53 = 0.0;
            double delta_brca1 = 0.0;

            // Delta TP53
            if (tp53_status == "+/+") {
                delta_tp53 = 0.0;
            } else if (tp53_status == "+/-") {
                delta_tp53 = low_delta_;
            } else if (tp53_status == "-/-") {
                delta_tp53 = high_delta_;
            }

            // Delta BRCA1
            if (brca1_status == "+/+") {
                delta_brca1 = 0.0;
            } else if (brca1_status == "+/-") {
                delta_brca1 = low_delta_;
            } else if (brca1_status == "-/-") {
                delta_brca1 = 2 * high_delta_;
            }

            // D1 = Δ(TP53)
            double delta_d1 = delta_tp53;

            // D2 = Δ(TP53) + Δ(BRCA1)
            double delta_d2 = delta_tp53 + delta_brca1;

            return {delta_d1, delta_d2};
        }
    };

    /// InstabilityDeltas: Static utility class for calculating instability deltas
    ///
    /// Supports:
    /// 1. Legacy API: getDeltas(tp53, brca1, low_delta, high_delta) - 2 params
    /// 2. Service-based: setCalculator() + getDeltas(tp53, brca1)
    class InstabilityDeltas {
    public:
        /// Delta values (configurable, with reasonable defaults)
        static constexpr double DELTA_LOW = 0.001;
        static constexpr double DELTA_MEDIUM = 0.002;
        static constexpr double DELTA_HIGH = 0.003;
        static constexpr double DELTA_VERY_HIGH = 0.004;

    private:
        /// Global calculator instance (can be replaced for custom behavior)
        inline static std::unique_ptr<IInstabilityDeltaCalculator> calculator_ = nullptr;

    public:
        /// Set a custom calculator (dependency injection)
        /// @param calculator Custom calculator instance (takes ownership)
        static void setCalculator(std::unique_ptr<IInstabilityDeltaCalculator> calculator) {
            calculator_ = std::move(calculator);
        }

        /// Reset to default behavior (no custom calculator)
        static void resetCalculator() {
            calculator_.reset();
        }

        /// Get deltas using the configured calculator (service-based API)
        /// If no calculator is set, falls back to legacy behavior with default coefficients
        /// @param tp53_status String representation ("+/+", "+/-", "-/-")
        /// @param brca1_status String representation ("+/+", "+/-", "-/-")
        /// @return pair<double, double> (delta_d1, delta_d2)
        static std::pair<double, double> getDeltas(
            const std::string& tp53_status,
            const std::string& brca1_status) {

            if (calculator_) {
                return calculator_->calculate(tp53_status, brca1_status);
            }

            // Fallback: use default coefficients (backward compatible)
            return getDeltas(tp53_status, brca1_status, DELTA_LOW, DELTA_HIGH);
        }

        /// Compute (delta_d1, delta_d2) based on TP53 and BRCA1 genetic status.
        /// Legacy API with 2 explicit coefficients (backward compatible)
        ///
        /// Matrix:
        /// - D1 depends only on TP53: Δ(D1) = Δ(TP53)
        /// - D2 depends on both genes: Δ(D2) = Δ(TP53) + Δ(BRCA1)
        ///
        /// @param tp53_status String representation ("+/+", "+/-", "-/-")
        /// @param brca1_status String representation ("+/+", "+/-", "-/-")
        /// @param low_delta Low delta value (default DELTA_LOW)
        /// @param high_delta High delta value (default DELTA_HIGH)
        /// @return pair<double, double> (delta_d1, delta_d2)
        static std::pair<double, double> getDeltas(
            const std::string& tp53_status,
            const std::string& brca1_status,
            double low_delta,
            double high_delta) {

            // Use SimpleDeltaCalculator for legacy API
            SimpleDeltaCalculator calc(low_delta, high_delta);
            return calc.calculate(tp53_status, brca1_status);
        }
    };

} // namespace domain

