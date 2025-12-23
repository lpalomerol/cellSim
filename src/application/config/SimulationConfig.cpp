#include "SimulationConfig.h"
#include "../../domain/adapters/Logger.h"
#include "../../domain/adapters/NullLogger.h"

namespace application {

    SimulationConfig SimulationConfig::loadDefault(bool verbose) {
        SimulationConfig cfg;
        cfg.max_t = 80;
        cfg.n_cells = 1000;
        cfg.seed = -1;
        cfg.neoplasm_k = 0.03;
        cfg.verbose = verbose;
        cfg.use_random_noise = true;
        cfg.division_rate = 0.001;
        cfg.apoptosis_threshold = 10.0;

        // Gene parameters
        cfg.gene_thresholds = {{"TP53", 0.0010}, {"BRCA1", 0.0015}};
        cfg.gene_instability_k = {{"TP53", 0.0010}, {"BRCA1", 0.0015}};

        // Logger
        auto logger = std::make_shared<domain::adapters::Logger>();
        logger->setVerbose(verbose);
        cfg.logger = logger;

        return cfg;
    }

    SimulationConfig SimulationConfig::loadInteractive(bool verbose) {
        SimulationConfig cfg;
        cfg.max_t = 30;
        cfg.n_cells = 1;
        cfg.seed = 4;
        cfg.neoplasm_k = 0.5;
        cfg.verbose = verbose;
        cfg.use_random_noise = true;
        cfg.division_rate = 0.001;
        cfg.apoptosis_threshold = 10.0;

        // Gene parameters (custom for interactive)
        cfg.gene_thresholds = {{"BRCA1", 0.0}, {"TP53", 0.00}};
        cfg.gene_instability_k = {{"BRCA1", 0.0}, {"TP53", 0.0}};

        // Logger
        auto logger = std::make_shared<domain::adapters::Logger>();
        logger->setVerbose(verbose);
        cfg.logger = logger;

        return cfg;
    }

    SimulationConfig SimulationConfig::loadSingleCell(bool verbose) {
        SimulationConfig cfg;
        cfg.max_t = 100;
        cfg.n_cells = 1;
        cfg.seed = 42;
        cfg.neoplasm_k = 0.05;
        cfg.verbose = verbose;
        cfg.use_random_noise = true;
        cfg.division_rate = 0.1;
        cfg.apoptosis_threshold = 0.01;

        // Gene parameters
        cfg.gene_thresholds = {{"BRCA1", 0.01}, {"TP53", 0.25}};
        cfg.gene_instability_k = {{"BRCA1", 0.01}, {"TP53", 0.5}};

        // Logger
        auto logger = std::make_shared<domain::adapters::Logger>();
        logger->setVerbose(verbose);
        cfg.logger = logger;

        return cfg;
    }

    SimulationConfig SimulationConfig::loadScenario(const std::string& scenario_name, bool verbose) {
        SimulationConfig cfg;
        cfg.verbose = verbose;
        cfg.max_t = 50;
        cfg.n_cells = 1000;
        cfg.seed = -1;
        cfg.division_rate = 0.001;

        // Crear logger
        auto logger = std::make_shared<domain::adapters::Logger>();
        logger->setVerbose(verbose);
        cfg.logger = logger;

        if (scenario_name == "default") {
            cfg.neoplasm_k = 0.05;
            cfg.gene_thresholds = {{"BRCA1", 0.01}, {"TP53", 0.01}};
            cfg.gene_instability_k = {{"BRCA1", 0.01}, {"TP53", 0.01}};
        } else if (scenario_name == "no_mutations") {
            cfg.neoplasm_k = 0.0;
            cfg.gene_thresholds = {{"BRCA1", 0.0}, {"TP53", 0.0}};
            cfg.gene_instability_k = {{"BRCA1", 0.0}, {"TP53", 0.0}};
            cfg.division_rate = 0.0;
        } else if (scenario_name == "high_brca_apoptosis") {
            cfg.neoplasm_k = 0.02;
            cfg.gene_thresholds = {{"BRCA1", 0.5}, {"TP53", 0.01}};
            cfg.gene_instability_k = {{"BRCA1", 0.1}, {"TP53", 0.01}};
            cfg.division_rate = 0.0;
        } else if (scenario_name == "high_tp53_mutation") {
            cfg.neoplasm_k = 0.8;
            cfg.gene_thresholds = {{"BRCA1", 0.001}, {"TP53", 0.3}};
            cfg.gene_instability_k = {{"BRCA1", 0.001}, {"TP53", 0.2}};
            cfg.division_rate = 0.0;
        } else if (scenario_name == "cell_division_healthy") {
            cfg.neoplasm_k = 0.0;
            cfg.gene_thresholds = {{"BRCA1", 0.0}, {"TP53", 0.0}};
            cfg.gene_instability_k = {{"BRCA1", 0.0}, {"TP53", 0.0}};
            cfg.division_rate = 0.1;  // 10% division rate
        } else {
            // Default si escenario no reconocido
            cfg = loadDefault(verbose);
        }

        return cfg;
    }

} // namespace application

