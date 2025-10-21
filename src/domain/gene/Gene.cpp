#include "Gene.h"

namespace  domain {
    Gene::Gene() : state(Active) {}

    Gene::Gene(const State initial_state) : state(initial_state) {}

    void Gene::disable() {
        if (state == Active) {
            state = PartiallyDisabled;
        } else if (state == PartiallyDisabled) {
            state = Disabled;
        }
    }

    Gene::State Gene::getState() const {
        return state;
    }
}