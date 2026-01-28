#include "PopulationTracker.h"
#include <filesystem>

namespace application {

    std::string PopulationTracker::toMarkdown(const std::string& scenario_name, int run_number,
                                             const std::string& config_desc,
                                             const std::string& config_json) const {
        std::ostringstream oss;

        oss << "# Población Celular - Escenario: " << scenario_name << " (Run #" << run_number << ")\n\n";

        oss << "## Resumen Ejecutivo\n";
        oss << "- **Escenario:** " << scenario_name << "\n";
        oss << "- **Run:** #" << run_number << "\n";
        oss << "- **Configuración:** " << config_desc << "\n";
        oss << "- **Años simulados:** " << snapshots_.size() - 1 << "\n";
        if (!snapshots_.empty()) {
            oss << "- **Población inicial:** " << snapshots_[0].total_cells << " células\n";
            oss << "- **Población final:** " << snapshots_.back().total_cells << " células\n";
        }
        oss << "\n";

        oss << "---\n\n";

        // Sección de configuración si está disponible
        if (!config_json.empty()) {
            oss << "## Configuración JSON\n\n";
            oss << "```json\n";
            oss << config_json << "\n";
            oss << "```\n\n";
            oss << "---\n\n";
        }

        oss << "## Evolución Anual de Población\n\n";

        // Encabezado de tabla con D1/D2
        oss << "| Año | Total (V+M) | Vivas (P+N) | Muertas (Apoptosis) | Neoplásticas (Vivas) | Protegidas (Vivas) | Min D1 | Max D1 | Min D2 | Max D2 | TP53++ (%) | TP53+- (%) | TP53-- (%) |\n";
        oss << "|-----|-------------|-------------|---------------------|----------------------|-------------------|--------|--------|--------|--------|-----------|-----------|----------|\n";

        // Filas de datos
        for (const auto& snap : snapshots_) {
            oss << "| " << snap.year << " | ";
            oss << snap.total_cells << " | ";
            oss << snap.alive_cells << " | ";
            oss << snap.dead_cells_cumulative << " | ";
            oss << snap.neoplastic_alive << " | ";
            oss << snap.protected_alive << " | ";
            oss << formatDouble(snap.min_d1) << " | ";
            oss << formatDouble(snap.max_d1) << " | ";
            oss << formatDouble(snap.min_d2) << " | ";
            oss << formatDouble(snap.max_d2) << " | ";
            oss << formatPercent(snap.tp53_plus_plus_pct) << " | ";
            oss << formatPercent(snap.tp53_plus_minus_pct) << " | ";
            oss << formatPercent(snap.tp53_minus_minus_pct) << " |\n";
        }

        oss << "---\n\n";

        oss << "## Apoptosis en Células Neoplásticas\n\n";

        // Segunda tabla: Apoptosis
        oss << "| Año | Neoplásticas (Vivas) | Susceptibles a Apoptosis | Resistentes a Apoptosis | % Inmortales |\n";
        oss << "|-----|----------------------|--------------------------|--------------------------|---------------|\n";

        for (const auto& snap : snapshots_) {
            int total_neo = snap.neoplastic_apoptosis_susceptible + snap.neoplastic_apoptosis_resistant;
            double pct_inmortales = (total_neo > 0) ? (100.0 * snap.neoplastic_apoptosis_resistant / total_neo) : 0.0;

            oss << "| " << snap.year << " | ";
            oss << snap.neoplastic_alive << " | ";
            oss << snap.neoplastic_apoptosis_susceptible << " | ";
            oss << snap.neoplastic_apoptosis_resistant << " | ";
            oss << std::fixed << std::setprecision(1) << pct_inmortales << "% |\n";
        }

        oss << "\n";

        // Definiciones
        oss << "---\n\n";
        oss << "## Definiciones\n\n";
        oss << "- **Total (V+M):** Todas las células (vivas + muertas acumuladas)\n";
        oss << "- **Vivas (P+N):** Células vivas actuales (protegidas + neoplásticas vivas)\n";
        oss << "- **Muertas (Apoptosis):** Total acumulado de células muertas\n";
        oss << "- **Neoplásticas (Vivas):** Células transformadas que siguen vivas\n";
        oss << "- **Protegidas (Vivas):** Células normales/sanas que siguen vivas\n";
        oss << "- **Min D1:** Mínimo D1 (DNA damage counter) en población viva\n";
        oss << "- **Max D1:** Máximo D1 (DNA damage counter) en población viva\n";
        oss << "- **Min D2:** Mínimo D2 (Immunosuppression counter) en población viva\n";
        oss << "- **Max D2:** Máximo D2 (Immunosuppression counter) en población viva\n";
        oss << "- **TP53++:** Porcentaje de células con TP53 wild-type (+/+)\n";
        oss << "- **TP53+-:** Porcentaje de células con TP53 heterocigoto (+/-)\n";
        oss << "- **TP53--:** Porcentaje de células con TP53 homocigoto (-/-)\n";
        oss << "\n";
        oss << "### Apoptosis en Neoplásticas\n";
        oss << "- **Susceptibles a Apoptosis:** Neoplásticas que pueden ser eliminadas por señal de apoptosis\n";
        oss << "- **Resistentes a Apoptosis:** Neoplásticas inmortales que evaden apoptosis\n";
        oss << "- **% Inmortales:** Porcentaje de neoplásticas que son resistentes a apoptosis\n";

        return oss.str();
    }

