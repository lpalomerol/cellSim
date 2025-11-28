//
//

#include "AgenticCell.h"
#include <iostream>
#include "../signal/NeoplasmSignal.h"
#include "../signal/CellDivisionSignal.h"
#include "../signal/ApoptosisSignal.h"
#include "../exception/CellDeathException.h"
#include "../exception/NeoplasticException.h"
#include "../shared/Threshold.h"
#include "../adapters/RandomNoise.h"

namespace domain {

    /**
     * Construct an AgenticCell.
     * - Injects the provided noise source into all genes.
     * - Sets genome verbosity and captures the RNG seed (if available).
     */
    AgenticCell::AgenticCell(std::unique_ptr<INoiseSource> noise, Genome genome, double neoplasm_k, double low_delta_instability, double high_delta_instability, double division_rate, bool verbose)
        : noise_(std::move(noise)), genome_(std::move(genome)), base_neoplasm_k_(neoplasm_k), neoplasm_k_(domain::shared::Threshold(neoplasm_k)), is_neoplastic_(false), verbose_(verbose), low_delta_instability_(low_delta_instability), high_delta_instability_(high_delta_instability), division_rate_(division_rate) {
        // Inject the noise source into all genes via the Genome API
        genome_.setNoiseSourceForAll(noise_.get());
        // Propagate verbose flag to the genome and genes
        genome_.setVerbose(verbose_);

        // Store the RNG seed (if the noise source provides one)
        if (noise_) {
            seed_ = noise_->getSeed();
        } else {
            seed_ = 0;
        }
        if (verbose_) {
            std::cout << "[Trace] AgenticCell seed: " << seed_ << "\n";
        }
    }

    /**
     * Run a single cell cycle. Phases are executed in order; exceptions are
     * used to signal cell death or neoplastic conversion and stop further work.
     */
    void AgenticCell::live() {
        try {
            phase0_BaselineAssessment();
            phase1_G1IntegrityCheckpoint();
            phase2_Endocytosis();
            phase3_NuclearDynamics();
            phase4_CytoplasmicRemodeling();
            phase5_Exocytosis();
        } catch (const NeoplasticException& e) {
            if (verbose_) {
                std::cout << "[Trace] Neoplastic during live(): " << e.what() << "\n";
            }
            return;
        } catch (const CellDeathException& e) {
            if (verbose_) {
                std::cout << "[Trace] Cell death during live(): " << e.what() << "\n";
            }
            return;
        }
    }

    /** Show details when verbose */
    void AgenticCell::phase0_BaselineAssessment() const {
        if (verbose_) {
            details();
        }
    }

    /**
     * G1 integrity checkpoint: throws if the cell is dead or already neoplastic.
     */
    void AgenticCell::phase1_G1IntegrityCheckpoint() const {
        if (!alive()) {
            throw CellDeathException("dead@phase1");
        }
        if (is_neoplastic_) {
            throw NeoplasticException("neoplastic@phase1");
        }
    }

    /**
     * Endocytosis phase: process all incoming messages.
     * - Validates destination
     * - Displays message details when verbose
     * - Future: handle message types (apoptosis, stress, etc.)
     */
    void AgenticCell::phase2_Endocytosis() {
        while (!incoming_messages_.empty()) {
            auto msg = std::move(incoming_messages_.front());
            incoming_messages_.pop();

            if (msg && verbose_) {
                std::cout << "[Endocytosis] Cell [" << cell_id_ << "] processing message\n";
                std::cout << "  - Type: " << static_cast<int>(msg->type()) << "\n";
                std::cout << "  - Source: " << msg->sourceId() << "\n";
                std::cout << "  - Message: " << msg->message() << "\n";

                const auto& targets = msg->targetIds();
                std::cout << "  - Targets: ";
                if (targets.empty()) {
                    std::cout << "(broadcast)\n";
                } else {
                    for (size_t i = 0; i < targets.size(); ++i) {
                        if (i > 0) std::cout << ", ";
                        std::cout << targets[i];
                    }
                    std::cout << "\n";
                }
            }

            // Handle specific message types
            if (msg->type() == ISignal::Type::Apoptosis) {
                attemptApoptosis();
            }
        }
    }

    /**
     * Nuclear dynamics: advance all genes and update neoplasm threshold.
     * Also checks for death and increments age for living cells.
     */
    void AgenticCell::phase3_NuclearDynamics() {
        // Advance genes using the current genomic instability modifier
        genome_.liveAllGenes(genomic_instability_);
        adjust_neoplasm_k();
        if (!alive()) {
            throw CellDeathException("dead@phase3");
        }
        increaseAge();
    }

    /**
     * Cytoplasmic remodeling: attempt neoplasm development (unless protected)
     * and update immunosuppression for the next cycle.
     */
    void AgenticCell::phase4_CytoplasmicRemodeling() {
        if (!isNeoplasticProtected()) {
            develop_neoplasm();
        }
        // Update genomic instability every cycle
        updateGenomicInstability();
        // Attempt cell division
        attemptDivision();
    }

