#include "CellFactory.h"
#include "../adapters/RandomNoise.h"

namespace domain {

std::unique_ptr<AgenticCell> CellFactory::createNormalCell(
    const Genome& genome,
    const ports::ILoggerPtr& logger) {

    auto noise = std::make_unique<adapters::RandomNoise>();

    return createCustomCell(
        std::move(noise),
        genome,
        Defaults::LOW_DELTA_INSTABILITY,
        Defaults::HIGH_DELTA_INSTABILITY,
        Defaults::DIVISION_RATE,
        Defaults::NEOPLASTIC_DIVISION_RATE,
        Defaults::ENABLE_BIG_BANG_MODE,
        Defaults::D1_PRIMER_THRESHOLD,
        Defaults::D2_APOPTOSIS_THRESHOLD,
        logger
    );
}

std::unique_ptr<AgenticCell> CellFactory::createCustomCell(
    std::unique_ptr<INoiseSource> noise,
    const Genome& genome,
    double low_delta_instability,
    double high_delta_instability,
    double division_rate,
    double neoplastic_division_rate,
    bool enable_big_bang_mode,
    double d1_primer_threshold,
    double d2_apoptosis_threshold,
    const ports::ILoggerPtr& logger,
    double delta_noise_cv) {

    InstabilityConfig instability{low_delta_instability, high_delta_instability,
                                  999.0, 999.0, delta_noise_cv};
    DivisionConfig division{division_rate, neoplastic_division_rate, enable_big_bang_mode};
    ThresholdConfig thresholds{d1_primer_threshold, d2_apoptosis_threshold};

    return std::make_unique<AgenticCell>(
        std::move(noise),
        genome,
        instability,
        division,
        thresholds,
        logger
    );
}

} // namespace domain

