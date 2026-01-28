#include <iostream>
#include <cxxopts.hpp>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <chrono>
#include <limits>
#include <iomanip>
#include <sstream>
#include <memory>
#include "../src/application/config/JsonConfigLoader.h"
#include "../src/application/simulation/PopulationTracker.h"
#include "../src/domain/tissue/Tissue.h"
#include "../src/domain/cell/CellFactory.h"
#include "../src/domain/cell/AgenticCell.h"
#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/adapters/RandomNoise.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

// Función para capturar snapshot del tejido (igual que run_all_scenarios)
void captureSnapshotFromTissue(domain::Tissue* tissue, int year,
                              application::PopulationTracker& tracker,
                              int* previous_alive,
                              int* cumulative_dead) {
    int alive_count = 0;
    int neoplastic_alive = 0;
    int protected_alive = 0;
    int neoplastic_susceptible = 0;
    int neoplastic_resistant = 0;

    double min_d1 = std::numeric_limits<double>::max();
    double max_d1 = std::numeric_limits<double>::lowest();
    double min_d2 = std::numeric_limits<double>::max();
    double max_d2 = std::numeric_limits<double>::lowest();

    int tp53_pp = 0, tp53_pm = 0, tp53_mm = 0;

    for (size_t i = 0; i < tissue->size(); ++i) {
        auto* cell = tissue->getCell(i);
        if (cell && cell->alive()) {
            alive_count++;

            std::string tp53_status = cell->getTP53Status();
            if (tp53_status == "+/+") tp53_pp++;
            else if (tp53_status == "+/-") tp53_pm++;
            else if (tp53_status == "-/-") tp53_mm++;

            auto* agentic_v2 = dynamic_cast<domain::AgenticCell*>(cell);
            if (agentic_v2) {
                double d1 = agentic_v2->getD1();
                double d2 = agentic_v2->getD2();

                min_d1 = std::min(min_d1, d1);
                max_d1 = std::max(max_d1, d1);
                min_d2 = std::min(min_d2, d2);
                max_d2 = std::max(max_d2, d2);
            }

            if (cell->isNeoplastic()) {
                neoplastic_alive++;
                if (agentic_v2) {
                    if (agentic_v2->getD2() >= 5.0) {
                        neoplastic_resistant++;
                    } else {
                        neoplastic_susceptible++;
                    }
                }
            } else {
                protected_alive++;
            }
        }
    }

    if (alive_count == 0) {
        min_d1 = 0.0; max_d1 = 0.0;
        min_d2 = 0.0; max_d2 = 0.0;
    } else if (min_d1 == std::numeric_limits<double>::max()) {
        min_d1 = 0.0; max_d1 = 0.0;
        min_d2 = 0.0; max_d2 = 0.0;
    }

    int dead_this_year = 0;
    if (year > 0) {
        dead_this_year = *previous_alive - alive_count;
        if (dead_this_year < 0) dead_this_year = 0;
    }
    *previous_alive = alive_count;
    *cumulative_dead += dead_this_year;

    int total = alive_count + *cumulative_dead;
    int total_tp53 = tp53_pp + tp53_pm + tp53_mm;
    double tp53_pp_pct = (total_tp53 > 0) ? static_cast<double>(tp53_pp) / total_tp53 : 0.0;
    double tp53_pm_pct = (total_tp53 > 0) ? static_cast<double>(tp53_pm) / total_tp53 : 0.0;
    double tp53_mm_pct = (total_tp53 > 0) ? static_cast<double>(tp53_mm) / total_tp53 : 0.0;

    application::YearlySnapshot snap{
        year, total, alive_count, *cumulative_dead, neoplastic_alive, protected_alive,
        min_d1, max_d1, min_d2, max_d2,
        tp53_pp_pct, tp53_pm_pct, tp53_mm_pct,
        neoplastic_susceptible, neoplastic_resistant
    };

    tracker.addSnapshot(snap);
}

