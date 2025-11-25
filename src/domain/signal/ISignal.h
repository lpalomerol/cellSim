// filepath: /home/luis/CLionProjects/cellSim/src/domain/signal/ISignal.h
// Interface for signals used between cells and tissue

#pragma once

#include <cstdint>
#include <string>

namespace domain {

    struct ISignal {
        enum class Type {
            Neoplasm,
            Apoptosis
        };

        virtual ~ISignal() = default;

        // Type of the signal
        virtual Type type() const noexcept = 0;

        // Origin identifier (id of the cell or tissue that produced the signal).
        // Use a sentinel value (e.g., -1 cast) if not applicable.
        virtual std::uint64_t sourceId() const noexcept = 0;

        // Optional textual message
        virtual const std::string& message() const noexcept = 0;
    };

} // namespace domain

