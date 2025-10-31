//
// Created by luis on 31/10/25.
//

#include "Gene.h"
#include <iostream>

namespace domain {

    void Gene::mutate() {
        switch (state) {
            case State::PlusPlus:
                state = State::PlusMinus;
                break;
            case State::PlusMinus:
                state = State::MinusMinus;
                break;
            case State::MinusMinus:
                state = State::PlusPlus;
                break;
        }
    }

    bool Gene::enabled() const {
        return state != State::MinusMinus;
    }

    std::string Gene::status() const {
        switch (state) {
            case State::PlusPlus: return "+/+";
            case State::PlusMinus: return "+/-";
            case State::MinusMinus: return "-/-";
            default: return "?";
        }
    }

    void Gene::live() {
        if (noise_ && noise_->next().u01 > mutation_threshold_) {
            mutate();
        }
    }

}