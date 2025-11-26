#pragma once

#include <cstddef>

namespace domain {

    /**
     * GeneticTrackingData: Value object that holds the result of genetic tracking.
     * No lógica, solo datos de valor.
     */
    struct GeneticTrackingData {
        int brca_het_tp53_hom_plus = 0;      // BRCA1 "+/-" y TP53 "+/+"
        int brca_het_tp53_het = 0;           // BRCA1 "+/-" y TP53 "+/-"
        int brca_het_tp53_hom_minus = 0;     // BRCA1 "+/-" y TP53 "-/-"
        int brca_hom_minus = 0;              // BRCA1 "-/-"

        // Neoplasms by category
        int neo_brca_het_tp53_hom_plus = 0;
        int neo_brca_het_tp53_het = 0;
        int neo_brca_het_tp53_hom_minus = 0;

        void reset() {
            brca_het_tp53_hom_plus = 0;
            brca_het_tp53_het = 0;
            brca_het_tp53_hom_minus = 0;
            brca_hom_minus = 0;
            neo_brca_het_tp53_hom_plus = 0;
            neo_brca_het_tp53_het = 0;
            neo_brca_het_tp53_hom_minus = 0;
        }

        [[nodiscard]] std::size_t totalCells() const {
            return brca_het_tp53_hom_plus + brca_het_tp53_het + brca_het_tp53_hom_minus + brca_hom_minus;
        }

        [[nodiscard]] std::size_t totalNeoplasms() const {
            return neo_brca_het_tp53_hom_plus + neo_brca_het_tp53_het + neo_brca_het_tp53_hom_minus;
        }
    };

} // namespace domain

