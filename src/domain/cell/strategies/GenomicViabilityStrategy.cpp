#include "GenomicViabilityStrategy.h"
#include "../AgenticCell.h"

namespace domain {

    bool GenomicViabilityStrategy::isAlive(const AgenticCell& cell) const {
        // Rule 1: If cell has evaded apoptosis (is neoplastic), always alive (immortal)
        if (cell.hasEvadedApoptosis()) {
            return true;
        }

        // Rule 2: Delegate viability decision to genome
        // Genome checks BRCA1/TP53 interaction:
        // - BRCA1 -/- + TP53 functional → lethal (TP53 detects damage)
        // - BRCA1 -/- + TP53 -/- → viable (TP53 cannot detect damage)
        return cell.getGenome().isCellViable();
    }

} // namespace domain

