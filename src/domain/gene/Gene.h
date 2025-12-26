//
// Created by luis on 31/10/25.
//

#pragma once
#include <string>
#include "../ports/INoiseSource.h"
#include "../ports/ILogger.h"
#include "../ports/ILoggeable.h"
#include "../shared/Threshold.h"
#include "GeneStatus.h"

namespace domain{
    class Gene : public ports::ILoggeable {
    public:
        // Kept for backward compatibility with tests and existing code
        enum class State { PlusPlus, PlusMinus, MinusMinus };

        // Name is taken by value and moved into the member to avoid unnecessary copies
        explicit Gene(std::string name,
            State initial = State::PlusPlus,
            double mutation_threshold = 0.1,
            double mutation_instability_k = 0.0,
            ports::ILoggerPtr logger = nullptr
        );

        [[nodiscard]] const std::string& name() const;

        /// Get functional status of gene (NEW: semantic API)
        [[nodiscard]] GeneStatus getStatus() const;

        /// Legacy compatibility: return string representation
        [[nodiscard]] std::string status() const;

        // Return a detail string: "NAME[status] p(mut)=threshold", e.g. "TP53[+/-] p(mut)=0.1"
        [[nodiscard]] std::string details(bool apply_instability = true) const;

        // ILoggeable implementation
        std::string getLogCategory() const override { return "GENE"; }

        void mutate();
        // Live: advance gene state based on noise and mutation threshold
        // apply_instability: if true, adds mutation_instability_k_ to threshold
        // genomic_instability: multiplier for mutation probability (default 1.0 for normal cells)
        void live(bool apply_instability = true, double genomic_instability = 1.0);

        [[nodiscard]] double getMutationThreshold() const { return mutation_threshold_.value(); }
        [[nodiscard]] double getMutationInstabilityK() const { return mutation_instability_k_; }
        void setNoiseSource(INoiseSource* noise);

        [[nodiscard]] bool enabled() const;

        // Allow forcing the gene state (useful for tests and factory)
        void setState(State s);


    private:
        std::string name_;
        State state_;
        domain::shared::Threshold mutation_threshold_;
        double mutation_instability_k_;
        INoiseSource* noise_;
        ports::ILoggerPtr logger_;

        // Calculate the effective mutation threshold based on instability and genomic factors
        [[nodiscard]] double calculateMutationThreshold(bool apply_instability, double genomic_instability) const;
    };
}
