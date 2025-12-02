#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <memory>
#include <iomanip>
#include <limits>
#include "../src/domain/cell/CellFactory.h"
#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/adapters/FixedNoise.h"
#include "../src/domain/adapters/RandomNoise.h"
#include "../src/domain/adapters/Logger.h"
#include "../src/domain/tissue/Tissue.h"

/**
 * single_cell_evolution.cpp
 *
 * Programa para evaluar la evolución de una única célula en un tejido.
 *
 * Simula el comportamiento de una célula única a lo largo del tiempo y registra:
 * - Estado de mutaciones (BRCA1, TP53, etc.)
 * - Identificación de neoplasia
 * - Evolución del genoma
 * - Cambios en inestabilidad cromosómica
 */

int main(int argc, char* argv[]) {
    std::cout << "=== Single Cell Evolution Simulator ===" << std::endl;
    std::cout << "Simulating evolution of a single cell in a tissue\n" << std::endl;

    // Parámetros de simulación
    int max_t = 100;                      // Años de simulación
    double neoplasm_k = 0.05;            // Probabilidad base de neoplasia
    bool use_random_noise = true;         // true = RandomNoise, false = FixedNoise
    unsigned seed = 42u;                  // Semilla para reproducibilidad

    // Umbrales y parámetros de inestabilidad por gen
    std::unordered_map<std::string, double> gene_thresholds{
        {"BRCA1", 0.01},
        {"TP53", 0.25}
    };
    std::unordered_map<std::string, double> gene_instability_k{
        {"BRCA1", 0.01},
        {"TP53", 0.5}
    };

    // Crear logger con trazas verbose
    auto logger = std::make_shared<domain::adapters::Logger>();
    logger->setVerbose(true);  // Cambiar a true si quieres detalles en consola

    // Crear genoma con parámetros específicos
    domain::Genome genome = domain::genome_factory::makeDefaultGenome(
        gene_thresholds,
        gene_instability_k,
        logger
    );

    // Crear fuente de ruido
    std::unique_ptr<domain::INoiseSource> noise;
    if (use_random_noise) {
        std::cout << "Ruido: RandomNoise (seed=" << seed << ")" << std::endl;
        noise = std::make_unique<adapters::RandomNoise>(seed);
    } else {
        std::cout << "Ruido: FixedNoise(1.0)" << std::endl;
        noise = std::make_unique<adapters::FixedNoise>(domain::CellNoise{1.0});
    }


    // Crear un tejido para contener la célula
    auto tissue = std::make_shared<domain::Tissue>(logger);
    tissue->setId(1);  // Asignar ID al tejido

    // Crear una única célula
    auto single_cell = domain::cell_factory::createAgenticCell(
        std::move(noise),
        std::move(genome),
        neoplasm_k,
        0.0001,    // low_delta_instability
        0.0002,    // high_detal_instability
        0.1,     // division_rate
        0.01,      // apoptosis_instablity_threshold
        logger
    );

    // Agregar la célula al tejido
    tissue->addCell(std::move(single_cell));


    std::cout << "\nParámetros de simulación:" << std::endl;
    std::cout << "  - Años simulados: " << max_t << std::endl;
    std::cout << "  - Células: 1 (única)" << std::endl;
    std::cout << "  - Probabilidad base neoplasia (neoplasm_k): " << neoplasm_k << std::endl;
    std::cout << "\nGenes y umbrales:" << std::endl;
    for (const auto& [gene, threshold] : gene_thresholds) {
        std::cout << "  - " << gene << ": threshold=" << threshold
                  << ", instability_k=" << gene_instability_k.at(gene) << std::endl;
    }
    std::cout << "\n" << std::string(50, '-') << std::endl;

    // Abrir fichero de salida para registrar evolución
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

    while (year < max_t) {
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
        if (year % 10 == 0 || year == max_t - 1) {
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
        if (year < max_t) {
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

