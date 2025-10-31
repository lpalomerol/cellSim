//
// Created by luis on 31/10/25.
//

#pragma once
#include "../ports/ICell.h"
#include "../ports/INoiseSource.h"
#include "../gene/Gene.h"

namespace domain {

    struct AgenticCellParams {
        Gene::State TP53  = Gene::State::PlusPlus;
        Gene::State BRCA1 = Gene::State::PlusMinus;
        double TP53_mutation_threshold = 0.05; // umbral por defecto para TP53
        double BRCA1_mutation_threshold = 0.01; // umbral por defecto para BRCA1
        double tumor_k = 0.1; // probabilidad de degeneración a tumoral cuando TP53 está inactivo
    };

    class AgenticCell final : public domain::ICell {
    public:
        explicit AgenticCell(INoiseSource& noise, const AgenticCellParams& params = AgenticCellParams())
        : params_(params),
        noise_(noise),
        gene_tp53_(&noise, params.TP53, params.TP53_mutation_threshold),
        gene_brca1_(&noise, params.BRCA1, params.BRCA1_mutation_threshold),
        is_tumoral_(false) {}

        void live() override;
        bool alive() override;
        bool tumoral() override;
        [[nodiscard]] std::string getTP53() const;
        [[nodiscard]] std::string getBRCA1() const;

    private:
        INoiseSource& noise_;
        AgenticCellParams params_;
        Gene gene_tp53_;
        Gene gene_brca1_;
        bool is_tumoral_;
    };
} // domain
