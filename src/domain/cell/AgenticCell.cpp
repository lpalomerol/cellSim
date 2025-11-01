//
// Created by luis on 31/10/25.
//

#include "AgenticCell.h"

namespace domain {

    AgenticCell::AgenticCell(std::unique_ptr<INoiseSource> noise, Genome genome, double neoplasm_k)
        : noise_(std::move(noise)), genome_(std::move(genome)), neoplasm_k_(neoplasm_k), is_neoplastic_(false) {
        // Inyectar la fuente de ruido en todos los genes del genoma usando la API de Genome
        genome_.setNoiseSourceForAll(noise_.get());
    }

    void AgenticCell::live() {
        // Delegar en la versión con traza para evitar duplicación.
        // No necesitamos el índice de tick aquí, usamos 0 como valor por defecto.
        (void) liveWithTrace(0);
    }

    AgenticCell::TickTrace AgenticCell::liveWithTrace(int tickIndex) {
        TickTrace tt;
        tt.init(tickIndex, alive(), isNeoplastic());

        // If the cell was dead before this tick, do not advance genes or consume noise:
        // return a trace reflecting no changes to avoid any side-effects (no "revival" activity).
        if (!tt.alive_before) {
            tt.markDeadNoChange();
            return tt;
        }

        // Advance genes with trace and set alive_after accordingly
        auto gene_traces = genome_.liveAllGenesWithTrace();
        tt.setGeneTracesAndAliveAfter(std::move(gene_traces), alive());


        // If already neoplastic before, nothing to sample
        if (is_neoplastic_) {
            tt.markAlreadyNeoplastic();
            return tt;
        }

        // Determine probability p for neoplasm: protected if TP53 enabled
        const Gene* tp53 = genome_.getGene("TP53");
        double p = (tp53 && tp53->enabled()) ? 0.0 : neoplasm_k_;

        // Sample and decide (decideNeoplasm updates is_neoplastic_ by reference)
        double sample = noise_->next().u01;
        tt.decideNeoplasm(p, sample, is_neoplastic_);

        return tt;
    }

    bool AgenticCell::alive() {
        const Gene* brca1 = genome_.getGene("BRCA1");
        return brca1 && brca1->enabled();
    }

    bool AgenticCell::isNeoplastic() {
        return is_neoplastic_;
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