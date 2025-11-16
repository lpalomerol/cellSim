// Threshold.h
// Value Object para representar un threshold con límites y operaciones aritméticas/comparación
// No contiene lógica de negocio

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
    // Constructor con límites por defecto [0, 1]
    Threshold(double value, double lower_limit = 0.0, double upper_limit = 1.0)
        : value_(value), lower_limit_(lower_limit), upper_limit_(upper_limit) {
        if (lower_limit_ > upper_limit_) {
            throw std::invalid_argument("El límite inferior no puede ser mayor que el límite superior.");
        }
        clamp();
    }

    // Operadores aritméticos
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

    // Operadores de comparación
    bool operator<(const Threshold& other) const { return value_ < other.value_; }
    bool operator<=(const Threshold& other) const { return value_ <= other.value_; }
    bool operator>(const Threshold& other) const { return value_ > other.value_; }
    bool operator>=(const Threshold& other) const { return value_ >= other.value_; }
    bool operator==(const Threshold& other) const { return value_ == other.value_; }
    bool operator!=(const Threshold& other) const { return value_ != other.value_; }

    // Getter para el valor
    double value() const { return value_; }
    double lower_limit() const { return lower_limit_; }
    double upper_limit() const { return upper_limit_; }

private:
    // Asegura que el valor esté dentro de los límites
    void clamp() {
        value_ = std::clamp(value_, lower_limit_, upper_limit_);
    }
};

// Non-member multiplicative helpers to improve ergonomics
inline Threshold operator*(Threshold t, double factor) { t *= factor; return t; }
inline Threshold operator*(double factor, Threshold t) { t *= factor; return t; }

} // namespace shared
} // namespace domain
