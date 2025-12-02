#pragma once

#include "BaseSignal.h"
#include <string>
#include <vector>
#include <cstdint>

namespace domain {

    class ApoptosisSignal : public BaseSignal {
    public:
        /**
         * Construct an ApoptosisSignal.
         * @param source_id ID del origen (célula o tejido)
         * @param message Mensaje descriptivo (default vacío)
         * @param target_ids IDs de destino. Si vacío, broadcast.
         */
        explicit ApoptosisSignal(std::uint64_t source_id,
                                std::string message = "",
                                std::vector<std::uint64_t> target_ids = {})
            : BaseSignal(source_id, std::move(message), std::move(target_ids)) {}

        Type type() const noexcept override { return Type::Apoptosis; }
    };

} // namespace domain

