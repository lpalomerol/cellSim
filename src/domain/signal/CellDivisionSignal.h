#pragma once

#include "ISignal.h"
#include "../cell/AgenticCell.h"
#include <memory>
#include <string>
#include <vector>

namespace domain {

    class CellDivisionSignal : public ISignal {
    public:
        // Construct a CellDivisionSignal with the daughter cell
        // - source_id: ID of the parent cell
        // - daughter_cell: the newly created daughter cell
        // - msg: optional message (default "cell_division")
        // - targets: optional target IDs (empty = broadcast)
        explicit CellDivisionSignal(std::uint64_t source_id,
                                   std::unique_ptr<AgenticCell> daughter_cell,
                                   std::string msg = "cell_division",
                                   std::vector<std::uint64_t> targets = {})
            : src_(source_id), daughter_cell_(std::move(daughter_cell)), msg_(std::move(msg)), targets_(std::move(targets)) {}

        Type type() const noexcept override { return Type::CellDivision; }
        std::uint64_t sourceId() const noexcept override { return src_; }
        const std::string& message() const noexcept override { return msg_; }
        const std::vector<std::uint64_t>& targetIds() const noexcept override { return targets_; }

        // Access the daughter cell
        AgenticCell* getDaughterCell() const noexcept { return daughter_cell_.get(); }

        // Take ownership of the daughter cell
        std::unique_ptr<AgenticCell> takeDaughterCell() noexcept { return std::move(daughter_cell_); }

    private:
        std::uint64_t src_ = static_cast<std::uint64_t>(-1);
        std::unique_ptr<AgenticCell> daughter_cell_;
        std::string msg_;
        std::vector<std::uint64_t> targets_;
    };

} // namespace domain


