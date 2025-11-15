//
// Created by luis on 31/10/25.
//

#include "AgenticCell.h"
#include <iostream>
#include "../exception/CellDeathException.h"
#include "../exception/NeoplasticException.h"

namespace domain {
    AgenticCell::AgenticCell(std::unique_ptr<INoiseSource> noise, Genome genome, double neoplasm_k, bool verbose)
        : noise_(std::move(noise)), genome_(std::move(genome)), neoplasm_k_(neoplasm_k), is_neoplastic_(false), verbose_(verbose) {
        // Inject the noise source into all genes in the genome via Genome API
        genome_.setNoiseSourceForAll(noise_.get());
        // Ensure genome and its genes respect the verbose flag
        genome_.setVerbose(verbose_);

        // Store and expose the seed used (requires INoiseSource::getSeed())
        if (noise_) {
            seed_ = noise_->getSeed();
        } else {
            seed_ = 0;
        }
        if (verbose_) {
            // Print the seed for traceability; the simulator can also read it via details() or getSeed()
            std::cout << "[Trace] AgenticCell seed: " << seed_ << "\n";

        }

    }

    void AgenticCell::live() {
        // Execute phases in sequence; phases throw exceptions if the cell dies or becomes neoplastic
        try {
            phase0_BaselineAssessment();

            phase1_G1IntegrityCheckpoint();

            phase2_Endocytosis();

            phase3_NuclearDynamics();

            phase4_CytoplasmicRemodeling();

            phase5_Exocytosis();

        } catch (const NeoplasticException& e) {
            if (verbose_) {
                std::cout << "[Trace] neoplastic@live: " << e.what() << "\n";
            }
            return;
        } catch (const CellDeathException& e) {
            if (verbose_) {
                std::cout << "[Trace] dead@live: " << e.what() << "\n";
            }
            // End the live() cycle silently
            return;
        }
    }

    // phase0: show details if verbose
    void AgenticCell::phase0_BaselineAssessment() const {
        if (verbose_) {
            details();
        }
    }

    // phase1: integrity checks
    void AgenticCell::phase1_G1IntegrityCheckpoint() const {
        if (!alive()) {
            throw CellDeathException("dead@phase1");
        }
        if (is_neoplastic_) {
            throw NeoplasticException("neoplastic@phase1");
        }
    }

    // phase2: endocytosis (currently stub)
    void AgenticCell::phase2_Endocytosis() {

    }

    // phase3: nuclear processes (genes + internal adjustments + age increment)
    void AgenticCell::phase3_NuclearDynamics() {
        // For now use this to advance all genes
        // Pass current immunosuppression as a multiplicative factor to gene mutation thresholds
        genome_.liveAllGenes(immunosuppression_);
        adjust_neoplasm_k();
        if (!alive()) {
            throw CellDeathException("dead@phase3");
        }
        increaseAge();
    }

    // phase4: cytoplasmic processes / phenotypic evaluation
    void AgenticCell::phase4_CytoplasmicRemodeling() {
        if (!isNeoplasticProtected()) {
            develop_neoplasm();
        }
        // Update immunosuppression indicator every cycle in phase4
        updateImmunosuppression();
    }

    // phase5: exocytosis (stub for now)
    void AgenticCell::phase5_Exocytosis() {
        // intentionally empty for now
    }

    // Update immunosuppression indicator.
    // Rule: immunosuppression_ evolves by squaring its previous value each iteration.
    // Additionally, TP53 mutational state adds an offset:
    // - "+/-" adds +0.1
    // - "-/-" adds +0.2
    void AgenticCell::updateImmunosuppression() {
        const Gene *tp53 = genome_.getGene("TP53");

        double previous = immunosuppression_;
        // base evolution: square the previous value
        double next = previous * previous;

        if (tp53) {
             std::string st = tp53->status();
             if (st == "+/-") {
                next += 0.1;
             } else if (st == "-/-") {
                next += 0.2;
             }
         }

        // Ensure the immunosuppression acts as a multiplicative degrader starting at 1.0
        // and unbounded above (i.e., it can grow to represent progressive degradation).
        // Therefore we only enforce a lower bound of 1.0.
        if (next < 1.0) next = 1.0;

        immunosuppression_ = next;

        if (verbose_) {
            std::cout << "[Trace] immunosuppression update: prev=" << previous << " -> next=" << immunosuppression_
                      << " | TP53_status=" << (tp53 ? tp53->status() : "?") << "\n";
        }
    }

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

    void AgenticCell::details() const {
        std::string cell_is_alive = (alive() ? "yes" : "no");
        std::string cell_is_neoplastic = (isNeoplastic() ? "yes" : "no");
        std::string cell_is_neoplastic_protected = (isNeoplasticProtected() ? "yes" : "no");
        if (verbose_) {
            std::cout << "[Cell details] "<<
                "Alive: [" <<cell_is_alive << "] |  "<<
                "Neoplastic protected ["<< cell_is_neoplastic_protected<< "] | "<<
                "Neoplastic: [" << cell_is_neoplastic << "] | " <<
                "Seed: [" << seed_ << "]" << " | Age: [" << age_ << "]" << " | Immunosuppression: [" << immunosuppression_ << "]\n";
            // Always show genome state (useful for debugging even if the cell is dead)
            std::cout << "Genome details:\n";
            genome_.details();
        }

    }

    // Implement mutateGene: delegate to Genome::mutate
    void AgenticCell::mutateGene(const std::string& name) {
        genome_.mutate(name);
    }

    // Encapsulate sampling the noise and deciding neoplasm
    void AgenticCell::develop_neoplasm() {
        if (!noise_) return; // safety
        double sample = noise_->next().u01;
        // Trace: show sample and threshold
        if (verbose_) std::cout << "[Trace] sampling for neoplasm: sample=" << sample << " threshold=" << neoplasm_k_ << "\n";
        if (sample < neoplasm_k_) {
            is_neoplastic_ = true;
            if (verbose_) std::cout << "[Trace] Result: cell becomes NEOPLASTIC\n";
        } else {
            if (verbose_) std::cout << "[Trace] Result: no neoplasm developed (sample >= threshold)\n";
        }
    }

    bool AgenticCell::isNeoplasticProtected() const {
        const Gene *tp53 = genome_.getGene("TP53");
        return (tp53 && tp53->enabled());
    }

    void AgenticCell::adjust_neoplasm_k() {
    }

    // Increment age only if the cell is alive (defensive)
    void AgenticCell::increaseAge() {
        if (alive()) {
            ++age_;
        }
    }

} // domain
