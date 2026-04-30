/// calibration_sweep: parameter sweep to empirically calibrate D1/D2 thresholds.
///
/// Simplification applied (PI-agreed):
///   δ_high = HIGH_DELTA_RATIO * δ_low     (default 2.0)
///   θ_D2   = D2_D1_RATIO      * θ_D1     (default 2.5)
///
/// Free variables swept:
///   δ_low  in [low_delta_min, low_delta_max] (log scale, low_delta_steps points)
///   θ_D1   in [d1_min, d1_max]              (linear,   d1_steps points)
///
/// Per grid point: runs N simulations with different random seeds.
///
/// Output CSV columns:
///   low_delta, d1_threshold, d2_threshold, n_runs,
///   median_onset, p10_onset, p90_onset, pct_transformed_by_max_t
///   (onset = -1 means no neoplastic transformation in that run)

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <memory>
#include <limits>

#include <cxxopts.hpp>

#include "../src/application/simulation/Simulation.h"
#include "../src/domain/cell/CellFactory.h"
#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/adapters/RandomNoise.h"
#include "../src/domain/adapters/NullLogger.h"

namespace {

/// Build and run one simulation; return firstTimeNeoplastic() or -1.
int runOnce(double low_delta, double d1_threshold, double d2_threshold,
            int n_cells, int max_t, unsigned base_seed, double delta_cv) {
    auto logger = std::make_shared<domain::adapters::NullLogger>();
    application::Simulation sim(max_t);

    for (int i = 0; i < n_cells; ++i) {
        domain::Genome genome = domain::genome_factory::makeDefaultGenome(
            {{"BRCA1", 0.1}, {"TP53", 0.1}},
            {{"BRCA1", 0.0}, {"TP53", 0.0}},
            logger
        );

        unsigned seed = base_seed + static_cast<unsigned>(i);
        auto noise = std::make_unique<domain::adapters::RandomNoise>(seed);

        auto cell = domain::CellFactory::createCustomCell(
            std::move(noise),
            genome,
            low_delta,
            low_delta * 2.0,   // δ_high = 2 * δ_low
            0.001,             // division_rate
            0.001,             // neoplastic_division_rate
            false,             // big_bang_mode
            d1_threshold,
            d2_threshold,
            logger,
            delta_cv
        );

        sim.addCell(std::move(cell));
    }

    sim.run();
    return sim.firstTimeNeoplastic();
}

/// Compute median of a sorted vector (may contain -1 = no event).
double median(std::vector<int>& v) {
    if (v.empty()) return -1.0;
    std::sort(v.begin(), v.end());
    std::size_t n = v.size();
    if (n % 2 == 0) return (v[n/2 - 1] + v[n/2]) / 2.0;
    return static_cast<double>(v[n/2]);
}

double percentile(std::vector<int>& sorted_v, double p) {
    if (sorted_v.empty()) return -1.0;
    std::size_t idx = static_cast<std::size_t>(p / 100.0 * (sorted_v.size() - 1));
    return static_cast<double>(sorted_v[idx]);
}

/// Generate N log-spaced values between lo and hi.
std::vector<double> logspace(double lo, double hi, int n) {
    std::vector<double> v(n);
    if (n == 1) { v[0] = lo; return v; }
    double log_lo = std::log10(lo), log_hi = std::log10(hi);
    for (int i = 0; i < n; ++i) {
        v[i] = std::pow(10.0, log_lo + i * (log_hi - log_lo) / (n - 1));
    }
    return v;
}

/// Generate N linear-spaced values between lo and hi.
std::vector<double> linspace(double lo, double hi, int n) {
    std::vector<double> v(n);
    if (n == 1) { v[0] = lo; return v; }
    for (int i = 0; i < n; ++i) {
        v[i] = lo + i * (hi - lo) / (n - 1);
    }
    return v;
}

} // anonymous namespace

