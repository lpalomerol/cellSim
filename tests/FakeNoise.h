#pragma once
#include "../src/domain/ports/INoiseSource.h"
#include <vector>
#include <cstddef>

class FakeNoise : public domain::INoiseSource {
public:
    explicit FakeNoise(std::vector<domain::CellNoise> seq)
      : seq_(std::move(seq)) {}

    domain::CellNoise next() override {
        if (seq_.empty()) return domain::CellNoise{0.0};
        auto v = seq_[idx_ < seq_.size() ? idx_ : seq_.size()-1];
        ++idx_;
        return v;
    }
private:
    std::vector<domain::CellNoise> seq_;
    std::size_t idx_{0};
};