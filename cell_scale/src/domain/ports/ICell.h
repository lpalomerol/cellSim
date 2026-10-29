#pragma once

#include <string>
#include <cstdint>
#include <functional>
#include <memory>
#include "../shared/CellLifeStage.h"

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

        // === Damage Accumulators ===
        [[nodiscard]] virtual double getD1() const = 0;
        [[nodiscard]] virtual double getD2() const = 0;

        // Effective values allow paper interception analyses to attenuate the
        // consequences of accumulated D1/D2 without resetting their raw state.
        [[nodiscard]] virtual double getEffectiveD1() const { return getD1(); }
        [[nodiscard]] virtual double getEffectiveD2() const { return getD2(); }
        virtual void setAttenuation(double /*r_d1*/, double /*r_d2*/) {}

        // 0 means that no independent Primed->Tumoral founder has been assigned.
        [[nodiscard]] virtual std::uint64_t getFounderLineageId() const { return 0; }

        // === Cell Stage ===
        [[nodiscard]] virtual CellLifeStage getCurrentCellLifeStage() const = 0;

        // === Genomic queries ===
        [[nodiscard]] virtual bool hasBRCA1Mutation() const = 0;
        [[nodiscard]] virtual bool hasTP53Function() const = 0;

        // === Age ===
        [[nodiscard]] virtual std::uint64_t getAge() const = 0;

        // === Apoptosis Evasion ===
        [[nodiscard]] virtual bool hasEvadedApoptosis() const = 0;

        // === Division ===
        /// Returns owned daughter cell after a division tick, or nullptr if no division occurred.
        virtual std::unique_ptr<ICell> takePendingDaughter() = 0;

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
