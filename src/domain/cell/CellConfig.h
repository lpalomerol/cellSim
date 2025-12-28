#pragma once

namespace domain {

    /// Configuration for cell instability parameters
    /// Groups delta (rate of increase) for D1 and D2 counters
    struct InstabilityConfig {
        /// Delta for heterozygous mutations (e.g., TP53 +/-)
        double low_delta = 0.001;

        /// Delta for homozygous recessive mutations (e.g., TP53 -/-)
        double high_delta = 0.003;

        /// Maximum value for D1 (DNA damage) - prevents overflow
        /// Default 999.0 acts as saturation limit
        double max_d1 = 999.0;

        /// Maximum value for D2 (immunosuppression) - prevents overflow
        /// Default 999.0 acts as saturation limit
        double max_d2 = 999.0;
    };

    /// Configuration for cell division rates
    /// Groups parameters controlling how frequently cells divide
    struct DivisionConfig {
        /// Probability of normal cell division per cycle (default 0.001)
        double base_rate = 0.001;

        /// Probability of neoplastic cell division per cycle (default 0.001)
        double neoplastic_rate = 0.001;

        /// Enable Big Bang mode: neoplastic cells divide faster (default false)
        bool enable_big_bang = false;
    };

    /// Configuration for cell threshold values
    /// Groups thresholds that control cell state transitions and fate
    struct ThresholdConfig {
        /// D1 threshold to enter PRIMER state (neoplastic checkpoint)
        /// When D1 > this value and cell is in certain genotypes, cell becomes PRIMER stage
        double d1_primer = 2.0;

        /// D2 threshold for apoptosis resistance
        /// When D2 > this value, cell evades extrinsic apoptosis signals
        /// Also triggers transformation to neoplastic when cell is in PRIMER stage
        double d2_apoptosis = 5.0;

        /// Base probability threshold for neoplastic transformation
        /// Initial value for neoplasm threshold (increases with age/divisions)
        double neoplasm_k = 0.002;
    };

} // namespace domain

