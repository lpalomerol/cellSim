#include <iostream>
#include <string>
#include <unordered_map>
#include "../src/application/simulation/InteractiveSimulation.h"
#include "../src/domain/cell/CellFactory.h"
#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/adapters/FixedNoise.h" // para crear FixedNoise con valor 0


int main() {
    std::cout << "Interactive simulation: creating one cell via builder" << std::endl;
    int max_t = 80;
    application::InteractiveSimulation sim(max_t);

    // Parámetros para la célula (genoma por defecto, neoplasm_k)
    double neoplasm_k = 0.003;

    // Umbrales específicos por gen solicitados: BRCA1=0.1, TP53=0.15
    std::unordered_map<std::string, double> gene_thresholds{{"BRCA1", 0.1}, {"TP53", 0.15}};
    // Inestabilidad: +0.1 para ambos en caso de inestabilidad
    std::unordered_map<std::string, double> gene_instability_k{{"BRCA1", 0.1}, {"TP53", 0.1}};

    domain::Genome genome = domain::genome_factory::makeDefaultGenome(gene_thresholds, gene_instability_k);


    // Crear una AgenticCell usando FixedNoise (siempre 0.0) y añadirla a la simulación
    auto no_mutation_noise = std::make_unique<adapters::FixedNoise>(domain::CellNoise{0.0});
    sim.addCell(domain::cell_factory::createAgenticCell(
        std::move(no_mutation_noise),
        std::move(genome),
        neoplasm_k
    ));

    std::cout << "Creada 1 célula. Presiona Enter para avanzar año a año (q para salir)." << std::endl;
    std::cout << "Año actual: " << sim.currentYear() << " / " << sim.maxYears() << std::endl;

    // Avanzar año a año hasta que termine o el usuario pulse 'q' en el menú
    while (true) {
        bool can_continue = sim.step();
        std::cout << "Año: " << sim.currentYear() << std::endl;
        if (!can_continue) {
            std::cout << "Simulación completada (alcanzado max_t = " << sim.maxYears() << ")." << std::endl;
            break;
        }
    }

    return 0;
}
