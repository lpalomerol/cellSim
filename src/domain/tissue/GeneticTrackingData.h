#pragma once

#include <cstddef>
#include <array>
#include <numeric>

namespace domain {

    /**
     * GeneticTrackingData: Value object that holds genetic tracking results.
     * Uses arrays internally for simplicity, but provides named getters for clarity.
     */
    struct GeneticTrackingData {
        // Genetic categories (indices for arrays)
        enum class Category : int {
            BRCA_HET_TP53_HOM_PLUS = 0,    // BRCA1 "+/-" y TP53 "+/+"
            BRCA_HET_TP53_HET = 1,         // BRCA1 "+/-" y TP53 "+/-"
            BRCA_HET_TP53_HOM_MINUS = 2,   // BRCA1 "+/-" y TP53 "-/-"
            BRCA_HOM_MINUS = 3             // BRCA1 "-/-"
        };

        // Total cells by genetic category (includes all states: normal, neoplastic, dead)
        std::array<int, 4> cells = {};

        // Identified neoplasms by category (cells that have emitted neoplasm signal)
        // Note: BRCA_HOM_MINUS cannot be neoplastic, so only 3 categories
        std::array<int, 3> neoplasms = {};

        // Active neoplastic cells by category (currently alive and neoplastic)
        std::array<int, 3> active_neoplasms = {};

        // Named getters for backward compatibility and clarity
        [[nodiscard]] int getBrcaHetTp53HomPlus() const { return cells[static_cast<int>(Category::BRCA_HET_TP53_HOM_PLUS)]; }
        [[nodiscard]] int getBrcaHetTp53Het() const { return cells[static_cast<int>(Category::BRCA_HET_TP53_HET)]; }
        [[nodiscard]] int getBrcaHetTp53HomMinus() const { return cells[static_cast<int>(Category::BRCA_HET_TP53_HOM_MINUS)]; }
        [[nodiscard]] int getBrcaHomMinus() const { return cells[static_cast<int>(Category::BRCA_HOM_MINUS)]; }

        [[nodiscard]] int getNeoBrcaHetTp53HomPlus() const { return neoplasms[static_cast<int>(Category::BRCA_HET_TP53_HOM_PLUS)]; }
        [[nodiscard]] int getNeoBrcaHetTp53Het() const { return neoplasms[static_cast<int>(Category::BRCA_HET_TP53_HET)]; }
        [[nodiscard]] int getNeoBrcaHetTp53HomMinus() const { return neoplasms[static_cast<int>(Category::BRCA_HET_TP53_HOM_MINUS)]; }

        [[nodiscard]] int getActiveNeoBrcaHetTp53HomPlus() const { return active_neoplasms[static_cast<int>(Category::BRCA_HET_TP53_HOM_PLUS)]; }
        [[nodiscard]] int getActiveNeoBrcaHetTp53Het() const { return active_neoplasms[static_cast<int>(Category::BRCA_HET_TP53_HET)]; }
        [[nodiscard]] int getActiveNeoBrcaHetTp53HomMinus() const { return active_neoplasms[static_cast<int>(Category::BRCA_HET_TP53_HOM_MINUS)]; }

        // Direct member access for original code (public for backward compatibility)
        // These map to the array-based storage
        int& brca_het_tp53_hom_plus = cells[static_cast<int>(Category::BRCA_HET_TP53_HOM_PLUS)];
        int& brca_het_tp53_het = cells[static_cast<int>(Category::BRCA_HET_TP53_HET)];
        int& brca_het_tp53_hom_minus = cells[static_cast<int>(Category::BRCA_HET_TP53_HOM_MINUS)];
        int& brca_hom_minus = cells[static_cast<int>(Category::BRCA_HOM_MINUS)];

        int& neo_brca_het_tp53_hom_plus = neoplasms[static_cast<int>(Category::BRCA_HET_TP53_HOM_PLUS)];
        int& neo_brca_het_tp53_het = neoplasms[static_cast<int>(Category::BRCA_HET_TP53_HET)];
        int& neo_brca_het_tp53_hom_minus = neoplasms[static_cast<int>(Category::BRCA_HET_TP53_HOM_MINUS)];

        int& active_neo_brca_het_tp53_hom_plus = active_neoplasms[static_cast<int>(Category::BRCA_HET_TP53_HOM_PLUS)];
        int& active_neo_brca_het_tp53_het = active_neoplasms[static_cast<int>(Category::BRCA_HET_TP53_HET)];
        int& active_neo_brca_het_tp53_hom_minus = active_neoplasms[static_cast<int>(Category::BRCA_HET_TP53_HOM_MINUS)];

        // Reset all counters to zero
        void reset() {
            cells.fill(0);
            neoplasms.fill(0);
            active_neoplasms.fill(0);
        }

        [[nodiscard]] std::size_t totalCells() const {
            return std::accumulate(cells.begin(), cells.end(), 0);
        }

        [[nodiscard]] std::size_t totalNeoplasms() const {
            return std::accumulate(neoplasms.begin(), neoplasms.end(), 0);
        }

        [[nodiscard]] std::size_t totalActiveNeoplasms() const {
            return std::accumulate(active_neoplasms.begin(), active_neoplasms.end(), 0);
        }
    };

} // namespace domain

