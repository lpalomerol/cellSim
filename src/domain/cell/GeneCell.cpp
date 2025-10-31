//
// Created by luis on 21/10/25.
//

#include "GeneCell.h"
#include "OncoMatrix.h"

namespace domain {
    void GeneCell::live() {
        // 1) Matriz del tick (depende solo de p_brca/p_tp53)
        TransitionMatrix P = build_onco_matrix(params_.p_mutation_brca,
                                               params_.p_mutation_tp53);

        // 2) Elegir transición con una única muestra
        const auto z = noise_.next();            // trae, p.ej., z.markov_u01
        int cur  = static_cast<int>(onco_);
        int next = sampleNextState(P, cur, z.markov_u01);
        onco_ = static_cast<OncoState>(next);

        // 3) Mapear a fenotipo (determinista)
        switch (onco_) {
            case OncoState::S0:
            case OncoState::S1:
                state_ = CellState::Alive;
                break;
            case OncoState::Tumoral:
                state_ = CellState::Alive;
                break; // si quieres “viva pero tumoral”
            case OncoState::Apoptotic:
                state_ = CellState::Apoptotic;
                is_alive_ = false;
                break;
        }
    }

    bool GeneCell::alive() {
        return is_alive_;

    }




}
