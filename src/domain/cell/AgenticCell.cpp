//
// Created by luis on 31/10/25.
//

#include "AgenticCell.h"

namespace domain {

    AgenticCell::AgenticCell(std::unique_ptr<INoiseSource> noise, const Genome& genome, double tumor_k)
        : noise_(std::move(noise)), genome_(genome), tumor_k_(tumor_k), is_tumoral_(false) {
        // Inyectar la fuente de ruido en todos los genes del genoma
        for (auto& pair : const_cast<std::unordered_map<std::string, Gene>&>(genome_.genes())) {
            pair.second.setNoiseSource(noise_.get());
        }
    }

    void AgenticCell::live() {
        // Mutar todos los genes del genoma
        for (auto& pair : const_cast<std::unordered_map<std::string, Gene>&>(genome_.genes())) {
            pair.second.live();
        }
        // Si ya es tumoral no necesitamos evaluar
        if (is_tumoral_) return;
        // Determinar probabilidad p: 0 si TP53 está activo (protección), usar tumor_k_ si inactivo
        const Gene* tp53 = genome_.getGene("TP53");
        double p = (tp53 && tp53->enabled()) ? 0.0 : tumor_k_;
        if (p <= 0.0) return;
        auto sample = noise_->next().u01;
        if (sample < p) {
            is_tumoral_ = true;
        }
    }

    bool AgenticCell::alive() {
        const Gene* brca1 = genome_.getGene("BRCA1");
        return brca1 && brca1->enabled();
    }

    bool AgenticCell::tumoral() {
        return is_tumoral_;
    }

    std::string AgenticCell::getTP53() const {
        const Gene* tp53 = genome_.getGene("TP53");
        return tp53 ? tp53->status() : "?";
    }

    std::string AgenticCell::getBRCA1() const {
        const Gene* brca1 = genome_.getGene("BRCA1");
        return brca1 ? brca1->status() : "?";
    }

} // domain