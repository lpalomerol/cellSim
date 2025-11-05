#include <iostream>
#include "../src/application/simulation/Simulation.h"
#include "../src/domain/cell/CellFactory.h"
#include "../src/domain/gene/GenomeFactory.h"

int main() {
    std::cout << "Interactive simulation: creating one cell via builder" << std::endl;

    int max_t = 80;
    application::Simulation sim(max_t);

    // Parámetros para la célula (seed, genoma por defecto, neoplasm_k)
    unsigned seed = 42u;
    double neoplasm_k = 0.003;

    domain::Genome genome = domain::genome_factory::makeDefaultGenome();

    // Crear una AgenticCell usando el "builder"/factory y añadirla a la simulación
    sim.addCell(domain::cell_factory::createAgenticCell(seed, std::move(genome), neoplasm_k));

    // Ejecutar la simulación
    sim.run();

    int first_neoplastic = sim.firstTimeNeoplastic();
    if (first_neoplastic == -1) {
        std::cout << "No neoplasia detected within " << max_t << " years." << std::endl;
    } else {
        std::cout << "First neoplastic year: " << first_neoplastic << std::endl;
    }

    return 0;
}
