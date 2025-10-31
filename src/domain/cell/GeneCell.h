// GeneCell.h
#pragma once
#include "../ports/ICell.h"
#include "../ports/INoiseSource.h"
#include "../gene/Gene.h"

namespace domain {
    enum class BRCA1State { PlusMinus, MinusMinus };
    enum class TP53State { PlusPlus, PlusMinus, MinusMinus };

    struct GeneCellParams {
        double p_mutation_brca = 0.1;
        double p_mutation_tp53 = 0.2;
        double p_homeostasis = 0.95;
    };

    class GeneCell final : public ICell {
    public:
        GeneCell(INoiseSource& noise,
                 const GeneCellParams& params,
                 const Gene brca1 = Gene::PartiallyDisabled,
                 const Gene tp53 = Gene::Active
            )
            : noise_(noise), params_(params), brca1_(brca1), tp53_(tp53) {}

        void live() override;
        bool alive() override;
        CellState state() override;

    private:
        INoiseSource& noise_;
        GeneCellParams params_;
        Gene brca1_;
        Gene tp53_;
        bool is_alive_ = true;
    };
}