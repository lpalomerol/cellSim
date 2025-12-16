#pragma once

#include <string>

namespace domain {

    /// CellLifeStage: Enum representing the 5 distinct cellular life stages.
    /// Derived on-the-fly from (TP53 status, BRCA1 status, D1, D2).
    enum class CellLifeStage {
        BASELINE,     ///< Normal cell, no genetic instability detected
        MID_STAGE1,   ///< TP53 +/- & BRCA1 +/- (first instability detected)
        MID_STAGE2,   ///< TP53 -/- & BRCA1 -/- (severe instability)
        PRIMER,       ///< TP53 -/- & D1 > 2.0 (pre-tumoral, visible to tissue)
        TUMORAL       ///< is_neoplastic_ == true (transformation completed)
    };

    /// Convert CellLifeStage enum to human-readable string.
    /// @param stage The stage to convert
    /// @return String representation of the stage
    inline std::string toString(CellLifeStage stage) {
        switch (stage) {
            case CellLifeStage::BASELINE:
                return "BASELINE";
            case CellLifeStage::MID_STAGE1:
                return "MID_STAGE1";
            case CellLifeStage::MID_STAGE2:
                return "MID_STAGE2";
            case CellLifeStage::PRIMER:
                return "PRIMER";
            case CellLifeStage::TUMORAL:
                return "TUMORAL";
            default:
                return "UNKNOWN";
        }
    }

} // namespace domain

