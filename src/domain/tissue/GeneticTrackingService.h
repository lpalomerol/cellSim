#pragma once

#include <vector>
#include <memory>
#include "GeneticTrackingData.h"
#include "../ports/ICell.h"

namespace domain {

    /**
     * GeneticTrackingService: Domain Service que recopila información genética
     * de células sin conocer sus implementaciones específicas.
     * Sigue el patrón DDD: recibe interfaces, no implementaciones concretas.
     */
    class GeneticTrackingService {
    public:
        /**
         * Analizar un conjunto de células que implementen IGeneticProfile (a través de ICell)
         * y retornar datos de tracking genético.
         */
        [[nodiscard]] static GeneticTrackingData analyze(
            const std::vector<std::unique_ptr<ICell>>& cells) {

            GeneticTrackingData tracking;
            tracking.reset();

            for (const auto& cell : cells) {
                if (!cell) continue;

                // ICell hereda de IGeneticProfile, así que podemos usar los métodos directamente
                const std::string brca = cell->getBRCA1Status();
                const std::string tp53 = cell->getTP53Status();
                const bool isNeo = cell->isNeoplastic();
                const bool isAlive = cell->alive();

                if (brca == "+/-") {
                    if (tp53 == "+/+") {
                        ++tracking.brca_het_tp53_hom_plus;
                        if (isNeo) {
                            ++tracking.neo_brca_het_tp53_hom_plus;
                            if (isAlive) ++tracking.active_neo_brca_het_tp53_hom_plus;
                        }
                    } else if (tp53 == "+/-") {
                        ++tracking.brca_het_tp53_het;
                        if (isNeo) {
                            ++tracking.neo_brca_het_tp53_het;
                            if (isAlive) ++tracking.active_neo_brca_het_tp53_het;
                        }
                    } else if (tp53 == "-/-") {
                        ++tracking.brca_het_tp53_hom_minus;
                        if (isNeo) {
                            ++tracking.neo_brca_het_tp53_hom_minus;
                            if (isAlive) ++tracking.active_neo_brca_het_tp53_hom_minus;
                        }
                    }
                } else if (brca == "-/-") {
                    ++tracking.brca_hom_minus;
                }
            }

            return tracking;
        }
    };

} // namespace domain

