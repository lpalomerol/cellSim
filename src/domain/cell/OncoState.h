// OncoState.h
#pragma once
#include <array>

namespace domain {
    enum class OncoState : int { TP53_plus_plus=0, TP53_plus_minus=1, TP53_minus_minus=2, Apoptotic=3 };

    constexpr int N_ONCO_STATES = 4;
    using TransitionMatrix = std::array<std::array<double, N_ONCO_STATES>, N_ONCO_STATES>;

    inline int sampleNextState(const TransitionMatrix& P, int s, double u) {
        double acc = 0.0;
        for (int j = 0; j < N_ONCO_STATES; ++j) {
            acc += P[s][j];
            if (u < acc) {
                return j;
            }
        }
        return s; // seguridad numérica
    }
}