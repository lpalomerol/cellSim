#pragma once

#include "ISignal.h"
#include <string>
#include <vector>
#include <any>
#include <memory>

namespace domain {

    /**
     * BaseSignal: clase base que encapsula la lógica común de todas las signals.
     * Soporta un payload genérico (std::any) para extensibilidad.
     */
    class BaseSignal : public ISignal {
    public:
        /**
         * Constructor base para todas las signals
         * @param source_id ID del origen (célula o tejido)
         * @param message Mensaje descriptivo (default vacío)
         * @param targets IDs de destino (vacío = broadcast)
         * @param payload Payload genérico (default vacío)
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

        // Acceso al payload genérico
        [[nodiscard]] const std::any& payload() const noexcept { return payload_; }
        [[nodiscard]] std::any& payload() noexcept { return payload_; }

        // Helper: obtener payload tipado (con verificación)
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

