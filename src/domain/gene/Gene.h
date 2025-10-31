//
// Created by luis on 31/10/25.
//

#pragma once
#include <string>
#include "../ports/INoiseSource.h"

namespace domain{
    class Gene {
    public:
        enum class State { PlusPlus, PlusMinus, MinusMinus };

        Gene(INoiseSource* noise, State initial = State::PlusPlus, double mutation_threshold = 0.1, double mutation_instability_k = 0.0)
            : state(initial), noise_(noise), mutation_threshold_(mutation_threshold), mutation_instability_k_(mutation_instability_k) {}

        std::string status() const;
        void mutate();
        void live();
        double getMutationThreshold() const { return mutation_threshold_; }
        double getMutationInstabilityK() const { return mutation_instability_k_; }

        [[nodiscard]] bool enabled() const;

    private:
        State state;
        INoiseSource* noise_;
        double mutation_threshold_;
        double mutation_instability_k_;
    };
}
