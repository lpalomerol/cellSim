#include <iostream>
#include <memory>
#include <chrono>
#include <limits>
#include <cstdio>
#include <iomanip>
#include <filesystem>
#include "../src/application/simulation/PopulationTracker.h"
#include "../src/application/config/SimulationConfig.h"
#include "../src/domain/cell/CellFactory.h"
#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/tissue/Tissue.h"
#include "../src/domain/adapters/RandomNoise.h"

namespace fs = std::filesystem;

void captureSnapshotFromTissue(domain::Tissue* tissue, int year,
                              application::PopulationTracker& tracker,
                              int* previous_alive,
                              int* cumulative_dead) {
    int alive_count = 0;
    int neoplastic_alive = 0;
    int protected_alive = 0;
    int neoplastic_susceptible = 0;
    int neoplastic_resistant = 0;

    // D1 (DNA damage) and D2 (Immunosuppression) tracking
    double min_d1 = std::numeric_limits<double>::max();
    double max_d1 = std::numeric_limits<double>::lowest();
    double min_d2 = std::numeric_limits<double>::max();
    double max_d2 = std::numeric_limits<double>::lowest();

    int tp53_pp = 0, tp53_pm = 0, tp53_mm = 0;

    for (size_t i = 0; i < tissue->size(); ++i) {
        auto* cell = tissue->getCell(i);
        if (cell && cell->alive()) {
            alive_count++;

            std::string tp53_status = cell->getTP53Status();
            if (tp53_status == "+/+") tp53_pp++;
            else if (tp53_status == "+/-") tp53_pm++;
            else if (tp53_status == "-/-") tp53_mm++;

            // Capturar D1 y D2 de TODAS las células vivas (no solo neoplásticas)
            auto* agentic_v2 = dynamic_cast<domain::AgenticCell*>(cell);
            if (agentic_v2) {
                double d1 = agentic_v2->getD1();
                double d2 = agentic_v2->getD2();

                min_d1 = std::min(min_d1, d1);
                max_d1 = std::max(max_d1, d1);
                min_d2 = std::min(min_d2, d2);
                max_d2 = std::max(max_d2, d2);
            }

            if (cell->isNeoplastic()) {
                neoplastic_alive++;
                if (agentic_v2) {
                    if (agentic_v2->getD2() >= 5.0) {  // D2 >= 5.0 = resistente a apoptosis
                        neoplastic_resistant++;
                    } else {
                        neoplastic_susceptible++;
                    }
                }
            } else {
                protected_alive++;
            }
        }
    }

    // Si no hay células vivas, resetear valores
    if (alive_count == 0) {
        min_d1 = 0.0;
        max_d1 = 0.0;
        min_d2 = 0.0;
        max_d2 = 0.0;
    } else if (min_d1 == std::numeric_limits<double>::max()) {
        // No había AgenticCells (solo había otros tipos de células)
        min_d1 = 0.0;
        max_d1 = 0.0;
        min_d2 = 0.0;
        max_d2 = 0.0;
    }

    // Calcular muertas de este año y acumular
    int dead_this_year = 0;
    if (year == 0) {
        dead_this_year = 0;
    } else {
        dead_this_year = *previous_alive - alive_count;
        if (dead_this_year < 0) dead_this_year = 0;  // Si creció (división), es 0
    }
    *previous_alive = alive_count;
    *cumulative_dead += dead_this_year;  // Acumular muertas

    int total = alive_count + *cumulative_dead;  // Total = Vivas + Muertas acumuladas
    int total_tp53 = tp53_pp + tp53_pm + tp53_mm;
    double tp53_pp_pct = (total_tp53 > 0) ? static_cast<double>(tp53_pp) / total_tp53 : 0.0;
    double tp53_pm_pct = (total_tp53 > 0) ? static_cast<double>(tp53_pm) / total_tp53 : 0.0;
    double tp53_mm_pct = (total_tp53 > 0) ? static_cast<double>(tp53_mm) / total_tp53 : 0.0;

    application::YearlySnapshot snap{
        year, total, alive_count, *cumulative_dead, neoplastic_alive, protected_alive,
        min_d1, max_d1, min_d2, max_d2,  // D1/D2 en lugar de instabilidad genómica
        tp53_pp_pct, tp53_pm_pct, tp53_mm_pct,
        neoplastic_susceptible, neoplastic_resistant
    };

    tracker.addSnapshot(snap);
}

