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
        // Save copy to avoid revival side-effects if cell currently dead
        Genome genome_before = genome_.clone();

        // Avanzar todos los genes del genoma usando la API de Genome
        genome_.liveAllGenes();

        // If the cell was dead before the call, restore genome to prevent revival
        if (! ( [&]() { const Gene* brca1 = genome_before.getGene("BRCA1"); return brca1 && brca1->enabled(); }() ) ) {
            // restore previous genome state (prevent revival)
            genome_ = std::move(genome_before);
            return;
        }

        // Si ya está en estado neoplásico no necesitamos evaluar
        if (is_neoplastic_) return;
        // Determinar probabilidad p: 0 si TP53 está activo (protección), usar neoplasm_k_ si inactivo
        const Gene* tp53 = genome_.getGene("TP53");
        double p = (tp53 && tp53->enabled()) ? 0.0 : neoplasm_k_;
        if (p <= 0.0) return;
        auto sample = noise_->next().u01;
        if (sample < p) {
            is_neoplastic_ = true;
        }
    }

    AgenticCell::TickTrace AgenticCell::liveWithTrace(int tickIndex) {
        TickTrace tt;
        tt.tick = tickIndex;
        tt.alive_before = alive();
        tt.is_neoplastic_before = isNeoplastic();

        // Save a copy of the genome so we can prevent "revival" side-effects if the cell was dead
        Genome genome_before = genome_.clone();

        // Advance genes with trace
        auto gene_traces = genome_.liveAllGenesWithTrace();
        tt.gene_traces = std::move(gene_traces);

        // Update alive after gene transitions
        tt.alive_after = alive();

        // If the cell was dead before this tick, prevent it from reviving: restore previous genome
        if (!tt.alive_before) {
            // restore genome state to prevent revival
            genome_ = std::move(genome_before);
            tt.alive_after = false;
        }

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