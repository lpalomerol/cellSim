// filepath: /home/luis/CLionProjects/cellSim/src/domain/tissue/Tissue.cpp
// Implementation for Tissue collection (moved to domain/tissue)

#include "Tissue.h"
#include <iostream>

namespace domain {

    void Tissue::live() {
        for (std::size_t i = 0; i < cells_.size(); ++i) {
            ICell* c = cells_[i].get();
            if (!c) continue;
            try {
                c->live();
            } catch (const std::exception& e) {
                std::cerr << "[Tissue] cell[" << i << "] exception: " << e.what() << "\n";
            } catch (...) {
                std::cerr << "[Tissue] cell[" << i << "] unknown exception\n";
            }
        }
    }

    void Tissue::addCell(std::unique_ptr<ICell> cell) {
        if (!cell) return;
        // Assign a stable id to the cell before storing it
        std::uint64_t id = next_cell_id_.fetch_add(1, std::memory_order_relaxed);
        cell->setId(id);

        // Inject a signal emitter that enqueues signals into signals_new_
        cell->setSignalEmitter([this](std::unique_ptr<ISignal> sig){
            if (!sig) return;
            // If it's a neoplasm signal, print a message immediately
            if (sig->type() == ISignal::Type::Neoplasm) {
                std::cout << "[Tissue] Neoplasm signal detected from cell id=" << sig->sourceId() << " message='" << sig->message() << "'\n";
            }
            std::lock_guard<std::mutex> lk(signals_mutex_);
            signals_new_.push_back(std::move(sig));
        });

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
        std::lock_guard<std::mutex> lk(signals_mutex_);
        signals_new_.clear();
    }

    std::vector<std::unique_ptr<ISignal>> Tissue::stealEmittedSignals() {
        std::lock_guard<std::mutex> lk(signals_mutex_);
        std::vector<std::unique_ptr<ISignal>> out;
        out.swap(signals_new_);
        return out;
    }

} // namespace domain
