#pragma once

#include <string>

namespace domain {

    /// GeneStatus: Value object representing the functional state of a gene
    /// Maps genotype (+/+, +/-, -/-) to functional phenotype (enabled, partially_enabled, disabled)
    /// This encapsulates the genotype-to-phenotype mapping and provides semantic queries
    class GeneStatus {
    public:
        enum class State {
            ENABLED,              // +/+ (wildtype, fully functional, both alleles working)
            PARTIALLY_ENABLED,    // +/- (heterozygous, semi-functional, one allele working)
            DISABLED,             // -/- (homozygous recessive, non-functional, no working alleles)
            UNKNOWN               // ? (invalid/unknown state)
        };

        /// Factory methods for clarity and type safety
        static GeneStatus enabled() {
            return GeneStatus(State::ENABLED);
        }

        static GeneStatus partiallyEnabled() {
            return GeneStatus(State::PARTIALLY_ENABLED);
        }

        static GeneStatus disabled() {
            return GeneStatus(State::DISABLED);
        }

        static GeneStatus unknown() {
            return GeneStatus(State::UNKNOWN);
        }

        /// Parse from string representation (for legacy compatibility, I/O, and tests)
        /// Converts "+/+", "+/-", "-/-", "?" strings to GeneStatus values
        static GeneStatus fromString(const std::string& str) {
            if (str == "+/+") return enabled();
            if (str == "+/-") return partiallyEnabled();
            if (str == "-/-") return disabled();
            return unknown();
        }

        // ===== Semantic query methods: more intuitive than checking raw state =====

        /// Is gene fully functional (both alleles working)?
        bool isEnabled() const {
            return state_ == State::ENABLED;
        }

        /// Is gene partially functional (one allele working)?
        bool isPartiallyEnabled() const {
            return state_ == State::PARTIALLY_ENABLED;
        }

        /// Is gene non-functional (no alleles working)?
        bool isDisabled() const {
            return state_ == State::DISABLED;
        }

        /// Is state unknown/invalid?
        bool isUnknown() const {
            return state_ == State::UNKNOWN;
        }

        /// Query: Does this gene have at least one functional copy?
        /// Useful for tumor suppressor logic: "Can TP53 still suppress tumors?"
        bool hasFunctionalCopy() const {
            return state_ == State::ENABLED || state_ == State::PARTIALLY_ENABLED;
        }

        /// Query: Is gene completely non-functional?
        /// Useful for lethal allele logic: "Is BRCA1 completely lost?"
        bool isNonFunctional() const {
            return state_ == State::DISABLED;
        }

        // ===== String representation =====

        /// Convert to string representation for output, logging, display
        /// Returns the standard genetic notation: "+/+", "+/-", "-/-", "?"
        std::string toString() const {
            switch (state_) {
                case State::ENABLED: return "+/+";
                case State::PARTIALLY_ENABLED: return "+/-";
                case State::DISABLED: return "-/-";
                case State::UNKNOWN: return "?";
                default: return "?";
            }
        }

        // ===== Operators =====

        /// Equality comparison
        bool operator==(const GeneStatus& other) const {
            return state_ == other.state_;
        }

        /// Inequality comparison
        bool operator!=(const GeneStatus& other) const {
            return state_ != other.state_;
        }

        // ===== Mutation simulation =====

        /// Simulate mutation: transition to next less-functional state
        /// +/+ → +/- → -/- (progressive loss of function)
        /// Already disabled genes cannot mutate further
        GeneStatus mutated() const {
            switch (state_) {
                case State::ENABLED:
                    return partiallyEnabled();  // Lose one allele
                case State::PARTIALLY_ENABLED:
                    return disabled();  // Lose last allele
                case State::DISABLED:
                case State::UNKNOWN:
                    return *this;  // Cannot mutate further
                default:
                    return *this;
            }
        }

    private:
        explicit GeneStatus(State s) : state_(s) {}
        State state_;
    };

} // namespace domain

