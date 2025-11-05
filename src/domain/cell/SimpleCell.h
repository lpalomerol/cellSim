// GeneCell.h
#pragma once
#include "../ports/ICell.h"
#include "../ports/INoiseSource.h"

#include "OncoState.h"

namespace domain {

    struct SimpleCellParams {

        double p_mutation_brca = 0.001;
        double p_mutation_tp53 = 0.005;
    };

    class SimpleCell final : public ICell {
    public:
        SimpleCell(INoiseSource& noise,
                 const SimpleCellParams& params)
            : noise_(noise), params_(params) {}

        void live() override;
        bool alive() const override;
        // Indica si la célula ha adquirido un estado neoplásico
        bool isNeoplastic() const override;
        CellState state()  { return state_;}
        OncoState getOncoState() { return onco_;}

        // No-op: SimpleCell no tiene genoma interno, por tanto no puede mutar genes.
        void mutateGene(const std::string& name) override { (void)name; }

    private:
        INoiseSource& noise_;
        SimpleCellParams params_;
        bool is_alive_ = true;
        CellState state_ = CellState::Alive;
        OncoState onco_ = OncoState::TP53_plus_plus;


    };
}