int main(int argc, char* argv[]) {
    cxxopts::Options opts("calibration_sweep",
        "Parameter sweep for D1/D2 calibration (δ_high=2*δ_low, θ_D2=ratio*θ_D1)");

    opts.add_options()
        ("low-delta-min",   "Min δ_low",              cxxopts::value<double>()->default_value("0.02"))
        ("low-delta-max",   "Max δ_low",              cxxopts::value<double>()->default_value("0.5"))
        ("low-delta-steps", "Steps for δ_low (log scale)", cxxopts::value<int>()->default_value("8"))
        ("d1-min",          "Min θ_D1",               cxxopts::value<double>()->default_value("1.2"))
        ("d1-max",          "Max θ_D1",               cxxopts::value<double>()->default_value("4.0"))
        ("d1-steps",        "Steps for θ_D1 (linear)", cxxopts::value<int>()->default_value("8"))
        ("d2-ratio",        "θ_D2 = ratio * θ_D1",    cxxopts::value<double>()->default_value("2.5"))
        ("high-delta-ratio","δ_high = ratio * δ_low",  cxxopts::value<double>()->default_value("2.0"))
        ("delta-cv",        "Lognormal noise CV for D1/D2 deltas (0=deterministic)", cxxopts::value<double>()->default_value("0.0"))
        ("runs",            "Repetitions per grid point", cxxopts::value<int>()->default_value("100"))
        ("cells",           "Initial cells per run",   cxxopts::value<int>()->default_value("100"))
        ("max-t",           "Simulation years",        cxxopts::value<int>()->default_value("100"))
        ("output",          "Output CSV file (default: stdout)", cxxopts::value<std::string>()->default_value(""))
        ("h,help",          "Print help");

    auto result = opts.parse(argc, argv);
    if (result.count("help")) {
        std::cout << opts.help() << "\n";
        return 0;
    }

    double low_delta_min   = result["low-delta-min"].as<double>();
    double low_delta_max   = result["low-delta-max"].as<double>();
    int    low_delta_steps = result["low-delta-steps"].as<int>();
    double d1_min          = result["d1-min"].as<double>();
    double d1_max          = result["d1-max"].as<double>();
    int    d1_steps        = result["d1-steps"].as<int>();
    double d2_ratio        = result["d2-ratio"].as<double>();
    double delta_cv        = result["delta-cv"].as<double>();
    int    runs            = result["runs"].as<int>();
    int    n_cells         = result["cells"].as<int>();
    int    max_t           = result["max-t"].as<int>();
    std::string output_path = result["output"].as<std::string>();

    auto low_deltas   = logspace(low_delta_min, low_delta_max, low_delta_steps);
    auto d1_thresholds = linspace(d1_min, d1_max, d1_steps);

    int total_points = low_delta_steps * d1_steps;
    std::cerr << "Grid: " << low_delta_steps << " x " << d1_steps
              << " = " << total_points << " points"
              << ", " << runs << " runs each"
              << ", " << n_cells << " cells"
              << ", " << max_t << " years\n";
    std::cerr << "Total simulations: " << total_points * runs << "\n";

    std::ostream* out_ptr = &std::cout;
    std::ofstream file_out;
    if (!output_path.empty()) {
        file_out.open(output_path);
        if (!file_out.is_open()) {
            std::cerr << "Error: cannot open output file: " << output_path << "\n";
            return 1;
        }
        out_ptr = &file_out;
        std::cerr << "Writing to: " << output_path << "\n";
    }
    std::ostream& out = *out_ptr;

    // CSV header
    out << "low_delta,high_delta,d1_threshold,d2_threshold,"
        << "n_runs,median_onset,p10_onset,p90_onset,pct_transformed\n";

    int completed = 0;
    for (double low_delta : low_deltas) {
        for (double d1_threshold : d1_thresholds) {
            double d2_threshold = d2_ratio * d1_threshold;

            std::vector<int> onsets;
            onsets.reserve(runs);
            int transformed = 0;

            for (int r = 0; r < runs; ++r) {
                unsigned seed = static_cast<unsigned>(r * 9973 + 1);
                int onset = runOnce(low_delta, d1_threshold, d2_threshold,
                                    n_cells, max_t, seed, delta_cv);
                onsets.push_back(onset);
                if (onset != -1) ++transformed;
            }

            // Sort for percentile computation (keep -1 at front — no-event runs)
            std::sort(onsets.begin(), onsets.end());

            // For median/percentile, exclude -1 (no transformation) runs
            std::vector<int> event_onsets;
            for (int o : onsets) { if (o != -1) event_onsets.push_back(o); }

            double med  = event_onsets.empty() ? -1.0 : median(event_onsets);
            double p10  = event_onsets.empty() ? -1.0 : percentile(event_onsets, 10.0);
            double p90  = event_onsets.empty() ? -1.0 : percentile(event_onsets, 90.0);
            double pct  = static_cast<double>(transformed) / runs * 100.0;

            out << std::fixed
                << low_delta    << ","
                << (low_delta * 2.0) << ","
                << d1_threshold << ","
                << d2_threshold << ","
                << runs         << ","
                << med          << ","
                << p10          << ","
                << p90          << ","
                << pct          << "\n";

            ++completed;
            std::cerr << "\r  Progress: " << completed << "/" << total_points
                      << " (" << static_cast<int>(100.0 * completed / total_points) << "%)"
                      << std::flush;
        }
    }
    std::cerr << "\nDone.\n";

    return 0;
}
