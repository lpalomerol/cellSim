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

        // ========== Sección "config" (configuración básica) ==========
        try {
            if (!j.contains("config")) {
                throw std::runtime_error("Error: falta la sección 'config' en el JSON");
            }
            auto& config_section = j["config"];

            cfg.seed = config_section.at("seed").get<long>();
            cfg.verbose = config_section.at("verbose").get<bool>();
            cfg.use_random_noise = config_section.at("use_random_noise").get<bool>();

            // description es opcional, se usa solo para logging si existe
            // No se almacena en SimulationConfig actualmente

        } catch (const json::type_error& e) {
            throw std::runtime_error("Error: tipo incorrecto en sección 'config': " + std::string(e.what()));
        } catch (const json::out_of_range& e) {
            throw std::runtime_error("Error: campo faltante en sección 'config': " + std::string(e.what()));
        }

        // ========== Sección "simulation_context" ==========
        try {
            if (!j.contains("simulation_context")) {
                throw std::runtime_error("Error: falta la sección 'simulation_context' en el JSON");
            }
            auto& sim_section = j["simulation_context"];

            cfg.max_t = sim_section.at("max_t").get<int>();
            cfg.n_cells = sim_section.at("n_cells").get<int>();

        } catch (const json::type_error& e) {
            throw std::runtime_error("Error: tipo incorrecto en sección 'simulation_context': " + std::string(e.what()));
        } catch (const json::out_of_range& e) {
            throw std::runtime_error("Error: campo faltante en sección 'simulation_context': " + std::string(e.what()));
        }

        // ========== Sección "tissue_parameters" ==========
        try {
            if (!j.contains("tissue_parameters")) {
                throw std::runtime_error("Error: falta la sección 'tissue_parameters' en el JSON");
            }
            auto& tissue_section = j["tissue_parameters"];

            cfg.neoplasm_k = tissue_section.at("neoplasm_k").get<double>();
            cfg.division_rate = tissue_section.at("division_rate").get<double>();
            cfg.neoplastic_division_rate = tissue_section.at("neoplastic_division_rate").get<double>();
            cfg.enable_big_bang_mode = tissue_section.at("enable_big_bang_mode").get<bool>();

            // Genes: TP53 y BRCA1
            if (!tissue_section.contains("genes")) {
                throw std::runtime_error("Error: falta el objeto 'genes' en 'tissue_parameters'");
            }
            auto& genes = tissue_section["genes"];

            // Validar que existan TP53 y BRCA1
            if (!genes.contains("TP53")) {
                throw std::runtime_error("Error: falta gen 'TP53' en 'genes'");
            }
            if (!genes.contains("BRCA1")) {
                throw std::runtime_error("Error: falta gen 'BRCA1' en 'genes'");
            }

            // Extraer TP53
            auto& tp53 = genes["TP53"];
            double tp53_mutation = tp53.at("mutation_rate").get<double>();
            double tp53_instability = tp53.at("instability_rate").get<double>();

            // Extraer BRCA1
            auto& brca1 = genes["BRCA1"];
            double brca1_mutation = brca1.at("mutation_rate").get<double>();
            double brca1_instability = brca1.at("instability_rate").get<double>();

            // Mapear a SimulationConfig
            cfg.gene_thresholds = {
                {"TP53", tp53_mutation},
                {"BRCA1", brca1_mutation}
            };
            cfg.gene_instability_k = {
                {"TP53", tp53_instability},
                {"BRCA1", brca1_instability}
            };

        } catch (const json::type_error& e) {
            throw std::runtime_error("Error: tipo incorrecto en sección 'tissue_parameters': " + std::string(e.what()));
        } catch (const json::out_of_range& e) {
            throw std::runtime_error("Error: campo faltante en sección 'tissue_parameters': " + std::string(e.what()));
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
        if (cfg.neoplasm_k < 0.0 || cfg.neoplasm_k > 1.0) {
            throw std::runtime_error("Error: neoplasm_k debe estar en [0.0, 1.0], recibido: " + std::to_string(cfg.neoplasm_k));
        }
        if (cfg.division_rate < 0.0) {
            throw std::runtime_error("Error: division_rate debe ser >= 0.0, recibido: " + std::to_string(cfg.division_rate));
        }
        if (cfg.neoplastic_division_rate < 0.0) {
            throw std::runtime_error("Error: neoplastic_division_rate debe ser >= 0.0, recibido: " + std::to_string(cfg.neoplastic_division_rate));
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

