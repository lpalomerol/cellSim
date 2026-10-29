#include "JsonConfigLoader.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include "../../domain/adapters/Logger.h"

using json = nlohmann::json;

namespace application {

    SimulationConfig JsonConfigLoader::loadFromFile(const std::string& filepath) {
        // Abrir archivo
        std::ifstream ifs(filepath);
        if (!ifs.is_open()) {
            throw std::runtime_error("Error: no se pudo abrir el archivo de configuración: " + filepath);
        }

        // Parsear JSON
        json j;
        try {
            ifs >> j;
        } catch (const json::parse_error& e) {
            throw std::runtime_error("Error: JSON malformado en " + filepath + ": " + e.what());
        }

        SimulationConfig cfg;

        // ========== Section "config" ==========
        try {
            if (!j.contains("config")) {
                throw std::runtime_error("Missing section 'config' in JSON");
            }
            auto& config_section = j["config"];

            cfg.seed = config_section.at("seed").get<long>();
            cfg.verbose = config_section.at("verbose").get<bool>();
            cfg.use_random_noise = config_section.at("use_random_noise").get<bool>();

        } catch (const json::type_error& e) {
            throw std::runtime_error(std::string("Type error in section 'config': ") + e.what());
        } catch (const json::out_of_range& e) {
            throw std::runtime_error(std::string("Missing field in section 'config': ") + e.what());
        }

        // ========== Section "simulation_context" ==========
        try {
            if (!j.contains("simulation_context")) {
                throw std::runtime_error("Missing section 'simulation_context' in JSON");
            }
            auto& sim_section = j["simulation_context"];

            cfg.max_t = sim_section.at("max_t").get<int>();
            cfg.n_cells = sim_section.at("n_cells").get<int>();

        } catch (const json::type_error& e) {
            throw std::runtime_error(std::string("Type error in section 'simulation_context': ") + e.what());
        } catch (const json::out_of_range& e) {
            throw std::runtime_error(std::string("Missing field in section 'simulation_context': ") + e.what());
        }

        // ========== Section "tissue_parameters" ==========
        try {
            if (!j.contains("tissue_parameters")) {
                throw std::runtime_error("Missing section 'tissue_parameters' in JSON");
            }
            auto& tissue_section = j["tissue_parameters"];

            cfg.division_rate = tissue_section.at("division_rate").get<double>();
            cfg.neoplastic_division_rate = tissue_section.at("neoplastic_division_rate").get<double>();
            cfg.enable_big_bang_mode = tissue_section.at("enable_big_bang_mode").get<bool>();

            // Threshold and delta parameters (optional, fall back to defaults)
            if (tissue_section.contains("d1_threshold")) {
                cfg.d1_threshold = tissue_section.at("d1_threshold").get<double>();
            }
            if (tissue_section.contains("d2_threshold")) {
                cfg.d2_threshold = tissue_section.at("d2_threshold").get<double>();
            }
            if (tissue_section.contains("low_delta")) {
                cfg.low_delta = tissue_section.at("low_delta").get<double>();
            }
            if (tissue_section.contains("high_delta")) {
                cfg.high_delta = tissue_section.at("high_delta").get<double>();
            }

            if (!tissue_section.contains("genes")) {
                throw std::runtime_error("Missing object 'genes' in 'tissue_parameters'");
            }
            auto& genes = tissue_section["genes"];

            if (!genes.contains("TP53")) {
                throw std::runtime_error("Missing gene 'TP53' in 'genes'");
            }
            if (!genes.contains("BRCA1")) {
                throw std::runtime_error("Missing gene 'BRCA1' in 'genes'");
            }

            auto& tp53 = genes["TP53"];
            double tp53_mutation = tp53.at("mutation_rate").get<double>();
            double tp53_instability = tp53.at("instability_rate").get<double>();

            auto& brca1 = genes["BRCA1"];
            double brca1_mutation = brca1.at("mutation_rate").get<double>();
            double brca1_instability = brca1.at("instability_rate").get<double>();

            cfg.gene_thresholds = {
                {"TP53", tp53_mutation},
                {"BRCA1", brca1_mutation}
            };
            cfg.gene_instability_k = {
                {"TP53", tp53_instability},
                {"BRCA1", brca1_instability}
            };

        } catch (const json::type_error& e) {
            throw std::runtime_error(std::string("Type error in section 'tissue_parameters': ") + e.what());
        } catch (const json::out_of_range& e) {
            throw std::runtime_error(std::string("Missing field in section 'tissue_parameters': ") + e.what());
        }

        // Crear logger
        auto logger = std::make_shared<domain::adapters::Logger>();
        logger->setVerbose(cfg.verbose);
        cfg.logger = logger;

        // Validar configuración
        validate(cfg);

        return cfg;
    }

