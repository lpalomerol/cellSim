#pragma once

#include <vector>
#include <memory>
#include <cstddef>
#include <atomic>
#include <set>
#include <functional>
#include <string>
#include <iomanip>
#include "../ports/ICell.h"
#include "../ports/ILogger.h"
#include "../ports/ILoggeable.h"
#include "../adapters/NullLogger.h"
#include "../signal/ISignal.h"
#include "../cell/CellLifeStage.h"
#include "GeneticTrackingData.h"

namespace domain {

    /// TissueV2: Collection of ICell instances using AgenticCell (with D1/D2)
    /// Compatible with Tissue interface, uses CellFactory for cell creation
    class Tissue : public ports::ILoggeable {
    public:
        // Constructor: accepts optional logger
        explicit Tissue(ports::ILoggerPtr logger = nullptr)
            : logger_(logger ? logger : std::make_shared<adapters::NullLogger>()) {}

        // ILoggeable implementation
        [[nodiscard]] std::string getLogCategory() const override { return "TISSUE"; }

        // Run a lifecycle tick for every contained cell
        void live();

        // Stats from last live() call
        [[nodiscard]] int lastDeathCount() const { return last_death_count_; }
        [[nodiscard]] int lastBirthCount() const { return last_birth_count_; }

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
        void setId(std::uint64_t id);
        [[nodiscard]] std::uint64_t id() const;

        // Get all live cells
        [[nodiscard]] std::vector<ICell*> getLiveCells();

        // Get all cells with specific CellLifeStage (requires dynamic_cast to AgenticCell)
        [[nodiscard]] std::vector<ICell*> getCellsByStage(CellLifeStage stage);

        // Returns true once tumoral cells >= non-tumoral alive cells (set during live())
        [[nodiscard]] bool isSaturated() const { return saturated_; }

    private:
        ports::ILoggerPtr logger_;
        std::vector<std::unique_ptr<ICell>> cells_;
        std::uint64_t tissue_id_ = 0;
        std::atomic<std::uint64_t> next_cell_id_{1};
        int last_death_count_ = 0;
        int last_birth_count_ = 0;
        bool saturated_ = false;
    };

} // namespace domain

