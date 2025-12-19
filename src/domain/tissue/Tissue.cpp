#include "Tissue.h"
#include "../cell/AgenticCell.h"
#include "../exception/CellDeathException.h"

namespace domain {

void Tissue::live() {
    std::vector<std::size_t> dead_indices;
    std::vector<std::unique_ptr<ICell>> new_daughters;

    for (std::size_t i = 0; i < cells_.size(); ++i) {
        if (!cells_[i]) continue;

        try {
            cells_[i]->live();

            // Check if cell divided (only AgenticCell supports this)
            auto* agentic_cell = dynamic_cast<AgenticCell*>(cells_[i].get());
            if (agentic_cell) {
                auto daughter = agentic_cell->takePendingDaughter();
                if (daughter) {
                    logger_->logCell("[TissueV2] Cell " + std::to_string(agentic_cell->id()) + " divided, daughter will be added");
                    new_daughters.push_back(std::move(daughter));
                }
            }
        } catch (const CellDeathException& e) {
            logger_->logCell("[TissueV2] Cell died: " + std::string(e.what()));
            dead_indices.push_back(i);
        } catch (const std::exception& e) {
            logger_->logCell("[TissueV2] Cell exception: " + std::string(e.what()));
            dead_indices.push_back(i);
        }
    }

    // Remove dead cells (from back to front to preserve indices)
    for (auto it = dead_indices.rbegin(); it != dead_indices.rend(); ++it) {
        cells_.erase(cells_.begin() + *it);
    }

    // Add new daughter cells
    for (auto& daughter : new_daughters) {
        addCell(std::move(daughter));
    }
}

void Tissue::addCell(std::unique_ptr<ICell> cell) {
    if (!cell) {
        logger_->logCell("[TissueV2] Attempted to add nullptr cell");
        return;
    }

    std::uint64_t cell_id = next_cell_id_.fetch_add(1, std::memory_order_relaxed);
    cell->setId(cell_id);

    logger_->logCell("[TissueV2] Added cell with id=" + std::to_string(cell_id));
    cells_.push_back(std::move(cell));
}

std::size_t Tissue::size() const {
    return cells_.size();
}

ICell* Tissue::getCell(std::size_t idx) {
    if (idx >= cells_.size()) return nullptr;
    return cells_[idx].get();
}

const ICell* Tissue::getCell(std::size_t idx) const {
    if (idx >= cells_.size()) return nullptr;
    return cells_[idx].get();
}

void Tissue::clear() {
    cells_.clear();
    logger_->logCell("[TissueV2] Cleared all cells");
}

void Tissue::setId(std::uint64_t id) {
    tissue_id_ = id;
}

std::uint64_t Tissue::id() const {
    return tissue_id_;
}

std::vector<ICell*> Tissue::getLiveCells() {
    std::vector<ICell*> live;
    for (auto& cell : cells_) {
        if (cell) {
            live.push_back(cell.get());
        }
    }
    return live;
}

std::vector<ICell*> Tissue::getCellsByStage(CellLifeStage stage) {
    std::vector<ICell*> result;
    for (auto& cell : cells_) {
        if (!cell) continue;

        // Try to cast to AgenticCell to access getCellLifeStage()
        auto* agg_cell = dynamic_cast<AgenticCell*>(cell.get());
        if (agg_cell && agg_cell->getCurrentCellLifeStage() == stage) {
            result.push_back(cell.get());
        }
    }
    return result;
}

} // namespace domain

