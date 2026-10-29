/**
 * @file TissueScaleSimulator.h
 * @brief Domain models, math functions, and stochastic simulation engine for
 * the tissue-scale BRCA1-ONSET model (10^7 cells, heterogeneous tissue restraint,
 * and somatic driver checkpoint relaxation).
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <unordered_map>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace tissue_scale {

/**
 * @brief Discrete state key identifying a distinct cell cohort within the tissue.
 */
struct CellCohortKey {
    uint8_t brca1_loss = 0;   ///< Somatic BRCA1 loss: 0 (wildtype/heterozygous carrier), 1 (homozygous null)
    uint8_t tp53_hits = 0;    ///< Somatic TP53 hits: 0, 1 (heterozygous), or 2 (biallelic hit)
    uint8_t driver_hit = 0;   ///< Somatic permissive driver (PTEN-like): 0 (none), 1 (acquired)
    uint8_t early_brca1_bypass = 0; ///< 1 if BRCA1 loss occurred before biallelic TP53 inactivation
    uint8_t cohort_age = 0;   ///< Cell cohort replicative age in years
    uint8_t d1_low_hits = 0;  ///< Number of low-level D1 damage accumulation events
    uint8_t d1_high_hits = 0; ///< Number of high-level D1 damage accumulation events
    uint8_t d2_low_hits = 0;  ///< Number of low-level D2 restraint damage events
    uint8_t d2_high_hits = 0; ///< Number of high-level D2 restraint damage events
    uint32_t d1_age_accum = 0;///< Cumulative age integral for D1 damage
    uint32_t d2_age_accum = 0;///< Cumulative age integral for D2 damage

    bool operator==(const CellCohortKey& other) const noexcept {
        return brca1_loss == other.brca1_loss &&
               tp53_hits == other.tp53_hits &&
               driver_hit == other.driver_hit &&
               early_brca1_bypass == other.early_brca1_bypass &&
               cohort_age == other.cohort_age &&
               d1_low_hits == other.d1_low_hits &&
               d1_high_hits == other.d1_high_hits &&
               d2_low_hits == other.d2_low_hits &&
               d2_high_hits == other.d2_high_hits &&
               d1_age_accum == other.d1_age_accum &&
               d2_age_accum == other.d2_age_accum;
    }
};

/**
 * @brief 64-bit FNV-1a state hasher for CellCohortKey.
 */
struct CellCohortHash {
    size_t operator()(const CellCohortKey& k) const noexcept {
        size_t hash = 1469598103934665603ULL;
        auto mix = [&](uint64_t val) {
            hash ^= val;
            hash *= 1099511628211ULL;
        };
        mix(k.brca1_loss);
        mix(k.tp53_hits);
        mix(k.driver_hit);
        mix(k.early_brca1_bypass);
        mix(k.cohort_age);
        mix(k.d1_low_hits);
        mix(k.d1_high_hits);
        mix(k.d2_low_hits);
        mix(k.d2_high_hits);
        mix(k.d1_age_accum);
        mix(k.d2_age_accum);
        return hash;
    }
};

/**
 * @brief Biological and tissue-scale model parameters.
 */
struct TissueSimulationParameters {
    double mu_tp53;          ///< Somatic TP53 mutation rate per cell per cycle
    double beta_brca1;       ///< Somatic BRCA1 loss-of-heterozygosity (LOH) rate
    double delta_low;        ///< Damage increment for heterozygous / moderate instability
    double delta_high;       ///< Damage increment for homozygous / high instability
    double theta_d1;         ///< Intrinsic cellular transformation barrier threshold (D1)
    double theta_d2_mean;    ///< Baseline tissue-restraint barrier threshold (D2)
    double tau_expansion;    ///< Post-transformation clonal doubling time in years
    double sigma_theta_d2;   ///< Log-normal standard deviation for inter-individual D2 variation
    double gamma_driver = 0.0;     ///< Transition probability for acquiring permissive driver (PTEN-like)
    double alpha_bypass = 0.0;     ///< Probability of surviving BRCA1 loss prior to biallelic TP53 loss
};

