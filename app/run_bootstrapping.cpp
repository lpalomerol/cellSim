/// run_bootstrapping: Kaplan-Meier style bootstrap calibration for CellSim.
///
/// Runs N independent simulations (each with a different seed) to produce a
/// cumulative tumour-onset curve.  Results are compared against the BRCA1
/// cumulative breast-cancer risk data from Kuchenbaecker et al. (JAMA 2017).
///
/// Biological assumptions:
///   - All cells start as BRCA1 +/- (germline carrier, via makeDefaultGenome)
///   - All cells start as TP53 +/+ (wild-type)
///   - 1 tick = 1 year
///   - Tumour onset = first year where (neoplastic_alive / alive_cells) >= threshold
///
/// Usage:
///   ./run_bootstrapping [options]
///
/// Key options:
///   --n-runs N              Bootstrap seeds (default: 1000)
///   --tumor-threshold F     Neoplastic fraction for onset (default: 0.05 = 5%)
///   --n-cells N             Cells per tissue (default: 500)
///   --max-t N               Years of follow-up (default: 80)
///   --low-delta F           D1/D2 increment per tick, +/- genotype (default: 0.15)
///   --high-delta F          D1/D2 increment per tick, -/- genotype (default: 0.40)
///   --brca1-rate F          BRCA1 mutation rate per tick (default: 0.008)
///   --tp53-rate F           TP53 mutation rate per tick (default: 0.003)
///   --d1-threshold F        D1 level triggering PRIMER state (default: 2.0)
///   --d2-threshold F        D2 level triggering immune evasion (default: 5.0)
///   --output FILE           Per-run CSV output (default: bootstrapping_results.csv)
///   --verbose               Print per-seed progress

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <memory>
#include <cmath>
#ifdef BOOTSTRAPPING_USE_OMP
#  include <omp.h>
#endif

#include <cxxopts.hpp>

#include "../src/application/simulation/Simulation.h"
#include "../src/domain/cell/CellFactory.h"
#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/adapters/RandomNoise.h"
#include "../src/domain/adapters/NullLogger.h"
#include "../src/application/bootstrapping/BootstrappingMetrics.h"

