#pragma once

#include "IViabilityStrategy.h"

namespace domain {

    /// Default viability strategy: genomic-based survival criteria
    ///
    /// Integrates multiple biological signals to determine cell viability:
    /// 1. Neoplastic cells with apoptosis evasion → always alive (immortal)
    /// 2. Genomic configuration:
    ///    - BRCA1 -/- + TP53 functional → NOT viable (intrinsic apoptosis)
    ///    - BRCA1 -/- + TP53 -/- → viable (no checkpoint detection)
    /// 3. Future: D1 extreme → mitotic catastrophe
    /// 4. Future: Age > limit → replicative senescence
    ///
    /// This strategy acts as an INTEGRATOR of signals, not just a data query.
    /// The Genome provides INFORMATION (queries), the Strategy makes DECISIONS.
    class GenomicViabilityStrategy : public IViabilityStrategy {
    public:
        GenomicViabilityStrategy() = default;

        [[nodiscard]] bool isAlive(const AgenticCell& cell) const override;

    private:
        /// Evaluates viability based on genomic configuration
        /// Integrates BRCA1/TP53 state with future signals (D1, D2, age)
        [[nodiscard]] bool evaluateGenomicViability(const AgenticCell& cell) const;
    };

} // namespace domain

