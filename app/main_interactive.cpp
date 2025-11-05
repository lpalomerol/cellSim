#include <iostream>
#include <string>
#include "../src/application/simulation/InteractiveSimulation.h"
#include "../src/domain/cell/CellFactory.h"
#include "../src/domain/gene/GenomeFactory.h"


int main() {
    std::cout << "Interactive simulation: creating one cell via builder" << std::endl;
    char command;
    int max_t = 80;
    application::InteractiveSimulation sim(max_t);

    // Parámetros para la célula (seed, genoma por defecto, neoplasm_k)
    unsigned seed = 42u;
    double neoplasm_k = 0.003;

    domain::Genome genome = domain::genome_factory::makeDefaultGenome();

    // Crear una AgenticCell usando la fábrica y añadirla a la simulación

    sim.addCell(domain::cell_factory::createAgenticCell(seed, std::move(genome), neoplasm_k));

    std::cout << "Creada 1 célula. Presiona Enter para avanzar año a año (Ctrl+C para salir)." << std::endl;
    std::cout << "Año actual: " << sim.currentYear() << " / " << sim.maxYears() << std::endl;

    std::string line;
    // Avanzar año a año hasta que termine
    while (true) {
        // Delegar la lectura del comando interactivo a la simulación
        bool can_continue = sim.step();
        std::cout << "Año: " << sim.currentYear() << std::endl;
        if (!can_continue) {
            std::cout << "Simulación completada (alcanzado max_t = " << sim.maxYears() << ")." << std::endl;
            break;
        }
        std::cout << "Presiona Enter para avanzar al siguiente año..." << std::endl;
    }

    return 0;
}
