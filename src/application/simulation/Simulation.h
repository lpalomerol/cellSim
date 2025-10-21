#pragma once
#include <vector>
#include <memory>
#include "../../domain/ports/ICell.h"

namespace application {
    class Simulation {
    public:
        explicit Simulation(int max_t_years) : max_t_(max_t_years) {}
        void addCell(std::unique_ptr<domain::ICell> cell);
        void run(); // dt=1 año, hasta max_t_
    private:
        int max_t_;
        std::vector<std::unique_ptr<domain::ICell>> cells_;
    };
}