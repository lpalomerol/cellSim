#pragma once
#include "../../domain/ports/INoiseSource.h"
namespace adapters {

    class FixedNoise : public domain::INoiseSource {
    public:
        explicit FixedNoise(domain::CellNoise fixed) : fixed_(fixed), seed_(0) {}
        domain::CellNoise next() override { return fixed_; }
        [[nodiscard]] std::uint64_t getSeed() const override { return seed_; }
    private:
        domain::CellNoise fixed_;
        std::uint64_t seed_;
    };

}