int main(int argc, char* argv[]) {
    try {
        cxxopts::Options options("cellSim_cli", "Cell Evolution Simulator - JSON Configuration Mode");

        options.add_options()
            ("c,config", "JSON configuration file (required)", cxxopts::value<std::string>())
            ("o,output", "Output directory for CSV/MD files (default: traces/cli)", cxxopts::value<std::string>()->default_value("traces/cli"))
            ("m,max-t", "Simulation years (override)", cxxopts::value<int>())
            ("n,cells", "Number of cells (override)", cxxopts::value<int>())
            ("s,seed", "Random seed, -1=random (override)", cxxopts::value<long>())
            ("k,neoplasm-k", "Base neoplasm probability 0.0-1.0 (override)", cxxopts::value<double>())
            ("v,verbose", "Enable detailed logging (override)", cxxopts::value<bool>()->default_value("false"))
            ("d,division-rate", "Cell division rate (override)", cxxopts::value<double>())
            ("h,help", "Show help");

        auto result = options.parse(argc, argv);

        if (result.count("help") || argc == 1) {
            std::cout << options.help() << std::endl;
            return 0;
        }

        if (!result.count("config")) {
            std::cerr << "Error: --config argument is required\n\n";
            std::cout << options.help() << std::endl;
            return 1;
        }

        // Load configuration
        std::string config_path = result["config"].as<std::string>();
        std::string output_dir = result["output"].as<std::string>();

        std::cout << "Loading configuration from: " << config_path << "\n" << std::endl;

        auto cfg = application::JsonConfigLoader::loadFromFile(config_path);

        // Apply CLI overrides
        if (result.count("max-t")) cfg.max_t = result["max-t"].as<int>();
        if (result.count("cells")) cfg.n_cells = result["cells"].as<int>();
        if (result.count("seed")) cfg.seed = result["seed"].as<long>();
        if (result.count("neoplasm-k")) cfg.neoplasm_k = result["neoplasm-k"].as<double>();
        if (result.count("verbose")) {
            cfg.verbose = result["verbose"].as<bool>();
            cfg.logger->setVerbose(cfg.verbose);
        }
        if (result.count("division-rate")) cfg.division_rate = result["division-rate"].as<double>();

        // Print configuration
        std::cout << "=== Simulation Configuration ===" << std::endl;
        std::cout << "\n[Basic Config]" << std::endl;
        std::cout << "  Seed: " << cfg.seed << std::endl;
        std::cout << "  Verbose mode: " << (cfg.verbose ? "enabled" : "disabled") << std::endl;
        std::cout << "  Random noise: " << (cfg.use_random_noise ? "yes" : "no") << std::endl;

        std::cout << "\n[Simulation Context]" << std::endl;
        std::cout << "  Cells: " << cfg.n_cells << std::endl;
        std::cout << "  Maximum time (years): " << cfg.max_t << std::endl;

        std::cout << "\n[Tissue Parameters]" << std::endl;
        std::cout << "  Neoplasm probability: " << cfg.neoplasm_k << std::endl;
        std::cout << "  Normal division rate: " << cfg.division_rate << std::endl;
        std::cout << "  Neoplastic division rate: " << cfg.neoplastic_division_rate << std::endl;
        std::cout << "  Big Bang mode: " << (cfg.enable_big_bang_mode ? "enabled" : "disabled") << std::endl;

        std::cout << "\n  Configured genes:" << std::endl;
        for (const auto& [gene, mutation_rate] : cfg.gene_thresholds) {
            double instability_rate = cfg.gene_instability_k.at(gene);
            std::cout << "    - " << gene << ": mutation=" << mutation_rate
                      << ", instability=" << instability_rate << std::endl;
        }

        std::cout << "\n[Output]" << std::endl;
        std::cout << "  Directory: " << output_dir << std::endl;
        std::cout << "\n" << std::string(50, '=') << "\n" << std::endl;

        // Create output directory
        fs::create_directories(output_dir);

        // Start simulation
        std::cout << "Starting simulation..." << std::flush;
        auto start_time = std::chrono::high_resolution_clock::now();

        // Create Tissue
        auto tissue = std::make_unique<domain::Tissue>(cfg.logger);

        // Create cells
        for (int i = 0; i < cfg.n_cells; ++i) {
            domain::Genome genome = domain::genome_factory::makeDefaultGenome(
                cfg.gene_thresholds,
                cfg.gene_instability_k,
                cfg.logger
            );

            unsigned unique_seed = (cfg.seed == -1) ? (100 + i) : (static_cast<unsigned>(cfg.seed) + i);
            std::unique_ptr<domain::INoiseSource> noise;
            if (cfg.use_random_noise) {
                noise = std::make_unique<domain::adapters::RandomNoise>(unique_seed);
            } else {
                noise = std::make_unique<domain::adapters::RandomNoise>(42);
            }

            auto cell = domain::CellFactory::createCustomCell(
                std::move(noise),
                genome,
                cfg.neoplasm_k,
                0.0001,  // low_delta (default from Simulations)
                0.0002,  // high_delta (default from Simulations)
                cfg.division_rate,
                cfg.neoplastic_division_rate,
                cfg.enable_big_bang_mode,
                2.0,  // d1_primer_threshold
                5.0,  // d2_apoptosis_threshold
                cfg.logger
            );

            tissue->addCell(std::move(cell));
        }

        // Create tracker and run simulation
        application::PopulationTracker tracker;
        int previous_alive = cfg.n_cells;
        int cumulative_dead = 0;

        // Capture initial state (year 0)
        captureSnapshotFromTissue(tissue.get(), 0, tracker, &previous_alive, &cumulative_dead);

        // Run simulation year by year
        for (int year = 1; year <= cfg.max_t; ++year) {
            tissue->live();
            captureSnapshotFromTissue(tissue.get(), year, tracker, &previous_alive, &cumulative_dead);
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        std::cout << " ✓ (" << duration.count() << "ms)\n" << std::endl;

        // Build config description for files
        std::ostringstream config_desc;
        config_desc << "neoplasm_k=" << cfg.neoplasm_k
                   << ", div=" << cfg.division_rate
                   << ", cells=" << cfg.n_cells
                   << ", max_t=" << cfg.max_t;

        // Extract scenario name from config file
        std::string scenario_name = fs::path(config_path).stem().string();

        // Save results
        tracker.saveToFiles(output_dir, scenario_name, 1, config_desc.str());

        // Print summary
        const auto& snaps = tracker.snapshots();
        int final_alive = snaps.back().alive_cells;
        int final_neoplastic = snaps.back().neoplastic_alive;
        int final_resistant = snaps.back().neoplastic_apoptosis_resistant;
        int final_dead = snaps.back().dead_cells_cumulative;

        std::cout << std::string(50, '-') << std::endl;
        std::cout << "Simulation Results:" << std::endl;
        std::cout << std::string(50, '-') << std::endl;
        std::cout << "  Final alive cells: " << final_alive << std::endl;
        std::cout << "  Final neoplastic: " << final_neoplastic << std::endl;
        std::cout << "  Apoptosis resistant: " << final_resistant << std::endl;
        std::cout << "  Cumulative dead: " << final_dead << std::endl;
        std::cout << "\n  TP53 distribution (final year):" << std::endl;
        std::cout << "    +/+: " << std::fixed << std::setprecision(1)
                  << (snaps.back().tp53_plus_plus_pct * 100) << "%" << std::endl;
        std::cout << "    +/-: " << (snaps.back().tp53_plus_minus_pct * 100) << "%" << std::endl;
        std::cout << "    -/-: " << (snaps.back().tp53_minus_minus_pct * 100) << "%" << std::endl;
        std::cout << "\n  D1/D2 ranges (final year):" << std::endl;
        std::cout << "    D1 (DNA damage): " << snaps.back().min_d1 << " - " << snaps.back().max_d1 << std::endl;
        std::cout << "    D2 (Immunosuppression): " << snaps.back().min_d2 << " - " << snaps.back().max_d2 << std::endl;

        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "Files saved to: " << output_dir << "/" << std::endl;
        std::cout << "  - " << scenario_name << "_run1_population.csv" << std::endl;
        std::cout << "  - " << scenario_name << "_run1_report.md" << std::endl;
        std::cout << std::string(50, '=') << std::endl;

        return 0;

    } catch (const std::runtime_error& caught) {
        std::cerr << "Configuration error: " << caught.what() << std::endl;
        return 1;
    } catch (const std::exception& caught) {
        std::cerr << "Error: " << caught.what() << std::endl;
        return 1;
    }
}

