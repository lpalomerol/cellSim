//
// Created by luis on 31/10/25.
//

#pragma once
#include <cstdint>
#include "../ports/ICell.h"
#include "../ports/INoiseSource.h"
#include "../gene/Genome.h"
#include <memory>
#include <string>
#include "../shared/Threshold.h"


namespace domain {

    class AgenticCell final : public ICell {
    public:
        // Construct an AgenticCell. Genome is taken by value to make ownership explicit
        // and allow move-semantics from caller. Noise source is owned via unique_ptr.
        // Added optional low/high genomic instability deltas (defaults kept for backward compatibility)
        AgenticCell(std::unique_ptr<INoiseSource> noise,
                    Genome genome,
                    double neoplasm_k = 0.002,
                    double low_delta_instability = 0.0001,
                    double high_delta_instability = 0.0002,
                    bool verbose = false);

        // Run a single lifecycle tick for the cell
        void live() override;
        // Query liveness
        bool alive() const override;
        // Query whether the cell has become neoplastic
        bool isNeoplastic() const override;
        [[nodiscard]] std::string getTP53() const;
        [[nodiscard]] std::string getBRCA1() const;

        // IGeneticProfile implementation
        [[nodiscard]] std::string getBRCA1Status() const override { return getBRCA1(); }
        [[nodiscard]] std::string getTP53Status() const override { return getTP53(); }

        // Print cell and genome details (read-only). Implementation may be verbose-controlled.
        void details() const override;

        // Force a gene mutation in the internal genome
        void mutateGene(const std::string& name) override;

        // Check whether TP53 protects the cell from neoplasm (i.e., TP53 enabled)
        bool isNeoplasticProtected() const;

        // Expose the seed used by the noise source for traceability
        std::uint64_t getSeed() const { return seed_; }

        // Expose cell age (useful for tests and tracing)
        std::uint64_t getAge() const { return age_; }

        // ID management (implements ICell contract)
        void setId(std::uint64_t id) override;
        std::uint64_t id() const override;

        // Allow Tissue to inject a signal emitter callback
        void setSignalEmitter(std::function<void(std::unique_ptr<domain::ISignal>)> emitter) override;

    private:
        std::unique_ptr<INoiseSource> noise_;
        // callback to emit signals to the owning tissue; default empty
        std::function<void(std::unique_ptr<domain::ISignal>)> signal_emitter_;
        Genome genome_;
        // Base (inalterable) neoplasm k provided at construction — used as baseline
        double base_neoplasm_k_ = 0.002;
        domain::shared::Threshold neoplasm_k_;
        bool is_neoplastic_;
        bool verbose_ = false;
        std::uint64_t seed_ = 0; // records the RNG seed used by the noise source

        // Age counter incremented each tick when the cell is alive
        std::uint64_t age_ = 0;

        // Stable id for the cell (default -1 meaning unassigned)
        std::uint64_t cell_id_ = static_cast<std::uint64_t>(-1);

        // Encapsulate neoplasm development logic (samples noise and applies threshold)
        void develop_neoplasm();

        // Increment age by one tick (defensive: only increments if the cell remains alive)
        void increaseAge();

        // Lifecycle phases (keeps live() method small and testable)
        void phase0_BaselineAssessment() const;
        void phase1_G1IntegrityCheckpoint() const;
        void phase2_Endocytosis();
        void phase3_NuclearDynamics();
        void phase4_CytoplasmicRemodeling();
        void phase5_Exocytosis();

        // Adjust neoplasm probability (placeholder for future behavior)
        void adjust_neoplasm_k();

        // Indicator of genomic instability. Starts at 1.0 and is updated in phase4.
        // This acts as a multiplicative degrader of the biological system: it starts at 1.0
        // and may grow without an upper bound (values >1 represent progressive instability).
        double genomic_instability_ = 1.0;

        // Small configurable deltas applied based on TP53 state when updating genomic instability.
        // Defaults updated: heterozygous adds 0.0001, homozygous adds 0.0002.
        double low_delta_instability_ = 0.0001;
        double high_delta_instability_ = 0.0002;

        // Update the genomic instability indicator based on TP53 status and previous value.
        // The implementation evolves the value by multiplying it by itself (squaring),
        // then adds offsets depending on TP53 mutation state (see .cpp).
        // Note: genomic_instability_ is clamped to a minimum of 1.0 but not capped above.
        void updateGenomicInstability();

    public:
        // Expose the genomic instability indicator for tests/tracing
        [[nodiscard]] double getGenomicInstability() const { return genomic_instability_; }
        // Expose the configured deltas for tests or external observation
        [[nodiscard]] double getLowDeltaInstability() const { return low_delta_instability_; }
        [[nodiscard]] double getHighDeltaInstability() const { return high_delta_instability_; }
    };
} // domain
