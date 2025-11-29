#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
#include "../src/application/simulation/InteractiveSimulation.h"
#include "../src/domain/cell/CellFactory.h"
#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/adapters/FixedNoise.h" // para crear FixedNoise con valor 0
#include "../src/domain/adapters/RandomNoise.h" // para crear RandomNoise cuando use_random_noise == true
#include "../src/domain/adapters/Logger.h"


int main() {
    std::cout << "Interactive simulation: creating one cell via builder" << std::endl;
    int max_t = 30;
    application::InteractiveSimulation sim(max_t);

    // Parámetros para la célula (genoma por defecto, neoplasm_k)
    double neoplasm_k = 0.5;

    // Umbrales específicos por gen solicitados: BRCA1=0.1, TP53=0.15
    std::unordered_map<std::string, double> gene_thresholds{{"BRCA1", 0.01}, {"TP53", 0.25}};
    // Inestabilidad: +0.1 para ambos en caso de inestabilidad
    std::unordered_map<std::string, double> gene_instability_k{{"BRCA1", 0.01}, {"TP53", 0.5}};

    // En modo interactivo activamos trazas verbose para inspección
    bool verbose = true;

    // Crear logger real e inyectarlo
    auto logger = std::make_shared<domain::adapters::Logger>();
    logger->setVerbose(verbose);

    domain::Genome genome = domain::genome_factory::makeDefaultGenome(gene_thresholds, gene_instability_k, logger);

    // Elegir el tipo de ruido: true = aleatorio (RandomNoise), false = fijo (FixedNoise{0.0})
    bool use_random_noise = true; // <- cambia aquí si quieres FixedNoise
    unsigned seed = 4u; // semilla usada si use_random_noise == true

    std::unique_ptr<domain::INoiseSource> noise;
    if (use_random_noise) {
        std::cout << "Usando ruido aleatorio: RandomNoise(seed=" << seed << ")" << std::endl;
        noise = std::make_unique<adapters::RandomNoise>(seed);
    } else {
        noise = std::make_unique<adapters::FixedNoise>(domain::CellNoise{1.0});
        std::cout << "Usando ruido fijo: FixedNoise(u01=0.0)" << std::endl;
    }

    // Crear una AgenticCell usando la fuente de ruido seleccionada y añadirla a la simulación
    sim.addCell(
        domain::cell_factory::createAgenticCell(
        std::move(noise),
        std::move(genome),
        neoplasm_k,
        0.0001,
        0.0002,
        0.001,
        logger)
    );

    std::cout << "Creada 1 célula. Presiona Enter para avanzar año a año." << std::endl;
    std::cout << "Opciones en cada paso: [Enter]=No mutar, 1=Mutar BRCA1, 2=Mutar TP53, q=Salir" << std::endl;
    std::cout << "Año actual: " << sim.currentYear() << " / " << sim.maxYears() << std::endl;

    while (true) {
        std::string line;
        std::cout << "(Enter/1/2/q) > ";
        if (!std::getline(std::cin, line)) { // EOF (Ctrl+D) o error
            std::cout << "Entrada cerrada (EOF). Saliendo de la simulación." << std::endl;
            break;
        }

        application::MenuOption option = application::MenuOption::Nada;
        if (line.empty()) {
            option = application::MenuOption::Nada;
        } else {
            char c = line[0];
            if (c == '1') option = application::MenuOption::MutarBRCA;
            else if (c == '2') option = application::MenuOption::MutarTP53;
            else if (c == 'q' || c == 'Q') option = application::MenuOption::Quit;
            else option = application::MenuOption::Nada;
        }

        bool can_continue = sim.step(option);
        std::cout << "Año: " << sim.currentYear() << "\n";
        std::cout << "Neoplásicas hasta ahora: " << sim.currentNeoplasticCount() << "\n";

        if (!can_continue) {
            std::cout << "Simulación completada (alcanzado max_t = " << sim.maxYears() << ")." << std::endl;
            break;
        }
    }

    return 0;
}
