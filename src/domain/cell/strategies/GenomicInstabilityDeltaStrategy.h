#pragma once

#include "IInstabilityDeltaStrategy.h"
#include "../../gene/GeneStatus.h"

namespace domain {

    /// Default instability delta strategy: genomic-based calculation
    ///
    /// Model:
    /// - D1 (DNA damage) depends only on TP53 status
    /// - D2 (Immunosuppression) depends on TP53 + BRCA1 status (additive)
    ///
    /// Parameters:
    /// - low_delta: increment for heterozygous mutations (+/-)
    /// - high_delta: increment for homozygous recessive mutations (-/-)
    ///
    /// BRCA1 contribution to D2:
    /// - BRCA1 -/-: 2x high_delta (severe immunosuppression)
    /// - BRCA1 +/-: 1x low_delta (partial immunosuppression)
    /// - BRCA1 +/+: 0 (no contribution)
    class GenomicInstabilityDeltaStrategy : public IInstabilityDeltaStrategy {
    public:
        /// Constructor
        /// @param low_delta Delta increment for heterozygous state (default 0.001)
        /// @param high_delta Delta increment for homozygous recessive state (default 0.003)
        GenomicInstabilityDeltaStrategy(double low_delta, double high_delta)
            : low_delta_(low_delta), high_delta_(high_delta) {}

        [[nodiscard]] InstabilityDeltas calculateDeltas(const AgenticCell& cell) const override;

    private:
        double low_delta_;
        double high_delta_;

        /// Helper: calculate delta for a single gene based on its status
        [[nodiscard]] double calculateGeneDelta(const GeneStatus& gene_status) const;
    };

} // namespace domain

