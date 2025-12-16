#pragma once

#include <cstdint>
#include "../ports/ICell.h"
#include "../ports/INoiseSource.h"
#include "../ports/ILogger.h"
#include "../ports/ILoggeable.h"
#include "../signal/ISignal.h"
#include "../gene/Genome.h"
#include <memory>
#include <string>
#include <queue>
#include "../shared/Threshold.h"
#include "CellLifeStage.h"

namespace domain {

    /// AgenticCell_v2: Enhanced version with D1 (DNA damage) + D2 (immunosuppression)
    ///
    /// Improvements over v1:
    /// - Separates genomic_instability into D1 and D2 (on-the-fly derivation of CellLifeStage)
    /// - Phase 2 (Endocytosis): Apoptosis decision based on D2 (not genomic_instability)
    /// - Phase 4 (Cytoplasmic Remodeling): Updates D1 and D2 separately with different deltas
    class AgenticCell_v2 final : public ICell, public ports::ILoggeable {
    public:
        /// Constructor: Creates an AgenticCell_v2 with D1 and D2 instability counters
        /// @param noise Random noise source (owned by this cell)
        /// @param genome Genetic makeup (taken by value)
        /// @param neoplasm_k Base neoplasm threshold (default 0.002)
        /// @param low_delta_instability Delta for TP53 +/- (default 0.0001)
        /// @param high_delta_instability Delta for TP53 -/- (default 0.0002)
        /// @param division_rate Probability of cell division in phase4 (default 0.001)
        /// @param neoplastic_division_rate Probability for neoplastic cells in Big Bang mode (default 0.001)
        /// @param enable_big_bang_mode If true, neoplastic cells divide faster (default false)
        /// @param apoptosis_instability_threshold Max D2 for apoptosis to work (default 10.0)
        /// @param logger Optional logger (default NullLogger)
        /// @param d1_primer_threshold D1 threshold to enter PRIMER state (default 2.0)
        /// @param d2_apoptosis_threshold D2 threshold to resist extrinsic apoptosis (default 5.0)
        AgenticCell_v2(std::unique_ptr<INoiseSource> noise,
                       Genome genome,
                       double neoplasm_k = 0.002,
                       double low_delta_instability = 0.0001,
                       double high_delta_instability = 0.0002,
                       double division_rate = 0.001,
                       double neoplastic_division_rate = 0.001,
                       bool enable_big_bang_mode = false,
                       double apoptosis_instability_threshold = 10.0,
                       const ports::ILoggerPtr& logger = nullptr,
                       double d1_primer_threshold = 2.0,
                       double d2_apoptosis_threshold = 5.0);

        // === ILoggeable implementation ===
        std::string getLogCategory() const override { return "CELL_V2"; }

        // === ICell interface ===
        void live() override;
        bool alive() const override;
        bool isNeoplastic() const override;
        std::string getTP53() const;
        std::string getBRCA1() const;
        std::string getBRCA1Status() const override { return getBRCA1(); }
        std::string getTP53Status() const override { return getTP53(); }
        void details() const override;
        void mutateGene(const std::string& name) override;
        void setId(std::uint64_t id) override;
        std::uint64_t id() const override;
        void setSignalEmitter(std::function<void(std::unique_ptr<domain::ISignal>)> emitter) override;
        void receiveMessage(std::unique_ptr<domain::ISignal> signal) override;

        // === New methods for D1 + D2 ===

        /// Get D1 (DNA damage counter)
        [[nodiscard]] double getD1() const { return d1_dna_damage_; }

        /// Get D2 (Immunosuppression counter)
        [[nodiscard]] double getD2() const { return d2_immunosuppression_; }

        /// Get current cell life stage (derived on-the-fly from genetics + D1 + D2)
        [[nodiscard]] CellLifeStage getCurrentCellLifeStage() const;

        // === Existing public methods ===
        bool isNeoplasticProtected() const;
        std::uint64_t getSeed() const { return seed_; }
        std::uint64_t getAge() const { return age_; }
        bool hasEvasedApoptosis() const { return has_evaded_apoptosis_; }
        std::unique_ptr<AgenticCell_v2> clone() const;

    private:
        // === Dependencies ===
        std::unique_ptr<INoiseSource> noise_;
        ports::ILoggerPtr logger_;
        std::function<void(std::unique_ptr<domain::ISignal>)> signal_emitter_;
        std::queue<std::unique_ptr<domain::ISignal>> incoming_messages_;
        Genome genome_;

        // === Neoplasm tracking ===
        double base_neoplasm_k_ = 0.002;
        domain::shared::Threshold neoplasm_k_;
        bool is_neoplastic_ = false;
        bool has_evaded_apoptosis_ = false;

        // === Cell identity ===
        std::uint64_t seed_ = 0;
        std::uint64_t age_ = 0;
        std::uint64_t cell_id_ = static_cast<std::uint64_t>(-1);

        // === Division rates ===
        double division_rate_ = 0.001;
        double neoplastic_division_rate_ = 0.001;
        bool enable_big_bang_mode_ = false;

        // === Apoptosis threshold ===
        double apoptosis_instability_threshold_ = 10.0;

        // === NEW: D1 and D2 instability counters ===
        /// D1: DNA damage counter (genomic instability). Starts at 1.0, grows each tick in phase4
        double d1_dna_damage_ = 1.0;

        /// D2: Immunosuppression counter (immune evasion). Starts at 1.0, grows each tick in phase4
        double d2_immunosuppression_ = 1.0;

        // === Thresholds (configurable) ===
        /// D1 threshold to enter PRIMER state (default 2.0)
        double d1_primer_threshold_ = 2.0;

        /// D2 threshold to resist extrinsic apoptosis (default 5.0)
        double d2_apoptosis_threshold_ = 5.0;

        // === Instability deltas (same as before) ===
        double low_delta_instability_ = 0.0001;
        double high_delta_instability_ = 0.0002;

        // === Private lifecycle phases ===
        void phase0_BaselineAssessment() const;
        void phase1_G1IntegrityCheckpoint() const;
        void phase2_Endocytosis();         // ADJUSTED: apoptosis uses D2
        void phase3_NuclearDynamics();
        void phase4_CytoplasmicRemodeling(); // ADJUSTED: updates D1 and D2
        void phase5_Exocytosis();

        // === Private helper methods ===
        void develop_neoplasm();
        void increaseAge();
        void adjust_neoplasm_k();
        void attemptDivision();
        void attemptApoptosis();

        /// Update D1 and D2 based on BRCA1 and TP53 status
        void updateInstability();
    };

} // namespace domain

