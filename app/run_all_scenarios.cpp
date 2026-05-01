#include <iostream>
#include <memory>
#include <chrono>
#include <limits>
#include <cstdio>
#include <iomanip>
#include <filesystem>
#include <numeric>
#include <algorithm>
#include <cmath>
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

            double d1 = cell->getD1();
            double d2 = cell->getD2();
            min_d1 = std::min(min_d1, d1);
            max_d1 = std::max(max_d1, d1);
            min_d2 = std::min(min_d2, d2);
            max_d2 = std::max(max_d2, d2);

            if (cell->isNeoplastic()) {
                neoplastic_alive++;
                if (cell->hasEvadedApoptosis()) {
                    neoplastic_resistant++;
                } else {
                    neoplastic_susceptible++;
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
    double low_delta;
    double high_delta;
    double division_rate;
    double neoplastic_division_rate = 0.001;
    bool enable_big_bang_mode = false;
    int max_t = 50;
    double d1_primer_threshold = 2.0;
    double d2_apoptosis_threshold = 5.0;
    double noise_cv = 0.0;
    int n_cells = 0;   // 0 = use base_cfg.n_cells
    int n_runs = 1;
};

// Run a single simulation pass; returns {onset_year (-1 if none), final_penetrance_pct}
static std::pair<int,double> runOnce(const ScenarioConfig& scenario,
                                     const application::SimulationConfig& base_cfg,
                                     int run_idx) {
    int n_cells = scenario.n_cells > 0 ? scenario.n_cells : base_cfg.n_cells;
    auto tissue = std::make_unique<domain::Tissue>(base_cfg.logger);

    for (int i = 0; i < n_cells; ++i) {
        domain::Genome genome = domain::genome_factory::makeDefaultGenome(
            {{"BRCA1", scenario.brca1_threshold}, {"TP53", scenario.tp53_threshold}},
            {{"BRCA1", 0.0}, {"TP53", 0.0}},
            base_cfg.logger
        );
        unsigned seed = static_cast<unsigned>(run_idx * 10000 + 100 + i);
        auto cell = domain::CellFactory::createCustomCell(
            std::make_unique<domain::adapters::RandomNoise>(seed),
            genome,
            scenario.low_delta, scenario.high_delta,
            scenario.division_rate, scenario.neoplastic_division_rate,
            scenario.enable_big_bang_mode,
            scenario.d1_primer_threshold, scenario.d2_apoptosis_threshold,
            base_cfg.logger, scenario.noise_cv
        );
        tissue->addCell(std::move(cell));
    }

    application::PopulationTracker tracker;
    int previous_alive = n_cells, cumulative_dead = 0;
    captureSnapshotFromTissue(tissue.get(), 0, tracker, &previous_alive, &cumulative_dead);

    int onset = -1;
    for (int year = 1; year <= scenario.max_t; ++year) {
        tissue->live();
        captureSnapshotFromTissue(tissue.get(), year, tracker, &previous_alive, &cumulative_dead);
        if (onset < 0 && tracker.snapshots().back().neoplastic_alive > 0) {
            onset = year;
        }
    }

    const auto& last = tracker.snapshots().back();
    double penetrance = last.alive_cells > 0
        ? 100.0 * last.neoplastic_alive / last.alive_cells : 0.0;

    // Save trace for first run only
    if (run_idx == 0) {
        std::string output_dir = "traces/" + scenario.name;
        fs::create_directories(output_dir);
        std::string config_desc = "BRCA1=" + std::to_string(scenario.brca1_threshold) +
                                  ", TP53=" + std::to_string(scenario.tp53_threshold) +
                                  ", div=" + std::to_string(scenario.division_rate);
        tracker.saveToFiles(output_dir, scenario.name, 1, config_desc);
    }

    return {onset, penetrance};
}

void runScenario(const ScenarioConfig& scenario, const application::SimulationConfig& base_cfg) {
    std::cout << std::defaultfloat << std::setprecision(6);
    std::cout << "\n▶ Escenario: " << std::setw(35) << std::left << scenario.name;
    std::cout << " | " << scenario.description << "\n";
    std::cout << "  Parámetros:\n";
    std::cout << "    BRCA1=" << scenario.brca1_threshold
              << ", TP53=" << scenario.tp53_threshold << "\n";
    std::cout << "    low_delta=" << scenario.low_delta
              << ", high_delta=" << scenario.high_delta
              << ", division=" << scenario.division_rate;
    if (scenario.noise_cv > 0.0) std::cout << ", noise_cv=" << scenario.noise_cv;
    std::cout << "\n";
    std::cout << ", duration=" << scenario.max_t << " años"
              << ", n_runs=" << scenario.n_runs << "\n";
    std::cout << "  Ejecutando..." << std::flush;

    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<int> onsets;
    std::vector<double> penetrances;

    for (int r = 0; r < scenario.n_runs; ++r) {
        auto [onset, pct] = runOnce(scenario, base_cfg, r);
        onsets.push_back(onset);
        penetrances.push_back(pct);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Compute stats
    auto stats = [](const std::vector<double>& v) -> std::pair<double,double> {
        double mean = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
        double var = 0.0;
        for (double x : v) var += (x - mean) * (x - mean);
        return {mean, std::sqrt(var / v.size())};
    };

    std::vector<double> onset_d;
    int no_event = 0;
    for (int o : onsets) {
        if (o >= 0) onset_d.push_back(static_cast<double>(o));
        else ++no_event;
    }

    auto [mean_pen, sd_pen] = stats(penetrances);

    std::cout << " ✓\n";
    std::cout << "  Penetrancia final: " << std::fixed << std::setprecision(1)
              << mean_pen << "% ± " << sd_pen << "%";
    if (scenario.n_runs > 1) {
        std::cout << "  (n_runs=" << scenario.n_runs << ", sin_evento=" << no_event << ")";
    }
    std::cout << "\n";

    if (!onset_d.empty()) {
        auto [mean_on, sd_on] = stats(onset_d);
        std::cout << "  Onset (año):       " << mean_on << " ± " << sd_on
                  << "  [min=" << *std::min_element(onset_d.begin(), onset_d.end())
                  << ", max=" << *std::max_element(onset_d.begin(), onset_d.end()) << "]\n";
    }

    std::cout << "  Tiempo: " << duration.count() << "ms\n";
}

int main() {
    std::cout << "\n╔═══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  VALIDACIÓN - 17 ESCENARIOS: 9 CTRL + 3 BIG BANG + 5 CALIB  ║\n";
    std::cout << "║  Parámetros: BRCA1, TP53, low_delta,                        ║\n";
    std::cout << "║              high_delta, division_rate, neoplastic_div        ║\n";
    std::cout << "║  Escenarios 10-12: Big Bang (TP53 -/- con división)           ║\n";
    std::cout << "║  Escenarios 13-17: Calibrated (lognormal noise + constant)    ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════╝\n";

    auto cfg = application::SimulationConfig::loadScenario("default", false);
    cfg.max_t = 50;
    cfg.n_cells = 1000;

    std::vector<ScenarioConfig> scenarios = {
        // === CONTROLES BASALES ===
        {"01_ctrl_baseline_no_mutations_no_division",
         "Baseline: Sin mutaciones, sin reproducción",
         0.0, 0.0,           // BRCA1=0, TP53=0
         0.5, 1.0,           // low_delta=0.5, high_delta=1.0
         0.0,                // division_rate=0
         0.0,                // neoplastic_division_rate=0
         false,              // enable_big_bang_mode=false
         10},                // max_t=10 años

        {"02_ctrl_baseline_no_mutations_high_division",
         "Baseline: Sin mutaciones, reproducción 15%",
         0.0, 0.0,           // BRCA1=0, TP53=0
         0.5, 1.0,           // low_delta=0.5, high_delta=1.0
         0.15,               // division_rate=15%
         0.0,                // neoplastic_division_rate=0
         false,              // enable_big_bang_mode=false
         10},                // max_t=10 años (corto para evitar explosión celular)

        // === CONTROLES PARAMÉTRICOS ===
        {"03_ctrl_brca_mutations_high",
         "Mutaciones BRCA1 altas (20%)",
         0.2, 0.0,           // BRCA1=0.2 (20%), TP53=0
         0.5, 1.0,           // low_delta=0.5, high_delta=1.0
         0.0,                // division_rate=0
         0.0,                // neoplastic_division_rate=0
         false,              // enable_big_bang_mode=false
         50},                // max_t=50 años

        {"04_ctrl_tp53_mutations_high",
         "Mutaciones TP53 altas (10%) con neoplasma - SIN incremento D1/D2",
         0.0, 0.10,          // BRCA1=0, TP53=0.10 (10%)
         0.0, 0.0,           // low_delta=0.0, high_delta=0.0 (D1/D2 NO aumentan)
         0.0,                // division_rate=0
         0.0,                // neoplastic_division_rate=0
         false,              // enable_big_bang_mode=false
         50},                // max_t=50 años

        {"05_ctrl_tp53_mutations_high_unstable",
         "TP53 altas (10%) + inestabilidad alta",
         0.0, 0.10,          // BRCA1=0, TP53=0.10 (10%)
         0.1, 0.5,           // low_delta=0.1, high_delta=0.5 (más inestabilidad)
         0.0,                // division_rate=0
         0.0,                // neoplastic_division_rate=0
         false,              // enable_big_bang_mode=false
         50},                // max_t=50 años

        // === ESCENARIOS REALISTAS ===
        {"06_realistic_baseline",
         "Realista baseline: Mutaciones moderadas, sin división",
         0.05, 0.01,         // BRCA1=0.05 (5%), TP53=0.01 (1%)
         0.5, 1.0,           // low_delta=0.5, high_delta=1.0
         0.0,                // division_rate=0
         0.0,                // neoplastic_division_rate=0
         false,              // enable_big_bang_mode=false
         50},                // max_t=50 años

        {"07_realistic_low_tp53_instability",
         "Realista: TP53 baja (0.5%), inestabilidad moderada",
         0.05, 0.005,        // BRCA1=0.05 (5%), TP53=0.005 (0.5%)
         0.5, 1.0,           // low_delta=0.5, high_delta=1.0
         0.05,               // division_rate=5%
         0.0,                // neoplastic_division_rate=0
         false,              // enable_big_bang_mode=false
         80},                // max_t=80 años

        {"08_realistic_high_tp53_instability",
         "Realista: TP53 moderada (2%), inestabilidad alta",
         0.05, 0.02,         // BRCA1=0.05 (5%), TP53=0.02 (2%)
         1.0, 1.5,           // low_delta=1.0, high_delta=1.5 (inestabilidad más alta)
         0.05,               // division_rate=5%
         0.0,                // neoplastic_division_rate=0
         false,              // enable_big_bang_mode=false
         80},                // max_t=80 años

        {"09_realistic_balanced",
         "Realista: Parámetros balanceados",
         0.05, 0.025,        // BRCA1=0.05 (5%), TP53=0.025 (2.5%)
         0.1, 0.3,           // low_delta=0.1, high_delta=0.3 (↑ aumentado para evasión inmune)
         0.05,               // division_rate=5%
         0.0,                // neoplastic_division_rate=0
         false,              // enable_big_bang_mode=false
         80},                // max_t=80 años

        // === BIG BANG TUMORAL ===
        {"10_big_bang_tumoral",
         "Big Bang Tumoral: TP53 -/- con división acelerada (20%)",
         0.05, 0.02,         // BRCA1=0.05 (5%), TP53=0.02 (2%)
         0.5, 1.0,           // low_delta=0.5, high_delta=1.0 (↑ aumentado para evasión inmune)
         0.0,                // division_rate=0 (células normales no se dividen)
         0.20,               // neoplastic_division_rate=20% (células TP53 -/- dividen)
         true,               // enable_big_bang_mode=true
         30},                // max_t=30 años

        {"11_big_bang_tumoral_reproduccion",
         "Big Bang con Reproducción Normal: TP53 -/- (10%) + Normal (5%)",
         0.05, 0.01,         // BRCA1=0.05 (5%), TP53=0.01 (1%)
         0.05, .3,           // low_delta=0.05, high_delta=0.3 (↑ aumentado para evasión inmune)
         0.055,              // division_rate=5% (células normales sí se dividen)
         0.25,               // neoplastic_division_rate=10% (células TP53 -/- dividen menos)
         true,               // enable_big_bang_mode=true
         80},                // max_t=80 años

        {"12_big_bang_tumoral_low_threshold",
         "Big Bang con Bajo Threshold TP53: TP53 -/- (10%) + Normal (5%)",
         0.05, 0.01,         // BRCA1=0.05 (5%), TP53=0.01 (1%)
         0.2, 0.5,           // low_delta=0.2, high_delta=0.5
         0.05,               // division_rate=5%
         0.10,               // neoplastic_division_rate=10%
         true,               // enable_big_bang_mode=true
         80},                // max_t=80 años

        // === CALIBRATED + LOGNORMAL NOISE ===
        // Parameters from calibration_sweep: δ_low=0.0317, θ_D1=4.0, θ_D2=10.0
        // → 100% penetrance, median onset ~50 years (BRCA1 carrier profile)
        {"13_calibrated_lognormal_cv03",
         "Calibrated BRCA1 carrier profile + lognormal noise (CV=0.3)",
         0.05, 0.01,
         0.0317, 0.0634,
         0.05,
         0.0,
         false,
         80,
         4.0,
         10.0,
         0.3,
         0,    // n_cells = default
         10},  // n_runs

        {"14_calibrated_lognormal_cv05",
         "Calibrated BRCA1 carrier profile + lognormal noise (CV=0.5)",
         0.05, 0.01,
         0.0317, 0.0634,
         0.05,
         0.0,
         false,
         80,
         4.0,
         10.0,
         0.5,
         0,    // n_cells = default
         10},  // n_runs

        {"15_calibrated_lognormal_cv05_2x",
         "Calibrated + CV=0.5, N=2000 (2x scenario 14)",
         0.05, 0.01,
         0.0317, 0.0634,
         0.05,
         0.0,
         false,
         80,
         4.0,
         10.0,
         0.5,
         2000, // n_cells
         10},  // n_runs

        // === CALIBRATED + CONSTANT INCREMENT (baseline comparison) ===
        {"16_calibrated_constant_n1000",
         "Calibrated params, constant increment (no noise), N=1000",
         0.05, 0.01,
         0.0317, 0.0634,
         0.05,
         0.0,
         false,
         80,
         4.0,
         10.0,
         0.0,  // noise_cv=0 → constant increment
         0,    // n_cells = default (1000)
         10},  // n_runs

        {"17_calibrated_constant_n2000",
         "Calibrated params, constant increment (no noise), N=2000",
         0.05, 0.01,
         0.0317, 0.0634,
         0.05,
         0.0,
         false,
         80,
         4.0,
         10.0,
         0.0,  // noise_cv=0 → constant increment
         2000, // n_cells
         10}   // n_runs
    };

    auto total_start = std::chrono::system_clock::now();

    // Ejecutar solo el escenario 09 para validar cambios
    for (const auto& scenario : scenarios) {
        runScenario(scenario, cfg);
    }

    auto total_end = std::chrono::system_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::seconds>(total_end - total_start);

    std::cout << "\n╔═══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  ✓ VALIDACIÓN COMPLETADA - 14 ESCENARIOS                     ║\n";
    std::cout << "║  • Escenarios 1-9:  Validación de controles (no Big Bang)    ║\n";
    std::cout << "║  • Escenario 10:    Big Bang puro (neoplasm=20%, div=20%)    ║\n";
    std::cout << "║  • Escenario 11:    Big Bang + reproducción normal           ║\n";
    std::cout << "║  • Escenario 12:    Big Bang bajo threshold TP53             ║\n";
    std::cout << "║  • Escenario 13-14: Calibrated + lognormal (CV 0.3/0.5)     ║\n";
    std::cout << "║  Tiempo total: " << std::setw(2) << total_duration.count() << "s\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════╝\n\n";

    return 0;
}

