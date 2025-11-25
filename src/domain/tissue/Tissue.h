// filepath: /home/luis/CLionProjects/cellSim/src/domain/tissue/Tissue.h
//
// Tissue moved to its own domain subfolder (tissue)

#pragma once

#include <vector>
#include <memory>
#include <cstddef>
#include <atomic>
#include <mutex>
#include "../ports/ICell.h"
#include "../signal/ISignal.h"

namespace domain {

    // Tissue: simple collection of ICell instances that can be stepped as a group.
    // Behaves like an AgenticCellCollection: add cells, query count/access by index
    // and run a collective live() across all contained cells.
    class Tissue {
    public:
        Tissue() = default;

        // Run a lifecycle tick for every contained cell. Exceptions thrown by
        // individual cells are caught and logged (if desired) so other cells
        // continue to be processed.
        void live();

        // Add a cell to the tissue (takes ownership). Assigns a stable id to the cell.
        void addCell(std::unique_ptr<ICell> cell);

        // Number of cells currently stored
        std::size_t size() const;

        // Access cell by index. Returns nullptr if out-of-range.
        ICell* getCell(std::size_t idx);
        const ICell* getCell(std::size_t idx) const;

        // Clear all cells
        void clear();

        // Tissue id management
        void setId(std::uint64_t id) { tissue_id_ = id; }
        std::uint64_t id() const { return tissue_id_; }

        // Access signals emitted by cells during the last live() (thread-safe read)
        std::vector<std::unique_ptr<ISignal>> stealEmittedSignals();

    private:
        std::vector<std::unique_ptr<ICell>> cells_;
        std::atomic<std::uint64_t> next_cell_id_{0};
        std::uint64_t tissue_id_ = static_cast<std::uint64_t>(-1);

        // Signals emitted by cells during the current turn; protected by mutex
        std::vector<std::unique_ptr<ISignal>> signals_new_;
        std::mutex signals_mutex_;
    };

} // namespace domain
