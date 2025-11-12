#pragma once
#include "../src/domain/ports/INoiseSource.h"
#include <vector>
#include <cstddef>
#include <cstdint>

class FakeNoise : public domain::INoiseSource {
public:
    explicit FakeNoise(std::vector<domain::CellNoise> sequence, std::uint64_t seed = 0)
        : seq_(std::move(sequence)), seed_(seed) {}

    domain::CellNoise next() override {
        if (seq_.empty()) {
            return domain::CellNoise{0.0};
        }
        // Devuelve el valor siguiente o el último si ya pasamos el final
        const auto& val = seq_[std::min(idx_, seq_.size() - 1)];
        ++idx_;
        return val;
    }

    [[nodiscard]] std::uint64_t getSeed() const override { return seed_; }

private:
    std::vector<domain::CellNoise> seq_;
    std::size_t idx_ = 0;
    std::uint64_t seed_ = 0;
};