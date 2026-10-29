#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "../../domain/ports/ILogger.h"

namespace application {

    /**
     * SimulationConfig: Centraliza toda la configuración de simulaciones.
     * Evita duplicación de parámetros entre main.cpp, interactive, etc.
     */
    struct SimulationConfig {
        // Simulation parameters
        int max_t = 100;                    // Simulation years
        int n_cells = 1000;                 // Initial cell count
        long seed = -1;                     // RNG seed (-1 = vary per run)
        bool verbose = false;               // Verbose logging
        bool use_random_noise = true;       // true = RandomNoise, false = FixedNoise
        double division_rate = 0.001;       // Normal division rate
        double neoplastic_division_rate = 0.001;  // Neoplastic division rate (Big Bang)
        bool enable_big_bang_mode = false;  // Enable Big Bang (accelerated neoplastic division)

        // Threshold and delta parameters
        double d1_threshold = 2.0;          // D1 (DNA damage) primer/progression threshold
        double d2_threshold = 5.0;          // D2 (Immunosuppression) apoptosis resistance threshold
        double low_delta = 0.0001;          // Low genomic instability delta
        double high_delta = 0.0002;         // High genomic instability delta

        // Parámetros de genes
        std::unordered_map<std::string, double> gene_thresholds;
        std::unordered_map<std::string, double> gene_instability_k;

        // Logger
        domain::ports::ILoggerPtr logger;

        /**
         * Cargar configuración por nombre de escenario
         */
        [[nodiscard]] static SimulationConfig loadScenario(const std::string& scenario_name, bool verbose = false);

        /**
         * Cargar configuración por defecto
         */
        [[nodiscard]] static SimulationConfig loadDefault(bool verbose = false);

        /**
         * Configuración para simulaciones interactivas
         */
        [[nodiscard]] static SimulationConfig loadInteractive(bool verbose = true);

        /**
         * Configuración para simulación de célula única
         */
        [[nodiscard]] static SimulationConfig loadSingleCell(bool verbose = true);
    };

} // namespace application

