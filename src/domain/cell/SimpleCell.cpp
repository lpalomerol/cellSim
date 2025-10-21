//
// Created by luis on 20/10/25.
//

#include "SimpleCell.h"

#include <iostream>

namespace domain {
    void SimpleCell::live() {
        std::cout << "I'm alive!!" << std::endl;
        std::cout << "Cell age: " << age_ << " years." << std::endl;
        std::cout << "Cell homeostasis probability: " << cfg_.p_homeostasis << std::endl;
        std::cout << "Noise " << noise_.next().homeostasis_u01 << std::endl;
        // Implementación del mét. live
    }
    bool SimpleCell::alive() {
        return true;

    }
}
