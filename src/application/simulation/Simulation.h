#pragma once
#include <vector>
#include <array>
#include <memory>
#include "../../domain/ports/ICell.h"
#include "PopulationTracker.h"

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

    protected:
        int max_t_ = 0;
        std::vector<std::unique_ptr<domain::ICell>> cells_;
        std::vector<std::array<int, 3>> cell_state_counter_;
        PopulationTracker population_tracker_;

        // Capturar snapshot anual de población
        void captureAnnualSnapshot(int year);
    };
}