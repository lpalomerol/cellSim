//
// Created by luis on 31/10/25.
//

#include "AgenticCell.h"
#include <iostream> // Incluir iostream para usar std::cout

namespace domain {

    void AgenticCell::live() {
        gene_tp53_.live();
        gene_brca1_.live();
    }

    bool AgenticCell::alive() {
        // La célula está viva si BRCA1 no es minusminus
        return gene_brca1_.enabled();

    }

    bool AgenticCell::tumoral() {
        bool enabled = gene_tp53_.enabled();
        return !enabled;
    }

    std::string AgenticCell::getTP53() const {
        return gene_tp53_.status();
    }

    std::string AgenticCell::getBRCA1() const {
        return gene_brca1_.status();
    }

} // domain