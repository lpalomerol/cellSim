#include "AgenticCell.h"
#include "../signal/NeoplasmSignal.h"
#include "../signal/CellDivisionSignal.h"
#include "../exception/CellDeathException.h"
#include "../shared/Threshold.h"
#include "../adapters/RandomNoise.h"
#include "../adapters/NullLogger.h"

namespace domain {

    /// Constructor: Initialize AgenticCell with D1 and D2 counters
    AgenticCell::AgenticCell(std::unique_ptr<INoiseSource> noise,
                                   Genome genome,
                                   double neoplasm_k,
                                   double low_delta_instability,
                                   double high_delta_instability,
                                   double division_rate,
                                   double neoplastic_division_rate,
                                   bool enable_big_bang_mode,
                                   double apoptosis_instability_threshold,
                                   const ports::ILoggerPtr& logger,
                                   double d1_primer_threshold,
                                   double d2_apoptosis_threshold)
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
          d2_immunosuppression_(1.0),
          d1_primer_threshold_(d1_primer_threshold),
          d2_apoptosis_threshold_(d2_apoptosis_threshold) {

        genome_.setNoiseSourceForAll(noise_.get());
        if (noise_) {
            seed_ = noise_->getSeed();
        } else {
            seed_ = 0;
        }

        logger_->logCell("[Trace] AgenticCell constructed: seed=" + std::to_string(seed_)
                      + " | d1=" + std::to_string(d1_dna_damage_)
                      + " | d2=" + std::to_string(d2_immunosuppression_)
                      + " | d1_threshold=" + std::to_string(d1_primer_threshold_)
                      + " | d2_threshold=" + std::to_string(d2_apoptosis_threshold_));
    }

    // ===== Simple Status Queries =====

    bool AgenticCell::alive() const {
        // If cell has evaded apoptosis (is neoplastic), always alive
        if (has_evaded_apoptosis_) {
            return true;
        }

        const Gene* brca1 = genome_.getGene("BRCA1");
        const Gene* tp53 = genome_.getGene("TP53");

        // BRCA1 missing or check: if not present, cell is dead
        if (!brca1) {
            return false;
        }

        // BRCA1 -/- is lethal ONLY if TP53 is functional
        if (brca1->status() == "-/-") {
            // TP53 -/- cannot kill the cell → alive
            if (tp53 && tp53->status() == "-/-") {
                return true;
            }
            // TP53 is functional (+/+ or +/-) → detects damage → dead
            return false;
        }

        // BRCA1 +/- (normal case) → cell is alive
        return true;
    }

    bool AgenticCell::isNeoplastic() const {
        return is_neoplastic_;
    }

    bool AgenticCell::isNeoplasticProtected() const {
        const Gene* tp53 = genome_.getGene("TP53");
        if (!tp53) return false;
        std::string status = tp53->status();
        return status != "-/-";
    }

    std::string AgenticCell::getTP53() const {
        const Gene* tp53 = genome_.getGene("TP53");
        return tp53 ? tp53->status() : "?";
    }

    std::string AgenticCell::getBRCA1() const {
        const Gene* brca1 = genome_.getGene("BRCA1");
        return brca1 ? brca1->status() : "?";
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
            logger_->logCell("[Trace] Cell [" + std::to_string(cell_id_) + "] received message");
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

        if (tp53_status == "+/-" && brca1_status == "+/-") {
            return CellLifeStage::UNSTABLE;
        }

        if (tp53_status == "+/+" && brca1_status == "+/-") {
            return CellLifeStage::BASELINE;
        }

        if (tp53_status == "-/-") {
            if (d1_dna_damage_ > d1_primer_threshold_) {
                return CellLifeStage::PRIMER;
            }
            return CellLifeStage::UNPROTECTED;
        }

        logger_->logCell("[Trace] Unexpected genotype: TP53=" + tp53_status +
                      ", BRCA1=" + brca1_status + ". Defaulting to DEAD.");
        return CellLifeStage::DEAD;
    }

    // ===== PHASES =====

    void AgenticCell::phase0_BaselineAssessment() const {
        CellLifeStage stage = getCurrentCellLifeStage();
        logger_->logCell("[Phase0] Cycle start: stage=" + toString(stage) +
                       ", age=" + std::to_string(age_) +
                       ", d1=" + std::to_string(d1_dna_damage_) +
                       ", d2=" + std::to_string(d2_immunosuppression_));

        if (is_neoplastic_) {
            logger_->logCell("[Phase0] Cell is NEOPLASTIC (is_neoplastic_=true)");
        }
        if (has_evaded_apoptosis_) {
            logger_->logCell("[Phase0] Cell has EVADED apoptosis (immortal)");
        }
    }

