// filepath: /home/luis/CLionProjects/cellSim/app/main_random_cells.cpp
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <random>
#include <chrono>
#include <iomanip> // para formateo de casillas
#include <limits>

#include "../src/domain/cell/CellFactory.h"
#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/adapters/RandomNoise.h"
#include "../src/domain/cell/AgenticCell.h"
#include "../src/application/ExperimentalTracking.h"
#include "../src/domain/tissue/Tissue.h"
#include "../src/domain/signal/NeoplasmSignal.h"
#include <sstream>

static void parseArgs(int argc, char** argv, int &n_cells, int &max_t, long &seed) {
    if (argc > 1) {
        std::string arg1(argv[1]);
        if (arg1 == "--help" || arg1 == "-h") {
            std::cout << "Uso: random_cells [n_cells] [max_t] [seed]\n";
            std::cout << " Ejemplo: random_cells 20 100 42\n";
            std::exit(0);
        }
        n_cells = std::stoi(arg1);
    }
    if (argc > 2) max_t = std::stoi(argv[2]);
    if (argc > 3) seed = std::stol(argv[3]);
}

int main(int argc, char** argv) {
    std::cout << "Simulación: array de células generadas aleatoriamente\n";

    // Valores por defecto; pueden sobreescribirse desde argv
    int n_cells = 1000; // número de células por defecto (ajusta según necesites)
    int max_t = 50;   // años máximo
    long seed = -1;    // semilla base (-1 = usar semilla aleatoria por cell)
    bool verbose = false; // trazas por célula

    // Para ejecuciones locales rápidas: descomenta y ajusta estas líneas para fijar parámetros manualmente.
    // (Si las dejas comentadas, el programa seguirá usando los valores por defecto o los que pases por argv.)
    // n_cells = 100; // ejemplo: descomenta y cambia según necesites
    // max_t = 100;   // ejemplo
    // seed = 42;     // ejemplo: usa -1 para semillas aleatorias
    //
    // Parsing posicional ligero (la función hará lo mínimo necesario).
    parseArgs(argc, argv, n_cells, max_t, seed);

    if (n_cells <= 0) n_cells = 1;
    if (max_t <= 0) max_t = 1;

    // Umbrales e inestabilidad inicial (todos los genomas usarán los mismos valores)
    std::unordered_map<std::string, double> gene_thresholds{{"BRCA1", 0.01}, {"TP53", 0.01}};
    std::unordered_map<std::string, double> gene_instability_k{{"BRCA1", 0.01}, {"TP53", 0.01}};

    std::cout << "Creando " << n_cells << " células con los mismos thresholds iniciales.\n";

    // Generador para seeds si seed < 0
    std::mt19937 seed_gen(static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<unsigned> seed_dist(1, 0xFFFFFFFEu);

    // Usar Tissue para agrupar y gestionar las células
    domain::Tissue tissue;
    tissue.setId(0);

    // Vamos a crear y añadir las células al tissue
    for (int i = 0; i < n_cells; ++i) {
        unsigned cell_seed;
        if (seed >= 0) {
            cell_seed = static_cast<unsigned>(seed) + static_cast<unsigned>(i);
        } else {
            cell_seed = seed_dist(seed_gen);
        }

        // Crear genoma idéntico para todas las células a partir de los mapas
        domain::Genome genome = domain::genome_factory::makeDefaultGenome(gene_thresholds, gene_instability_k, verbose);

        // Crear fuente de ruido aleatoria por célula
        auto noise = std::make_unique<adapters::RandomNoise>(cell_seed);

        // Construir la célula mediante la factoría (inyecta noise y genome)
        auto cell = domain::cell_factory::createAgenticCell(std::move(noise), std::move(genome), 0.02, 0.01, 0.02, verbose);
        tissue.addCell(std::move(cell));
    }

    std::cout << "Creadas " << tissue.size() << " células. Presiona Enter para avanzar 1 año (q + Enter para salir).\n";
    int current_year = 0;

    while (current_year < max_t) {
        std::string line;
        std::cout << "(Enter para tick / q para salir) > ";
        if (!std::getline(std::cin, line)) {
            std::cout << "Entrada cerrada. Saliendo.\n";
            break;
        }
        if (!line.empty() && (line[0] == 'q' || line[0] == 'Q')) {
            std::cout << "Usuario pidió salir.\n";
            break;
        }

        // Avanzar un año
        ++current_year;
        // ejecutar un tick del tejido (las células harán live() internamente)
        tissue.live();

        // recoger señales emitidas por las células durante el live()
        auto emitted = tissue.stealEmittedSignals();
        size_t neoplasm_signals = 0;
        for (auto &s : emitted) {
            if (!s) continue;
            if (s->type() == domain::ISignal::Type::Neoplasm) {
                ++neoplasm_signals;
                auto* ns = dynamic_cast<domain::NeoplasmSignal*>(s.get());
                if (ns) {
                    std::cout << "[Signal] Neoplasm from cell id=" << ns->sourceId() << " message='" << ns->message() << "'\n";
                }
            }
        }

        int neoplastic_count = 0;
        for (std::size_t i = 0; i < tissue.size(); ++i) {
            auto* c = tissue.getCell(i);
            if (c && c->isNeoplastic()) ++neoplastic_count;
        }

        // Mostrar resumen compacto
        // std::cout << "Año " << current_year << " / " << max_t << " | Neoplásicas: " << neoplastic_count << " / " << cells.size() << "\n";

        // Contadores por combinaciones solicitadas:
        app::ExperimentalTracking track; // agrupa todos los contadores y estadísticas

        for (std::size_t i = 0; i < tissue.size(); ++i) {
            auto* c = tissue.getCell(i);
            auto* ac = dynamic_cast<domain::AgenticCell*>(c);
            if (!ac) continue;
            track.observeCell(ac);
         }


         // Delegar la impresión al tracker: crear una vista de punteros a las células en el tissue
         std::vector<domain::ICell*> cell_ptrs;
         cell_ptrs.reserve(tissue.size());
         for (std::size_t i = 0; i < tissue.size(); ++i) cell_ptrs.push_back(tissue.getCell(i));
         track.printSummary(current_year, max_t, neoplastic_count, cell_ptrs, verbose);

        if (current_year >= max_t) {
             std::cout << "Alcanzado año máximo. Fin de la simulación.\n";
             break;
         }
     }

    std::cout << "Simulación finalizada.\n";
    return 0;
}
