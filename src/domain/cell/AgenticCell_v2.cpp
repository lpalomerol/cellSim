#include "AgenticCell_v2.h"
#include "../signal/NeoplasmSignal.h"
#include "../signal/CellDivisionSignal.h"
#include "../signal/ApoptosisSignal.h"
#include "../exception/CellDeathException.h"
#include "../exception/NeoplasticException.h"
#include "../shared/Threshold.h"
#include "../adapters/RandomNoise.h"
#include "../adapters/NullLogger.h"
#include "InstabilityDeltas.h"

namespace domain {

    /// Constructor: Initialize AgenticCell_v2 with D1 and D2 counters
    AgenticCell_v2::AgenticCell_v2(std::unique_ptr<INoiseSource> noise,
                                   Genome genome,
                                   double neoplasm_k,
                                   double low_delta_instability,
                                   double high_delta_instability,
                                   double division_rate,
                                   double neoplastic_division_rate,
                                   bool enable_big_bang_mode,
                                   double apoptosis_instability_threshold,
                                   ports::ILoggerPtr logger)
        : noise_(std::move(noise)),
          logger_(logger ? logger : std::make_shared<adapters::NullLogger>()),
          genome_(std::move(genome)),
          base_neoplasm_k_(neoplasm_k),
          neoplasm_k_(domain::shared::Threshold(neoplasm_k)),
          is_neoplastic_(false),
          division_rate_(division_rate),
          neoplastic_division_rate_(neoplastic_division_rate),
          enable_big_bang_mode_(enable_big_bang_mode),
          apoptosis_instability_threshold_(apoptosis_instability_threshold),
          low_delta_instability_(low_delta_instability),
          high_delta_instability_(high_delta_instability),
          d1_dna_damage_(1.0),
          d2_immunosuppression_(1.0) {

        // Inject noise source into all genes
        genome_.setNoiseSourceForAll(noise_.get());

        // Capture RNG seed for traceability
        if (noise_) {
            seed_ = noise_->getSeed();
        } else {
            seed_ = 0;
        }

        logger_->logCell("[Trace] AgenticCell_v2 constructed: seed=" + std::to_string(seed_)
                      + " | d1=" + std::to_string(d1_dna_damage_)
                      + " | d2=" + std::to_string(d2_immunosuppression_));
    }

    // ===== Getters: D1 and D2 =====

    // Already declared inline in header:
    // double getD1() const { return d1_dna_damage_; }
    // double getD2() const { return d2_immunosuppression_; }

    // ===== Simple Status Queries =====

    bool AgenticCell_v2::alive() const {
        const Gene* brca1 = genome_.getGene("BRCA1");

        // Normal cells: BRCA1 must be enabled (+/+ or +/-)
        if (!has_evaded_apoptosis_) {
            if (!brca1) return false;
            std::string brca1_status = brca1->status();
            return brca1_status != "-/-";
        }

        // Immortal cells (evaded apoptosis): always alive
        return true;
    }

    bool AgenticCell_v2::isNeoplastic() const {
        return is_neoplastic_;
    }

    bool AgenticCell_v2::isNeoplasticProtected() const {
        const Gene* tp53 = genome_.getGene("TP53");
        if (!tp53) return false;

        // TP53 +/+ and +/- protect from neoplasm
        // Only TP53 -/- allows tumors
        std::string status = tp53->status();
        return status != "-/-";
    }

    std::string AgenticCell_v2::getTP53() const {
        const Gene* tp53 = genome_.getGene("TP53");
        return tp53 ? tp53->status() : "?";
    }

    std::string AgenticCell_v2::getBRCA1() const {
        const Gene* brca1 = genome_.getGene("BRCA1");
        return brca1 ? brca1->status() : "?";
    }

    // ===== ID Management =====

    void AgenticCell_v2::setId(std::uint64_t id) {
        cell_id_ = id;
    }

    std::uint64_t AgenticCell_v2::id() const {
        return cell_id_;
    }

    // ===== Details and Logging =====