namespace {

using application::bootstrapping::ClinicalPoint;
using application::bootstrapping::RunResult;
using application::bootstrapping::KUCHENBAECKER_BRCA1;
using application::bootstrapping::N_CLINICAL;
using application::bootstrapping::cumulativeRisk;
using application::bootstrapping::computeSSE;
using application::bootstrapping::computeWeightedSSE;
using application::bootstrapping::medianOnset;
using application::bootstrapping::medianSaturation;
using application::bootstrapping::makeCellSeed;

// ---------------------------------------------------------------------------
// Post-process tracker snapshots with a configurable fraction threshold.
// Returns the first year where neoplastic_alive/alive_cells >= threshold,
// or -1 if it never happens.
// ---------------------------------------------------------------------------
int onsetYear(const application::PopulationTracker& tracker, double threshold) {
    for (const auto& snap : tracker.snapshots()) {
        if (snap.alive_cells > 0) {
            double frac = static_cast<double>(snap.neoplastic_alive) / snap.alive_cells;
            if (frac >= threshold) return snap.year;
        }
    }
    return -1;
}

// ---------------------------------------------------------------------------
// Run one simulation with the given seed; return onset year and final %.
// ---------------------------------------------------------------------------
RunResult runOnce(int seed, int n_cells, int max_t,
                  double low_delta, double high_delta,
                  double brca1_rate, double tp53_rate,
                  double d1_threshold, double d2_threshold,
                  double tumor_threshold,
                  bool big_bang = false,
                  double neoplastic_div_rate = 0.1)
{
    auto logger = std::make_shared<domain::adapters::NullLogger>();
    application::Simulation sim(max_t);

    for (int i = 0; i < n_cells; ++i) {
        // makeDefaultGenome initialises BRCA1 +/- and TP53 +/+ by design.
        domain::Genome genome = domain::genome_factory::makeDefaultGenome(
            {{"BRCA1", brca1_rate}, {"TP53", tp53_rate}},
            {{"BRCA1", 0.0},        {"TP53", 0.0}},
            logger
        );

        // Unique seed per (run, cell) to ensure independence between runs.
        unsigned cell_seed = makeCellSeed(seed, i);
        auto noise = std::make_unique<domain::adapters::RandomNoise>(cell_seed);

        auto cell = domain::CellFactory::createCustomCell(
            std::move(noise), genome,
            low_delta, high_delta,
            0.001,                  // division_rate (normal)
            neoplastic_div_rate,    // neoplastic_division_rate
            big_bang,               // big_bang_mode
            d1_threshold, d2_threshold,
            logger,
            0.0                     // delta_noise_cv
        );
        sim.addCell(std::move(cell));
    }

    sim.run();

    const auto& tracker = sim.populationTracker();
    int onset = onsetYear(tracker, tumor_threshold);

    double final_neo_pct = 0.0;
    auto milestones = application::bootstrapping::computeMilestones(tracker.snapshots());

    if (!tracker.snapshots().empty()) {
        const auto& last = tracker.snapshots().back();
        if (last.alive_cells > 0)
            final_neo_pct = 100.0 * last.neoplastic_alive / last.alive_cells;
    }

    return {seed, onset, final_neo_pct, milestones.year_25, milestones.year_50, milestones.year_90};
}

// ---------------------------------------------------------------------------
// Run a batch of n_runs with a fixed parameter set; return all results.
// ---------------------------------------------------------------------------
std::vector<RunResult> runBatch(int n_runs, int n_cells, int max_t,
                                double low_delta, double high_delta,
                                double brca1_rate, double tp53_rate,
                                double d1_threshold, double d2_threshold,
                                double tumor_threshold,
                                bool big_bang = false,
                                double neoplastic_div_rate = 0.1)
{
    std::vector<RunResult> results(n_runs, {-1, -1, 0.0});
#ifdef BOOTSTRAPPING_USE_OMP
    #pragma omp parallel for schedule(dynamic, 4)
#endif
    for (int s = 0; s < n_runs; ++s)
        results[s] = runOnce(s, n_cells, max_t,
                             low_delta, high_delta,
                             brca1_rate, tp53_rate,
                             d1_threshold, d2_threshold,
                             tumor_threshold,
                             big_bang, neoplastic_div_rate);
    return results;
}

// ---------------------------------------------------------------------------
// Sweep mode: grid search over brca1_rate × low_delta.
// high_delta = high_delta_ratio * low_delta  (same convention as calibration_sweep).
// ---------------------------------------------------------------------------
struct SweepResult {
    double brca1_rate;
    double low_delta;
    double high_delta;
    double sse;
    double risk[7];   // at ages {21,30,40,50,60,70,80}
};

void runSweep(int sweep_n_runs, int n_cells, int max_t,
              double tp53_rate, double d1_threshold, double d2_threshold,
              double tumor_threshold, double high_delta_ratio,
              bool big_bang, double neoplastic_div_rate,
              const std::string& sweep_output,
              bool fine_grid = false,
              bool weighted_sse = false)
{
    // Coarse grid (Phase 3)
    static const double BRCA1_COARSE[] = {0.010, 0.020, 0.040, 0.060, 0.080, 0.100};
    static const double LD_COARSE[]    = {0.02,  0.04,  0.06,  0.08,  0.10,  0.15};
    // Fine grid (Phase 4) — centred on best coarse result (brca1=0.040, low_d=0.100–0.150)
    static const double BRCA1_FINE[]   = {0.030, 0.035, 0.040, 0.045, 0.050, 0.055};
    static const double LD_FINE[]      = {0.090, 0.100, 0.110, 0.120, 0.130, 0.140, 0.150};
    static constexpr int NB_C = 6, ND_C = 6;
    static constexpr int NB_F = 6, ND_F = 7;
    static constexpr int AGES[] = {21, 30, 40, 50, 60, 70, 80};

    const double* BRCA1_RATES = fine_grid ? BRCA1_FINE  : BRCA1_COARSE;
    const double* LOW_DELTAS  = fine_grid ? LD_FINE      : LD_COARSE;
    const int NB = fine_grid ? NB_F : NB_C;
    const int ND = fine_grid ? ND_F : ND_C;
    const int total_combos = NB * ND;
    int combo = 0;

    std::cout << "  Grid search: " << NB << " brca1_rates x " << ND << " low_deltas"
              << "  (" << total_combos << " combos x " << sweep_n_runs << " runs each)\n"
              << "  high_delta = " << high_delta_ratio << " * low_delta"
              << "  big_bang=" << (big_bang ? "true" : "false")
              << "  neoplastic_div_rate=" << neoplastic_div_rate
              << (fine_grid ? "  [FINE GRID]" : "  [COARSE GRID]")
              << (weighted_sse ? "  [WEIGHTED SSE]" : "  [UNWEIGHTED SSE]") << "\n\n";

    std::vector<SweepResult> sweep_results;
    sweep_results.reserve(total_combos);

    for (int bi = 0; bi < NB; ++bi) {
        for (int di = 0; di < ND; ++di) {
            ++combo;
            double br = BRCA1_RATES[bi];
            double ld = LOW_DELTAS[di];
            double hd = high_delta_ratio * ld;

            std::cout << "  [" << std::setw(2) << combo << "/" << total_combos << "]"
                      << "  brca1=" << std::fixed << std::setprecision(3) << br
                      << "  low_d=" << ld
                      << "  high_d=" << std::setprecision(3) << hd
                      << "  ... " << std::flush;

            auto results = runBatch(sweep_n_runs, n_cells, max_t,
                                    ld, hd, br, tp53_rate,
                                    d1_threshold, d2_threshold, tumor_threshold,
                                    big_bang, neoplastic_div_rate);

            SweepResult sr;
            sr.brca1_rate = br;
            sr.low_delta  = ld;
            sr.high_delta = hd;
            sr.sse        = weighted_sse ? computeWeightedSSE(results) : computeSSE(results);
            for (int i = 0; i < 7; ++i)
                sr.risk[i] = cumulativeRisk(results, AGES[i]);

            std::cout << "SSE=" << std::fixed << std::setprecision(1) << sr.sse << "\n";
            sweep_results.push_back(sr);
        }
    }

    // Sort by SSE ascending
    std::sort(sweep_results.begin(), sweep_results.end(),
              [](const SweepResult& a, const SweepResult& b){ return a.sse < b.sse; });

    // Write CSV
    {
        std::ofstream csv(sweep_output);
        csv << "brca1_rate,low_delta,high_delta," << (weighted_sse ? "sse_weighted" : "sse");
        for (int a : AGES) csv << ",risk_" << a;
        csv << "\n";
        for (const auto& sr : sweep_results) {
            csv << std::fixed << std::setprecision(4)
                << sr.brca1_rate << "," << sr.low_delta << "," << sr.high_delta << ","
                << std::setprecision(2) << sr.sse;
            for (int i = 0; i < 7; ++i)
                csv << "," << std::setprecision(1) << sr.risk[i];
            csv << "\n";
        }
    }
    std::cout << "\n  Sweep CSV written -> " << sweep_output << "\n";

    // Print top-5
    std::cout << "\n  Top-5 combos by SSE:\n";
    std::cout << "  " << std::string(72, '-') << "\n";
    std::cout << "  " << std::left
              << std::setw(10) << "brca1"
              << std::setw(10) << "low_d"
              << std::setw(10) << "high_d"
              << std::setw(10) << "SSE"
              << std::setw(8) << "r30" << std::setw(8) << "r40"
              << std::setw(8) << "r50" << std::setw(8) << "r60"
              << std::setw(8) << "r70" << std::setw(8) << "r80\n";
    std::cout << "  " << std::string(72, '-') << "\n";

    int top = std::min(5, (int)sweep_results.size());
    for (int i = 0; i < top; ++i) {
        const auto& sr = sweep_results[i];
        std::cout << "  " << std::left << std::fixed
                  << std::setw(10) << std::setprecision(3) << sr.brca1_rate
                  << std::setw(10) << std::setprecision(3) << sr.low_delta
                  << std::setw(10) << std::setprecision(3) << sr.high_delta
                  << std::setw(10) << std::setprecision(1) << sr.sse;
        // risk at ages 30,40,50,60,70,80 (indices 1-6)
        for (int j = 1; j <= 6; ++j)
            std::cout << std::setw(8) << std::setprecision(1) << sr.risk[j];
        std::cout << "\n";
    }
    std::cout << "\n  Kuchenbaecker target:               "
              << "  4.0   26.0  46.0  58.0  65.0  70.0\n";
    std::cout << "  " << std::string(72, '-') << "\n";
}

} // anonymous namespace