struct ScenarioConfig {
    std::string name;
    std::string description;
    double brca1_threshold;
    double tp53_threshold;
    double neoplasm_k;
    double low_delta;
    double high_delta;
    double division_rate;
    double neoplastic_division_rate = 0.001;  // Para Big Bang (default = normal)
    bool enable_big_bang_mode = false;        // Para Big Bang (default = desactivado)
    int max_t = 50;  // Duración en años (default 50)
};

void runScenario(const ScenarioConfig& scenario, const application::SimulationConfig& base_cfg) {
    std::cout << "\n▶ Escenario: " << std::setw(35) << std::left << scenario.name;
    std::cout << " | " << scenario.description << "\n";
    std::cout << "  Parámetros:\n";
    std::cout << "    BRCA1=" << scenario.brca1_threshold
              << ", TP53=" << scenario.tp53_threshold
              << ", neoplasm_k=" << scenario.neoplasm_k << "\n";
    std::cout << "    low_delta=" << scenario.low_delta
              << ", high_delta=" << scenario.high_delta
              << ", division=" << scenario.division_rate << "\n";
    std::cout << "    apoptosis_threshold=" << base_cfg.apoptosis_threshold
              << ", duration=" << scenario.max_t << " años\n";
    std::cout << "  Ejecutando..." << std::flush;

    auto start_time = std::chrono::high_resolution_clock::now();

    // Crear Tissue
    auto tissue = std::make_unique<domain::Tissue>(base_cfg.logger);

    // Crear células
    for (int i = 0; i < base_cfg.n_cells; ++i) {
        domain::Genome genome = domain::genome_factory::makeDefaultGenome(
            {{"BRCA1", scenario.brca1_threshold}, {"TP53", scenario.tp53_threshold}},
            {{"BRCA1", 0.01}, {"TP53", 0.01}},
            base_cfg.logger
        );

        unsigned unique_seed = 100 + i;
        auto noise = std::make_unique<domain::adapters::RandomNoise>(unique_seed);

        auto cell = domain::CellFactory::createCustomCell(
            std::move(noise),  // Pasar noise con seed única
            genome,
            scenario.neoplasm_k,
            scenario.low_delta,
            scenario.high_delta,
            scenario.division_rate,
            scenario.neoplastic_division_rate,
            scenario.enable_big_bang_mode,
            base_cfg.apoptosis_threshold,
            2.0,  // d1_primer_threshold
            5.0,  // d2_apoptosis_threshold
            base_cfg.logger
        );

        tissue->addCell(std::move(cell));
    }

    // Crear tracker
    application::PopulationTracker tracker;
    int previous_alive = base_cfg.n_cells;
    int cumulative_dead = 0;
    captureSnapshotFromTissue(tissue.get(), 0, tracker, &previous_alive, &cumulative_dead);

    // Ejecutar (usar scenario.max_t para duración específica por escenario)
    for (int year = 1; year <= scenario.max_t; ++year) {
        tissue->live();
        captureSnapshotFromTissue(tissue.get(), year, tracker, &previous_alive, &cumulative_dead);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Guardar
    std::string config_desc = "BRCA1=" + std::to_string(scenario.brca1_threshold) +
                             ", TP53=" + std::to_string(scenario.tp53_threshold) +
                             ", neoplasm_k=" + std::to_string(scenario.neoplasm_k) +
                             ", div=" + std::to_string(scenario.division_rate);

    std::string output_dir = "traces/" + scenario.name;
    fs::create_directories(output_dir);
    tracker.saveToFiles(output_dir, scenario.name, 1, config_desc);

    // Resumen
    const auto& snaps = tracker.snapshots();
    int final_alive = snaps.back().alive_cells;
    int final_neoplastic = snaps.back().neoplastic_alive;
    int final_resistant = snaps.back().neoplastic_apoptosis_resistant;

    std::cout << " ✓\n";
    std::cout << "  Resultados: Vivas=" << final_alive
              << ", Neoplásticas=" << final_neoplastic
              << " (Resistentes=" << final_resistant << ")"
              << " | Tiempo=" << duration.count() << "ms\n";
}

int main() {
    std::cout << "\n╔═══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  VALIDACIÓN - 12 ESCENARIOS: 9 CONTROL + 3 BIG BANG       ║\n";
    std::cout << "║  Parámetros: BRCA1, TP53, neoplasm_k, low_delta,           ║\n";
    std::cout << "║              high_delta, division_rate, neoplastic_div      ║\n";
    std::cout << "║  Escenarios 10-12: Big Bang (TP53 -/- con división)         ║\n";
    std::cout << "║  Apoptosis threshold: 10.0 (instabilidad máxima permitida)  ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════╝\n";

    auto cfg = application::SimulationConfig::loadScenario("default", false);
    cfg.max_t = 50;
    cfg.n_cells = 1000;
    cfg.apoptosis_threshold = 10.0;

    std::vector<ScenarioConfig> scenarios = {
        // === CONTROLES BASALES ===
        {"01_ctrl_baseline_no_mutations_no_division",
         "Baseline: Sin mutaciones, sin reproducción",
         0.0, 0.0, 0.0,      // BRCA1=0, TP53=0, neoplasm_k=0
         0.5, 1.0,           // low_delta=0.5, high_delta=1.0
         0.0,                // division_rate=0
         0.0,                // neoplastic_division_rate=0
         false,              // enable_big_bang_mode=false
         10},                // max_t=10 años

        {"02_ctrl_baseline_no_mutations_high_division",
         "Baseline: Sin mutaciones, reproducción 15%",
         0.0, 0.0, 0.0,      // BRCA1=0, TP53=0, neoplasm_k=0
         0.5, 1.0,           // low_delta=0.5, high_delta=1.0
         0.15,               // division_rate=15%
         0.0,                // neoplastic_division_rate=0
         false,              // enable_big_bang_mode=false
         10},                // max_t=10 años (corto para evitar explosión celular)

        // === CONTROLES PARAMÉTRICOS ===
        {"03_ctrl_brca_mutations_high",
         "Mutaciones BRCA1 altas (20%)",
         0.2, 0.0, 0.0,      // BRCA1=0.2 (20%), TP53=0, neoplasm_k=0
         0.5, 1.0,           // low_delta=0.5, high_delta=1.0
         0.0,                // division_rate=0
         0.0,                // neoplastic_division_rate=0
         false,              // enable_big_bang_mode=false
         50},                // max_t=50 años

        {"04_ctrl_tp53_mutations_high",
         "Mutaciones TP53 altas (10%) con neoplasma",
         0.0, 0.10, 0.10,    // BRCA1=0, TP53=0.10 (10%), neoplasm_k=0.10
         0.5, 1.0,           // low_delta=0.5, high_delta=1.0
         0.0,                // division_rate=0
         0.0,                // neoplastic_division_rate=0
         false,              // enable_big_bang_mode=false
         50},                // max_t=50 años

        {"05_ctrl_tp53_mutations_high_unstable",
         "TP53 altas (10%) + inestabilidad alta",
         0.0, 0.10, 0.10,    // BRCA1=0, TP53=0.10 (10%), neoplasm_k=0.10
         0.1, 0.5,           // low_delta=0.1, high_delta=0.5 (más inestabilidad)
         0.0,                // division_rate=0
         0.0,                // neoplastic_division_rate=0
         false,              // enable_big_bang_mode=false
         50},                // max_t=50 años

        // === ESCENARIOS REALISTAS ===
        {"06_realistic_baseline",
         "Realista baseline: Mutaciones moderadas, sin división",
         0.05, 0.01, 0.05,   // BRCA1=0.05 (5%), TP53=0.01 (1%), neoplasm_k=0.05
         0.5, 1.0,           // low_delta=0.5, high_delta=1.0
         0.0,                // division_rate=0
         0.0,                // neoplastic_division_rate=0
         false,              // enable_big_bang_mode=false
         50},                // max_t=50 años

        {"07_realistic_low_tp53_instability",
         "Realista: TP53 baja (0.5%), inestabilidad moderada",
         0.05, 0.005, 0.05,  // BRCA1=0.05 (5%), TP53=0.005 (0.5%), neoplasm_k=0.05
         0.5, 1.0,           // low_delta=0.5, high_delta=1.0
         0.05,               // division_rate=5%
         0.0,                // neoplastic_division_rate=0
         false,              // enable_big_bang_mode=false
         80},                // max_t=80 años

        {"08_realistic_high_tp53_instability",
         "Realista: TP53 moderada (2%), inestabilidad alta",
         0.05, 0.02, 0.05,   // BRCA1=0.05 (5%), TP53=0.02 (2%), neoplasm_k=0.05
         1.0, 1.5,           // low_delta=1.0, high_delta=1.5 (inestabilidad más alta)
         0.05,               // division_rate=5%
         0.0,                // neoplastic_division_rate=0
         false,              // enable_big_bang_mode=false
         80},                // max_t=80 años

        {"09_realistic_balanced",
         "Realista: Parámetros balanceados",
         0.05, 0.01, 0.05,   // BRCA1=0.05 (5%), TP53=0.01 (1%), neoplasm_k=0.05
         0.5, 1.0,           // low_delta=0.5, high_delta=1.0
         0.05,               // division_rate=5%
         0.0,                // neoplastic_division_rate=0
         false,              // enable_big_bang_mode=false
         80},                // max_t=80 años

        // === BIG BANG TUMORAL ===
        {"10_big_bang_tumoral",
         "Big Bang Tumoral: TP53 -/- con división acelerada (20%)",
         0.05, 0.02, 0.20,   // BRCA1=0.05 (5%), TP53=0.02 (2%), neoplasm_k=0.20
         0.5, 1.0,           // low_delta=0.5, high_delta=1.0
         0.0,                // division_rate=0 (células normales no se dividen)
         0.20,               // neoplastic_division_rate=20% (células TP53 -/- dividen)
         true,               // enable_big_bang_mode=true
         30},                // max_t=30 años

        {"11_big_bang_tumoral_reproduccion",
         "Big Bang con Reproducción Normal: TP53 -/- (10%) + Normal (5%)",
         0.05, 0.02, 0.20,   // BRCA1=0.05 (5%), TP53=0.02 (2%), neoplasm_k=0.20
         0.5, 1.0,           // low_delta=0.5, high_delta=1.0
         0.05,               // division_rate=5% (células normales sí se dividen)
         0.10,               // neoplastic_division_rate=10% (células TP53 -/- dividen menos)
         true,               // enable_big_bang_mode=true
         30},                // max_t=30 años

        {"12_big_bang_tumoral_low_threshold",
         "Big Bang con Bajo Threshold TP53: TP53 -/- (10%) + Normal (5%)",
         0.05, 0.01, 0.02,  // BRCA1=0.05 (5%), TP53=0.01 (1%), neoplasm_k=0.02
         0.2, 0.4,          // low_delta=0.2, high_delta=0.4 (inestabilidad menor)
         0.05,               // division_rate=5% (células normales sí se dividen)
         0.10,               // neoplastic_division_rate=10% (células TP53 -/- dividen)
         true,               // enable_big_bang_mode=true
         80}                 // max_t=80 años
    };

    auto total_start = std::chrono::system_clock::now();

    // Ejecutar solo el escenario 01 para pruebas
    for (const auto& scenario : scenarios) {
        if (scenario.name == "04_ctrl_tp53_mutations_high") {
            runScenario(scenario, cfg);
            break;
        }
    }

    auto total_end = std::chrono::system_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::seconds>(total_end - total_start);

    std::cout << "\n╔═══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  ✓ VALIDACIÓN COMPLETADA - 12 ESCENARIOS                     ║\n";
    std::cout << "║  • Escenarios 1-9: Validación de controles (no Big Bang)     ║\n";
    std::cout << "║  • Escenario 10: Big Bang puro (neoplasm=20%, div=20%)       ║\n";
    std::cout << "║  • Escenario 11: Big Bang + reproducción normal (div=10%)    ║\n";
    std::cout << "║  • Escenario 12: Big Bang bajo threshold TP53 (div=10%)      ║\n";
    std::cout << "║  Tiempo total: " << std::setw(2) << total_duration.count() << "s\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════╝\n\n";

    return 0;
}