    void AgenticCell_v2::details() const {
        std::string cell_is_alive = (alive() ? "yes" : "no");
        std::string cell_is_neoplastic = (isNeoplastic() ? "yes" : "no");
        std::string cell_is_protected = (isNeoplasticProtected() ? "yes" : "no");

        logger_->logCell("[Cell V2 Details] "
                      "Alive: [" + cell_is_alive + "] | "
                      "Neoplastic Protected: [" + cell_is_protected + "] | "
                      "Neoplastic: [" + cell_is_neoplastic + "] | "
                      "Seed: [" + std::to_string(seed_) + "] | "
                      "Age: [" + std::to_string(age_) + "] | "
                      "d1: [" + std::to_string(d1_dna_damage_) + "] | "
                      "d2: [" + std::to_string(d2_immunosuppression_) + "]");
        logger_->logCell("Genome details:");
        genome_.details();
    }

    // ===== Signal Handling =====

    void AgenticCell_v2::setSignalEmitter(std::function<void(std::unique_ptr<domain::ISignal>)> emitter) {
        signal_emitter_ = std::move(emitter);
    }

    void AgenticCell_v2::receiveMessage(std::unique_ptr<domain::ISignal> signal) {
        if (!signal) return;

        const auto& targets = signal->targetIds();
        bool should_accept = targets.empty();  // broadcast

        if (!should_accept) {
            for (auto target_id : targets) {
                if (target_id == cell_id_) {
                    should_accept = true;
                    break;
                }
            }
        }

        if (should_accept) {
            incoming_messages_.push(std::move(signal));
            logger_->logCell("[Trace] Cell [" + std::to_string(cell_id_) + "] received message");
        } else {
            logger_->logCell("[Trace] Cell [" + std::to_string(cell_id_) + "] ignored message (not in targets)");
        }
    }

    // ===== Gene Mutation =====

    void AgenticCell_v2::mutateGene(const std::string& name) {
        genome_.mutate(name);
    }

    // ===== Placeholder: Methods to be implemented in later sub-steps =====

    void AgenticCell_v2::live() {
        // TODO: Implement full 6-phase lifecycle in sub-step 3.5
        logger_->logCell("[Trace] AgenticCell_v2::live() - NOT YET IMPLEMENTED");
    }

    CellLifeStage AgenticCell_v2::getCurrentCellLifeStage() const {
        // TODO: Implement in sub-step 3.2
        return CellLifeStage::BASELINE;
    }

    void AgenticCell_v2::phase0_BaselineAssessment() const {
        // TODO: Implement in sub-step 3.5
    }

    void AgenticCell_v2::phase1_G1IntegrityCheckpoint() const {
        // TODO: Implement in sub-step 3.5
    }

    void AgenticCell_v2::phase2_Endocytosis() {
        // TODO: Implement in sub-step 3.3
    }

    void AgenticCell_v2::phase3_NuclearDynamics() {
        // TODO: Implement in sub-step 3.5
    }

    void AgenticCell_v2::phase4_CytoplasmicRemodeling() {
        // TODO: Implement in sub-step 3.4
    }

    void AgenticCell_v2::phase5_Exocytosis() {
        // TODO: Implement in sub-step 3.5
    }

    void AgenticCell_v2::updateInstability() {
        // TODO: Implement in sub-step 3.4
    }

    void AgenticCell_v2::develop_neoplasm() {
        // TODO: Implement in sub-step 3.5
    }

    void AgenticCell_v2::increaseAge() {
        // TODO: Implement in sub-step 3.5
    }

    void AgenticCell_v2::adjust_neoplasm_k() {
        // TODO: Implement in sub-step 3.5
    }

    void AgenticCell_v2::attemptDivision() {
        // TODO: Implement in sub-step 3.5
    }

    void AgenticCell_v2::attemptApoptosis() {
        // TODO: Implement in sub-step 3.3
    }

    std::unique_ptr<AgenticCell_v2> AgenticCell_v2::clone() const {
        // TODO: Implement in sub-step 3.5
        return nullptr;
    }

} // namespace domain

