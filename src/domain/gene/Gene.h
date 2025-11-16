//
// Created by luis on 31/10/25.
//

#pragma once
#include <string>
#include "../ports/INoiseSource.h"
#include "../shared/Threshold.h"

namespace domain{
    class Gene {
    public:
        enum class State { PlusPlus, PlusMinus, MinusMinus };
        // Name is taken by value and moved into the member to avoid unnecessary copies
        explicit Gene(std::string name, State initial = State::PlusPlus, double mutation_threshold = 0.1, double mutation_instability_k = 0.0, bool verbose = false);
        [[nodiscard]] const std::string& name() const;
        [[nodiscard]] std::string status() const;
        // Return a detail string: "NAME[status]", e.g. "TP53[+/-]"
        [[nodiscard]] std::string details() const;

        [[nodiscard]] std::string details(bool unstable) const;

        void mutate();
        void live();
        void live(bool apply_instability);
        // Overload: accepts an immunosuppression factor (>1 increases mutation probability)
        void live(bool apply_instability, double immunosuppression);
        [[nodiscard]] double getMutationThreshold() const { return mutation_threshold_.value(); }
        [[nodiscard]] double getMutationInstabilityK() const { return mutation_instability_k_; }
        void setNoiseSource(INoiseSource* noise);

        [[nodiscard]] bool enabled() const;

        // Allow forcing the gene state (useful for tests and factory)
        void setState(State s);

        [[nodiscard]] double get_mutation_threshold(bool apply_instability) const;

        // Per-gene verbose control
        void setVerbose(bool v);

    private:
        std::string name_;
        State state_;
        domain::shared::Threshold mutation_threshold_;
        double mutation_instability_k_;
        INoiseSource* noise_;
        bool verbose_ = false;
    };
}
