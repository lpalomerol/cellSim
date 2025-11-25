#pragma once

#include <string>
#include <cstdint>
#include <functional>
#include <memory>
#include "../../domain/cell/CellState.h"
#include "../cell/OncoState.h"

namespace domain {
    struct ISignal; // forward

    struct ICell {
        virtual ~ICell() = default;
        virtual void live() = 0;
        virtual bool alive() const = 0 ;
        // Indica si la célula presenta neoplasia (estado neoplásico)
        virtual bool isNeoplastic() const = 0;

        // Imprime detalles de la célula (por defecto no hace nada). Se marca const
        // porque no debería mutar el estado al mostrar información.
        virtual void details() const {}

        // Nueva: permitir que la simulación o tests soliciten una mutación sobre
        // un gen del genoma interno de la célula.
        virtual void mutateGene(const std::string& name) = 0;

        // Identificador estable de la célula. Por defecto no hace nada para mantener
        // compatibilidad con implementaciones existentes.
        virtual void setId(std::uint64_t /*id*/) {}
        virtual std::uint64_t id() const { return static_cast<std::uint64_t>(-1); }

        // Nuevo: inyectar un emisor de señales para que la célula pueda enviar
        // eventos al Tissue. Por defecto es un no-op para compatibilidad.
        virtual void setSignalEmitter(std::function<void(std::unique_ptr<ISignal>)> /*emitter*/) {}

    };
}
