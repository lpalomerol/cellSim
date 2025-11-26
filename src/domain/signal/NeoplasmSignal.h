// filepath: /home/luis/CLionProjects/cellSim/src/domain/signal/NeoplasmSignal.h

#pragma once

#include "ISignal.h"
#include <string>
#include <vector>

namespace domain {

    class NeoplasmSignal : public ISignal {
    public:
        // Construct a NeoplasmSignal with optional target IDs
        // - Empty targets: broadcast to all
        // - Non-empty targets: directed to specific cells
        explicit NeoplasmSignal(std::uint64_t source_id,
                               std::string msg = "neoplasm",
                               std::vector<std::uint64_t> targets = {})
            : src_(source_id), msg_(std::move(msg)), targets_(std::move(targets)) {}

        Type type() const noexcept override { return Type::Neoplasm; }
        std::uint64_t sourceId() const noexcept override { return src_; }
        const std::string& message() const noexcept override { return msg_; }
        const std::vector<std::uint64_t>& targetIds() const noexcept override { return targets_; }

    private:
        std::uint64_t src_ = static_cast<std::uint64_t>(-1);
        std::string msg_;
        std::vector<std::uint64_t> targets_;
    };

} // namespace domain

