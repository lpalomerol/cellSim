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
        int total_cells;              // Alive + dead ever
        int alive_cells;             // Protected + neoplastic alive
        int dead_cells_cumulative;   // Cumulative apoptosis count
        int neoplastic_alive;        // Live neoplastic cells
        int protected_alive;         // Live healthy cells

        // D1/D2 Model: DNA damage and Immunosuppression counters
        double min_d1;               // Min D1 (DNA damage) in live population
        double max_d1;               // Max D1 (DNA damage) in live population
        double min_d2;               // Min D2 (Immunosuppression) in live population
        double max_d2;               // Max D2 (Immunosuppression) in live population

        double tp53_plus_plus_pct;    // TP53 +/+ (%)
        double tp53_plus_minus_pct;   // TP53 +/- (%)
        double tp53_minus_minus_pct;  // TP53 -/- (%)

        int neoplastic_apoptosis_susceptible;  // Neoplastic cells that accept apoptosis
        int neoplastic_apoptosis_resistant;    // Neoplastic cells that evade apoptosis (immortal)
    };

    /**
     * PopulationTracker: captures and exports yearly population snapshots
     */
    class PopulationTracker {
    public:
        explicit PopulationTracker(double tumor_threshold = 0.1)
            : tumor_threshold_(tumor_threshold) {}

        void addSnapshot(const YearlySnapshot& snapshot) {
            snapshots_.push_back(snapshot);
        }

        // Export to Markdown
        std::string toMarkdown(const std::string& scenario_name, int run_number,
                              const std::string& config_desc,
                              const std::string& config_json = "") const;

        // Export to CSV
        std::string toCSV(const std::string& scenario_name, int run_number,
                         const std::string& config_json = "") const;

        // Export summary CSV
        std::string toSummaryCsv(const std::string& scenario_name, int run_number,
                                const std::string& config_desc,
                                long execution_time_ms,
                                const std::string& config_json = "") const;

        // Guardar a archivos (incluyendo SUMMARY.csv)
        void saveToFiles(const std::string& output_dir,
                        const std::string& scenario_name,
                        int run_number,
                        const std::string& config_desc,
                        long execution_time_ms = 0,
                        const std::string& config_json = "") const;

        // Getter
        [[nodiscard]] const std::vector<YearlySnapshot>& snapshots() const {
            return snapshots_;
        }

        // Returns the first year the tumor threshold is exceeded (-1 if never)
        [[nodiscard]] int getTumorThresholdYear() const;

    private:
        std::vector<YearlySnapshot> snapshots_;
        double tumor_threshold_;  // Neoplastic fraction above which a tumor is declared (default 10%)

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

