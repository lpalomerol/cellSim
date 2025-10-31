#pragma once
#include "../src/domain/ports/INoiseSource.h"
#include <vector>
#include <cstddef>

class FakeNoise : public domain::INoiseSource {
public:
    explicit FakeNoise(std::vector<domain::CellNoise> sequence)
        : seq_(std::move(sequence)) {}

    domain::CellNoise next() override {
        if (seq_.empty()) {
            return domain::CellNoise{0.0};
        }
        // Devuelve el valor siguiente o el último si ya pasamos el final
        const auto& val = seq_[std::min(idx_, seq_.size() - 1)];
        ++idx_;
        return val;
    }

private:
    std::vector<domain::CellNoise> seq_;
    std::size_t idx_ = 0;
};