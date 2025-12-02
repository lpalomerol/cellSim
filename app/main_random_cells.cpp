// filepath: /home/luis/CLionProjects/cellSim/app/main_random_cells.cpp
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <random>

#include "../src/domain/cell/CellFactory.h"
#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/adapters/RandomNoise.h"
#include "../src/domain/adapters/Logger.h"
#include "../src/domain/cell/AgenticCell.h"
#include "../src/domain/tissue/Tissue.h"

static void parseArgs(int argc, char** argv, int &n_cells, int &max_t, long &seed, std::string &scenario) {
    if (argc > 1) {
        std::string arg1(argv[1]);
        if (arg1 == "--help" || arg1 == "-h") {
            std::cout << "Uso: random_cells [n_cells] [max_t] [seed] [--scenario SCENARIO]\n";
            std::cout << " Ejemplo: random_cells 20 100 42 --scenario no_mutations\n";
            std::cout << "\n";
            std::cout << "Escenarios disponibles:\n";
            std::cout << "  default                - Mutaciones normales (thresholds: BRCA1=0.01, TP53=0.01)\n";
            std::cout << "  no_mutations           - Sin mutaciones (thresholds: BRCA1=0.0, TP53=0.0)\n";
            std::cout << "  high_brca_apoptosis    - Alta mutación en BRCA1 (threshold=0.5) para apoptosis masiva\n";
            std::cout << "  high_tp53_mutation     - Alta mutación en TP53 (threshold=0.3) para muchas neoplasias\n";
            std::cout << "  cell_division_healthy  - Sin mutaciones con división celular (1% rate) para crecimiento\n";
            std::exit(0);
        }
        n_cells = std::stoi(arg1);
    }
    if (argc > 2) max_t = std::stoi(argv[2]);
    if (argc > 3) seed = std::stol(argv[3]);

    // Buscar el parámetro --scenario
    for (int i = 4; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "--scenario" && i + 1 < argc) {
            scenario = argv[i + 1];
            break;
        }
    }
}

