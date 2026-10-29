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
        /// @param kappa_brca1 BRCA1 D2 accumulation structural multiplier (default 2.0)
        /// @param age_drift Annual age drift added to D1 and D2 per year of cell age (default 1e-5)
        GenomicInstabilityDeltaStrategy(double low_delta, double high_delta,
                                        double kappa_brca1 = 2.0, double age_drift = 1e-5)
            : low_delta_(low_delta), high_delta_(high_delta),
              kappa_brca1_(kappa_brca1), age_drift_(age_drift) {}

        [[nodiscard]] InstabilityDeltas calculateDeltas(const ICell& cell, INoiseSource* noise) const override;

    private:
        double low_delta_;
        double high_delta_;
        double kappa_brca1_;
        double age_drift_;

        /// Helper: calculate delta for a gene given its string status ("+/+", "+/-", "-/-")
        [[nodiscard]] double deltaForStatus(const std::string& status) const;
    };

} // namespace domain

