// filepath: src/application/simulation/InteractiveSimulation.h
#pragma once

#include <vector>
#include <array>
#include <memory>

#include "Simulation.h"
#include "../../domain/ports/ICell.h"

namespace application {

// Una implementación interactiva que permite avanzar la simulación año a año.
// Contrato mínimo:
// - constructor(max_t)
// - addCell(unique_ptr<ICell>)
// - bool step(): ejecuta un año; devuelve true si quedan años por ejecutar
// - int currentYear() const
// - int neoplasticCount() const
// - int firstTimeNeoplastic() const (devuelve -1 si nunca)

class InteractiveSimulation {
public:
    explicit InteractiveSimulation(int max_t_years = 80);

    void addCell(std::unique_ptr<domain::ICell> cell);

    // Ejecuta un solo año (tick). Devuelve true si después de ejecutar el año
    // la simulación puede seguir (no alcanzó max_t), false si ya no quedan años.
    bool step();

    int currentYear() const { return current_year_; }
    int maxYears() const { return max_t_; }
    int neoplasticCount() const { return current_neoplastic_count_; }

    // Devuelve el primer año en el que apareció neoplasia (1-based), o -1 si nunca
    int firstTimeNeoplastic() const { return first_time_neoplastic_; }

private:
    int max_t_;
    int current_year_;
    int current_neoplastic_count_;
    int first_time_neoplastic_;
    std::vector<std::unique_ptr<domain::ICell>> cells_;
};

} // namespace application

