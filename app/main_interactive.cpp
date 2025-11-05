#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
#include "../src/application/simulation/InteractiveSimulation.h"
#include "../src/domain/cell/CellFactory.h"
#include "../src/domain/gene/GenomeFactory.h"
#include "../src/domain/adapters/FixedNoise.h" // para crear FixedNoise con valor 0
#include "../src/domain/adapters/RandomNoise.h" // para crear RandomNoise cuando use_random_noise == true


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

    // Elegir el tipo de ruido: true = aleatorio (RandomNoise), false = fijo (FixedNoise{0.0})
    bool use_random_noise = false; // <- cambia aquí si quieres FixedNoise
    unsigned seed = 42u; // semilla usada si use_random_noise == true

    std::unique_ptr<domain::INoiseSource> noise;
    if (use_random_noise) {
        std::cout << "Usando ruido aleatorio: RandomNoise(seed=" << seed << ")" << std::endl;
        noise = std::make_unique<adapters::RandomNoise>(seed);
    } else {
        noise = std::make_unique<adapters::FixedNoise>(domain::CellNoise{1.0});
        std::cout << "Usando ruido fijo: FixedNoise(u01=0.0)" << std::endl;
    }

    // Crear una AgenticCell usando la fuente de ruido seleccionada y añadirla a la simulación
    sim.addCell(domain::cell_factory::createAgenticCell(
        std::move(noise),
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
