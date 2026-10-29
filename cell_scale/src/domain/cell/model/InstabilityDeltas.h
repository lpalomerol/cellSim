#pragma once
#include <string>

namespace domain {
    /// Value object representing DNA damage and immunosuppression increments
    /// Encapsulates the pair of deltas calculated by instability strategies
    ///
    /// Benefits over std::pair<double, double>:
    /// - Self-documenting: d1() and d2() vs first/second
    /// - Type-safe: cannot confuse order
    /// - Extensible: easy to add mutation_rate or other metrics
    /// - Helper methods: totalInstability(), isSignificant(), etc.
    class InstabilityDeltas {
    public:
        /// Factory: create from individual delta values
        static InstabilityDeltas create(double d1, double d2) {
            return InstabilityDeltas(d1, d2);
        }

        /// Getters
        [[nodiscard]] double d1() const { return d1_; }
        [[nodiscard]] double d2() const { return d2_; }

        /// Convenience: apply deltas to current values
        [[nodiscard]] double applyToD1(double current_d1) const {
            return current_d1 + d1_;
        }

        [[nodiscard]] double applyToD2(double current_d2) const {
            return current_d2 + d2_;
        }

        /// Total instability increase (sum of both)
        [[nodiscard]] double totalInstability() const {
            return d1_ + d2_;
        }

        /// Check if deltas are significant (above threshold)
        [[nodiscard]] bool isSignificant(double threshold = 0.001) const {
            return d1_ > threshold || d2_ > threshold;
        }

        /// Equality
        [[nodiscard]] bool operator==(const InstabilityDeltas& other) const {
            return d1_ == other.d1_ && d2_ == other.d2_;
        }

        [[nodiscard]] bool operator!=(const InstabilityDeltas& other) const {
            return !(*this == other);
        }

        /// String representation (for logging)
        [[nodiscard]] std::string toString() const {
            return "InstabilityDeltas{D1=" + std::to_string(d1_) +
                   ", D2=" + std::to_string(d2_) + "}";
        }

    private:
        explicit InstabilityDeltas(double d1, double d2)
            : d1_(d1), d2_(d2) {}

        double d1_;  // DNA damage delta
        double d2_;  // Immunosuppression delta
    };

} // namespace domain

