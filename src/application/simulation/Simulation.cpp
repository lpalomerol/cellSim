#include "Simulation.h"
#include <array>
#include <algorithm>
#include <limits>
#include "../../domain/cell/AgenticCell.h"

namespace application {

    Simulation::Simulation(int max_t_years) {
        max_t_ = max_t_years;
        cell_state_counter_ = std::vector(max_t_, std::array<int, 3>{0, 0});
    }

    void Simulation::addCell(std::unique_ptr<domain::ICell> cell) {
        cells_.emplace_back(std::move(cell));
    }

    // Helper: ejecuta un ciclo celular y devuelve el conteo de células neoplásticas
    int Simulation::executeCellCycle() {
        int neoplastic_count = 0;
        for (auto& c : cells_) {
            c->live();
            if (c->isNeoplastic()) {
                neoplastic_count += 1;
            }
        }
        return neoplastic_count;
    }

    void Simulation::captureAnnualSnapshot(int year) {
        // Contar células por categoría
        int alive_count = 0;
        int neoplastic_alive = 0;
        int protected_alive = 0;
        int neoplastic_apoptosis_susceptible = 0;
        int neoplastic_apoptosis_resistant = 0;

        double min_instability = std::numeric_limits<double>::max();
        double max_instability = std::numeric_limits<double>::lowest();

        int tp53_plus_plus = 0;
        int tp53_plus_minus = 0;
        int tp53_minus_minus = 0;

        for (auto& cell : cells_) {
            if (cell->alive()) {
                alive_count++;

                // Obtener estado TP53
                std::string tp53_status = cell->getTP53Status();
                if (tp53_status == "+/+") {
                    tp53_plus_plus++;
                } else if (tp53_status == "+/-") {
                    tp53_plus_minus++;
                } else if (tp53_status == "-/-") {
                    tp53_minus_minus++;
                }

                // Contar neoplásticas vs protegidas
                if (cell->isNeoplastic()) {
                    neoplastic_alive++;

                    // Verificar si la neoplástica ha evasado apoptosis (es inmortal)
                    // Intentar hacer dynamic_cast a AgenticCell para acceder al método
                    auto* agentic_cell = dynamic_cast<domain::AgenticCell*>(cell.get());
                    if (agentic_cell && agentic_cell->hasEvasedApoptosis()) {
                        neoplastic_apoptosis_resistant++;
                    } else {
                        neoplastic_apoptosis_susceptible++;
                    }
                } else {
                    protected_alive++;
                }
            }
        }

        // Calcular inestabilidad genómica (incluye todas las células vivas)
        if (alive_count > 0) {
            min_instability = std::numeric_limits<double>::max();
            max_instability = std::numeric_limits<double>::lowest();

            for (auto& cell : cells_) {
                if (cell->alive()) {
                    // Obtener inestabilidad real de la célula usando dynamic_cast
                    auto* agentic_cell = dynamic_cast<domain::AgenticCell*>(cell.get());
                    if (agentic_cell) {
                        double instability = agentic_cell->getGenomicInstability();
                        min_instability = std::min(min_instability, instability);
                        max_instability = std::max(max_instability, instability);
                    }
                }
            }

            // Si no se pudo obtener, usar defaults
            if (min_instability == std::numeric_limits<double>::max()) {
                min_instability = 0.0;
                max_instability = 0.0;
            }
        } else {
            min_instability = 0.0;
            max_instability = 0.0;
        }

        int total_cells = cells_.size();
        int dead_cells = total_cells - alive_count;

        // Calcular porcentajes de TP53 (solo de células vivas)
        int total_alive_for_pct = tp53_plus_plus + tp53_plus_minus + tp53_minus_minus;
        double tp53_pp_pct = (total_alive_for_pct > 0) ? static_cast<double>(tp53_plus_plus) / total_alive_for_pct : 0.0;
        double tp53_pm_pct = (total_alive_for_pct > 0) ? static_cast<double>(tp53_plus_minus) / total_alive_for_pct : 0.0;
        double tp53_mm_pct = (total_alive_for_pct > 0) ? static_cast<double>(tp53_minus_minus) / total_alive_for_pct : 0.0;

        // Crear snapshot
        YearlySnapshot snapshot{
            year,
            total_cells,
            alive_count,
            dead_cells,
            neoplastic_alive,
            protected_alive,
            min_instability,
            max_instability,
            tp53_pp_pct,
            tp53_pm_pct,
            tp53_mm_pct,
            neoplastic_apoptosis_susceptible,
            neoplastic_apoptosis_resistant
        };

        population_tracker_.addSnapshot(snapshot);
    }

    void Simulation::run() {
        // Capturar estado inicial (año 0)
        captureAnnualSnapshot(0);

        for (int t = 0; t < max_t_; ++t) {
            int neoplastic_count = executeCellCycle();
            cell_state_counter_[t][2] = neoplastic_count;

            // Capturar snapshot después de cada año
            captureAnnualSnapshot(t + 1);
        }
    }

    int Simulation::firstTimeNeoplastic() {
        for (int t = 0; t < max_t_; ++t) {
            if (cell_state_counter_[t][2] > 0) {
                return t + 1;
            }
        }
        return -1;
    }


}