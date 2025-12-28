#pragma once

#include "IViabilityStrategy.h"

namespace domain {

    /// Default viability strategy: genomic-based survival criteria
    ///
    /// Rules:
    /// 1. Cells that evaded apoptosis → always alive (immortal/neoplastic)
    /// 2. Otherwise, viability determined by Genome::isCellViable()
    ///    - BRCA1 -/- is lethal IF TP53 is functional (detects DNA damage)
    ///    - BRCA1 -/- is NOT lethal if TP53 -/- (cannot detect damage)
    ///
    /// This strategy encapsulates the genomic checkpoint logic that determines
    /// whether a cell can survive based on its genetic state.
    class GenomicViabilityStrategy : public IViabilityStrategy {
    public:
        GenomicViabilityStrategy() = default;

        [[nodiscard]] bool isAlive(const AgenticCell& cell) const override;
    };

} // namespace domain