    void AgenticCell::phase1_G1IntegrityCheckpoint() const {
        if (!alive()) {
            logger_->logCell("[Phase1] Cell is DEAD (intrinsic apoptosis: BRCA1 -/-)");
            throw CellDeathException("intrinsic_apoptosis@phase1");
        }
    }

    void AgenticCell::phase2_Endocytosis() {
        while (!incoming_messages_.empty()) {
            auto signal = std::move(incoming_messages_.front());
            incoming_messages_.pop();

            if (!signal) continue;

            logger_->logCell("[Phase2] Processing signal type=" + std::to_string(static_cast<int>(signal->type())));

            if (signal->type() == ISignal::Type::Apoptosis) {
                // PART 1: Apoptosis (universal)
                if (d2_immunosuppression_ > d2_apoptosis_threshold_) {
                    logger_->logCell("[Apoptosis] Extrinsic BLOCKED: D2=" +
                                   std::to_string(d2_immunosuppression_) + " > " +
                                   std::to_string(d2_apoptosis_threshold_) + " (immune evasion)");

                    // PART 2: Neoplasm (conditional, only if PRIMER)
                    CellLifeStage current_stage = getCurrentCellLifeStage();
                    if (current_stage == CellLifeStage::PRIMER && !is_neoplastic_) {
                        logger_->logCell("[Phase2] Cell in PRIMER state - developing neoplasm");
                        develop_neoplasm();
                        logger_->logCell("[Phase2] Cell TRANSFORMED to TUMORAL (is_neoplastic_ = true)");
                    }
                } else {
                    logger_->logCell("[Apoptosis] Extrinsic ACCEPTED: D2=" +
                                   std::to_string(d2_immunosuppression_) + " <= " +
                                   std::to_string(d2_apoptosis_threshold_) + " (immune clearance)");
                    throw CellDeathException("extrinsic_apoptosis@phase2");
                }
            }
        }
    }

    void AgenticCell::phase3_NuclearDynamics() {
        logger_->logCell("[Phase3] Nuclear dynamics: genome evolution");
        logger_->logCell("[Phase3] Genome status BEFORE: TP53=" + getTP53() +
                       ", BRCA1=" + getBRCA1());

        // Make all genes live and potentially mutate
        // Use D1 (DNA damage) as genomic instability factor
        genome_.liveAllGenes(d1_dna_damage_);

        logger_->logCell("[Phase3] Genome status AFTER: TP53=" + getTP53() +
                       ", BRCA1=" + getBRCA1() +
                       ", D1=" + std::to_string(d1_dna_damage_));
    }

    void AgenticCell::phase4_CytoplasmicRemodeling() {
        // Calculate instability deltas based on current genetic state
        auto [delta_d1, delta_d2] = calculateInstabilityDeltas();

        double prev_d1 = d1_dna_damage_;
        double prev_d2 = d2_immunosuppression_;

        d1_dna_damage_ = std::min(d1_dna_damage_ + delta_d1, 999.0);
        d2_immunosuppression_ = std::min(d2_immunosuppression_ + delta_d2, 999.0);

        logger_->logCell("[Phase4] d1_update: " + std::to_string(prev_d1) + " → " +
                       std::to_string(d1_dna_damage_) + " (delta=" + std::to_string(delta_d1) + ")");
        logger_->logCell("[Phase4] d2_update: " + std::to_string(prev_d2) + " → " +
                       std::to_string(d2_immunosuppression_) + " (delta=" + std::to_string(delta_d2) + ")");

        CellLifeStage current_stage = getCurrentCellLifeStage();
        if (current_stage == CellLifeStage::PRIMER && !is_neoplastic_) {
            std::string tp53_status = getTP53();
            logger_->logCell("[Phase4] Cell DETECTED in PRIMER state");
            logger_->logCell("[Phase4]   Reason: TP53=" + tp53_status + ", D1=" +
                           std::to_string(d1_dna_damage_) + " > " + std::to_string(d1_primer_threshold_));

            // Check if cell can evade immune surveillance (D2 > threshold)
            if (d2_immunosuppression_ > d2_apoptosis_threshold_) {
                logger_->logCell("[Phase4]   D2=" + std::to_string(d2_immunosuppression_) +
                               " > " + std::to_string(d2_apoptosis_threshold_) +
                               " → IMMUNE EVASION");
                logger_->logCell("[Phase4]   Cell TRANSFORMING to NEOPLASTIC (autonomous transformation)");
                develop_neoplasm();
            } else {
                logger_->logCell("[Phase4]   D2=" + std::to_string(d2_immunosuppression_) +
                               " <= " + std::to_string(d2_apoptosis_threshold_) +
                               " → IMMUNE SYSTEM DETECTS pretumoral cell");
                logger_->logCell("[Phase4]   Cell ELIMINATED by extrinsic apoptosis (immune surveillance)");
                throw CellDeathException("extrinsic_apoptosis@phase4_immune_surveillance");
            }
        }
    }

