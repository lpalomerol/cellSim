//
// Created by luis on 20/10/25.
//

#include "SimpleCell.h"

#include <iostream>

namespace domain {
    void SimpleCell::live() {
        std::cout << "I'm alive!!" << std::endl;
        // Implementación del mét. live
    }
    bool SimpleCell::alive() {
        return true;

    }
}
