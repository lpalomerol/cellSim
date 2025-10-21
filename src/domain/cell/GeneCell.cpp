//
// Created by luis on 21/10/25.
//

#include "GeneCell.h"


namespace domain {
    void GeneCell::live() {
        // Implementación del mét. live
        if (noise_.next().homeostasis_u01 > params_.p_homeostasis) {
            is_alive_ = false;
        }
        if (noise_.next().mutation_brca_u01 < params_.p_mutation_brca) {
            brca1_.disable();
        }
    }
    bool GeneCell::alive() {
        return is_alive_ && brca1_.getState() != Gene::Disabled;

    }
}
