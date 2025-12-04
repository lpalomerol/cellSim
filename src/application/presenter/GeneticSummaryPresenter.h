#pragma once

#include "../../domain/tissue/GeneticTrackingData.h"
#include <string>

namespace application {

/**
 * GeneticSummaryPresenter: Responsable de formatear datos genéticos para presentación.
 * Mantiene la lógica de formato fuera del dominio (Tissue).
 * Patrón: Application Service Layer (DDD)
 */
class GeneticSummaryPresenter {
public:
    /**
     * Formatea un resumen de tracking genético para logging/presentación
     * @param tracking Datos de tracking genético del tejido
     * @return String formateado con summary de poblaciones por genotipo
     */
    static std::string formatGeneticSummary(const domain::GeneticTrackingData& tracking);

private:
    static constexpr int BOX_WIDTH = 12;

    /**
     * Formatea una "caja" de estadísticas: total(neo/activo)
     * @param count Total de células en este genotipo
     * @param neos Total de neoplásticas (o -1 si no aplica)
     * @param active_neos Total de neoplásticas activas (o -1 si no aplica)
     * @return String como "1000(500/250)"
     */
    static std::string formatBox(int count, int neos, int active_neos);
};

} // namespace application

