#pragma once

#include <string>
#include <cstdint>
#include <functional>
#include <memory>

namespace domain {
    struct ISignal; // forward

    struct ICell {
        virtual ~ICell() = default;

        // === Lifecycle ===
        virtual void live() = 0;
        [[nodiscard]] virtual bool alive() const = 0;

        // === Genetic Profile (consolidated from IGeneticProfile) ===
        [[nodiscard]] virtual std::string getBRCA1Status() const = 0;
        [[nodiscard]] virtual std::string getTP53Status() const = 0;

        // === Neoplasia State ===
        [[nodiscard]] virtual bool isNeoplastic() const = 0;

        // === Information ===
        virtual void details() const {}

        // === Mutation ===
        virtual void mutateGene(const std::string& name) = 0;

        // === Identity ===
        virtual void setId(std::uint64_t /*id*/) {}
        [[nodiscard]] virtual std::uint64_t id() const { return static_cast<std::uint64_t>(-1); }

        // === Signaling ===
        virtual void setSignalEmitter(std::function<void(std::unique_ptr<ISignal>)> /*emitter*/) {}
        virtual void receiveMessage(std::unique_ptr<ISignal> /*signal*/) {}

    };
}
