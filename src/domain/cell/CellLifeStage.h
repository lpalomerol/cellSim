#pragma once

#include <string>

namespace domain {

    /// CellLifeStage: Enum representing the 6 distinct cellular life stages.
    /// Derived on-the-fly from (alive, is_neoplastic, TP53 status, BRCA1 status, D1, D2).
    ///
    /// Genetic basis:
    /// - BASELINE: TP53 +/+ & BRCA1 +/- (normal, protected by TP53)
    /// - UNSTABLE: TP53 +/- & BRCA1 +/- (heterozygous instability)
    /// - UNPROTECTED: TP53 -/- (any BRCA1 status: +/- or -/-) (no TP53 protection)
    /// - PRIMER: UNPROTECTED + D1 > 2.0 (pre-tumoral, visible to tissue)
    /// - TUMORAL: is_neoplastic_ == true (transformation completed)
    /// - DEAD: !alive() (cell is dead)
    enum class CellLifeStage {
        DEAD,         ///< Cell is not alive (!alive())
        BASELINE,     ///< TP53 +/+ & BRCA1 +/- (normal, TP53 protected)
        UNSTABLE,     ///< TP53 +/- & BRCA1 +/- (first instability, still somewhat protected)
        UNPROTECTED,  ///< TP53 -/- (any BRCA1: +/- or -/-) (no TP53 protection, vulnerable)
        PRIMER,       ///< TP53 -/- & D1 > 2.0 (pre-tumoral, tissue can detect)
        TUMORAL       ///< is_neoplastic_ == true (transformation completed, neoplasm active)
    };

    /// Convert CellLifeStage enum to human-readable string.
    /// @param stage The stage to convert
    /// @return String representation of the stage
    inline std::string toString(CellLifeStage stage) {
        switch (stage) {
            case CellLifeStage::DEAD:
                return "DEAD";
            case CellLifeStage::BASELINE:
                return "BASELINE";
            case CellLifeStage::UNSTABLE:
                return "UNSTABLE";
            case CellLifeStage::UNPROTECTED:
                return "UNPROTECTED";
            case CellLifeStage::PRIMER:
                return "PRIMER";
            case CellLifeStage::TUMORAL:
                return "TUMORAL";
            default:
                return "UNKNOWN";
        }
    }

} // namespace domain

