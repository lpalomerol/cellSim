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
        tt.tick = tickIndex;
        tt.alive_before = alive();
        tt.is_neoplastic_before = isNeoplastic();

        // If the cell was dead before this tick, do not advance genes or consume noise:
        // return a trace reflecting no changes to avoid any side-effects (no "revival" activity).
        if (!tt.alive_before) {
            tt.gene_traces = {};
            tt.alive_after = false;
            tt.is_neoplastic_after = isNeoplastic();
            tt.neoplasm_happened = false;
            tt.neoplasm_threshold = 0.0;
            tt.neoplasm_sample = -1.0;
            return tt;
        }

        // Advance genes with trace
        auto gene_traces = genome_.liveAllGenesWithTrace();
        tt.gene_traces = std::move(gene_traces);

        // Update alive after gene transitions
        tt.alive_after = alive();


        // If already neoplastic before, nothing to sample
        if (is_neoplastic_) {
            tt.is_neoplastic_after = isNeoplastic();
            tt.neoplasm_happened = false;
            tt.neoplasm_threshold = 0.0;
            tt.neoplasm_sample = -1.0;
            return tt;
        }

        // Determine probability p for neoplasm: protected if TP53 enabled
        const Gene* tp53 = genome_.getGene("TP53");
        double p = (tp53 && tp53->enabled()) ? 0.0 : neoplasm_k_;
        tt.neoplasm_threshold = p;
        if (p <= 0.0) {
            tt.neoplasm_sample = -1.0;
            tt.neoplasm_happened = false;
            tt.is_neoplastic_after = isNeoplastic();
            return tt;
        }

        // Sample and decide
        double sample = noise_->next().u01;
        tt.neoplasm_sample = sample;
        if (sample < p) {
            is_neoplastic_ = true;
            tt.neoplasm_happened = true;
        } else {
            tt.neoplasm_happened = false;
        }
        tt.is_neoplastic_after = isNeoplastic();
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