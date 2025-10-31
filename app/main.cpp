#include <iostream>
#include "../src/application/simulation/Simulation.h"
#include "../src/domain/adapters/RandomNoise.h"
#include "../src/domain/cell/CellFactory.h"

int main() {
    int n_cells = 1;
    int num_simulations = 100;
    std::vector results(num_simulations, std::vector(2, 0));
    domain::CellFactory factory;
    for (int k = 0; k < num_simulations; ++k) {
        std::cout << "Running simulation #" << (k+1) << "\n";

        adapters::RandomNoise noise(k);
        constexpr int max_t = 80; // 1 año por tick
        application::Simulation sim(max_t);

        for (int i = 0; i < n_cells; ++i) {
            sim.addCell(factory.createGeneCell(noise, domain::GeneCellParams()));

        }

        sim.run();
        results[k][0] = sim.firstTimeDead();
        results[k][1] = sim.firstTimeTumoral();

    }


    // Calcula el ratio de valores -1 en results[][1]
    int count_minus_one = 0;
    for (const auto& res : results) {
        if (res[1] == -1) ++count_minus_one;
    }
    double ratio_minus_one = static_cast<double>(count_minus_one) / num_simulations;
    std::cout << "Ratio de -1 en First Tumoral Year: " << ratio_minus_one << "\n";

    // Calcula la probabilidad empírica de tener tumor en < 20 años
    // Calcula la probabilidad empírica de tener tumor en < 40 años
    // Calcula la probabilidad empírica de tener tumor en < 60 años
    int count_tumor_20 = 0;
    int count_tumor_40 = 0;
    int count_tumor_60 = 0;
    for (const auto& res : results) {
        if (res[1] != -1) {
            if (res[1] < 20) ++count_tumor_20;
            if (res[1] < 40) ++count_tumor_40;
            if (res[1] < 60) ++count_tumor_60;
        }
    }
    std::cout << "Proporción de tumor en <20 años: " << static_cast<double>(count_tumor_20) / num_simulations << "\n";
    std::cout << "Proporción de tumor en <40 años: " << static_cast<double>(count_tumor_40) / num_simulations << "\n";
    std::cout << "Proporción de tumor en <60 años: " << static_cast<double>(count_tumor_60) / num_simulations << "\n";

    return 0;
}


