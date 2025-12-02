// filepath: /home/luis/CLionProjects/cellSim/src/domain/tissue/Tissue.cpp
// Implementation for Tissue collection (moved to domain/tissue)

#include "Tissue.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include "GeneticTrackingService.h"
#include "../signal/CellDivisionSignal.h"
#include "../signal/ApoptosisSignal.h"
#include "../adapters/NullLogger.h"

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

        // Print genetic summary similar to ExperimentalTracking
        const int boxWidth = 12;

        // Lambda to create boxes with format: "total(identified/active)"
        auto makeBox = [&](int count, int neos, int active_neos) {
            std::ostringstream ss;
            ss << count;
            if (neos > 0 || active_neos > 0) {
                ss << "(" << neos;
                if (active_neos > 0) ss << "/" << active_neos;
                ss << ")";
            }
            return ss.str();
        };

        std::string b1 = makeBox(tracking.brca_het_tp53_hom_plus, tracking.neo_brca_het_tp53_hom_plus, tracking.active_neo_brca_het_tp53_hom_plus);
        std::string b2 = makeBox(tracking.brca_het_tp53_het, tracking.neo_brca_het_tp53_het, tracking.active_neo_brca_het_tp53_het);
        std::string b3 = makeBox(tracking.brca_het_tp53_hom_minus, tracking.neo_brca_het_tp53_hom_minus, tracking.active_neo_brca_het_tp53_hom_minus);
        std::string b4 = makeBox(tracking.brca_hom_minus, -1, -1);

        std::ostringstream summary;
        summary << "  Resumen genético [total(neo/activo)]: |"
                << std::setw(boxWidth) << b1 << " |"
                << std::setw(boxWidth) << b2 << " |"
                << std::setw(boxWidth) << b3 << " |"
                << std::setw(boxWidth) << b4 << " |\n"
                << "    [BRCA+/- TP53+/+] [BRCA+/- TP53+/-] [BRCA+/- TP53-/-] [BRCA-/-]";
        logger_->logTissue(summary.str());

        // Additional detailed summary
        logger_->logTissue("[Active Neoplasms] "
                      "TP53+/+: " + std::to_string(tracking.active_neo_brca_het_tp53_hom_plus) + " | "
                      "TP53+/-: " + std::to_string(tracking.active_neo_brca_het_tp53_het) + " | "
                      "TP53-/-: " + std::to_string(tracking.active_neo_brca_het_tp53_hom_minus) + " | "
                      "TOTAL: " + std::to_string(tracking.totalActiveNeoplasms()));
    }

    void Tissue::phase1_SignalIntegration() {
        // Currently placeholder for future signal processing logic
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

        // Inject a signal emitter that enqueues signals into signals_new_
        cell->setSignalEmitter([this](std::unique_ptr<ISignal> sig){
            if (!sig) return;

            // Handle Neoplasm signals
            if (sig->type() == ISignal::Type::Neoplasm) {
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

                // Find the cell and deliver the signal
                for (auto& cell_ptr : cells_) {
                    if (cell_ptr && cell_ptr->id() == source_id) {
                        cell_ptr->receiveMessage(std::move(apoptosis_sig));
                        logger_->logTissue("[Tissue] Apoptosis signal sent to neoplastic cell id=" + std::to_string(source_id));
                        break;
                    }
                }
            }
            // Handle CellDivision signals
            else if (sig->type() == ISignal::Type::CellDivision) {
                auto* division_sig = dynamic_cast<CellDivisionSignal*>(sig.get());
                if (division_sig) {
                    // Extract the daughter cell from the signal
                    auto daughter = division_sig->takeDaughterCell();
                    if (daughter) {
                        logger_->logTissue("[Tissue] Cell division signal detected from cell id=" + std::to_string(sig->sourceId()) + "; adding daughter cell");
                        // Add the daughter cell to the tissue (assigns a new ID)
                        addCell(std::move(daughter));
                    }
                }
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

    GeneticTrackingData Tissue::getGeneticTracking() const {
        // Use domain service to analyze genetic profiles - polymorphism replaces dynamic_cast
        return GeneticTrackingService::analyze(cells_);
    }

} // namespace domain
