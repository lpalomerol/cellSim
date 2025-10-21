#pragma once
#include  "CellNoise.h"

namespace domain {

    struct INoiseSource {
        virtual ~INoiseSource() = default;
        virtual CellNoise next() = 0;
    };


}




