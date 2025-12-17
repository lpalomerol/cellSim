#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <iomanip>
#include <limits>
#include "../src/application/config/SimulationConfig.h"
#include "../src/domain/cell/CellFactory_v2.h"
#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/adapters/FixedNoise.h"
#include "../src/domain/adapters/RandomNoise.h"
#include "../src/domain/tissue/TissueV2.h"

/**
 * single_cell_evolution.cpp
 *
 * Simula la evolución de una única célula en un tejido.
 * Registra: estado de mutaciones, neoplasia, evolución cromosómica, instabilidad.
 */

int main(int argc, char* argv[]) {
    std::cout << "=== Single Cell Evolution Simulator ===" << std::endl;
    std::cout << "Simulating evolution of a single cell in a tissue\n" << std::endl;

    // Cargar configuración
    auto config = application::SimulationConfig::loadSingleCell(/* verbose = */ true);

    std::cout << "\nParámetros de simulación:" << std::endl;
    std::cout << "  - Años simulados: " << config.max_t << std::endl;
    std::cout << "  - Células: 1 (única)" << std::endl;
    std::cout << "  - Probabilidad base neoplasia (neoplasm_k): " << config.neoplasm_k << std::endl;
    std::cout << "\nGenes y umbrales:" << std::endl;
    for (const auto& [gene, threshold] : config.gene_thresholds) {
        std::cout << "  - " << gene << ": threshold=" << threshold
                  << ", instability_k=" << config.gene_instability_k.at(gene) << std::endl;
    }
    std::cout << "\n" << std::string(50, '-') << std::endl;

    // Crear genoma
    domain::Genome genome = domain::genome_factory::makeDefaultGenome(
        config.gene_thresholds,
        config.gene_instability_k,
        config.logger
    );

    // Crear ruido
    std::unique_ptr<domain::INoiseSource> noise;
    if (config.use_random_noise) {
        std::cout << "Ruido: RandomNoise (seed=" << config.seed << ")" << std::endl;
        noise = std::make_unique<domain::adapters::RandomNoise>(static_cast<unsigned>(config.seed));
    } else {
        std::cout << "Ruido: FixedNoise(1.0)" << std::endl;
        noise = std::make_unique<domain::adapters::FixedNoise>(domain::CellNoise{1.0});
    }

    // Crear tejido
    auto tissue = std::make_shared<domain::TissueV2>(config.logger);
    tissue->setId(1);

    // Crear célula única (AgenticCell_v2)
    auto single_cell = domain::CellFactory_v2::createCustomCell(
        genome,
        config.neoplasm_k,
        0.0001,      // low_delta_instability
        0.0002,      // high_delta_instability
        config.division_rate,
        config.neoplastic_division_rate,
        config.enable_big_bang_mode,
        config.apoptosis_threshold,
        2.0,         // d1_primer_threshold
        5.0,         // d2_apoptosis_threshold
        config.logger
    );

    tissue->addCell(std::move(single_cell));

    // Abrir fichero de salida
    std::ofstream outfile("single_cell_evolution_log.txt");
    if (!outfile.is_open()) {
        std::cerr << "Error: no se pudo abrir fichero de salida" << std::endl;
        return 1;
    }

    outfile << "Single Cell Evolution Log\n";
    outfile << "Start time: Year 0\n";
    outfile << std::string(80, '=') << "\n\n";

    // Bucle de simulación automática
    int year = 0;
    int neoplastic_transitions = 0;
    int last_neoplastic_count = 0;

    while (year < config.max_t) {
        // Ejecutar un ciclo de vida del tejido (que contiene la célula)
        tissue->live();

        // Contar células neoplásticas en el tejido
        int current_neoplastic = 0;
        for (std::size_t i = 0; i < tissue->size(); ++i) {
            const auto* cell = tissue->getCell(i);
            if (cell && cell->isNeoplastic()) {
                current_neoplastic++;
            }
        }

        if (current_neoplastic > last_neoplastic_count) {
            neoplastic_transitions++;
            last_neoplastic_count = current_neoplastic;
        }

        // Imprimir estado actual en consola cada 10 años
        if (year % 10 == 0 || year == config.max_t - 1) {
            std::cout << "Year " << std::setw(3) << year
                      << " | Neoplastic cells: " << current_neoplastic
                      << " | Total cells: " << tissue->size() << std::endl;
        }

        // Registrar en fichero
        outfile << "Year: " << year << "\n";
        outfile << "  Neoplastic cells: " << current_neoplastic << "\n";
        outfile << "  Total cells: " << tissue->size() << "\n";
        outfile << "  Total transitions to neoplasia: " << neoplastic_transitions << "\n";
        outfile << "\n";

        year++;

        // Pausa interactiva: presionar Enter para continuar
        if (year < config.max_t) {
            std::cout << "\n[Presiona ENTER para ver el siguiente año...]";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    // Resumen final
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "Resumen final:" << std::endl;
    std::cout << "  - Años simulados: " << year << std::endl;
    std::cout << "  - Células neoplásicas finales: " << last_neoplastic_count << std::endl;
    std::cout << "  - Transiciones a neoplasia: " << neoplastic_transitions << std::endl;
    std::cout << "  - Células totales: " << tissue->size() << std::endl;

    outfile << "\n" << std::string(80, '=') << "\n";
    outfile << "Final Summary:\n";
    outfile << "  - Total years simulated: " << year << "\n";
    outfile << "  - Final neoplastic cells: " << last_neoplastic_count << "\n";
    outfile << "  - Total neoplasia transitions: " << neoplastic_transitions << "\n";
    outfile << "  - Total cells: " << tissue->size() << "\n";
    outfile.close();

    std::cout << "\nResultados guardados en: single_cell_evolution_log.txt" << std::endl;

    return 0;
}

