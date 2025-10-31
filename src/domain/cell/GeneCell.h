// GeneCell.h
#pragma once
#include "../ports/ICell.h"
#include "../ports/INoiseSource.h"
#include "../gene/Gene.h"
#include <array>

#include "OncoState.h"

namespace domain {

    constexpr int N_STATES = 3;

    //using TransitionMatrix = std::array<std::array<double, N_STATES>, N_STATES>;
    enum class BRCA1State { PlusMinus, MinusMinus };
    enum class TP53State { PlusPlus, PlusMinus, MinusMinus };

    struct GeneCellParams {

        double p_mutation_brca = 0.001;
        double p_mutation_tp53 = 0.005;
    };

    class GeneCell final : public ICell {
    public:
        GeneCell(INoiseSource& noise,
                 const GeneCellParams& params)
            : noise_(noise), params_(params) {}

        void live() override;
        bool alive() override;
        CellState state() override { return state_;}
        OncoState getOncoState() { return onco_;}
    private:
        INoiseSource& noise_;
        GeneCellParams params_;
        bool is_alive_ = true;
        CellState state_ = CellState::Alive;
        OncoState onco_ = OncoState::S0;


    };
}
