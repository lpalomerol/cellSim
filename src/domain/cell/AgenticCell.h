#pragma once

#include "../ports/ICell.h"
#include "../ports/INoiseSource.h"
#include "../ports/ILogger.h"
#include "../ports/ILoggeable.h"
#include "../signal/ISignal.h"
#include "../gene/Genome.h"
#include "CellConfig.h"
#include "strategies/IInstabilityDeltaStrategy.h"
#include "strategies/IViabilityStrategy.h"
#include <memory>
#include <string>
#include <queue>
#include "../shared/Threshold.h"
#include "CellLifeStage.h"

namespace domain {

    /// AgenticCell: Enhanced cell implementation with D1 (DNA damage) + D2 (immunosuppression)
    ///
    /// Improvements over v1:
    /// - Separates genomic_instability into D1 and D2 (on-the-fly derivation of CellLifeStage)
    /// - Phase 2 (Endocytosis): Apoptosis decision based on D2 (not genomic_instability)
    /// - Phase 4 (Cytoplasmic Remodeling): Updates D1 and D2 separately with different deltas
    class AgenticCell final : public ICell, public ports::ILoggeable {
    public:

        /// Constructor: Creates an AgenticCell using configuration objects (RECOMMENDED)
        /// Uses Parameter Objects pattern for cleaner API and easier testing
        /// @param noise Random noise source (owned by this cell)
        /// @param genome Genetic makeup (taken by value)
        /// @param instability Configuration for D1/D2 delta rates
        /// @param division Configuration for division rates
        /// @param thresholds Configuration for state transition thresholds
        /// @param logger Optional logger (default NullLogger)
        AgenticCell(std::unique_ptr<INoiseSource> noise,
                       Genome genome,
                       const InstabilityConfig& instability = {},
                       const DivisionConfig& division = {},
                       const ThresholdConfig& thresholds = {},
                       const ports::ILoggerPtr& logger = nullptr);

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

        // === ICell: Damage accumulators ===
        [[nodiscard]] double getD1() const override { return d1_dna_damage_; }
        [[nodiscard]] double getD2() const override { return d2_immunosuppression_; }

        // === ICell: Cell stage (derived on-the-fly from genetics + D1 + D2) ===
        [[nodiscard]] CellLifeStage getCurrentCellLifeStage() const override;

        // === ICell: Apoptosis evasion ===
        [[nodiscard]] bool hasEvadedApoptosis() const override { return has_evaded_apoptosis_; }

        // === ICell: Division ===
        /// Returns owned daughter cell after live() cycle, or nullptr if no division.
        std::unique_ptr<ICell> takePendingDaughter() override { return std::move(pending_daughter_); }

        // === Other public methods ===
        bool isNeoplasticProtected() const;
        std::uint64_t getSeed() const { return seed_; }
        std::uint64_t getAge() const { return age_; }
        std::unique_ptr<AgenticCell> clone() const;

        [[nodiscard]] const Genome& getGenome() const { return genome_; }

    private:
        // === Dependencies ===
        std::unique_ptr<INoiseSource> noise_;
        ports::ILoggerPtr logger_;
        std::function<void(std::unique_ptr<domain::ISignal>)> signal_emitter_;
        std::queue<std::unique_ptr<domain::ISignal>> incoming_messages_;
        Genome genome_;

        // === Neoplasm tracking ===
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

        // === Saturation limits for D1 and D2 ===
        double max_d1_ = 999.0;
        double max_d2_ = 999.0;

        // === Instability delta calculation strategy ===
        std::unique_ptr<IInstabilityDeltaStrategy> delta_strategy_;

        // === Viability calculation strategy ===
        std::unique_ptr<IViabilityStrategy> viability_strategy_;

        // === Pending daughter cell (for division without signals) ===
        std::unique_ptr<AgenticCell> pending_daughter_;

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
        std::unique_ptr<AgenticCell> attemptDivision();

        /// Calculate instability deltas (delta_d1, delta_d2) based on current genetic state
        /// @return pair<double, double> where first=delta_d1, second=delta_d2
        ///
        /// Decision matrix based on TP53 and BRCA1 status:
        /// - D1 (DNA damage) depends only on TP53
        /// - D2 (Immunosuppression) depends on both TP53 and BRCA1 (additive)
        ///
        /// Delegates to injected IInstabilityDeltaStrategy to calculate D1/D2 increments
        [[nodiscard]] InstabilityDeltas calculateInstabilityDeltas() const;

    };

} // namespace domain

