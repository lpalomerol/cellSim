#pragma once

#include <vector>
#include <memory>
#include <cstddef>
#include <atomic>
#include <mutex>
#include <set>
#include <functional>
#include <string>
#include <iomanip>
#include "../ports/ICell.h"
#include "../ports/ILogger.h"
#include "../ports/ILoggeable.h"
#include "../adapters/NullLogger.h"
#include "../signal/ISignal.h"
#include "GeneticTrackingData.h"

namespace domain {

    // Tissue: simple collection of ICell instances that can be stepped as a group.
    // Behaves like an AgenticCellCollection: add cells, query count/access by index
    // and run a collective live() across all contained cells.
    //
    // ⚠️  DEPRECATED: Use TissueV2 instead (see PASO7 and ADR-0007 when created)
    // Migration path:
    // - Tests: Replace domain::Tissue with domain::TissueV2
    // - Application: Replace with domain::TissueV2 or use application::TissueV2Adapter
    class [[deprecated("Use TissueV2 instead")]] Tissue : public ports::ILoggeable {
    public:
        // Constructor: accepts optional logger (if nullptr, NullLogger will be used by default)
        explicit Tissue(ports::ILoggerPtr logger = nullptr)
            : logger_(logger ? logger : std::make_shared<adapters::NullLogger>()) {}

        // ILoggeable implementation
        [[nodiscard]] std::string getLogCategory() const override { return "TISSUE"; }

        // Run a lifecycle tick for every contained cell. Exceptions thrown by
        // individual cells are caught and logged (if desired) so other cells
        // continue to be processed.
        void live();

        // Add a cell to the tissue (takes ownership). Assigns a stable id to the cell.
        void addCell(std::unique_ptr<ICell> cell);

        // Number of cells currently stored
        [[nodiscard]] std::size_t size() const;

        // Access cell by index. Returns nullptr if out-of-range.
        ICell* getCell(std::size_t idx);
        [[nodiscard]] const ICell* getCell(std::size_t idx) const;

        // Clear all cells
        void clear();

        // Tissue id management
        void setId(std::uint64_t id) { tissue_id_ = id; }
        [[nodiscard]] std::uint64_t id() const { return tissue_id_; }

        // Access signals emitted by cells during the last live() (thread-safe read)
        [[nodiscard]] std::vector<std::unique_ptr<ISignal>> stealEmittedSignals();

        // Set a callback listener for neoplasm detection events
        void setNeoplasmListener(std::function<void(std::uint64_t, const std::string&)> listener) {
            neoplasm_listener_ = std::move(listener);
        }

        // Get the list of identified neoplasms (cell IDs)
        [[nodiscard]] std::set<std::uint64_t> getIdentifiedNeoplasms() const {
            return identified_neoplasms_;
        }

    private:
        std::vector<std::unique_ptr<ICell>> cells_;
        std::atomic<std::uint64_t> next_cell_id_{0};
        std::uint64_t tissue_id_ = static_cast<std::uint64_t>(-1);
        ports::ILoggerPtr logger_;

        // Signals emitted by cells during the current turn; protected by mutex
        std::vector<std::unique_ptr<ISignal>> signals_new_;
        std::mutex signals_mutex_;

        // Neoplasm tracking
        std::set<std::uint64_t> identified_neoplasms_;
        std::function<void(std::uint64_t, const std::string&)> neoplasm_listener_;

        // Helper method to collect genetic tracking data using domain service
        [[nodiscard]] GeneticTrackingData getGeneticTracking() const;

        // Phase methods for live() lifecycle
        void phase0_Description() const;
        void phase1_SignalIntegration();
        void phase2_ExecuteCellCycles();

    private:
        // Helper: find cell by ID (linear search)
        [[nodiscard]] ICell* findCellById(std::uint64_t cell_id);


        // Helper: handle neoplasm signal from cell
        void handleNeoplasmSignal(std::unique_ptr<ISignal> sig);

        // Helper: handle cell division signal from cell
        void handleCellDivisionSignal(std::unique_ptr<ISignal> sig);
    };

} // namespace domain
