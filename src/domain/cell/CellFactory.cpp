#include "CellFactory.h"
#include "../adapters/RandomNoise.h"

namespace domain {

std::unique_ptr<AgenticCell> CellFactory::createNormalCell(
    const Genome& genome,
    const ports::ILoggerPtr& logger) {

    return createCustomCell(
        genome,
        Defaults::NEOPLASM_K,
        Defaults::LOW_DELTA_INSTABILITY,
        Defaults::HIGH_DELTA_INSTABILITY,
        Defaults::DIVISION_RATE,
        Defaults::NEOPLASTIC_DIVISION_RATE,
        Defaults::ENABLE_BIG_BANG_MODE,
        Defaults::APOPTOSIS_INSTABILITY_THRESHOLD,
        Defaults::D1_PRIMER_THRESHOLD,
        Defaults::D2_APOPTOSIS_THRESHOLD,
        logger
    );
}

std::unique_ptr<AgenticCell> CellFactory::createCustomCell(
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
    const ports::ILoggerPtr& logger) {

    auto noise = std::make_unique<adapters::RandomNoise>();

    return std::make_unique<AgenticCell>(
        std::move(noise),
        genome,
        neoplasm_k,
        low_delta_instability,
        high_delta_instability,
        division_rate,
        neoplastic_division_rate,
        enable_big_bang_mode,
        apoptosis_instability_threshold,
        logger,
        d1_primer_threshold,
        d2_apoptosis_threshold
    );
}

} // namespace domain

