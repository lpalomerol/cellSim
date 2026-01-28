#include <iostream>
#include <cxxopts.hpp>
#include <nlohmann/json.hpp>
#include "../src/application/config/JsonConfigLoader.h"
#include "../src/application/simulation/Simulations.h"

using json = nlohmann::json;

int main(int argc, char* argv[]) {
    try {
        // Configure CLI options
        cxxopts::Options options("cellSim_cli", "Cell Evolution Simulator - JSON Configuration Mode");

        options.add_options()
            ("c,config", "JSON configuration file (required)", cxxopts::value<std::string>())
            ("m,max-t", "Simulation years (override)", cxxopts::value<int>())
            ("n,cells", "Number of cells (override)", cxxopts::value<int>())
            ("s,seed", "Random seed, -1=random (override)", cxxopts::value<long>())
            ("k,neoplasm-k", "Base neoplasm probability 0.0-1.0 (override)", cxxopts::value<double>())
            ("v,verbose", "Enable detailed logging (override)", cxxopts::value<bool>()->default_value("false"))
            ("d,division-rate", "Cell division rate (override)", cxxopts::value<double>())
            ("h,help", "Show help");

        // Parse arguments
        auto result = options.parse(argc, argv);

        // Show help if requested or no arguments
        if (result.count("help") || argc == 1) {
            std::cout << options.help() << std::endl;
            return 0;
        }

        // Verify that --config was provided
        if (!result.count("config")) {
            std::cerr << "Error: --config argument is required\n\n";
            std::cout << options.help() << std::endl;
            return 1;
        }

        // Load base configuration from JSON
        std::string config_path = result["config"].as<std::string>();
        std::cout << "Loading configuration from: " << config_path << "\n" << std::endl;

        auto cfg = application::JsonConfigLoader::loadFromFile(config_path);

        // Apply CLI overrides if provided
        if (result.count("max-t")) {
            cfg.max_t = result["max-t"].as<int>();
        }
        if (result.count("cells")) {
            cfg.n_cells = result["cells"].as<int>();
        }
        if (result.count("seed")) {
            cfg.seed = result["seed"].as<long>();
        }
        if (result.count("neoplasm-k")) {
            cfg.neoplasm_k = result["neoplasm-k"].as<double>();
        }
        if (result.count("verbose")) {
            cfg.verbose = result["verbose"].as<bool>();
            cfg.logger->setVerbose(cfg.verbose);
        }
        if (result.count("division-rate")) {
            cfg.division_rate = result["division-rate"].as<double>();
        }

        // Print effective configuration
        std::cout << "=== Simulation Configuration ===" << std::endl;
        std::cout << "\n[Basic Config]" << std::endl;
        std::cout << "  Seed: " << cfg.seed << std::endl;
        std::cout << "  Verbose mode: " << (cfg.verbose ? "enabled" : "disabled") << std::endl;
        std::cout << "  Random noise: " << (cfg.use_random_noise ? "yes" : "no") << std::endl;

        std::cout << "\n[Simulation Context]" << std::endl;
        std::cout << "  Number of scenarios: 100" << std::endl;
        std::cout << "  Cells per scenario: " << cfg.n_cells << std::endl;
        std::cout << "  Maximum time (years): " << cfg.max_t << std::endl;

        std::cout << "\n[Tissue Parameters]" << std::endl;
        std::cout << "  Neoplasm probability: " << cfg.neoplasm_k << std::endl;
        std::cout << "  Normal division rate: " << cfg.division_rate << std::endl;
        std::cout << "  Neoplastic division rate: " << cfg.neoplastic_division_rate << std::endl;
        std::cout << "  Big Bang mode: " << (cfg.enable_big_bang_mode ? "enabled" : "disabled") << std::endl;

        std::cout << "\n  Configured genes:" << std::endl;
        for (const auto& [gene, mutation_rate] : cfg.gene_thresholds) {
            double instability_rate = cfg.gene_instability_k.at(gene);
            std::cout << "    - " << gene << ":" << std::endl;
            std::cout << "        mutation_rate: " << mutation_rate << std::endl;
            std::cout << "        instability_rate: " << instability_rate << std::endl;
        }

        std::cout << "\n" << std::string(50, '=') << "\n" << std::endl;

        // Run simulations
        std::cout << "Starting simulations..." << std::endl;
        application::Simulations sims(cfg);
        sims.runAll();

        std::cout << "\n" << std::string(50, '-') << std::endl;
        std::cout << "Simulation results:" << std::endl;
        std::cout << std::string(50, '-') << "\n" << std::endl;

        sims.printSummary();

        return 0;

    } catch (const std::runtime_error& caught) {
        std::cerr << "Configuration error: " << caught.what() << std::endl;
        return 1;
    } catch (const std::exception& caught) {
        std::cerr << "Error: " << caught.what() << std::endl;
        return 1;
    }
}

