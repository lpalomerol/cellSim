// GeneCell.h
#pragma once
#include "../ports/ICell.h"
#include "../ports/INoiseSource.h"
#include "../gene/Gene.h"
#include <array>

namespace domain {

    constexpr int N_STATES = 4;

    using TransitionMatrix = std::array<std::array<double, N_STATES>, N_STATES>;
    enum class BRCA1State { PlusMinus, MinusMinus };
    enum class TP53State { PlusPlus, PlusMinus, MinusMinus };

    struct GeneCellParams {

        domain::TransitionMatrix base_matrix = {
            {
                {{0.98, 0.02, 0,  0.0}},
                {{0.30, 0.60, 0, 0.10}},
                {{0.0, 0.0, 0, 1.0}}
            }};

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
        [[nodiscard]] Gene getBRCA1() const { return brca1_; }
        [[nodiscard]] Gene getTP53() const { return tp53_; }

    private:
        INoiseSource& noise_;
        GeneCellParams params_;
        Gene brca1_;
        Gene tp53_;
        bool is_alive_ = true;
        CellState state_ = CellState::Alive;
        static int sampleNextState(const TransitionMatrix& matrix, int current_state, double u);
        static auto modulatedMatrix(const TransitionMatrix &base, const Gene &brca1,
                                    const Gene &tp53) -> TransitionMatrix;

    };
}