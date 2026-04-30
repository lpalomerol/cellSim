#include "AgenticCell.h"
#include "../signal/NeoplasmSignal.h"
#include "../signal/CellDivisionSignal.h"
#include "../exception/CellDeathException.h"
#include "../shared/Threshold.h"
#include "../adapters/RandomNoise.h"
#include "../adapters/NullLogger.h"
#include "../gene/GeneConstants.h"
#include "strategies/GenomicInstabilityDeltaStrategy.h"
#include "strategies/GenomicViabilityStrategy.h"

namespace domain {

    /// Constructor: Initialize AgenticCell using Parameter Objects
    AgenticCell::AgenticCell(std::unique_ptr<INoiseSource> noise,
                                   Genome genome,
                                   const InstabilityConfig& instability,
                                   const DivisionConfig& division,
                                   const ThresholdConfig& thresholds,
                                   const ports::ILoggerPtr& logger)
        : noise_(std::move(noise)),
          logger_(logger ? logger : std::make_shared<adapters::NullLogger>()),
          genome_(std::move(genome)),
          is_neoplastic_(false),
          division_rate_(division.base_rate),
          neoplastic_division_rate_(division.neoplastic_rate),
          enable_big_bang_mode_(division.enable_big_bang),
          low_delta_instability_(instability.low_delta),
          high_delta_instability_(instability.high_delta),
          d1_dna_damage_(1.0),
          d2_immunosuppression_(1.0),
          d1_primer_threshold_(thresholds.d1_primer),
          d2_apoptosis_threshold_(thresholds.d2_apoptosis),
          max_d1_(instability.max_d1),
          max_d2_(instability.max_d2),
          delta_strategy_(std::make_unique<GenomicInstabilityDeltaStrategy>(
              instability.low_delta, instability.high_delta)),
          viability_strategy_(std::make_unique<GenomicViabilityStrategy>()) {

        genome_.setNoiseSourceForAll(noise_.get());
        if (noise_) {
            seed_ = noise_->getSeed();
        } else {
            seed_ = 0;
        }

    }


    // ===== Simple Status Queries =====

    bool AgenticCell::alive() const {
        // Delegate viability decision to injected strategy
        return viability_strategy_->isAlive(*this);
    }

    bool AgenticCell::isNeoplastic() const {
        return is_neoplastic_;
    }

    bool AgenticCell::isNeoplasticProtected() const {
        return genome_.hasTP53Function();
    }

    std::string AgenticCell::getTP53() const {
        const Gene* tp53 = genome_.getGene(GeneNames::TP53);
        return tp53 ? tp53->status() : GeneticStatusStrings::UNKNOWN;
    }

    std::string AgenticCell::getBRCA1() const {
        const Gene* brca1 = genome_.getGene(GeneNames::BRCA1);
        return brca1 ? brca1->status() : GeneticStatusStrings::UNKNOWN;
    }

    // ===== ID Management =====

    void AgenticCell::setId(std::uint64_t id) {
        cell_id_ = id;
    }

    std::uint64_t AgenticCell::id() const {
        return cell_id_;
    }

    // ===== Details and Logging =====

    void AgenticCell::details() const {
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

    void AgenticCell::setSignalEmitter(std::function<void(std::unique_ptr<domain::ISignal>)> emitter) {
        signal_emitter_ = std::move(emitter);
    }

    void AgenticCell::receiveMessage(std::unique_ptr<domain::ISignal> signal) {
        if (!signal) return;

        const auto& targets = signal->targetIds();
        bool should_accept = targets.empty();

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
        }
    }

    // ===== Gene Mutation =====

    void AgenticCell::mutateGene(const std::string& name) {
        genome_.mutate(name);
    }

    // ===== MAIN LIFECYCLE =====

    void AgenticCell::live() {
        // Execute one complete cell cycle: 6 phases
        try {
            phase0_BaselineAssessment();
            phase1_G1IntegrityCheckpoint();
            phase2_Endocytosis();
            phase3_NuclearDynamics();
            phase4_CytoplasmicRemodeling();
            phase5_Exocytosis();
        } catch (const CellDeathException& e) {
            logger_->logCell("[CellDeath] " + std::string(e.what()));
            throw;
        }
    }

    CellLifeStage AgenticCell::getCurrentCellLifeStage() const {
        if (!alive()) {
            return CellLifeStage::DEAD;
        }

        if (is_neoplastic_) {
            return CellLifeStage::TUMORAL;
        }

        std::string tp53_status = getTP53();
        std::string brca1_status = getBRCA1();

        auto tp53 = genome_.getGene(GeneNames::TP53);
        auto brca1 = genome_.getGene(GeneNames::BRCA1);

        if (!tp53 || !brca1) {
            logger_->logCell("[Warn] Unexpected genotype: TP53=" + tp53_status +
                          ", BRCA1=" + brca1_status + ". Defaulting to DEAD.");
            return CellLifeStage::DEAD;
        }
        if (tp53->getStatus().isPartiallyEnabled() &&
            brca1->getStatus().isPartiallyEnabled()) {
            return CellLifeStage::UNSTABLE;
        }

        if (tp53->getStatus().isEnabled() &&
            brca1->getStatus().isPartiallyEnabled()) {
            return CellLifeStage::BASELINE;
        }

        if (tp53->getStatus().isDisabled()) {
            if (d1_dna_damage_ > d1_primer_threshold_) {
                return CellLifeStage::PRIMER;
            }
            return CellLifeStage::UNPROTECTED;
        }

        logger_->logCell("[Warn] Unexpected genotype: TP53=" + tp53_status +
                      ", BRCA1=" + brca1_status + ". Defaulting to DEAD.");
        return CellLifeStage::DEAD;
    }

