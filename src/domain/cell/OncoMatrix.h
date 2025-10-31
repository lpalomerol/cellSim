// OncoMatrix.h
#pragma once
#include "OncoState.h"
#include <algorithm>

namespace domain {

    inline TransitionMatrix build_onco_matrix(double p_brca, double p_tp53) {
        auto clamp01 = [](double x){ return std::max(0.0, std::min(1.0, x)); };
        p_brca = clamp01(p_brca);
        p_tp53 = clamp01(p_tp53);

        TransitionMatrix P{{
            // from S0: to S0, S1, Tumoral, Apoptotic
            {{ 1.0 - (p_tp53 + p_brca),  p_tp53,                0.0,      p_brca }},
            // from S1: to S1, S0, Tumoral, Apoptotic  (columna S0=0)
            {{ 0.0,                      1.0 - (p_tp53 + p_brca), p_tp53,  p_brca }},
            // Tumoral (absorbing)
            {{ 0.0,                      0.0,                   1.0,       0.0 }},
            // Apoptotic (absorbing)
            {{ 0.0,                      0.0,                   0.0,       1.0 }},
        }};
        // Normalizaciones por si p_tp53+p_brca > 1 (redundante si clamp)
        for (int i = 0; i < N_ONCO_STATES; ++i) {
            double sum = 0.0; for (double v : P[i]) sum += v;
            if (sum > 0.0 && std::abs(sum - 1.0) > 1e-12)
                for (auto& v : P[i]) v /= sum;
        }
        return P;
    }

} // namespace domain