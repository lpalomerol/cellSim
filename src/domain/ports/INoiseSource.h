#pragma once
#include  "CellNoise.h"
#include <cstdint>

namespace domain {

    struct INoiseSource {
        virtual ~INoiseSource() = default;
        virtual CellNoise next() = 0;
        // Exponer la semilla usada por la fuente de ruido para trazabilidad
        [[nodiscard]] virtual std::uint64_t getSeed() const = 0;
    };


}
