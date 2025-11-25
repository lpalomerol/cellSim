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
#include <cstdlib>
#include <sstream>

int main(int argc, char** argv) {
    std::cout << "Simulación: array de células generadas aleatoriamente\n";

    // Valores por defecto; pueden sobreescribirse desde argv
    int n_cells = 1000; // número de células por defecto (ajusta según necesites)
    int max_t = 50;   // años máximo
    long seed = -1;    // semilla base (-1 = usar semilla aleatoria por cell)
    bool verbose = false; // trazas por célula

    // Parsear argumentos opcionales: <n_cells> <max_t> <seed> [-v]
    if (argc > 1) {
        if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
            std::cout << "Uso: random_cells [n_cells] [max_t] [seed] [-v]\n";
            std::cout << " Ejemplo: random_cells 20 100 42 -v\n";
            return 0;
        }
        try { n_cells = std::stoi(argv[1]); } catch (...) { /* keep default */ }
    }
    if (argc > 2) {
        try { max_t = std::stoi(argv[2]); } catch (...) { /* keep default */ }
    }
    if (argc > 3) {
        try { seed = std::stol(argv[3]); } catch (...) { /* keep default */ }
    }
    // flag -v en cualquier posición posterior a argv[1]
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-v" || std::string(argv[i]) == "--verbose") {
            verbose = true;
        }
    }

    if (n_cells <= 0) n_cells = 1;
    if (max_t <= 0) max_t = 1;

    // Umbrales e inestabilidad inicial (todos los genomas usarán los mismos valores)
    std::unordered_map<std::string, double> gene_thresholds{{"BRCA1", 0.01}, {"TP53", 0.01}};
    std::unordered_map<std::string, double> gene_instability_k{{"BRCA1", 0.01}, {"TP53", 0.01}};

    std::cout << "Creando " << n_cells << " células con los mismos thresholds iniciales.\n";

    // Generador para seeds si seed < 0
    std::mt19937 seed_gen(static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    std::uniform_int_distribution<unsigned> seed_dist(1, 0xFFFFFFFEu);

    std::vector<std::unique_ptr<domain::ICell>> cells;
    cells.reserve(static_cast<size_t>(n_cells));

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

        cells.emplace_back(std::move(cell));
    }

    std::cout << "Creadas " << cells.size() << " células. Presiona Enter para avanzar 1 año (q + Enter para salir).\n";
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
        int neoplastic_count = 0;
        for (auto &c : cells) {
            // cada célula ejecuta su ciclo de vida
            c->live();
            if (c->isNeoplastic()) ++neoplastic_count;
        }

        // Mostrar resumen compacto
        std::cout << "Año " << current_year << " / " << max_t << " | Neoplásicas: " << neoplastic_count << " / " << cells.size() << "\n";

        // Contadores por combinaciones solicitadas:
        int brca_het_tp53_hom_plus = 0; // BRCA1 "+/-"  y TP53 "+/+"
        int brca_het_tp53_het = 0;      // BRCA1 "+/-"  y TP53 "+/-"
        int brca_het_tp53_hom_minus = 0;// BRCA1 "+/-"  y TP53 "-/-"
        int brca_hom_minus = 0;         // BRCA1 "-/-" (cualquier TP53)

        // Neoplasias por categoría (solo para BRCA1 +/- categories solicitadas)
        int neo_brca_het_tp53_hom_plus = 0;
        int neo_brca_het_tp53_het = 0;
        int neo_brca_het_tp53_hom_minus = 0;

        // Min/max de genomic instability para las categorías con TP53 +/- y -/-
        double min_inst_tp53_het = std::numeric_limits<double>::infinity();
        double max_inst_tp53_het = -std::numeric_limits<double>::infinity();
        double min_inst_tp53_hom_minus = std::numeric_limits<double>::infinity();
        double max_inst_tp53_hom_minus = -std::numeric_limits<double>::infinity();

        for (auto &c : cells) {
            // Intentar downcast para leer los estados de genes. Si no es AgenticCell, omitir.
            domain::AgenticCell* ac = dynamic_cast<domain::AgenticCell*>(c.get());
            if (!ac) continue;
            std::string brca = ac->getBRCA1();
            std::string tp53 = ac->getTP53();
            double instability = ac->getGenomicInstability();
            bool isNeo = ac->isNeoplastic();

            if (brca == "+/-") {
                if (tp53 == "+/+") { ++brca_het_tp53_hom_plus; if (isNeo) ++neo_brca_het_tp53_hom_plus; }
                else if (tp53 == "+/-") { ++brca_het_tp53_het; if (isNeo) ++neo_brca_het_tp53_het;
                    if (instability < min_inst_tp53_het) min_inst_tp53_het = instability;
                    if (instability > max_inst_tp53_het) max_inst_tp53_het = instability;
                }
                else if (tp53 == "-/-") { ++brca_het_tp53_hom_minus; if (isNeo) ++neo_brca_het_tp53_hom_minus;
                    if (instability < min_inst_tp53_hom_minus) min_inst_tp53_hom_minus = instability;
                    if (instability > max_inst_tp53_hom_minus) max_inst_tp53_hom_minus = instability;
                }
            } else if (brca == "-/-") {
                ++brca_hom_minus;
            }
        }


        // Imprimir resumen por categorías pedido por el usuario
        // Imprimir una sola línea con 4 casillas: | val1(neos) | val2(neos) | val3(neos) | val4 |
        const int boxWidth = 9; // ancho interior para incluir cuenta y (neos)
        auto makeBox = [&](int count, int neos){
            std::ostringstream ss;
            ss << count;
            if (neos >= 0) ss << "(" << neos << ")"; // mostrar (neoplasias)
            return ss.str();
        };
        std::string b1 = makeBox(brca_het_tp53_hom_plus, neo_brca_het_tp53_hom_plus);
        std::string b2 = makeBox(brca_het_tp53_het, neo_brca_het_tp53_het);
        std::string b3 = makeBox(brca_het_tp53_hom_minus, neo_brca_het_tp53_hom_minus);
        std::string b4 = makeBox(brca_hom_minus, -1);

        std::cout << "  Resumen genético: |"
                  << std::setw(boxWidth) << b1 << " |"
                  << std::setw(boxWidth) << b2 << " |"
                  << std::setw(boxWidth) << b3 << " |"
                  << std::setw(boxWidth) << b4 << " |\n";
         // Leyenda compacta impresa solo si verbose para no ensuciar la salida
         if (verbose) {
             std::cout << "    [BRCA+/- TP53+/+] [BRCA+/- TP53+/-] [BRCA+/- TP53-/-] [BRCA-/-]\n";
         }

         // Añadir min/max de genomic instability para TP53 +/- y TP53 -/- (2 decimales)
         std::cout << std::fixed << std::setprecision(2);
         if (brca_het_tp53_het > 0) {
             std::cout << "    TP53(+/-) instability: min=" << min_inst_tp53_het << " max=" << max_inst_tp53_het << "\n";
         } else {
             std::cout << "    TP53(+/-) instability: min=N/A max=N/A\n";
         }
         if (brca_het_tp53_hom_minus > 0) {
             std::cout << "    TP53(-/-) instability: min=" << min_inst_tp53_hom_minus << " max=" << max_inst_tp53_hom_minus << "\n";
         } else {
             std::cout << "    TP53(-/-) instability: min=N/A max=N/A\n";
         }
         // Restaurar formato por si verbose necesita imprimir doubles con otro formato
         std::cout << std::defaultfloat;

         // (Opcional) mantener la salida detallada por célula si verbose
         if (verbose) {
            for (size_t i = 0; i < cells.size(); ++i) {
                domain::ICell* ic = cells[i].get();
                domain::AgenticCell* ac = dynamic_cast<domain::AgenticCell*>(ic);
                if (ac) {
                    std::cout << "  Cell[" << i << "] seed=" << ac->getSeed()
                              << " alive=" << (ac->alive() ? "yes" : "no")
                              << " neoplastic=" << (ac->isNeoplastic() ? "yes" : "no")
                              << " TP53=" << ac->getTP53()
                              << " BRCA1=" << ac->getBRCA1()
                              << " age=" << ac->getAge()
                              << " instability=" << ac->getGenomicInstability()
                              << "\n";
                } else {
                    std::cout << "  Cell[" << i << "] <no-agentic-info> neoplastic=" << (ic->isNeoplastic() ? "yes" : "no") << "\n";
                }
            }
        }

        if (neoplastic_count > 0) {
            std::cout << "! Hay neoplasia(s) detectadas en la población en el año " << current_year << "\n";
        }

        if (current_year >= max_t) {
            std::cout << "Alcanzado año máximo. Fin de la simulación.\n";
            break;
        }
    }

    std::cout << "Simulación finalizada.\n";
    return 0;
}
