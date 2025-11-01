#pragma once
#include <vector>
#include <array>
#include <memory>
#include "../../domain/ports/ICell.h"

namespace application {
    class Simulation {
    public:
        explicit Simulation(int max_t_years);
        void addCell(std::unique_ptr<domain::ICell> cell);
        void run(); // dt=1 año, hasta max_t_

        int firstTimeNeoplastic();

    private:
        int max_t_ = 0;
        std::vector<std::unique_ptr<domain::ICell>> cells_;
        std::vector<std::array<int, 3>> cell_state_counter_;
    };
}