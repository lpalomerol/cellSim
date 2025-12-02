#pragma once
#include <random>
#include "../ports/INoiseSource.h"

namespace domain::adapters {

    class RandomNoise : public INoiseSource {
    public:
        explicit RandomNoise(unsigned seed = 42)
            : rng_(seed), U01_(0.0, 1.0), seed_(seed) {}

        CellNoise next() override {
            return {U01_(rng_)};
        }

        [[nodiscard]] std::uint64_t getSeed() const override { return seed_; }

    private:
        std::mt19937 rng_;
        std::uniform_real_distribution<double> U01_;
        std::uint64_t seed_;
    };

}
