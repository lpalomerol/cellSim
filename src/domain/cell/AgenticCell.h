//
// Created by luis on 31/10/25.
//

#pragma once
#include "../ports/ICell.h"
#include "../ports/INoiseSource.h"
#include "../gene/Gene.h"
#include <memory>

namespace domain {

    struct AgenticCellParams {
        Gene::State TP53  = Gene::State::PlusPlus;
        Gene::State BRCA1 = Gene::State::PlusMinus;
        double TP53_mutation_threshold = 0.0025; // umbral por defecto para TP53
        double BRCA1_mutation_threshold = 0.001; // umbral por defecto para BRCA1
        double tumor_k = 0.002; // probabilidad de degeneración a tumoral cuando TP53 está inactivo
        double mutation_instability_k = 0.005; // nuevo parámetro para inestabilidad mutacional
    };

    class AgenticCell final : public domain::ICell {
    public:
        explicit AgenticCell(std::unique_ptr<INoiseSource> noise, const AgenticCellParams& params = AgenticCellParams())
        : params_(params),
          noise_(std::move(noise)),
          gene_tp53_(noise_.get(), params_.TP53, params_.TP53_mutation_threshold + params_.mutation_instability_k),
          gene_brca1_(noise_.get(), params_.BRCA1, params_.BRCA1_mutation_threshold + params_.mutation_instability_k),
          is_tumoral_(false) {}

        void live() override;
        bool alive() override;
        bool tumoral() override;
        [[nodiscard]] std::string getTP53() const;
        [[nodiscard]] std::string getBRCA1() const;

    private:
        AgenticCellParams params_;
        std::unique_ptr<INoiseSource> noise_;
        Gene gene_tp53_;
        Gene gene_brca1_;
        bool is_tumoral_;
    };
} // domain
