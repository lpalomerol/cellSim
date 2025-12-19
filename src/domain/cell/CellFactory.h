#pragma once

#include <memory>
#include <string>
#include "AgenticCell.h"
#include "../gene/Genome.h"
#include "../ports/ILogger.h"

namespace domain {

/// Factory for creating AgenticCell instances
/// Encapsulates construction logic and default parameters
class CellFactory {
public:
    /// Create a normal AgenticCell with standard parameters
    static std::unique_ptr<AgenticCell> createNormalCell(
        const Genome& genome,
        const ports::ILoggerPtr& logger = nullptr);

    /// Create an AgenticCell with custom parameters
    static std::unique_ptr<AgenticCell> createCustomCell(
        const Genome& genome,
        double neoplasm_k,
        double low_delta_instability,
        double high_delta_instability,
        double division_rate,
        double neoplastic_division_rate,
        bool enable_big_bang_mode,
        double apoptosis_instability_threshold,
        double d1_primer_threshold,
        double d2_apoptosis_threshold,
        const ports::ILoggerPtr& logger = nullptr);

    /// Default parameters (package them for clarity)
    struct Defaults {
        static constexpr double NEOPLASM_K = 0.002;
        static constexpr double LOW_DELTA_INSTABILITY = 0.0001;
        static constexpr double HIGH_DELTA_INSTABILITY = 0.0002;
        static constexpr double DIVISION_RATE = 0.001;
        static constexpr double NEOPLASTIC_DIVISION_RATE = 0.001;
        static constexpr bool ENABLE_BIG_BANG_MODE = false;
        static constexpr double APOPTOSIS_INSTABILITY_THRESHOLD = 10.0;
        static constexpr double D1_PRIMER_THRESHOLD = 2.0;
        static constexpr double D2_APOPTOSIS_THRESHOLD = 5.0;
    };
};

} // namespace domain