    /** Exocytosis phase (placeholder) */
    void AgenticCell::phase5_Exocytosis() {
        // intentionally empty
    }

    /**
     * Update genomic instability metric.
     * Rules:
     * - The base progression squares the previous value each iteration.
     * - TP53 contributes an additive offset: "+/-" => low_delta_instability_, "-/-" => high_delta_instability_.
     * - The metric is bounded below by 1.0 (acts as a multiplicative degrader).
     */
    void AgenticCell::updateGenomicInstability() {
        const Gene *tp53 = genome_.getGene("TP53");

        double previous = genomic_instability_;
        double next = previous * previous;

        if (tp53) {
             std::string st = tp53->status();
             if (st == "+/-") {
                next += low_delta_instability_;
             } else if (st == "-/-") {
                next += high_delta_instability_;
             }
         }

        if (next < 1.0) next = 1.0;

        genomic_instability_ = next;

        if (verbose_) {
            std::cout << "[Trace] genomic_instability: prev=" << previous << " -> next=" << genomic_instability_
                      << " | TP53=" << (tp53 ? tp53->status() : "?") << "\n";
        }
    }

    /** Return whether the cell is alive (BRCA1 must be enabled). */
    bool AgenticCell::alive() const {
        const Gene *brca1 = genome_.getGene("BRCA1");
        return brca1 && brca1->enabled();
    }

    bool AgenticCell::isNeoplastic() const {
        return is_neoplastic_;
    }

    std::string AgenticCell::getTP53() const {
        const Gene *tp53 = genome_.getGene("TP53");
        return tp53 ? tp53->status() : "?";
    }

    std::string AgenticCell::getBRCA1() const {
        const Gene *brca1 = genome_.getGene("BRCA1");
        return brca1 ? brca1->status() : "?";
    }

    /** Print concise cell state and genome when verbose. */
    void AgenticCell::details() const {
        std::string cell_is_alive = (alive() ? "yes" : "no");
        std::string cell_is_neoplastic = (isNeoplastic() ? "yes" : "no");
        std::string cell_is_neoplastic_protected = (isNeoplasticProtected() ? "yes" : "no");
        if (verbose_) {
            std::cout << "[Cell details] "
                << "Alive: [" << cell_is_alive << "] | "
                << "Neoplastic protected: ["<< cell_is_neoplastic_protected<< "] | "
                << "Neoplastic: [" << cell_is_neoplastic << "] | "
                << "Seed: [" << seed_ << "] | Age: [" << age_ << "] | Genomic instability: [" << genomic_instability_ << "]\n";
            std::cout << "Genome details:\n";
            genome_.details();
        }

    }

    /** Delegate mutation to the Genome. */
    void AgenticCell::mutateGene(const std::string& name) {
        genome_.mutate(name);
    }

    void AgenticCell::setSignalEmitter(std::function<void(std::unique_ptr<domain::ISignal>)> emitter) {
        signal_emitter_ = std::move(emitter);
    }

    /**
     * Receive a directed message or broadcast signal.
     * - If targetIds is empty: it's a broadcast, accept it.
     * - If targetIds is non-empty: only accept if this cell's ID is in the list.
     */
    void AgenticCell::receiveMessage(std::unique_ptr<domain::ISignal> signal) {
        if (!signal) return;

        const auto& targets = signal->targetIds();

        // Validate if this message is for us
        bool should_accept = false;
        if (targets.empty()) {
            // Broadcast: everyone receives
            should_accept = true;
        } else {
            // Directed: check if our ID is in the list
            for (auto target_id : targets) {
                if (target_id == cell_id_) {
                    should_accept = true;
                    break;
                }
            }
        }

        if (should_accept) {
            incoming_messages_.push(std::move(signal));
            if (verbose_) {
                std::cout << "[Trace] Cell [" << cell_id_ << "] received message: "
                          << (incoming_messages_.back() ? incoming_messages_.back()->message() : "?") << "\n";
            }
        } else {
            if (verbose_) {
                std::cout << "[Trace] Cell [" << cell_id_ << "] ignored message (not in targetIds)\n";
            }
        }
    }

    /** Sample the noise source and set neoplastic flag if threshold crossed. */
    void AgenticCell::develop_neoplasm() {
        if (!noise_) return;
        double sample = noise_->next().u01;
        if (verbose_) std::cout << "[Trace] neoplasm sample=" << sample << " threshold=" << neoplasm_k_.value() << "\n";
        if (sample < neoplasm_k_.value()) {
            bool transitioned = !is_neoplastic_;
            is_neoplastic_ = true;
            if (verbose_) std::cout << "[Trace] Cell converted to neoplastic state\n";
            if (transitioned && signal_emitter_) {
                auto sig = std::make_unique<NeoplasmSignal>(id(), std::string("neoplasm"));
                signal_emitter_(std::move(sig));
            }
        } else {
            if (verbose_) std::cout << "[Trace] No neoplasm (sample >= threshold)\n";
        }
    }

