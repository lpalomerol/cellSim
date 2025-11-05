#pragma once

#include "../src/domain/cell/AgenticCell.h"

// Pequeñas implementaciones de fuentes de ruido para las pruebas.
// Se dejan en el namespace global llamado `test` para evitar polución del
// namespace `domain` y permitir referenciarlas como `test::DummyNoise`.

namespace test {

class DummyNoise : public domain::INoiseSource {
public:
    domain::CellNoise next() override { return domain::CellNoise{1.0}; }
};

class HighNoise : public domain::INoiseSource {
public:
    domain::CellNoise next() override { return domain::CellNoise{0.0}; }
};

} // namespace test

