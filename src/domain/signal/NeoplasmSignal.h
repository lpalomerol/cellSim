// filepath: /home/luis/CLionProjects/cellSim/src/domain/signal/NeoplasmSignal.h

#pragma once

#include "ISignal.h"
#include <string>

namespace domain {

    class NeoplasmSignal : public ISignal {
    public:
        explicit NeoplasmSignal(std::uint64_t source_id, std::string msg = "neoplasm")
            : src_(source_id), msg_(std::move(msg)) {}

        Type type() const noexcept override { return Type::Neoplasm; }
        std::uint64_t sourceId() const noexcept override { return src_; }
        const std::string& message() const noexcept override { return msg_; }

    private:
        std::uint64_t src_ = static_cast<std::uint64_t>(-1);
        std::string msg_;
    };

} // namespace domain