/**
 * @brief Represents a transformed founder lineage undergoing clonal expansion.
 */
struct FounderLineage {
    long double unmutated_cells = 0.0L; ///< Cells retaining functional BRCA1 allele (BRCA1+/-)
    long double null_cells = 0.0L;      ///< Cells with complete BRCA1 loss (BRCA1-/-)
    long double sum_d1_unmutated = 0.0L;///< Integrated D1 damage for unmutated sublineage
    bool enabled_by_checkpoint_relaxation = false; ///< True if lineage evolved via early BRCA1 loss bypass
};

/**
 * @brief Summary outcome metrics for a single virtual tissue simulation.
 */
struct TissueSimulationResult {
    int age_at_detection = -1;       ///< Age at which tumor reaches threshold (years), -1 if cancer-free at age 80
    uint64_t total_founders = 0;     ///< Total number of distinct cell transformation events
    uint64_t null_founders = 0;      ///< Number of transformation events occurring in BRCA1-null cells
    long double final_unmutated_cells = 0.0L; ///< Final tumor cells with retained BRCA1
    long double final_null_cells = 0.0L;      ///< Final tumor cells with BRCA1 loss
    int major_lineages_1pct = 0;     ///< Lineages comprising >= 1% of the final tumor mass
    int major_lineages_5pct = 0;     ///< Lineages comprising >= 5% of the final tumor mass
    double effective_founders = 0.0; ///< Inverse Simpson index: 1 / sum(f_i^2)
    bool has_checkpoint_relaxed_tumor = false; ///< Contains >=1 founder enabled by checkpoint relaxation
};

/**
 * @brief Evaluates cumulative intrinsic genomic damage D1.
 */
static inline double compute_d1(const CellCohortKey& k, const TissueSimulationParameters& p, double alpha_age) {
    return 1.0 + k.d1_low_hits * p.delta_low + k.d1_high_hits * p.delta_high + alpha_age * k.d1_age_accum;
}

/**
 * @brief Evaluates cumulative extrinsic tissue-restraint stress D2.
 */
static inline double compute_d2(const CellCohortKey& k, const TissueSimulationParameters& p, double alpha_age) {
    return 1.0 + k.d2_low_hits * p.delta_low + k.d2_high_hits * p.delta_high + alpha_age * k.d2_age_accum;
}

/**
 * @brief Fast binomial generator with Poisson approximation for large n and small p.
 */
static inline uint64_t sample_binomial(std::mt19937_64& rng, uint64_t n, double p) {
    if (n == 0 || p <= 0.0) return 0;
    if (p >= 1.0) return n;

    long double expected = static_cast<long double>(n) * p;
    if (n > 1000000ULL && p < 1e-4 && expected < 1e8L) {
        std::poisson_distribution<uint64_t> dist(static_cast<double>(expected));
        uint64_t sample = dist(rng);
        return (sample > n) ? n : sample;
    }

    std::binomial_distribution<uint64_t> dist(n, p);
    return dist(rng);
}

/**
 * @brief Fast Poisson generator with Gaussian approximation for large means.
 */
static inline uint64_t sample_poisson(std::mt19937_64& rng, long double mean) {
    if (mean <= 0.0L) return 0;
    if (mean > 1e8L) {
        std::normal_distribution<long double> dist(mean, std::sqrt(mean));
        long double sample = dist(rng);
        if (sample < 0.0L) sample = 0.0L;
        if (sample > static_cast<long double>(UINT64_MAX)) return UINT64_MAX;
        return static_cast<uint64_t>(std::llround(sample));
    }
    std::poisson_distribution<uint64_t> dist(static_cast<double>(mean));
    return dist(rng);
}

/**
 * @brief Simulates a single tissue cohort from birth to age max_years.
 */