    void AgenticCell::phase5_Exocytosis() {
        logger_->logCell("[Phase5] Exocytosis: attempting cell division");
        pending_daughter_ = attemptDivision();
        logger_->logCell("[Phase5] Exocytosis complete");
        increaseAge();
    }

    // ===== HELPERS =====

    void AgenticCell::increaseAge() {
        if (alive()) {
            age_++;
            logger_->logCell("[Misc] Age increased to " + std::to_string(age_));
        }
    }

    void AgenticCell::develop_neoplasm() {
        is_neoplastic_ = true;
        has_evaded_apoptosis_ = true;
        logger_->logCell("[Transform] Cell became NEOPLASTIC (is_neoplastic_=true, immortal)");
    }

    void AgenticCell::adjust_neoplasm_k() {
        // Placeholder: adjust neoplasm probability if needed
    }

    void AgenticCell::updateInstability() {
        // Placeholder: D1/D2 updated in phase4, not here
    }

    std::pair<double, double> AgenticCell::calculateInstabilityDeltas() const {
        // Extract current genetic status
        std::string tp53_status = getTP53();
        std::string brca1_status = getBRCA1();

        // Calculate delta per gene individually
        // Delta TP53: affects D1 directly and contributes to D2
        double delta_tp53 = 0.0;
        if (tp53_status == "+/+") {
            delta_tp53 = 0.0;  // Wild-type: no instability
        } else if (tp53_status == "+/-") {
            delta_tp53 = low_delta_instability_;  // Heterozygous: low instability
        } else if (tp53_status == "-/-") {
            delta_tp53 = high_delta_instability_;  // Homozygous recessive: high instability
        }

        // Delta BRCA1: contributes only to D2 (immunosuppression)
        double delta_brca1 = 0.0;
        if (brca1_status == "+/+") {
            delta_brca1 = 0.0;  // Wild-type: no contribution
        } else if (brca1_status == "+/-") {
            delta_brca1 = low_delta_instability_;  // Heterozygous: low contribution
        } else if (brca1_status == "-/-") {
            delta_brca1 = 2 * high_delta_instability_;  // Homozygous: very high contribution
        }

        // Decision matrix:
        // D1 (DNA damage) = Δ(TP53) only
        // D2 (Immunosuppression) = Δ(TP53) + Δ(BRCA1)
        double delta_d1 = delta_tp53;
        double delta_d2 = delta_tp53 + delta_brca1;

        return {delta_d1, delta_d2};
    }

    std::unique_ptr<AgenticCell> AgenticCell::attemptDivision() {
        double rate = is_neoplastic_ && enable_big_bang_mode_ ? neoplastic_division_rate_ : division_rate_;
        // Sample random value - use noise source if available
        double rnd = 0.0;
        if (noise_) {
            rnd = noise_->next().u01;  // Get uniform [0,1) value
        }

        if (rnd < rate) {
            logger_->logCell("[Division] Cell division triggered (random=" + std::to_string(rnd) + " < rate=" + std::to_string(rate) + ")");

            // Create and return daughter cell (clone)
            auto daughter = clone();
            logger_->logCell("[Division] Daughter cell created from cell " + std::to_string(cell_id_));
            return daughter;
        }

        // No division occurred
        return nullptr;
    }

    void AgenticCell::attemptApoptosis() {
        // Placeholder: apoptosis handled in phase2_Endocytosis
    }

    std::unique_ptr<AgenticCell> AgenticCell::clone() const {
        // Generate unique seed for daughter cell based on mother's seed and current state
        // This ensures each daughter has a different random sequence
        unsigned daughter_seed = static_cast<unsigned>(seed_ + age_ + cell_id_);

        auto daughter = std::make_unique<AgenticCell>(
            std::make_unique<adapters::RandomNoise>(daughter_seed),
            genome_,
            base_neoplasm_k_,
            low_delta_instability_,
            high_delta_instability_,
            division_rate_,
            neoplastic_division_rate_,
            enable_big_bang_mode_,
            apoptosis_instability_threshold_,
            logger_,
            d1_primer_threshold_,      // Inherit thresholds
            d2_apoptosis_threshold_
        );

        daughter->d1_dna_damage_ = d1_dna_damage_;
        daughter->d2_immunosuppression_ = d2_immunosuppression_;

        if (is_neoplastic_) {
            daughter->is_neoplastic_ = true;
            daughter->has_evaded_apoptosis_ = true;
        }

        logger_->logCell("[Misc] Cell cloned (daughter inherits d1=" + std::to_string(d1_dna_damage_) +
                       ", d2=" + std::to_string(d2_immunosuppression_) +
                       ", daughter_seed=" + std::to_string(daughter_seed) + ")");

        return daughter;
    }

} // namespace domain

