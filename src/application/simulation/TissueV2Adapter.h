#pragma once

#include <memory>
#include <vector>
#include <cstddef>
#include "../../domain/tissue/TissueV2.h"
#include "../../domain/ports/ICell.h"

namespace application {

    /// TissueV2Adapter: Wrapper adaptador que expone la interfaz compatible
    /// con código existente de Simulation, delegando a TissueV2 (Domain).
    ///
    /// Permite coexistencia segura: Simulation puede usar Tissue (original)
    /// o TissueV2Adapter sin cambios en su lógica (solo flag condicional).
    class TissueV2Adapter {
    public:
        explicit TissueV2Adapter(domain::ports::ILoggerPtr logger = nullptr)
            : tissue_(std::make_unique<domain::TissueV2>(logger)) {}

        /// Agregar célula al tejido (asigna ID único)
        void addCell(std::unique_ptr<domain::ICell> cell) {
            tissue_->addCell(std::move(cell));
        }

        /// Número de células en el tejido
        [[nodiscard]] std::size_t size() const {
            return tissue_->size();
        }

        /// Acceso a célula por índice (puede ser nullptr)
        domain::ICell* getCell(std::size_t idx) {
            return tissue_->getCell(idx);
        }

        /// Acceso const a célula por índice
        [[nodiscard]] const domain::ICell* getCell(std::size_t idx) const {
            return tissue_->getCell(idx);
        }

        /// Ejecutar ciclo de vida de todas las células
        void live() {
            tissue_->live();
        }

        /// Obtener todas las células vivas
        [[nodiscard]] std::vector<domain::ICell*> getLiveCells() {
            return tissue_->getLiveCells();
        }

        /// Obtener células de una etapa específica
        [[nodiscard]] std::vector<domain::ICell*> getCellsByStage(domain::CellLifeStage stage) {
            return tissue_->getCellsByStage(stage);
        }

        /// Limpiar todas las células
        void clear() {
            tissue_->clear();
        }

        /// Asignar ID al tejido
        void setId(std::uint64_t id) {
            tissue_->setId(id);
        }

        /// Obtener ID del tejido
        [[nodiscard]] std::uint64_t id() const {
            return tissue_->id();
        }

    private:
        std::unique_ptr<domain::TissueV2> tissue_;
    };

} // namespace application

