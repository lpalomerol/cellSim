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
        [[nodiscard]] bool alive() const override;
        // Returns true if the cell has acquired a neoplastic onco-state
        [[nodiscard]] bool isNeoplastic() const override;
        CellState state()  { return state_;}
        OncoState getOncoState() { return onco_; }

        // IGeneticProfile implementation - SimpleCell has no genes, returns default values
        [[nodiscard]] std::string getBRCA1Status() const override { return "?"; }
        [[nodiscard]] std::string getTP53Status() const override { return "?"; }

        // No-op: SimpleCell has no internal genome and cannot mutate genes.
        void mutateGene(const std::string& name) override { (void)name; }

    private:
        INoiseSource& noise_;
        SimpleCellParams params_;
        bool is_alive_ = true;
        CellState state_ = CellState::Alive;
        OncoState onco_ = OncoState::TP53_plus_plus;


    };
}
