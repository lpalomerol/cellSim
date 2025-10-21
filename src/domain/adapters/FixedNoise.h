#pragma once
#include "../../domain/ports/INoiseSource.h"
namespace adapters {

    class FixedNoise : public domain::INoiseSource {
    public:
        explicit FixedNoise(domain::CellNoise fixed) : fixed_(fixed) {}
        domain::CellNoise next() override { return fixed_; }
    private:
        domain::CellNoise fixed_;
    };

}