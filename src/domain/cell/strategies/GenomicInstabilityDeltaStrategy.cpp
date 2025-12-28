#include "GenomicInstabilityDeltaStrategy.h"
#include "../AgenticCell.h"
#include "../../gene/Gene.h"
#include "../../gene/GeneConstants.h"

namespace domain {

    std::pair<double, double> GenomicInstabilityDeltaStrategy::calculateDeltas(
        const AgenticCell& cell) const {

        const Gene* tp53 = cell.getGenome().getGene(GeneNames::TP53);
        const Gene* brca1 = cell.getGenome().getGene(GeneNames::BRCA1);

        // Calculate delta for TP53: affects D1 directly and contributes to D2
        double delta_tp53 = 0.0;
        if (tp53) {
            delta_tp53 = calculateGeneDelta(tp53->getStatus());
        }

        // Calculate delta for BRCA1: contributes only to D2 (immunosuppression)
        double delta_brca1 = 0.0;
        if (brca1) {
            auto brca1_status = brca1->getStatus();
            if (brca1_status.isEnabled()) {
                delta_brca1 = 0.0;  // Wildtype: no contribution
            } else if (brca1_status.isPartiallyEnabled()) {
                delta_brca1 = low_delta_;  // Heterozygous: low contribution
            } else if (brca1_status.isDisabled()) {
                delta_brca1 = 2 * high_delta_;  // Homozygous: very high contribution
            }
        }

        // Decision matrix:
        // D1 (DNA damage) = Δ(TP53) only
        // D2 (Immunosuppression) = Δ(TP53) + Δ(BRCA1)
        double delta_d1 = delta_tp53;
        double delta_d2 = delta_tp53 + delta_brca1;

        return {delta_d1, delta_d2};
    }

    double GenomicInstabilityDeltaStrategy::calculateGeneDelta(
        const GeneStatus& gene_status) const {

        if (gene_status.isEnabled()) {
            return 0.0;
        } else if (gene_status.isPartiallyEnabled()) {
            return low_delta_;
        } else if (gene_status.isDisabled()) {
            return high_delta_;
        }
        return 0.0;
    }

} // namespace domain

