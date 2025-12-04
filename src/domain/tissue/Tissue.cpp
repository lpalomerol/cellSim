// filepath: /home/luis/CLionProjects/cellSim/src/domain/tissue/Tissue.cpp
// Implementation for Tissue collection (moved to domain/tissue)

#include "Tissue.h"
#include <algorithm>
#include "GeneticTrackingService.h"
#include "../signal/CellDivisionSignal.h"
#include "../signal/ApoptosisSignal.h"
#include "../adapters/NullLogger.h"
#include "../../application/presenter/GeneticSummaryPresenter.h"

namespace domain {

    void Tissue::live() {
        phase0_Description();
        phase1_SignalIntegration();
        phase2_ExecuteCellCycles();
    }


    void Tissue::phase0_Description() const {
        auto tracking = getGeneticTracking();

        logger_->logTissue("[Tissue Description] id=" + std::to_string(tissue_id_)
                      + " | cells=" + std::to_string(cells_.size())
                      + " | identified_neoplasms=" + std::to_string(identified_neoplasms_.size())
                      + " | ACTIVE neoplasms=" + std::to_string(tracking.totalActiveNeoplasms()));

        // Usar presenter de application layer para formatear
        std::string summary = application::GeneticSummaryPresenter::formatGeneticSummary(tracking);
        logger_->logTissue(summary);

        logger_->logTissue("[Active Neoplasms] "
                      "TP53+/+: " + std::to_string(tracking.active_neo_brca_het_tp53_hom_plus) + " | "
                      "TP53+/-: " + std::to_string(tracking.active_neo_brca_het_tp53_het) + " | "
                      "TP53-/-: " + std::to_string(tracking.active_neo_brca_het_tp53_hom_minus) + " | "
                      "TOTAL: " + std::to_string(tracking.totalActiveNeoplasms()));
    }

    void Tissue::phase1_SignalIntegration() {
        // Currently placeholder for future signal processing logic
    }

    // Helper: find cell by ID using linear search
    ICell* Tissue::findCellById(std::uint64_t cell_id) {
        for (auto& cell_ptr : cells_) {
            if (cell_ptr && cell_ptr->id() == cell_id) {
                return cell_ptr.get();
            }
        }
        return nullptr;
    }

    // Helper: handle neoplasm signal from cell
    void Tissue::handleNeoplasmSignal(std::unique_ptr<ISignal> sig) {
        std::uint64_t source_id = sig->sourceId();
        std::string message = sig->message();

        logger_->logTissue("[Tissue] Neoplasm signal detected from cell id=" + std::to_string(source_id) + " message='" + message + "'");

        // Track the neoplasm
        identified_neoplasms_.insert(source_id);

        // Invoke listener callback if set
        if (neoplasm_listener_) {
            neoplasm_listener_(source_id, message);
        }

        // Send apoptosis signal back to the neoplastic cell
        std::vector<std::uint64_t> target_ids = {source_id};
        auto apoptosis_sig = std::make_unique<ApoptosisSignal>(
            tissue_id_,  // source: the tissue itself
            "apoptosis_response_to_neoplasm",
            target_ids   // directed to the neoplastic cell
        );

        if (ICell* target_cell = findCellById(source_id)) {
            target_cell->receiveMessage(std::move(apoptosis_sig));
            logger_->logTissue("[Tissue] Apoptosis signal sent to neoplastic cell id=" + std::to_string(source_id));
        }
    }

    // Helper: handle cell division signal from cell
    void Tissue::handleCellDivisionSignal(std::unique_ptr<ISignal> sig) {

        if (auto* division_sig = dynamic_cast<CellDivisionSignal*>(sig.get())) {
            if (auto daughter = division_sig->takeDaughterCell()) {
                logger_->logTissue("[Tissue] Cell division signal detected from cell id=" + std::to_string(sig->sourceId()) + "; adding daughter cell");
                addCell(std::move(daughter));
            }
        }
    }

    void Tissue::phase2_ExecuteCellCycles() {
        for (std::size_t i = 0; i < cells_.size(); ++i) {
            ICell* c = cells_[i].get();
            if (!c) continue;
            try {
                c->live();
            } catch (const std::exception& e) {
                logger_->logTissue("[Tissue] cell[" + std::to_string(i) + "] exception: " + std::string(e.what()));
            } catch (...) {
                logger_->logTissue("[Tissue] cell[" + std::to_string(i) + "] unknown exception");
            }
        }

        // Remove dead cells after all cells have executed their cycle
        cells_.erase(
            std::remove_if(cells_.begin(), cells_.end(),
                [this](const std::unique_ptr<ICell>& cell) {
                    if (!cell || !cell->alive()) {
                        if (cell) {
                            logger_->logTissue("[Tissue] Removing dead cell id=" + std::to_string(cell->id()));
                        }
                        return true;  // Mark for removal
                    }
                    return false;  // Keep the cell
                }),
            cells_.end()
        );
    }

    void Tissue::addCell(std::unique_ptr<ICell> cell) {
        if (!cell) return;

        // Assign a stable id to the cell before storing it
        std::uint64_t id = next_cell_id_.fetch_add(1, std::memory_order_relaxed);
        cell->setId(id);

        // Inject a signal emitter that processes signals via helpers
        cell->setSignalEmitter([this](std::unique_ptr<ISignal> sig){
            if (!sig) return;

            // Dispatch signal based on type
            // Note: Processed signals (Neoplasm, CellDivision) are consumed and not stored
            if (sig->type() == ISignal::Type::Neoplasm) {
                handleNeoplasmSignal(std::move(sig));
                // Signal was processed internally, don't collect it
            } else if (sig->type() == ISignal::Type::CellDivision) {
                handleCellDivisionSignal(std::move(sig));
                // Signal was processed internally, don't collect it
            } else {
                // For other signal types, store them in thread-safe queue
                std::lock_guard<std::mutex> lk(signals_mutex_);
                signals_new_.push_back(std::move(sig));
            }
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

    GeneticTrackingData Tissue::getGeneticTracking() const {
        // Use domain service to analyze genetic profiles - polymorphism replaces dynamic_cast
        return GeneticTrackingService::analyze(cells_);
    }

} // namespace domain
