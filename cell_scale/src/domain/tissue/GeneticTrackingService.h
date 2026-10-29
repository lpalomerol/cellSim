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

                // ICell inherits from IGeneticProfile — genetic query methods are available directly
                const std::string brca = cell->getBRCA1Status();
                const std::string tp53 = cell->getTP53Status();
                const bool isNeo = cell->isNeoplastic();
                const bool isAlive = cell->alive();

                if (brca == "+/-") {
                    if (tp53 == "+/+") {
                        ++tracking.cells[static_cast<int>(GeneticTrackingData::Category::BRCA_HET_TP53_HOM_PLUS)];
                        if (isNeo) {
                            ++tracking.neoplasms[static_cast<int>(GeneticTrackingData::Category::BRCA_HET_TP53_HOM_PLUS)];
                            if (isAlive) ++tracking.active_neoplasms[static_cast<int>(GeneticTrackingData::Category::BRCA_HET_TP53_HOM_PLUS)];
                        }
                    } else if (tp53 == "+/-") {
                        ++tracking.cells[static_cast<int>(GeneticTrackingData::Category::BRCA_HET_TP53_HET)];
                        if (isNeo) {
                            ++tracking.neoplasms[static_cast<int>(GeneticTrackingData::Category::BRCA_HET_TP53_HET)];
                            if (isAlive) ++tracking.active_neoplasms[static_cast<int>(GeneticTrackingData::Category::BRCA_HET_TP53_HET)];
                        }
                    } else if (tp53 == "-/-") {
                        ++tracking.cells[static_cast<int>(GeneticTrackingData::Category::BRCA_HET_TP53_HOM_MINUS)];
                        if (isNeo) {
                            ++tracking.neoplasms[static_cast<int>(GeneticTrackingData::Category::BRCA_HET_TP53_HOM_MINUS)];
                            if (isAlive) ++tracking.active_neoplasms[static_cast<int>(GeneticTrackingData::Category::BRCA_HET_TP53_HOM_MINUS)];
                        }
                    }
                } else if (brca == "-/-") {
                    ++tracking.cells[static_cast<int>(GeneticTrackingData::Category::BRCA_HOM_MINUS)];
                }
            }

            return tracking;
        }
    };

} // namespace domain

