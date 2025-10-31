//
// Created by luis on 31/10/25.
//

#include "AgenticCell.h"

namespace domain {

    void AgenticCell::live() {
        gene_tp53_.live();
        gene_brca1_.live();
    }

    bool AgenticCell::alive() {
        return true;

    }

    bool AgenticCell::tumoral() {
        return false;
    }

    std::string AgenticCell::getTP53() const {
        return gene_tp53_.status();
    }

    std::string AgenticCell::getBRCA1() const {
        return gene_brca1_.status();
    }

} // domain