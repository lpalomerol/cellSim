//
// Created by luis on 21/10/25.
//

#include "SimpleCell.h"
#include "OncoMatrix.h"

namespace domain {
    void SimpleCell::live() {
        // 1) Build transition matrix for this tick (depends on p_brca / p_tp53)
        TransitionMatrix P = build_onco_matrix(params_.p_mutation_brca,
                                               params_.p_mutation_tp53);

        // 2) Draw a single sample to select the next state
        const auto z = noise_.next();            // e.g., z.u01
        int cur  = static_cast<int>(onco_);
        int next = sampleNextState(P, cur, z.u01);
        onco_ = static_cast<OncoState>(next);

        // 3) Map onco-state to phenotype (deterministic)
        switch (onco_) {
            case OncoState::TP53_plus_plus:
            case OncoState::TP53_plus_minus:
            case OncoState::TP53_minus_minus:
                state_ = CellState::Alive; // alive (may be neoplastic for -/-)
                break;
            case OncoState::Apoptotic:
                state_ = CellState::Apoptotic;
                is_alive_ = false;
                break;
        }
    }

    bool SimpleCell::alive() const {
        return is_alive_;

    }

    bool SimpleCell::isNeoplastic() const {
        return onco_ == OncoState::TP53_minus_minus;
    }



}
