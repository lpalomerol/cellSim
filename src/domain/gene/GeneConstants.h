#pragma once

namespace domain {

    /// Constantes para nombres de genes
    /// Evita magic strings y errores de tipeo en el código
    namespace GeneNames {
        constexpr const char* TP53 = "TP53";
        constexpr const char* BRCA1 = "BRCA1";
    }

    /// Constantes para representación de estados genéticos
    /// Representa zigosidad: +/+ (homocigoto dominante), +/- (heterocigoto), -/- (homocigoto recesivo)
    namespace GeneticStatusStrings {
        constexpr const char* WILD_TYPE = "+/+";
        constexpr const char* HETEROZYGOUS = "+/-";
        constexpr const char* HOMOZYGOUS_RECESSIVE = "-/-";
        constexpr const char* UNKNOWN = "?";
    }

} // namespace domain

