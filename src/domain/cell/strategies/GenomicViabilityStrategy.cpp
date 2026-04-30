#include "GenomicViabilityStrategy.h"

namespace domain {

    bool GenomicViabilityStrategy::isAlive(const ICell& cell) const {
        if (cell.hasEvadedApoptosis()) {
            return true;
        }
        return evaluateGenomicViability(cell);
    }

    bool GenomicViabilityStrategy::evaluateGenomicViability(const ICell& cell) const {
        // BRCA1 -/- + functional TP53 → intrinsic apoptosis (TP53 detects BRCA1-induced damage)
        // BRCA1 -/- + TP53 -/-       → viable (no checkpoint detection)
        if (cell.hasBRCA1Mutation() && cell.hasTP53Function()) {
            return false;
        }

        // Future rules:
        // - if (cell.getD1() > CATASTROPHIC_THRESHOLD) return false;  // Mitotic catastrophe
        // - if (cell.getAge() > HAYFLICK_LIMIT) return false;         // Replicative senescence

        return true;
    }

} // namespace domain

