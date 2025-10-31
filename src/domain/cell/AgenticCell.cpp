//
// Created by luis on 31/10/25.
//

#include "AgenticCell.h"

namespace domain {

    void AgenticCell::live() {
        // Primero los genes pueden mutar
        gene_tp53_.live();
        gene_brca1_.live();

        // Si ya es tumoral no necesitamos evaluar
        if (is_tumoral_) return;

        // Determinar probabilidad p: 0 si TP53 está activo (protección), params_.tumor_k si inactivo
        double p = gene_tp53_.enabled() ? 0.0 : params_.tumor_k;
        if (p <= 0.0) return;

        // Muestra del ruido compartido
        auto sample = noise_.next().u01;
        if (sample < p) {
            is_tumoral_ = true;
        }
    }

    bool AgenticCell::alive() {
        // La célula está viva si BRCA1 no es minusminus
        return gene_brca1_.enabled();

    }

    bool AgenticCell::tumoral() {
        return is_tumoral_;
    }

    std::string AgenticCell::getTP53() const {
        return gene_tp53_.status();
    }

    std::string AgenticCell::getBRCA1() const {
        return gene_brca1_.status();
    }

} // domain