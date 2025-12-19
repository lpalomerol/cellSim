#include <iostream>
#include <string>
#include <memory>
#include "../src/application/simulation/InteractiveSimulation.h"
#include "../src/application/config/SimulationConfig.h"
#include "../src/domain/cell/CellFactory.h"
#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/adapters/RandomNoise.h"
#include "../src/domain/adapters/FixedNoise.h"

int main() {
    std::cout << "Interactive simulation: creating one cell via builder" << std::endl;

    // Cargar configuración interactiva
    auto config = application::SimulationConfig::loadInteractive(/* verbose = */ true);

    application::InteractiveSimulation sim(config.max_t);

    // Crear genoma con parámetros de config
    domain::Genome genome = domain::genome_factory::makeDefaultGenome(
        config.gene_thresholds,
        config.gene_instability_k,
        config.logger
    );

    // Crear fuente de ruido
    std::unique_ptr<domain::INoiseSource> noise;
    if (config.use_random_noise) {
        std::cout << "Usando ruido aleatorio: RandomNoise(seed=" << config.seed << ")" << std::endl;
        noise = std::make_unique<domain::adapters::RandomNoise>(static_cast<unsigned>(config.seed));
    } else {
        noise = std::make_unique<domain::adapters::FixedNoise>(domain::CellNoise{1.0});
        std::cout << "Usando ruido fijo: FixedNoise(u01=1.0)" << std::endl;
    }

    // Crear célula
    auto cell_noise = std::make_unique<domain::adapters::RandomNoise>(42);
    sim.addCell(
        domain::CellFactory::createCustomCell(
            std::move(cell_noise),
            std::move(genome),
            config.neoplasm_k,
            0.01,
            0.2,
            config.division_rate,
            config.neoplastic_division_rate,
            config.enable_big_bang_mode,
            config.apoptosis_threshold,
            2.0,  // d1_primer_threshold
            5.0,  // d2_apoptosis_threshold
            config.logger)
    );

    std::cout << "Creada 1 célula. Presiona Enter para avanzar año a año." << std::endl;
    std::cout << "Opciones en cada paso: [Enter]=No mutar, 1=Mutar BRCA1, 2=Mutar TP53, q=Salir" << std::endl;
    std::cout << "Año actual: " << sim.currentYear() << " / " << sim.maxYears() << std::endl;

    while (true) {
        std::string line;
        std::cout << "(Enter/1/2/q) > ";
        if (!std::getline(std::cin, line)) {
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
