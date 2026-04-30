#pragma once

#include "ISignal.h"
#include <string>
#include <vector>
#include <any>
#include <memory>

namespace domain {

    /**
     * BaseSignal: base class encapsulating common signal logic.
     * Supports a generic payload (std::any) for extensibility.
     */
    class BaseSignal : public ISignal {
    public:
        /**
         * @param source_id  ID of the originating cell or tissue
         * @param message    Descriptive message (default empty)
         * @param targets    Target IDs (empty = broadcast)
         * @param payload    Generic payload (default empty)
         */
        BaseSignal(std::uint64_t source_id,
                  std::string message = "",
                  std::vector<std::uint64_t> targets = {},
                  std::any payload = std::any())
            : source_id_(source_id), message_(std::move(message)),
              targets_(std::move(targets)), payload_(payload) {}

        // ISignal interface
        std::uint64_t sourceId() const noexcept override { return source_id_; }
        const std::string& message() const noexcept override { return message_; }
        const std::vector<std::uint64_t>& targetIds() const noexcept override { return targets_; }

        [[nodiscard]] const std::any& payload() const noexcept { return payload_; }
        [[nodiscard]] std::any& payload() noexcept { return payload_; }

        // Typed payload access (safe cast — returns nullptr on type mismatch)
        template <typename T>
        [[nodiscard]] T* getPayloadAs() noexcept {
            try {
                return std::any_cast<T*>(payload_);
            } catch (const std::bad_any_cast&) {
                return nullptr;
            }
        }

    protected:
        std::uint64_t source_id_ = static_cast<std::uint64_t>(-1);
        std::string message_;
        std::vector<std::uint64_t> targets_;
        std::any payload_;
    };

} // namespace domain

