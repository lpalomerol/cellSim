//
//

#include "AgenticCell.h"
#include <iostream>
#include "../exception/CellDeathException.h"
#include "../exception/NeoplasticException.h"
#include "../shared/Threshold.h"

namespace domain {

    /**
     * Construct an AgenticCell.
     * - Injects the provided noise source into all genes.
     * - Sets genome verbosity and captures the RNG seed (if available).
     */
    AgenticCell::AgenticCell(std::unique_ptr<INoiseSource> noise, Genome genome, double neoplasm_k, double low_delta_instability, double high_delta_instability, bool verbose)
        : noise_(std::move(noise)), genome_(std::move(genome)), base_neoplasm_k_(neoplasm_k), neoplasm_k_(domain::shared::Threshold(neoplasm_k)), is_neoplastic_(false), verbose_(verbose), low_delta_instability_(low_delta_instability), high_delta_instability_(high_delta_instability) {
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

    /** Endocytosis phase (placeholder) */
    void AgenticCell::phase2_Endocytosis() {
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

    /** Sample the noise source and set neoplastic flag if threshold crossed. */
    void AgenticCell::develop_neoplasm() {
        if (!noise_) return;
        double sample = noise_->next().u01;
        if (verbose_) std::cout << "[Trace] neoplasm sample=" << sample << " threshold=" << neoplasm_k_.value() << "\n";
        if (sample < neoplasm_k_.value()) {
            is_neoplastic_ = true;
            if (verbose_) std::cout << "[Trace] Cell converted to neoplastic state\n";
        } else {
            if (verbose_) std::cout << "[Trace] No neoplasm (sample >= threshold)\n";
        }
    }

    /** Return true if TP53 is present and enabled. */
    bool AgenticCell::isNeoplasticProtected() const {
        const Gene *tp53 = genome_.getGene("TP53");
        return (tp53 && tp53->enabled());
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

} // namespace domain
