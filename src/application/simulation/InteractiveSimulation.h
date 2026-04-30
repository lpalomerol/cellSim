// filepath: src/application/simulation/InteractiveSimulation.h
#pragma once

#include <vector>
#include <array>
#include <memory>

#include "Simulation.h"
#include "../../domain/ports/ICell.h"

namespace application {

    enum class MenuOption {
        MutateBRCA1,
        MutateTP53,
        None,
        Quit
    };

class InteractiveSimulation : public Simulation {
public:
    explicit InteractiveSimulation(int max_t_years = 80);

    // Prints cell info when added
    void addCell(std::unique_ptr<domain::ICell> cell);

    // Executes one year tick. Returns true while years remain, false when max_t reached or Quit.
    bool step(MenuOption option = MenuOption::None);

    [[nodiscard]] int currentYear() const { return current_year_; }
    [[nodiscard]] int maxYears() const { return max_t_; }
    [[nodiscard]] int neoplasticCount() const { return current_neoplastic_count_; }
    [[nodiscard]] int currentNeoplasticCount() const { return current_neoplastic_count_; }

    [[nodiscard]] int numCells() const { return static_cast<int>(tissue_.size()); }

    // Returns the first year neoplasia appeared (1-based), or -1 if never
    [[nodiscard]] int firstTimeNeoplastic() const { return first_time_neoplastic_; }

private:
    int current_year_;
    int current_neoplastic_count_;
    int first_time_neoplastic_;
};

} // namespace application
