//
// Created by luis on 21/10/25.
//

#include "GeneCell.h"


namespace domain {
    void GeneCell::live() {
        const auto noise1 = noise_.next();
        if (noise1.mutation_brca_u01 < params_.p_mutation_brca)
            brca1_.disable();
        if (noise1.mutation_tp53_u01 < params_.p_mutation_tp53)
            tp53_.disable();

        // 2. Construye matriz actual (modulada)
        auto P = modulatedMatrix(params_.base_matrix, brca1_, tp53_);

        // 3. Muestra transición Markov
        const auto noise2 = noise_.next();
        int current = static_cast<int>(state_);
        int next = sampleNextState(P, current, noise2.markov_u01);
        state_ = static_cast<CellState>(next);

        is_alive_ = (state_ != CellState::Apoptotic);

    }
    bool GeneCell::alive() {
        return is_alive_;

    }

    CellState GeneCell::state() {
        return state_;

    }

    int GeneCell::sampleNextState(const TransitionMatrix& matrix, int current_state, double u) {
        double cumulative = 0.0;
        for (int next_state = 0; next_state < N_STATES; ++next_state) {
            cumulative += matrix[current_state][next_state];
            if (u < cumulative) {
                return next_state;
            }
        }
        return N_STATES - 1; // Fallback
    }

    TransitionMatrix GeneCell::modulatedMatrix(const TransitionMatrix& base, const Gene& brca1, const Gene& tp53) {
        TransitionMatrix modulated = base;
        return modulated;
    }



}
