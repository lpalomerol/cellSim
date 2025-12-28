#include "GenomicViabilityStrategy.h"
#include "../AgenticCell.h"
#include "../../gene/GeneConstants.h"

namespace domain {

    bool GenomicViabilityStrategy::isAlive(const AgenticCell& cell) const {
        // Rule 1: If cell has evaded apoptosis (is neoplastic), always alive (immortal)
        if (cell.hasEvadedApoptosis()) {
            return true;
        }

        // Rule 2: Evaluate integrated viability (genomic configuration + future: D1/D2/age)
        return evaluateGenomicViability(cell);
    }

    bool GenomicViabilityStrategy::evaluateGenomicViability(const AgenticCell& cell) const {
        const Genome& genome = cell.getGenome();

        // Check BRCA1 gene presence
        const Gene* brca1 = genome.getGene(GeneNames::BRCA1);
        if (!brca1) {
            // Missing BRCA1 → lethal
            return false;
        }

        // BRCA1 -/- (mutation) triggers apoptosis IF TP53 is functional
        // Biological rationale:
        // - BRCA1 -/- causes severe DNA damage
        // - Functional TP53 detects damage → triggers apoptosis
        // - Non-functional TP53 cannot detect damage → cell survives (but unstable)
        if (genome.hasBRCA1Mutation() && genome.hasTP53Function()) {
            // Intrinsic apoptosis: TP53 detects BRCA1-induced DNA damage
            return false;
        }

        // Future rules can be added here:
        // - if (cell.getD1() > CATASTROPHIC_THRESHOLD) return false;  // Mitotic catastrophe
        // - if (cell.getAge() > HAYFLICK_LIMIT) return false;         // Replicative senescence
        // - if (cell.getD2() > threshold && !cell.hasEvasedApoptosis()) return false; // Extrinsic

        return true;  // Cell is viable
    }

} // namespace domain

