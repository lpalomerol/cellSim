#pragma once

namespace domain {

    /// Gene name constants — avoids magic strings and typos
    namespace GeneNames {
        constexpr const char* TP53 = "TP53";
        constexpr const char* BRCA1 = "BRCA1";
    }

    /// String constants for gene state representation (zygosity notation)
    namespace GeneticStatusStrings {
        constexpr const char* WILD_TYPE = "+/+";
        constexpr const char* HETEROZYGOUS = "+/-";
        constexpr const char* HOMOZYGOUS_RECESSIVE = "-/-";
        constexpr const char* UNKNOWN = "?";
    }

} // namespace domain

