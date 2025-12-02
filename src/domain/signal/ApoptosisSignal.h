#pragma once

#include "ISignal.h"
#include <string>
#include <vector>
#include <cstdint>

namespace domain {

    class ApoptosisSignal : public ISignal {
    public:
        /**
         * Construct an ApoptosisSignal.
         * @param source_id The ID of the cell or tissue initiating apoptosis
         * @param message Optional message describing the apoptosis
         * @param target_ids Optional target cell IDs. If empty, broadcast to all cells.
         */
        ApoptosisSignal(std::uint64_t source_id,
                       const std::string& message = "",
                       const std::vector<std::uint64_t>& target_ids = {})
            : source_id_(source_id), message_(message), target_ids_(target_ids) {}

        Type type() const noexcept override {
            return Type::Apoptosis;
        }

        std::uint64_t sourceId() const noexcept override {
            return source_id_;
        }

        const std::string& message() const noexcept override {
            return message_;
        }

        const std::vector<std::uint64_t>& targetIds() const noexcept override {
            return target_ids_;
        }

    private:
        std::uint64_t source_id_;
        std::string message_;
        std::vector<std::uint64_t> target_ids_;
    };

} // namespace domain

