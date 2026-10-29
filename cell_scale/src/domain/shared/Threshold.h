// Value Object: a clamped double with arithmetic and comparison operators.
#pragma once

#include <algorithm>
#include <stdexcept>

namespace domain {
namespace shared {

class Threshold {
private:
    double value_;
    double lower_limit_;
    double upper_limit_;

public:
    Threshold(double value, double lower_limit = 0.0, double upper_limit = 1.0)
        : value_(value), lower_limit_(lower_limit), upper_limit_(upper_limit) {
        if (lower_limit_ > upper_limit_) {
            throw std::invalid_argument("lower_limit cannot exceed upper_limit.");
        }
        clamp();
    }

    Threshold& operator+=(double delta) {
        value_ += delta;
        clamp();
        return *this;
    }
    Threshold& operator-=(double delta) {
        value_ -= delta;
        clamp();
        return *this;
    }
    // Multiplicative operator: scales the value and clamps
    Threshold& operator*=(double factor) {
        value_ *= factor;
        clamp();
        return *this;
    }

    bool operator<(const Threshold& other) const { return value_ < other.value_; }
    bool operator<=(const Threshold& other) const { return value_ <= other.value_; }
    bool operator>(const Threshold& other) const { return value_ > other.value_; }
    bool operator>=(const Threshold& other) const { return value_ >= other.value_; }
    bool operator==(const Threshold& other) const { return value_ == other.value_; }
    bool operator!=(const Threshold& other) const { return value_ != other.value_; }

    double value() const { return value_; }
    double lower_limit() const { return lower_limit_; }
    double upper_limit() const { return upper_limit_; }

private:
    void clamp() {
        value_ = std::clamp(value_, lower_limit_, upper_limit_);
    }
};

// Non-member multiplicative helpers to improve ergonomics
inline Threshold operator*(Threshold t, double factor) { t *= factor; return t; }
inline Threshold operator*(double factor, Threshold t) { t *= factor; return t; }

} // namespace shared
} // namespace domain
