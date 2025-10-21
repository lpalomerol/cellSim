#pragma once
#include <random>
#include "../../domain/ports/INoiseSource.h"

namespace adapters {

    class RandomNoise : public domain::INoiseSource {
    public:
        explicit RandomNoise(unsigned seed = 42)
            : rng_(seed), U01_(0.0, 1.0) {}

        domain::CellNoise next() override {
            return {U01_(rng_)};
        }

    private:
        std::mt19937 rng_;
        std::uniform_real_distribution<double> U01_;
    };

}