// ===========================================================================
// main
// ===========================================================================
int main(int argc, char* argv[]) {

    cxxopts::Options opts("run_bootstrapping",
        "Bootstrap calibration of CellSim vs. Kuchenbaecker 2017 BRCA1 curve");

    opts.add_options()
        ("n-runs",          "Number of bootstrap seeds",
            cxxopts::value<int>()->default_value("1000"))
        ("tumor-threshold", "Neoplastic fraction threshold for onset (0–1)",
            cxxopts::value<double>()->default_value("0.05"))
        ("n-cells",         "Cells per simulated tissue",
            cxxopts::value<int>()->default_value("500"))
        ("max-t",           "Years of follow-up",
            cxxopts::value<int>()->default_value("80"))
        ("low-delta",       "D1/D2 delta per tick for +/- genotype",
            cxxopts::value<double>()->default_value("0.15"))
        ("high-delta",      "D1/D2 delta per tick for -/- genotype",
            cxxopts::value<double>()->default_value("0.40"))
        ("brca1-rate",      "BRCA1 mutation rate per tick",
            cxxopts::value<double>()->default_value("0.008"))
        ("tp53-rate",       "TP53 mutation rate per tick",
            cxxopts::value<double>()->default_value("0.003"))
        ("d1-threshold",    "D1 level triggering PRIMER state",
            cxxopts::value<double>()->default_value("2.0"))
        ("d2-threshold",    "D2 level triggering immune evasion (TUMORAL)",
            cxxopts::value<double>()->default_value("5.0"))
        ("output",          "Per-run CSV output path",
            cxxopts::value<std::string>()->default_value("bootstrapping_results.csv"))
        ("sweep",           "Run parameter grid search (brca1_rate x low_delta)")
        ("fine-sweep",      "Run fine-grained grid search around best Phase 3 region")
        ("sweep-output",    "Sweep results CSV path",
            cxxopts::value<std::string>()->default_value("sweep_results.csv"))
        ("sweep-n-runs",    "Runs per combo in sweep mode",
            cxxopts::value<int>()->default_value("200"))
        ("high-delta-ratio","high_delta = ratio * low_delta (sweep mode)",
            cxxopts::value<double>()->default_value("2.0"))
        ("weighted-sse",     "Use weighted SSE (1/σᵢ²) for calibration — σᵢ from Kuchenbaecker 95% CI")
        ("big-bang",        "Enable big bang mode (neoplastic cells divide faster)")
        ("neoplastic-div-rate", "Neoplastic division rate when big bang is on",
            cxxopts::value<double>()->default_value("0.1"))
        ("verbose,v",       "Print per-seed progress")
        ("h,help",          "Show help");

    auto args = opts.parse(argc, argv);
    if (args.count("help")) { std::cout << opts.help(); return 0; }

    const int    n_runs          = args["n-runs"].as<int>();
    const double tumor_threshold = args["tumor-threshold"].as<double>();
    const int    n_cells         = args["n-cells"].as<int>();
    const int    max_t           = args["max-t"].as<int>();
    const double low_delta       = args["low-delta"].as<double>();
    const double high_delta      = args["high-delta"].as<double>();
    const double brca1_rate      = args["brca1-rate"].as<double>();
    const double tp53_rate       = args["tp53-rate"].as<double>();
    const double d1_threshold    = args["d1-threshold"].as<double>();
    const double d2_threshold    = args["d2-threshold"].as<double>();
    const std::string output     = args["output"].as<std::string>();
    const bool verbose           = args.count("verbose") > 0;
    const bool sweep_mode        = args.count("sweep") > 0;
    const bool   fine_sweep        = args.count("fine-sweep") > 0;
    const std::string sweep_out  = args["sweep-output"].as<std::string>();
    const int  sweep_n_runs      = args["sweep-n-runs"].as<int>();
    const double high_delta_ratio= args["high-delta-ratio"].as<double>();
    const bool   big_bang        = args.count("big-bang") > 0;
    const bool   weighted_sse    = args.count("weighted-sse") > 0;
    const double neoplastic_div_rate = args["neoplastic-div-rate"].as<double>();

    // --- Header ---
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout <<   "║  CellSim Bootstrap Calibration — Kuchenbaecker 2017 BRCA1   ║\n";
    std::cout <<   "╚══════════════════════════════════════════════════════════════╝\n";

    // -----------------------------------------------------------------------
    // SWEEP MODE
    // -----------------------------------------------------------------------
    if (sweep_mode || fine_sweep) {
        std::cout << "  Mode: SWEEP  threshold=" << (tumor_threshold * 100.0) << "%"
                  << "  n_cells=" << n_cells << "  max_t=" << max_t
                  << "  tp53_rate=" << tp53_rate << "\n\n";
        runSweep(sweep_n_runs, n_cells, max_t,
                 tp53_rate, d1_threshold, d2_threshold,
                 tumor_threshold, high_delta_ratio,
                 big_bang, neoplastic_div_rate, sweep_out,
                 fine_sweep, weighted_sse);
        return 0;
    }

    // -----------------------------------------------------------------------
    // SINGLE-CONFIG MODE
    // -----------------------------------------------------------------------
    // SINGLE-CONFIG MODE
    // -----------------------------------------------------------------------
    std::cout << "  n_runs=" << n_runs
              << "  n_cells=" << n_cells
              << "  max_t=" << max_t
              << "  threshold=" << (tumor_threshold * 100.0) << "%\n";
    std::cout << "  low_delta=" << low_delta
              << "  high_delta=" << high_delta
              << "  brca1_rate=" << brca1_rate
              << "  tp53_rate=" << tp53_rate << "\n";
    std::cout << "  d1_threshold=" << d1_threshold
              << "  d2_threshold=" << d2_threshold << "\n\n";

    // --- Bootstrap loop ---
    auto results = runBatch(n_runs, n_cells, max_t,
                            low_delta, high_delta,
                            brca1_rate, tp53_rate,
                            d1_threshold, d2_threshold,
                            tumor_threshold,
                            big_bang, neoplastic_div_rate);

    if (verbose) std::cout << "  done.\n";

    // --- Write per-run CSV ---
    {
        std::ofstream csv(output);
        csv << "seed,onset_year,final_neoplastic_pct,year_25pct,year_50pct,year_90pct\n";
        for (const auto& r : results)
            csv << r.seed << ","
                << r.onset_year << ","
                << std::fixed << std::setprecision(2) << r.final_neoplastic_pct << ","
                << r.year_25pct << ","
                << r.year_50pct << ","
                << r.year_90pct << "\n";
    }
    std::cout << "  Per-run CSV written → " << output << "\n\n";

    // --- Cumulative risk table ---
    static constexpr int CHECK_AGES[] = {21, 30, 40, 50, 60, 70, 80};

    std::cout << "  Cumulative Risk vs. Kuchenbaecker 2017 (BRCA1):\n";
    std::cout << "  " << std::string(62, '-') << "\n";
    std::cout << "  " << std::left
              << std::setw(7)  << "Age"
              << std::setw(14) << "Simulated %"
              << std::setw(14) << "Clinical %"
              << "95% CI\n";
    std::cout << "  " << std::string(62, '-') << "\n";

    for (int age : CHECK_AGES) {
        double sim_risk = cumulativeRisk(results, age);

        // Find matching Kuchenbaecker anchor point (if any)
        const ClinicalPoint* kp = nullptr;
        for (const auto& k : KUCHENBAECKER_BRCA1)
            if (k.age == age) { kp = &k; break; }

        std::cout << "  " << std::left
                  << std::setw(7) << age
                  << std::setw(14) << std::fixed << std::setprecision(1) << sim_risk;
        if (kp) {
            std::cout << std::setw(14) << kp->risk
                      << "[" << kp->ci_lo << "–" << kp->ci_hi << "%]";
        } else {
            std::cout << std::setw(14) << "—" << "—";
        }
        std::cout << "\n";
    }
    std::cout << "  " << std::string(62, '-') << "\n";

    // Compute saturation milestones
    std::vector<int> sat25, sat50, sat90;
    sat25.reserve(results.size());
    sat50.reserve(results.size());
    sat90.reserve(results.size());
    for (const auto& r : results) {
        sat25.push_back(r.year_25pct);
        sat50.push_back(r.year_50pct);
        sat90.push_back(r.year_90pct);
    }

    std::cout << "  SSE vs. Kuchenbaecker = "
              << std::fixed << std::setprecision(2) << computeSSE(results)
              << "  (weighted SSE = " << std::setprecision(2) << computeWeightedSSE(results) << ")"
              << "  (median onset = " << std::setprecision(1) << medianOnset(results) << ")"
              << "  (median sat25 = " << std::setprecision(1) << medianSaturation(sat25) << ")"
              << "  (median sat50 = " << std::setprecision(1) << medianSaturation(sat50) << ")"
              << "  (median sat90 = " << std::setprecision(1) << medianSaturation(sat90) << ")\n\n";

    return 0;
}
