#pragma once

#include <string>
#include <stdexcept>
#include "SimulationConfig.h"

namespace application {

    /**
     * JsonConfigLoader: Carga configuración de simulación desde archivos JSON.
     *
     * El JSON debe estar estructurado en tres secciones:
     * - config: configuración básica (description, seed, verbose, use_random_noise)
     * - simulation_context: contexto de simulación (max_t, n_cells)
     * - tissue_parameters: parámetros del tejido (division_rate, genes)
     */
    class JsonConfigLoader {
    public:
        /**
         * Carga configuración desde un archivo JSON.
         * @param filepath Ruta al archivo JSON
         * @return SimulationConfig construido y validado
         * @throws std::runtime_error si el archivo no existe, JSON inválido, o validación falla
         */
        static SimulationConfig loadFromFile(const std::string& filepath);

    private:
        // Constructor privado - clase puramente estática
        JsonConfigLoader() = delete;

        /**
         * Valida que la configuración cargada tenga valores correctos.
         * @throws std::runtime_error si alguna validación falla
         */
        static void validate(const SimulationConfig& cfg);
    };

} // namespace application

