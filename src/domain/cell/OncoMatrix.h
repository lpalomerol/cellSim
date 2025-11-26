// OncoMatrix.h - REMOVED
#pragma once
#include <array>

namespace domain {
    constexpr int N_ONCO_STATES = 4;
    using TransitionMatrix = std::array<std::array<double, N_ONCO_STATES>, N_ONCO_STATES>;

    enum class OncoState : int {
        TP53_plus_plus = 0,
        TP53_plus_minus = 1,
        TP53_minus_minus = 2,
        Apoptotic = 3
    };

    // Stub functions used previously by SimpleCell; removed implementation.
    inline TransitionMatrix build_onco_matrix(double /*p_brca*/, double /*p_tp53*/) {
        TransitionMatrix P{};
        // identity-like default to avoid UB if called
        for (int i = 0; i < N_ONCO_STATES; ++i) for (int j = 0; j < N_ONCO_STATES; ++j) P[i][j] = (i==j) ? 1.0 : 0.0;
        return P;
    }

    inline int sampleNextState(const TransitionMatrix& P, int cur, double u01) {
        // deterministic fallback: find first j where cumulative >= u01
        double cum = 0.0;
        for (int j = 0; j < N_ONCO_STATES; ++j) {
            cum += P[cur][j];
            if (u01 <= cum) return j;
        }
        return N_ONCO_STATES - 1;
    }
}

// Nota: la implementación completa de la matriz de transición fue eliminada.
