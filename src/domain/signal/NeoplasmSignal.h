// filepath: /home/luis/CLionProjects/cellSim/src/domain/signal/NeoplasmSignal.h

#pragma once

#include "BaseSignal.h"
#include <string>
#include <vector>

namespace domain {

    class NeoplasmSignal : public BaseSignal {
    public:
        /**
         * Construct a NeoplasmSignal.
         * @param source_id ID del origen
         * @param message Mensaje (default "neoplasm")
         * @param targets IDs de destino (vacío = broadcast)
         */
        explicit NeoplasmSignal(std::uint64_t source_id,
                               std::string message = "neoplasm",
                               std::vector<std::uint64_t> targets = {})
            : BaseSignal(source_id, std::move(message), std::move(targets)) {}

        Type type() const noexcept override { return Type::Neoplasm; }
    };

} // namespace domain