int main(int argc, char** argv) {
    std::cout << "Simulación: array de células generadas aleatoriamente\n";

    // Valores por defecto; pueden sobreescribirse desde argv
    int n_cells = 1000; // número de células por defecto (ajusta según necesites)
    int max_t = 50;   // años máximo
    long seed = -1;    // semilla base (-1 = usar semilla aleatoria por cell)
    std::string scenario = "default"; // escenario por defecto
    bool verbose = true; // trazas por célula

    // Para ejecuciones locales rápidas: descomenta y ajusta estas líneas para fijar parámetros manualmente.
    // (Si las dejas comentadas, el programa seguirá usando los valores por defecto o los que pases por argv.)
    // n_cells = 100; // ejemplo: descomenta y cambia según necesites
    // max_t = 100;   // ejemplo
    // seed = 42;     // ejemplo: usa -1 para semillas aleatorias
    // scenario = "no_mutations"; // ejemplo: escenario sin mutaciones
    //
    // Parsing posicional ligero (la función hará lo mínimo necesario).
    parseArgs(argc, argv, n_cells, max_t, seed, scenario);

    if (n_cells <= 0) n_cells = 1;
    if (max_t <= 0) max_t = 1;

    // Configurar umbrales según el escenario
    std::unordered_map<std::string, double> gene_thresholds;
    std::unordered_map<std::string, double> gene_instability_k;
    double division_rate = 0.001; // Tasa de división por defecto (0.1%)

    if (scenario == "no_mutations") {
        std::cout << "Escenario: NO MUTATIONS\n";
        std::cout << "Todas las probabilidades de mutación están en 0.\n";
        gene_thresholds = {{"BRCA1", 0.0}, {"TP53", 0.0}};
        gene_instability_k = {{"BRCA1", 0.0}, {"TP53", 0.0}};
        division_rate = 0.0; // Sin división celular
    } else if (scenario == "high_brca_apoptosis") {
        std::cout << "Escenario: HIGH BRCA APOPTOSIS\n";
        std::cout << "Alta probabilidad de mutación en BRCA1 para causar apoptosis masiva.\n";
        std::cout << "Esperado: ~50% de células muertas en 10 años.\n";
        gene_thresholds = {{"BRCA1", 0.5}, {"TP53", 0.01}};
        gene_instability_k = {{"BRCA1", 0.1}, {"TP53", 0.01}};
        division_rate = 0.0; // Sin división celular
    } else if (scenario == "high_tp53_mutation") {
        std::cout << "Escenario: HIGH TP53 MUTATION (Neoplasias)\n";
        std::cout << "Alta probabilidad de mutación en TP53 para favorecer neoplasias.\n";
        std::cout << "BRCA1 bajo para evitar apoptosis masiva.\n";
        std::cout << "Esperado: muchas células transformadas a neoplasias (~80% en 20 años).\n";
        gene_thresholds = {{"BRCA1", 0.001}, {"TP53", 0.3}};
        gene_instability_k = {{"BRCA1", 0.001}, {"TP53", 0.2}};
        division_rate = 0.0; // Sin división celular
    } else if (scenario == "cell_division_healthy") {
        std::cout << "Escenario: CELL DIVISION (Healthy Growth)\n";
        std::cout << "Sin mutaciones y con división celular activa (ALTA TASA).\n";
        std::cout << "Esperado: crecimiento exponencial muy notable de la población.\n";
        gene_thresholds = {{"BRCA1", 0.0}, {"TP53", 0.0}};
        gene_instability_k = {{"BRCA1", 0.0}, {"TP53", 0.0}};
        division_rate = 0.1; // 10% división (TASA MUY ALTA para que sea notoria)
    } else {
        std::cout << "Escenario: DEFAULT (mutaciones normales)\n";
        gene_thresholds = {{"BRCA1", 0.01}, {"TP53", 0.01}};
        gene_instability_k = {{"BRCA1", 0.01}, {"TP53", 0.01}};
        division_rate = 0.01; // Sin división celular por defecto
    }

    std::cout << "Creando " << n_cells << " células con los mismos thresholds iniciales.\n";
    std::cout << "  BRCA1: threshold=" << gene_thresholds["BRCA1"] << ", k=" << gene_instability_k["BRCA1"] << "\n";
    std::cout << "  TP53:  threshold=" << gene_thresholds["TP53"] << ", k=" << gene_instability_k["TP53"] << "\n";
    std::cout << "  Division rate: " << division_rate << " (" << (division_rate * 100.0) << "%)\n";

    // Generador para seeds si seed < 0
    std::mt19937 seed_gen(static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<unsigned> seed_dist(1, 0xFFFFFFFEu);

    // Crear un logger real e inyectarlo
    auto logger = std::make_shared<domain::adapters::Logger>();
    logger->setVerbose(verbose);

    // Para control granular por categoría, puedes crear una configuración personalizada:
    domain::adapters::LogLevel custom_config(true, false, false);  // TISSUE=on, CELL=off, GENOME=off
    logger->setLogLevel(custom_config);

    // Usar Tissue para agrupar y gestionar las células
    domain::Tissue tissue(logger);
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
        domain::Genome genome = domain::genome_factory::makeDefaultGenome(gene_thresholds, gene_instability_k, logger);

        // Crear fuente de ruido aleatoria por célula
        auto noise = std::make_unique<adapters::RandomNoise>(cell_seed);

        // Construir la célula mediante la factoría (inyecta noise y genome)
        auto cell = domain::cell_factory::createAgenticCell(std::move(noise), std::move(genome), 0.02, 0.01, 0.02, division_rate, 10.0, logger);
        tissue.addCell(std::move(cell));
    }

    std::cout << "Creadas " << tissue.size() << " células. Presiona Enter para avanzar 1 año (q + Enter para salir).\n";
    int current_year = 0;

    while (current_year < max_t) {
        std::string line;
        std::cout << "(Año " << current_year << "/" << max_t << ") (Enter para tick / q para salir) > ";
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
        // Tissue.live() con verbose=true imprime:
        // - Descripción del tejido (id, número de células, neoplasias identificadas)
        // - Resumen genético por categoría
        // - Señales de neoplasma detectadas
        tissue.live();


        if (current_year >= max_t) {
             std::cout << "Alcanzado año máximo. Fin de la simulación.\n";
             break;
         }
     }

    std::cout << "\nSimulación finalizada.\n";
    std::cout << "Resumen final:\n";
    std::cout << "  Escenario: " << scenario << "\n";
    std::cout << "  Células finales: " << tissue.size() << "\n";
    std::cout << "  Años simulados: " << current_year << "\n";

    return 0;
}