    // ===== PHASES =====

    void AgenticCell::phase0_BaselineAssessment() const {
    }

    void AgenticCell::phase1_G1IntegrityCheckpoint() const {
        if (!alive()) {
            throw CellDeathException("intrinsic_apoptosis");
        }
    }

    void AgenticCell::phase2_Endocytosis() {
        while (!incoming_messages_.empty()) {
            auto signal = std::move(incoming_messages_.front());
            incoming_messages_.pop();

            if (!signal) continue;

            if (signal->type() == ISignal::Type::Apoptosis) {
                if (d2_immunosuppression_ > d2_apoptosis_threshold_) {
                    logger_->logCell("[Apoptosis] Extrinsic blocked (immune evasion): D2=" +
                                   std::to_string(d2_immunosuppression_));
                    CellLifeStage current_stage = getCurrentCellLifeStage();
                    if (current_stage == CellLifeStage::PRIMER && !is_neoplastic_) {
                        develop_neoplasm();
                    }
                } else {
                    logger_->logCell("[Apoptosis] Extrinsic accepted (immune clearance): D2=" +
                                   std::to_string(d2_immunosuppression_));
                    throw CellDeathException("extrinsic_apoptosis");
                }
            }
        }
    }

    void AgenticCell::phase3_NuclearDynamics() {
        // Drive genome evolution using D1 (DNA damage) as genomic instability factor
        genome_.liveAllGenes(d1_dna_damage_);
    }

    void AgenticCell::phase4_CytoplasmicRemodeling() {
        // Calculate instability deltas based on current genetic state
        InstabilityDeltas deltas = calculateInstabilityDeltas();

        double prev_d1 = d1_dna_damage_;
        double prev_d2 = d2_immunosuppression_;

        // Apply deltas with configured saturation limits
        d1_dna_damage_ = std::min(deltas.applyToD1(d1_dna_damage_), max_d1_);
        d2_immunosuppression_ = std::min(deltas.applyToD2(d2_immunosuppression_), max_d2_);

        CellLifeStage current_stage = getCurrentCellLifeStage();
        if (current_stage == CellLifeStage::PRIMER && !is_neoplastic_) {
            logger_->logCell("[Phase4] PRIMER detected: TP53=" + getTP53() +
                           " D1=" + std::to_string(d1_dna_damage_) +
                           " D2=" + std::to_string(d2_immunosuppression_));

            if (d2_immunosuppression_ > d2_apoptosis_threshold_) {
                develop_neoplasm();
            } else {
                logger_->logCell("[Phase4] Cell eliminated by immune surveillance");
                throw CellDeathException("extrinsic_apoptosis_immune_surveillance");
            }
        }
    }

    void AgenticCell::phase5_Exocytosis() {
        pending_daughter_ = attemptDivision();
        increaseAge();
    }

    // ===== HELPERS =====

    void AgenticCell::increaseAge() {
        if (alive()) age_++;
    }

    void AgenticCell::develop_neoplasm() {
        is_neoplastic_ = true;
        has_evaded_apoptosis_ = true;
        logger_->logCell("[Transform] Cell became NEOPLASTIC (is_neoplastic_=true, immortal)");
    }


    /// Calculate instability deltas using the injected strategy
    InstabilityDeltas AgenticCell::calculateInstabilityDeltas() const {
        return delta_strategy_->calculateDeltas(*this);
    }

    std::unique_ptr<AgenticCell> AgenticCell::attemptDivision() {
        double rate = is_neoplastic_ && enable_big_bang_mode_ ? neoplastic_division_rate_ : division_rate_;
        // Sample random value - use noise source if available
        double rnd = 0.0;
        if (noise_) {
            rnd = noise_->next().u01;  // Get uniform [0,1) value
        }

        if (rnd < rate) {
            auto daughter = clone();
            logger_->logCell("[Division] Daughter created from cell " + std::to_string(cell_id_));
            return daughter;
        }

        // No division occurred
        return nullptr;
    }


    std::unique_ptr<AgenticCell> AgenticCell::clone() const {
        // Generate unique seed for daughter cell based on mother's seed and current state
        // This ensures each daughter has a different random sequence
        unsigned daughter_seed = static_cast<unsigned>(seed_ + age_ + cell_id_);

        // Build configuration objects from current cell state
        InstabilityConfig instability{low_delta_instability_, high_delta_instability_, max_d1_, max_d2_};
        DivisionConfig division{division_rate_, neoplastic_division_rate_, enable_big_bang_mode_};
        ThresholdConfig thresholds{d1_primer_threshold_, d2_apoptosis_threshold_};

        auto daughter = std::make_unique<AgenticCell>(
            std::make_unique<adapters::RandomNoise>(daughter_seed),
            genome_,
            instability,
            division,
            thresholds,
            logger_
        );

        daughter->d1_dna_damage_ = d1_dna_damage_;
        daughter->d2_immunosuppression_ = d2_immunosuppression_;

        if (is_neoplastic_) {
            daughter->is_neoplastic_ = true;
            daughter->has_evaded_apoptosis_ = true;
        }

        return daughter;
    }

} // namespace domain

