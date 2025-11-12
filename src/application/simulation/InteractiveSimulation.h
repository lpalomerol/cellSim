// filepath: src/application/simulation/InteractiveSimulation.h
#pragma once

#include <vector>
#include <array>
#include <memory>

#include "Simulation.h"
#include "../../domain/ports/ICell.h"

namespace application {


    enum class MenuOption {
        MutarBRCA,
        MutarTP53,
        Nada,
        Quit // Nueva opción para detener la simulación desde el menú ('q')
    };


class InteractiveSimulation {
public:
    explicit InteractiveSimulation(int max_t_years = 80);

    void addCell(std::unique_ptr<domain::ICell> cell);

    // Ejecuta un solo año (tick). option indica la acción del usuario (mutar o nada).
    // Devuelve true si después de ejecutar el año la simulación puede seguir (no alcanzó max_t),
    // false si ya no quedan años o el usuario pidió salir.
    bool step(MenuOption option = MenuOption::Nada);

    int currentYear() const { return current_year_; }
    int maxYears() const { return max_t_; }
    int neoplasticCount() const { return current_neoplastic_count_; }
    // Compatibilidad: nombre alternativo usado en main_interactive
    int currentNeoplasticCount() const { return current_neoplastic_count_; }

    // Número de células en la simulación
    int numCells() const { return static_cast<int>(cells_.size()); }

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
