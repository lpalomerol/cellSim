#pragma once
#include "../ports/ICell.h"
#include "../ports/INoiseSource.h"

namespace domain {
    struct SimpleCellParams {
        double p_homeostasis = 0.95; // Keep homeostasis probability
    };

    class SimpleCell final : public ICell {
    public :

        SimpleCell(INoiseSource& noise, const SimpleCellParams& cfg = SimpleCellParams{})
            : noise_(noise), cfg_(cfg) {}

        void live() override;
        bool alive() override;
        CellState state() override;
    private:
        SimpleCellParams cfg_;
        INoiseSource& noise_;
        bool is_alive_ = true;
        int age_ = 0;

    };

}
