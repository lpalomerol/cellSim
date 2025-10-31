#include <iostream>
#include "../src/application/simulation/Simulation.h"
#include "../src/domain/adapters/RandomNoise.h"
#include "../src/domain/cell/CellFactory.h"

int main() {
    int n_cells = 100;
    int num_simulations = 100;
    std::vector results(num_simulations, std::vector(1, 0));
    domain::CellFactory factory;
    for (int k = 0; k < num_simulations; ++k) {
        adapters::RandomNoise noise(k);
        constexpr int max_t = 80; // 1 año por tick
        application::Simulation sim(max_t);

        for (int i = 0; i < n_cells; ++i) {
            sim.addCell(factory.createSimpleCell(noise, domain::SimpleCellParams()));

        }
        sim.run();
        results[k][1] = sim.firstTimeTumoral();

    }


    // Calcula el ratio de valores -1 en results[][1]
    int count_minus_one = 0;
    for (const auto& res : results) {
        if (res[0] == -1) ++count_minus_one;
    }
    double ratio_minus_one = static_cast<double>(count_minus_one) / num_simulations;
    std::cout << "Ratio de -1 en First Tumoral Year: " << ratio_minus_one << "\n";

    // Calcula la probabilidad empírica de tener tumor en < X años
    std::vector<int> thresholds = {10, 20, 30, 40, 50, 60, 70, 80};
    std::vector<int> counts(thresholds.size(), 0);

    for (const auto& res : results) {
        if (res[1] != -1) {
            for (size_t i = 0; i < thresholds.size(); ++i) {
                if (res[1] < thresholds[i]) ++counts[i];
            }
        }
    }

    for (size_t i = 0; i < thresholds.size(); ++i) {
        std::cout << "Proporción de tumor en <" << thresholds[i] << " años: "
                  << static_cast<double>(counts[i]) / num_simulations << "\n";
    }
    return 0;
}