    std::string PopulationTracker::toCSV(const std::string& scenario_name, int run_number,
                                        const std::string& config_json) const {
        std::ostringstream oss;

        // Líneas comentadas con configuración JSON - cada línea con # al inicio
        if (!config_json.empty()) {
            oss << "# ========== Configuration JSON ==========\n";
            // Procesar línea por línea del JSON
            std::istringstream iss(config_json);
            std::string line;
            while (std::getline(iss, line)) {
                oss << "# " << line << "\n";
            }
            oss << "# ==========================================\n";
            oss << "#\n";
        }

        // Encabezado CSV con D1/D2
        oss << "scenario,run,year,total_cells,alive_cells,dead_cells_cumulative,";
        oss << "neoplastic_alive,protected_alive,min_d1,max_d1,min_d2,max_d2,";
        oss << "tp53_plus_plus_pct,tp53_plus_minus_pct,tp53_minus_minus_pct,";
        oss << "neoplastic_apoptosis_susceptible,neoplastic_apoptosis_resistant\n";

        // Filas de datos
        for (const auto& snap : snapshots_) {
            oss << scenario_name << ",";
            oss << run_number << ",";
            oss << snap.year << ",";
            oss << snap.total_cells << ",";
            oss << snap.alive_cells << ",";
            oss << snap.dead_cells_cumulative << ",";
            oss << snap.neoplastic_alive << ",";
            oss << snap.protected_alive << ",";
            oss << formatDouble(snap.min_d1) << ",";
            oss << formatDouble(snap.max_d1) << ",";
            oss << formatDouble(snap.min_d2) << ",";
            oss << formatDouble(snap.max_d2) << ",";
            oss << formatDouble(snap.tp53_plus_plus_pct) << ",";
            oss << formatDouble(snap.tp53_plus_minus_pct) << ",";
            oss << formatDouble(snap.tp53_minus_minus_pct) << ",";
            oss << snap.neoplastic_apoptosis_susceptible << ",";
            oss << snap.neoplastic_apoptosis_resistant << "\n";
        }

        return oss.str();
    }

    int PopulationTracker::getTumorThresholdYear() const {
        for (const auto& snap : snapshots_) {
            if (snap.alive_cells > 0) {
                double neoplastic_pct = static_cast<double>(snap.neoplastic_alive) / snap.alive_cells;
                if (neoplastic_pct > tumor_threshold_) {
                    return snap.year;
                }
            }
        }
        return -1;  // Nunca se supera el threshold
    }

