//
// Created by luis on 20/10/25.
//

#include "SimpleCell.h"

namespace domain {
    void SimpleCell::live() {
        // Implementación del mét. live
        if (noise_.next().homeostasis_u01 > cfg_.p_homeostasis) {
            is_alive_ = false;
        }
    }
    bool SimpleCell::alive() {
        return is_alive_;

    }
}
