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

        Gene(INoiseSource* noise, State initial = State::PlusPlus)
            : state(initial), noise_(noise) {}

        std::string status() const;
        void mutate();
        void live();

        [[nodiscard]] bool enabled() const;

    private:
        State state;
        INoiseSource* noise_;
    };
}