    /** Return true if TP53 is NOT -/- (i.e., +/+ or +/- protect; only -/- allows tumors). */
    bool AgenticCell::isNeoplasticProtected() const {
        const Gene *tp53 = genome_.getGene("TP53");
        if (!tp53) return false;

        // Protect from neoplasm unless TP53 is -/- (MinusMinus)
        // This means:
        // - TP53 +/+ → Protected ✅
        // - TP53 +/- → Protected ✅ (but with degraded protection & increased instability)
        // - TP53 -/- → NOT protected ❌ (allows tumors)
        std::string status = tp53->status();
        return status != "-/-";
    }

    void AgenticCell::adjust_neoplasm_k() {
        // Compute neoplasm threshold as base_k multiplied by genomic instability.
        // Additionally apply small additive deltas depending on TP53 state (same notion as genomic instability).
        const Gene *tp53 = genome_.getGene("TP53");

        double previous = neoplasm_k_.value();
        double next = base_neoplasm_k_ * genomic_instability_;

        if (tp53) {
            std::string st = tp53->status();
            if (st == "+/-") {
                next += low_delta_instability_;
            } else if (st == "-/-") {
                next += high_delta_instability_;
            }
        }

        // Assign back using Threshold to ensure clamping to [0,1]
        neoplasm_k_ = domain::shared::Threshold(next);

        if (verbose_) {
            std::cout << "[Trace] neoplasm_k: base=" << base_neoplasm_k_ << " instability=" << genomic_instability_
                      << " prev=" << previous << " -> next=" << neoplasm_k_.value()
                      << " | TP53=" << (tp53 ? tp53->status() : "?") << "\n";
        }
    }

    /** Increment age only when cell is alive. */
    void AgenticCell::increaseAge() {
        if (alive()) {
            ++age_;
        }
    }

    void AgenticCell::setId(std::uint64_t id) {
        cell_id_ = id;
    }

    std::uint64_t AgenticCell::id() const {
        return cell_id_;
    }

    /**
     * Attempt cell division: sample noise and compare with division_rate.
     * If random value < division_rate, the cell attempts to divide.
     * Creates a daughter cell and emits a CellDivisionSignal to the tissue.
     */
    void AgenticCell::attemptDivision() {
        if (division_rate_ <= 0.0) {
            return; // Division disabled
        }

        double random_value = noise_->next().u01;
        if (random_value < division_rate_) {
            if (verbose_) {
                std::cout << "[Division] Cell [" << cell_id_ << "] attempting division "
                          << "(random=" << random_value << " < division_rate=" << division_rate_ << ")\n";
            }

            // Create daughter cell by cloning
            auto daughter = clone();

            if (signal_emitter_) {
                // Emit CellDivisionSignal with the daughter cell to the tissue
                auto sig = std::make_unique<CellDivisionSignal>(
                    id(),
                    std::move(daughter),
                    "cell_division"
                );
                signal_emitter_(std::move(sig));

                if (verbose_) {
                    std::cout << "[Division] Cell [" << cell_id_ << "] emitted daughter cell signal\n";
                }
            }
        }
    }

    /**
     * Create a clone (daughter cell) of this cell.
     * The daughter cell has:
     * - A cloned genome (same state but independent)
     * - A new random noise source (for genetic diversity)
     * - Age reset to 0
     * - Cell ID will be assigned by the tissue
     * - Same configuration parameters (division_rate, neoplasm_k, instability deltas)
     */
    std::unique_ptr<AgenticCell> AgenticCell::clone() const {
        // Create a new random noise source with a random seed for diversity
        // Use a seed derived from current seed + cell age for some variability
        unsigned new_seed = static_cast<unsigned>(seed_ + age_ + 1);
        auto new_noise = std::make_unique<adapters::RandomNoise>(new_seed);

        // Clone the genome
        Genome cloned_genome = genome_.clone();

        // Create daughter cell with same configuration
        auto daughter = std::make_unique<AgenticCell>(
            std::move(new_noise),
            cloned_genome,
            base_neoplasm_k_,
            low_delta_instability_,
            high_delta_instability_,
            division_rate_,
            verbose_
        );

        if (verbose_) {
            std::cout << "[Clone] Created daughter cell from parent [" << cell_id_ << "] with new seed=" << new_seed << "\n";
        }

        return daughter;
    }

    /**
     * Attempt apoptosis (programmed cell death) in response to an apoptosis signal.
     * This method disables the BRCA1 gene, which will cause the cell to be marked as dead
     * in the next phase (since alive() checks if BRCA1 is enabled).
     */
    void AgenticCell::attemptApoptosis() {
        if (verbose_) {
            std::cout << "[Apoptosis] Cell [" << cell_id_ << "] received apoptosis signal and is undergoing programmed cell death\n";
        }

        // Disable BRCA1 to trigger cell death
        genome_.mutate("BRCA1");

        // Optionally throw CellDeathException to immediately stop the current cycle
        throw CellDeathException("apoptosis@phase2");
    }

} // namespace domain
