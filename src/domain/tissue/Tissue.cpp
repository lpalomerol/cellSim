// filepath: /home/luis/CLionProjects/cellSim/src/domain/tissue/Tissue.cpp
// Implementation for Tissue collection (moved to domain/tissue)

#include "Tissue.h"
#include <iostream>
#include <sstream>
#include "GeneticTrackingService.h"
#include "../signal/CellDivisionSignal.h"

namespace domain {

    void Tissue::live() {
        phase0_Description();
        phase1_SignalIntegration();
        phase2_ExecuteCellCycles();
    }

    void Tissue::phase0_Description() const {
        if (verbose_) {
            auto tracking = getGeneticTracking();

            std::cout << "[Tissue Description] id=" << tissue_id_
                      << " | cells=" << cells_.size()
                      << " | identified_neoplasms=" << identified_neoplasms_.size() << "\n";

            // Print genetic summary similar to ExperimentalTracking
            const int boxWidth = 9;
            auto makeBox = [&](int count, int neos) {
                std::ostringstream ss;
                ss << count;
                if (neos >= 0) ss << "(" << neos << ")";
                return ss.str();
            };

            std::string b1 = makeBox(tracking.brca_het_tp53_hom_plus, tracking.neo_brca_het_tp53_hom_plus);
            std::string b2 = makeBox(tracking.brca_het_tp53_het, tracking.neo_brca_het_tp53_het);
            std::string b3 = makeBox(tracking.brca_het_tp53_hom_minus, tracking.neo_brca_het_tp53_hom_minus);
            std::string b4 = makeBox(tracking.brca_hom_minus, -1);

            std::cout << "  Resumen genético: |"
                      << std::setw(boxWidth) << b1 << " |"
                      << std::setw(boxWidth) << b2 << " |"
                      << std::setw(boxWidth) << b3 << " |"
                      << std::setw(boxWidth) << b4 << " |\n";

            std::cout << "    [BRCA+/- TP53+/+] [BRCA+/- TP53+/-] [BRCA+/- TP53-/-] [BRCA-/-]\n";
        }
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

            // Handle Neoplasm signals
            if (sig->type() == ISignal::Type::Neoplasm) {
                std::uint64_t source_id = sig->sourceId();
                std::string message = sig->message();

                std::cout << "[Tissue] Neoplasm signal detected from cell id=" << source_id << " message='" << message << "'\n";

                // Track the neoplasm
                identified_neoplasms_.insert(source_id);

                // Invoke listener callback if set
                if (neoplasm_listener_) {
                    neoplasm_listener_(source_id, message);
                }
            }
            // Handle CellDivision signals
            else if (sig->type() == ISignal::Type::CellDivision) {
                auto* division_sig = dynamic_cast<CellDivisionSignal*>(sig.get());
                if (division_sig) {
                    // Extract the daughter cell from the signal
                    auto daughter = division_sig->takeDaughterCell();
                    if (daughter) {
                        if (verbose_) {
                            std::cout << "[Tissue] Cell division signal detected from cell id="
                                      << sig->sourceId() << "; adding daughter cell\n";
                        }
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