    void JsonConfigLoader::validate(const SimulationConfig& cfg) {
        // Validar seed
        if (cfg.seed < -1) {
            throw std::runtime_error("Error: seed debe ser >= -1, recibido: " + std::to_string(cfg.seed));
        }

        // Validar simulation_context
        if (cfg.max_t <= 0) {
            throw std::runtime_error("Error: max_t debe ser > 0, recibido: " + std::to_string(cfg.max_t));
        }
        if (cfg.n_cells <= 0) {
            throw std::runtime_error("Error: n_cells debe ser > 0, recibido: " + std::to_string(cfg.n_cells));
        }

        // Validar tissue_parameters
        if (cfg.division_rate < 0.0) {
            throw std::runtime_error("Error: division_rate debe ser >= 0.0, recibido: " + std::to_string(cfg.division_rate));
        }
        if (cfg.neoplastic_division_rate < 0.0) {
            throw std::runtime_error("Error: neoplastic_division_rate debe ser >= 0.0, recibido: " + std::to_string(cfg.neoplastic_division_rate));
        }

        // Validar thresholds y deltas
        if (cfg.d1_threshold < 0.0) {
            throw std::runtime_error("Error: d1_threshold debe ser >= 0.0, recibido: " + std::to_string(cfg.d1_threshold));
        }
        if (cfg.d2_threshold < 0.0) {
            throw std::runtime_error("Error: d2_threshold debe ser >= 0.0, recibido: " + std::to_string(cfg.d2_threshold));
        }
        if (cfg.low_delta < 0.0) {
            throw std::runtime_error("Error: low_delta debe ser >= 0.0, recibido: " + std::to_string(cfg.low_delta));
        }
        if (cfg.high_delta < 0.0) {
            throw std::runtime_error("Error: high_delta debe ser >= 0.0, recibido: " + std::to_string(cfg.high_delta));
        }
        if (cfg.low_delta > cfg.high_delta) {
            throw std::runtime_error("Error: low_delta debe ser <= high_delta");
        }

        // Validar genes (deben existir TP53 y BRCA1 con valores >= 0.0)
        if (cfg.gene_thresholds.size() != 2 || cfg.gene_instability_k.size() != 2) {
            throw std::runtime_error("Error: se esperan exactamente 2 genes (TP53 y BRCA1)");
        }

        if (!cfg.gene_thresholds.contains("TP53") || !cfg.gene_thresholds.contains("BRCA1")) {
            throw std::runtime_error("Error: deben existir los genes TP53 y BRCA1 en gene_thresholds");
        }
        if (!cfg.gene_instability_k.contains("TP53") || !cfg.gene_instability_k.contains("BRCA1")) {
            throw std::runtime_error("Error: deben existir los genes TP53 y BRCA1 en gene_instability_k");
        }

        // Validar valores >= 0.0
        for (const auto& [gene, rate] : cfg.gene_thresholds) {
            if (rate < 0.0) {
                throw std::runtime_error("Error: mutation_rate de " + gene + " debe ser >= 0.0, recibido: " + std::to_string(rate));
            }
        }
        for (const auto& [gene, rate] : cfg.gene_instability_k) {
            if (rate < 0.0) {
                throw std::runtime_error("Error: instability_rate de " + gene + " debe ser >= 0.0, recibido: " + std::to_string(rate));
            }
        }
    }

} // namespace application

