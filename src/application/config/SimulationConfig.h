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
        // Parámetros de simulación
        int max_t = 100;                    // Años de simulación
        int n_cells = 1000;                 // Número de células
        long seed = -1;                     // Semilla (-1 = variar)
        bool verbose = false;               // Trazas verbose
        bool use_random_noise = true;       // true = RandomNoise, false = FixedNoise
        double division_rate = 0.001;       // Tasa de división normal
        double neoplastic_division_rate = 0.001;  // Tasa de división neoplástica (Big Bang)
        bool enable_big_bang_mode = false;  // Activar Big Bang (división acelerada neoplástica)

        // Parámetros de thresholds y deltas
        double d1_threshold = 2.0;          // D1 (DNA damage) primer/progression threshold
        double d2_threshold = 5.0;          // D2 (Immunosuppression) apoptosis resistance threshold
        double low_delta = 0.0001;          // Delta bajo para instabilidad genómica
        double high_delta = 0.0002;         // Delta alto para instabilidad genómica

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

