//
// Created by luis on 19/10/25.
//

#include "cell.h"
#include <iostream>


Cell::Cell(): k(0.0){
    std::cout << "I'm a cell";
}

void Cell::live() {
    std::cout << "Live, bip";
}