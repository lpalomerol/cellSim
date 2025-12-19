#pragma once

#include <string>
#include <vector>
#include <memory>
#include <iomanip>
#include <sstream>
#include <fstream>

namespace application {

    /**
     * YearlySnapshot: Captura del estado poblacional en un año específico
     */
    struct YearlySnapshot {
        int year;
        int total_cells;              // Vivas + Muertas
        int alive_cells;              // Protegidas + Neoplásticas vivas
        int dead_cells_cumulative;    // Apoptosis acumulada
        int neoplastic_alive;         // Células neoplásticas vivas
        int protected_alive;          // Células sanas vivas

        // D1/D2 Model: DNA damage and Immunosuppression counters
        double min_d1;                // Mínimo D1 (DNA damage) en población viva
        double max_d1;                // Máximo D1 (DNA damage) en población viva
        double min_d2;                // Mínimo D2 (Immunosuppression) en población viva
        double max_d2;                // Máximo D2 (Immunosuppression) en población viva

        double tp53_plus_plus_pct;    // TP53 +/+ (%)
        double tp53_plus_minus_pct;   // TP53 +/- (%)
        double tp53_minus_minus_pct;  // TP53 -/- (%)

        // Nuevos campos: Apoptosis en neoplásticas
        int neoplastic_apoptosis_susceptible;    // Neoplásticas que aceptan apoptosis
        int neoplastic_apoptosis_resistant;      // Neoplásticas que rechazan apoptosis (inmortales)
    };

    /**
     * PopulationTracker: Gestiona captura y exportación de datos poblacionales anuales
     */
    class PopulationTracker {
    public:
        PopulationTracker() = default;

        // Agregar snapshot de un año
        void addSnapshot(const YearlySnapshot& snapshot) {
            snapshots_.push_back(snapshot);
        }

        // Exportar a Markdown
        std::string toMarkdown(const std::string& scenario_name, int run_number,
                              const std::string& config_desc) const;

        // Exportar a CSV
        std::string toCSV(const std::string& scenario_name, int run_number) const;

        // Guardar a archivos
        void saveToFiles(const std::string& output_dir,
                        const std::string& scenario_name,
                        int run_number,
                        const std::string& config_desc) const;

        // Getter
        [[nodiscard]] const std::vector<YearlySnapshot>& snapshots() const {
            return snapshots_;
        }

    private:
        std::vector<YearlySnapshot> snapshots_;

        // Helper para formatear porcentajes
        static std::string formatPercent(double value) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(1) << (value * 100.0) << "%";
            return oss.str();
        }

        // Helper para formatear números decimales
        static std::string formatDouble(double value, int precision = 3) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(precision) << value;
            return oss.str();
        }
    };

} // namespace application