    std::string PopulationTracker::toSummaryCsv(const std::string& scenario_name, int run_number,
                                               const std::string& config_desc,
                                               long execution_time_ms,
                                               const std::string& config_json) const {
        std::ostringstream oss;

        // Configuración JSON como comentarios
        if (!config_json.empty()) {
            oss << "# ========== Configuration ==========\n";
            std::istringstream iss(config_json);
            std::string line;
            while (std::getline(iss, line)) {
                oss << "# " << line << "\n";
            }
            oss << "# ====================================\n";
            oss << "#\n";
        }

        // Información general
        int tumor_year = getTumorThresholdYear();

        oss << "key,value\n";
        oss << "scenario," << scenario_name << "\n";
        oss << "run," << run_number << "\n";
        oss << "execution_time_ms," << execution_time_ms << "\n";
        oss << "config_description,\"" << config_desc << "\"\n";
        oss << "tumor_threshold_pct," << (tumor_threshold_ * 100.0) << "\n";
        oss << "tumor_threshold_year," << tumor_year << "\n";
        oss << "total_years," << (snapshots_.size() > 0 ? snapshots_.back().year : 0) << "\n";

        if (!snapshots_.empty()) {
            oss << "initial_cells," << snapshots_[0].total_cells << "\n";
            oss << "final_alive_cells," << snapshots_.back().alive_cells << "\n";
            oss << "final_dead_cells," << snapshots_.back().dead_cells_cumulative << "\n";
            oss << "final_neoplastic_cells," << snapshots_.back().neoplastic_alive << "\n";
            double final_neo_pct = snapshots_.back().alive_cells > 0 ?
                (100.0 * snapshots_.back().neoplastic_alive / snapshots_.back().alive_cells) : 0.0;
            oss << "final_neoplastic_pct," << std::fixed << std::setprecision(2) << final_neo_pct << "\n";
        }

        // Tabla de evolución neoplástica año a año
        oss << "#\n";
        oss << "# ===== Neoplastic Percentage Evolution =====\n";
        oss << "year,neoplastic_pct,threshold_exceeded\n";

        for (const auto& snap : snapshots_) {
            double neo_pct = snap.alive_cells > 0 ?
                (100.0 * snap.neoplastic_alive / snap.alive_cells) : 0.0;
            int exceeded = (neo_pct > tumor_threshold_ * 100.0) ? 1 : 0;

            oss << snap.year << ","
                << std::fixed << std::setprecision(2) << neo_pct << ","
                << exceeded << "\n";
        }

        return oss.str();
    }

    void PopulationTracker::saveToFiles(const std::string& output_dir,
                                       const std::string& scenario_name,
                                       int run_number,
                                       const std::string& config_desc,
                                       long execution_time_ms,
                                       const std::string& config_json) const {
        namespace fs = std::filesystem;

        // Crear directorio si no existe
        fs::create_directories(output_dir);

        // Guardar Markdown
        std::string md_file = output_dir + "/" + scenario_name + "_run" + std::to_string(run_number) + "_POPULATION.md";
        std::ofstream md_stream(md_file);
        md_stream << toMarkdown(scenario_name, run_number, config_desc, config_json);
        md_stream.close();

        // Guardar CSV de población
        std::string csv_file = output_dir + "/" + scenario_name + "_run" + std::to_string(run_number) + "_POPULATION.csv";
        std::ofstream csv_stream(csv_file);
        csv_stream << toCSV(scenario_name, run_number, config_json);
        csv_stream.close();

        // Guardar CSV de resumen con threshold tumoral
        std::string summary_file = output_dir + "/" + scenario_name + "_run" + std::to_string(run_number) + "_SUMMARY.csv";
        std::ofstream summary_stream(summary_file);
        summary_stream << toSummaryCsv(scenario_name, run_number, config_desc, execution_time_ms, config_json);
        summary_stream.close();
    }

} // namespace application

