#include "PopulationTracker.h"
#include <filesystem>

namespace application {

    std::string PopulationTracker::toMarkdown(const std::string& scenario_name, int run_number,
                                             const std::string& config_desc) const {
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

        oss << "## Evolución Anual de Población\n\n";

        // Encabezado de tabla
        oss << "| Año | Total (V+M) | Vivas (P+N) | Muertas (Apoptosis) | Neoplásticas (Vivas) | Protegidas (Vivas) | Min Inest. | Max Inest. | TP53++ (%) | TP53+- (%) | TP53-- (%) |\n";
        oss << "|-----|-------------|-------------|---------------------|----------------------|-------------------|-----------|-----------|-----------|-----------|----------|\n";

        // Filas de datos
        for (const auto& snap : snapshots_) {
            oss << "| " << snap.year << " | ";
            oss << snap.total_cells << " | ";
            oss << snap.alive_cells << " | ";
            oss << snap.dead_cells_cumulative << " | ";
            oss << snap.neoplastic_alive << " | ";
            oss << snap.protected_alive << " | ";
            oss << formatDouble(snap.min_genomic_instability) << " | ";
            oss << formatDouble(snap.max_genomic_instability) << " | ";
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
        oss << "- **Min Inest.:** Índice mínimo de inestabilidad genómica en población viva\n";
        oss << "- **Max Inest.:** Índice máximo de inestabilidad genómica en población viva\n";
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

    std::string PopulationTracker::toCSV(const std::string& scenario_name, int run_number) const {
        std::ostringstream oss;

        // Encabezado CSV (con nuevos campos de apoptosis en neoplásticas)
        oss << "scenario,run,year,total_cells,alive_cells,dead_cells_cumulative,";
        oss << "neoplastic_alive,protected_alive,min_genomic_instability,max_genomic_instability,";
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
            oss << formatDouble(snap.min_genomic_instability) << ",";
            oss << formatDouble(snap.max_genomic_instability) << ",";
            oss << formatDouble(snap.tp53_plus_plus_pct) << ",";
            oss << formatDouble(snap.tp53_plus_minus_pct) << ",";
            oss << formatDouble(snap.tp53_minus_minus_pct) << ",";
            oss << snap.neoplastic_apoptosis_susceptible << ",";
            oss << snap.neoplastic_apoptosis_resistant << "\n";
        }

        return oss.str();
    }

    void PopulationTracker::saveToFiles(const std::string& output_dir,
                                       const std::string& scenario_name,
                                       int run_number,
                                       const std::string& config_desc) const {
        namespace fs = std::filesystem;

        // Crear directorio si no existe
        fs::create_directories(output_dir);

        // Guardar Markdown
        std::string md_file = output_dir + "/" + scenario_name + "_run" + std::to_string(run_number) + "_POPULATION.md";
        std::ofstream md_stream(md_file);
        md_stream << toMarkdown(scenario_name, run_number, config_desc);
        md_stream.close();

        // Guardar CSV
        std::string csv_file = output_dir + "/" + scenario_name + "_run" + std::to_string(run_number) + "_POPULATION.csv";
        std::ofstream csv_stream(csv_file);
        csv_stream << toCSV(scenario_name, run_number);
        csv_stream.close();
    }

} // namespace application