inline TissueSimulationResult simulate_tissue(
    const TissueSimulationParameters& params,
    uint64_t seed,
    uint64_t initial_cells = 10000000ULL,
    int max_years = 80,
    uint64_t tumor_detection_threshold = 500000ULL,
    double basal_division_rate = 0.001,
    double age_damage_rate = 1e-5
) {
    std::mt19937_64 rng(seed);
    std::normal_distribution<double> normal_dist(0.0, 1.0);

    // Assign individual-specific tissue restraint barrier threshold theta_D2
    double individual_theta_d2 = params.theta_d2_mean;
    if (params.sigma_theta_d2 > 0.0) {
        individual_theta_d2 = params.theta_d2_mean * std::exp(
            params.sigma_theta_d2 * normal_dist(rng) - 0.5 * params.sigma_theta_d2 * params.sigma_theta_d2
        );
    }

    // Cohort populations: current year state and next year state
    std::unordered_map<CellCohortKey, uint64_t, CellCohortHash> state, next_state;
    state.reserve(12000);
    next_state.reserve(12000);

    // Initial state: N0 cells with zero mutations, zero driver, and zero damage
    state[{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}] = initial_cells;

    std::vector<FounderLineage> founder_lineages;
    founder_lineages.reserve(64);
    uint64_t total_founders = 0;
    uint64_t null_founders = 0;

    for (int year = 1; year <= max_years; ++year) {
        next_state.clear();

        // 1. Process mutation and damage transitions for pre-transformation cohorts
        for (const auto& [cohort_key, cohort_count] : state) {
            if (cohort_count == 0) continue;

            // Optional driver acquisition (PTEN-like)
            uint64_t n_new_drivers = 0;
            if (!cohort_key.driver_hit && params.gamma_driver > 0.0) {
                n_new_drivers = sample_binomial(rng, cohort_count, params.gamma_driver);
            }
            uint64_t n_no_drivers = cohort_count - n_new_drivers;

            for (int driver_branch = 0; driver_branch < 2; ++driver_branch) {
                uint64_t sub_count = (driver_branch == 0) ? n_no_drivers : n_new_drivers;
                if (sub_count == 0) continue;

                uint8_t curr_driver = cohort_key.driver_hit || (driver_branch == 1);

                double d1_val = compute_d1(cohort_key, params, age_damage_rate);
                double prob_brca1_loss = cohort_key.brca1_loss ? 0.0 : std::min(1.0, params.beta_brca1 * d1_val);
                double prob_tp53_hit = (cohort_key.tp53_hits >= 2) ? 0.0 : std::min(1.0, params.mu_tp53 * d1_val);

                uint64_t n_brca1_events = sample_binomial(rng, sub_count, prob_brca1_loss);
                uint64_t n_no_brca1_events = sub_count - n_brca1_events;

                uint64_t n_brca1_with_tp53 = sample_binomial(rng, n_brca1_events, prob_tp53_hit);
                uint64_t n_no_brca1_with_tp53 = sample_binomial(rng, n_no_brca1_events, prob_tp53_hit);

                uint64_t counts[4] = {
                    n_no_brca1_events - n_no_brca1_with_tp53,
                    n_no_brca1_with_tp53,
                    n_brca1_events - n_brca1_with_tp53,
                    n_brca1_with_tp53
                };
                int brca1_mut_mask[4] = {0, 0, 1, 1};
                int tp53_mut_mask[4]  = {0, 1, 0, 1};

                for (int group = 0; group < 4; ++group) {
                    uint64_t n_cells = counts[group];
                    if (n_cells == 0) continue;

                    int new_brca1_loss = cohort_key.brca1_loss || brca1_mut_mask[group];
                    int new_tp53_hits = std::min(2, static_cast<int>(cohort_key.tp53_hits) + tp53_mut_mask[group]);

                    // Checkpoint bypass logic: If a NEW BRCA1 loss event occurs before biallelic TP53 inactivation,
                    // cells undergo death unless permissive driver is present and bypasses checkpoint with probability alpha_bypass.
                    // If the cohort had already survived early bypass in a prior year, it remains viable.
                    bool is_new_brca1_loss = (!cohort_key.brca1_loss && new_brca1_loss);
                    bool early_brca1_event = (is_new_brca1_loss && new_tp53_hits < 2);
                    uint64_t surviving_cells = n_cells;
                    uint8_t bypassed_flag = cohort_key.early_brca1_bypass;

                    if (early_brca1_event) {
                        if (curr_driver && params.alpha_bypass > 0.0) {
                            surviving_cells = sample_binomial(rng, n_cells, params.alpha_bypass);
                            bypassed_flag = 1;
                        } else {
                            surviving_cells = 0;
                        }
                    } else if (new_brca1_loss && new_tp53_hits < 2 && !bypassed_flag) {
                        surviving_cells = 0; // Unviable without driver bypass
                    }
                    if (surviving_cells == 0) continue;

                    CellCohortKey updated_key = cohort_key;
                    updated_key.brca1_loss = static_cast<uint8_t>(new_brca1_loss);
                    updated_key.tp53_hits = static_cast<uint8_t>(new_tp53_hits);
                    updated_key.driver_hit = curr_driver;
                    updated_key.early_brca1_bypass = bypassed_flag;

                    if (new_tp53_hits == 1) {
                        updated_key.d1_low_hits++;
                        updated_key.d2_low_hits++;
                    } else if (new_tp53_hits == 2) {
                        updated_key.d1_high_hits++;
                        updated_key.d2_high_hits++;
                    }

                    if (new_brca1_loss) {
                        updated_key.d2_high_hits = static_cast<uint8_t>(std::min<int>(255, updated_key.d2_high_hits + 2));
                    } else {
                        updated_key.d2_low_hits++;
                    }

                    updated_key.d1_age_accum += cohort_key.cohort_age;
                    updated_key.d2_age_accum += cohort_key.cohort_age;

                    double d1_new = compute_d1(updated_key, params, age_damage_rate);
                    double d2_new = compute_d2(updated_key, params, age_damage_rate);

                    bool is_transformed = false;
                    if (new_tp53_hits == 2 && d1_new > params.theta_d1) {
                        if (d2_new > individual_theta_d2) {
                            is_transformed = true;
                        } else {
                            continue;
                        }
                    }

                    if (is_transformed) {
                        total_founders += surviving_cells;
                        if (new_brca1_loss) null_founders += surviving_cells;

                        if (surviving_cells > 1000000ULL) {
                            std::cerr << "Error: Uncontrolled explosive founder generation (" << surviving_cells << ")\n";
                            std::exit(3);
                        }

                        for (uint64_t j = 0; j < surviving_cells; ++j) {
                            FounderLineage lineage;
                            lineage.enabled_by_checkpoint_relaxation = (bypassed_flag == 1);
                            if (new_brca1_loss) {
                                lineage.null_cells = 1.0L;
                            } else {
                                lineage.unmutated_cells = 1.0L;
                                lineage.sum_d1_unmutated = d1_new;
                            }
                            founder_lineages.push_back(lineage);
                        }
                        continue;
                    }

                    uint64_t n_divisions = sample_binomial(rng, surviving_cells, basal_division_rate);
                    CellCohortKey aging_cohort = updated_key;
                    aging_cohort.cohort_age = static_cast<uint8_t>(std::min<int>(255, static_cast<int>(cohort_key.cohort_age) + 1));
                    next_state[aging_cohort] += surviving_cells;

                    if (n_divisions > 0) {
                        CellCohortKey daughter_cohort = updated_key;
                        daughter_cohort.cohort_age = 0;
                        next_state[daughter_cohort] += n_divisions;
                    }
                }
            }
        }

        // 2. Transformed founder lineage expansion
        const double growth_factor = std::pow(2.0, 1.0 / params.tau_expansion);
        const double birth_rate = growth_factor - 1.0;

        long double total_unmutated_mass = 0.0L;
        long double total_null_mass = 0.0L;

        for (auto& lineage : founder_lineages) {
            if (lineage.unmutated_cells > 0.0L) {
                double avg_d1 = static_cast<double>(lineage.sum_d1_unmutated / lineage.unmutated_cells);
                double prob_brca1_second_hit = std::min(1.0, params.beta_brca1 * avg_d1);

                uint64_t n_unmutated_int = (lineage.unmutated_cells > static_cast<long double>(UINT64_MAX))
                    ? UINT64_MAX : static_cast<uint64_t>(lineage.unmutated_cells);

                uint64_t n_second_hits = sample_binomial(rng, n_unmutated_int, prob_brca1_second_hit);
                if (n_second_hits > 0) {
                    long double loss_fraction = static_cast<long double>(n_second_hits) / lineage.unmutated_cells;
                    lineage.sum_d1_unmutated *= std::max(0.0L, 1.0L - loss_fraction);
                    lineage.unmutated_cells -= n_second_hits;
                    lineage.null_cells += n_second_hits;
                }
                if (lineage.unmutated_cells > 0.0L) {
                    lineage.sum_d1_unmutated += lineage.unmutated_cells * params.delta_high;
                }
            }

            uint64_t n_unmutated = (lineage.unmutated_cells > static_cast<long double>(UINT64_MAX))
                ? UINT64_MAX : static_cast<uint64_t>(lineage.unmutated_cells);
            uint64_t n_null = (lineage.null_cells > static_cast<long double>(UINT64_MAX))
                ? UINT64_MAX : static_cast<uint64_t>(lineage.null_cells);

            uint64_t births_unmutated = sample_poisson(rng, static_cast<long double>(n_unmutated) * birth_rate);
            uint64_t births_null = sample_poisson(rng, static_cast<long double>(n_null) * birth_rate);

            if (n_unmutated > 0 && births_unmutated > 0) {
                long double expansion_factor = static_cast<long double>(n_unmutated + births_unmutated) / static_cast<long double>(n_unmutated);
                lineage.sum_d1_unmutated *= expansion_factor;
            }

            lineage.unmutated_cells = static_cast<long double>(n_unmutated + births_unmutated);
            lineage.null_cells = static_cast<long double>(n_null + births_null);

            total_unmutated_mass += lineage.unmutated_cells;
            total_null_mass += lineage.null_cells;
        }

        long double total_tumor_cells = total_unmutated_mass + total_null_mass;

        // 3. Clinical detection check
        if (total_tumor_cells >= static_cast<long double>(tumor_detection_threshold)) {
            int major_lineages_1pct = 0;
            int major_lineages_5pct = 0;
            long double sum_squared_frequencies = 0.0L;
            bool has_relaxed = false;

            for (const auto& lineage : founder_lineages) {
                long double lineage_size = lineage.unmutated_cells + lineage.null_cells;
                if (lineage_size <= 0.0L) continue;

                long double frequency = lineage_size / total_tumor_cells;
                if (frequency >= 0.01L) major_lineages_1pct++;
                if (frequency >= 0.05L) {
                    major_lineages_5pct++;
                    if (lineage.enabled_by_checkpoint_relaxation) has_relaxed = true;
                }
                sum_squared_frequencies += frequency * frequency;
            }

            double effective_founders = (sum_squared_frequencies > 0.0L)
                ? static_cast<double>(1.0L / sum_squared_frequencies) : 0.0;

            return {
                year,
                total_founders,
                null_founders,
                total_unmutated_mass,
                total_null_mass,
                major_lineages_1pct,
                major_lineages_5pct,
                effective_founders,
                has_relaxed
            };
        }

        state.swap(next_state);
    }

    long double total_unmutated_mass = 0.0L;
    long double total_null_mass = 0.0L;
    for (const auto& lineage : founder_lineages) {
        total_unmutated_mass += lineage.unmutated_cells;
        total_null_mass += lineage.null_cells;
    }

    return {
        -1,
        total_founders,
        null_founders,
        total_unmutated_mass,
        total_null_mass,
        0,
        0,
        0.0,
        false
    };
}

} // namespace tissue_scale
