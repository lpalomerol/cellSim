#pragma once
#include "../ports/INoiseSource.h"

namespace domain::adapters {

    class FixedNoise : public INoiseSource {
    public:
        explicit FixedNoise(CellNoise fixed) : fixed_(fixed), seed_(0) {}
        CellNoise next() override { return fixed_; }
        [[nodiscard]] std::uint64_t getSeed() const override { return seed_; }
    private:
        CellNoise fixed_;
        std::uint64_t seed_;
    };

}