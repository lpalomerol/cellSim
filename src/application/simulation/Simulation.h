#pragma once
#include <vector>
#include <array>
#include <memory>
#include "../../domain/ports/ICell.h"
#include "PopulationTracker.h"
#include "TissueV2Adapter.h"

namespace application {
    class Simulation {
    public:
        explicit Simulation(int max_t_years);
        void addCell(std::unique_ptr<domain::ICell> cell);
        void run(); // dt=1 año, hasta max_t_

        [[nodiscard]] int firstTimeNeoplastic();

        // Helper public para reutilización: ejecuta un año de ciclo celular
        [[nodiscard]] int executeCellCycle();

        // Acceso al tracker de población
        [[nodiscard]] const PopulationTracker& populationTracker() const {
            return population_tracker_;
        }

        /// PASO 6: Habilitar/deshabilitar uso de TissueV2 como Population Orchestrator
        /// Por defecto: false (usa Tissue original). Reversible en runtime.
        /// @param enable true = usar TissueV2Adapter, false = usar Tissue original
        void enableTissueV2(bool enable);

        /// Obtener estado actual: ¿está TissueV2 habilitado?
        [[nodiscard]] bool isTissueV2Enabled() const { return use_tissue_v2_; }

    protected:
        int max_t_ = 0;
        std::vector<std::unique_ptr<domain::ICell>> cells_;  // Tissue original
        std::unique_ptr<TissueV2Adapter> tissue_adapter_;     // PASO 6: Nuevo adaptador
        bool use_tissue_v2_ = false;                           // PASO 6: Flag de control
        std::vector<std::array<int, 3>> cell_state_counter_;
        PopulationTracker population_tracker_;

        // Capturar snapshot anual de población
        void captureAnnualSnapshot(int year);
    };
}