# ADR-0006: Tissue Manager

**Status**: Aceptada  
**Date**: 2025-12-17  
**Decision Makers**: Equipo de desarrollo  
**Relates to**: ADR-0001 (Capas), ADR-0002 (Agentic Cell), ADR-0004 (Apoptosis)

## Context

Las células no existen aisladas. En la biología real, existen dentro de **tejidos** (tissues), que son colecciones organizadas de células.

El proyecto cellSim necesaba un contenedor (Tissue) que:
- Mantenga colección de células vivas
- Ejecute el ciclo de vida de todas en paralelo (simulación)
- Capture excepciones de muerte (CellDeathException)
- Maneje divisiones (CellDivisionSignal)
- Proporcione consultas sobre estado del tejido
- Sea escalable a millones de células

**Tissue Manager** es la arquitectura y patrones para gestionar eficientemente estas responsabilidades.

## Decision

Se adopta un **patrón de gestor de colección** donde `TissueV2` es el contenedor central de células.

### 1. **Estructura de Datos Básica**

```cpp
class TissueV2 : public ports::ILoggeable {
private:
    std::vector<std::unique_ptr<ICell>> cells_;  // Contenedor de células
    ports::ILoggerPtr logger_;
    std::uint64_t tissue_id_;
};
```

**Diseño**:
- `cells_`: Vector de `unique_ptr<ICell>` para ownership seguro
- Cada célula tiene un ID único asignado por el Tissue
- ICell interface abstracta (funciona con cualquier implementación)
- Logger compartido para tracing

### 2. **Ciclo de Vida Principal: `live()`**

```cpp
void TissueV2::live() {
    // 1. Ejecutar ciclo de vida de cada célula
    std::vector<std::unique_ptr<ICell>> new_cells;  // Hijas
    
    for (auto& cell : cells_) {
        try {
            if (cell) {
                cell->live();  // Célula ejecuta su lógica
                // Si llegó aquí, no murió
            }
        } catch (const CellDeathException& e) {
            // Célula murió: remover (pero guardar en traces)
            cell = nullptr;
        } catch (const CellDivisionSignal& signal) {
            // Célula se dividió: agregar hija
            std::unique_ptr<ICell> daughter = signal.takeDaughterCell();
            daughter->setId(next_cell_id_++);
            new_cells.push_back(std::move(daughter));
        }
    }
    
    // 2. Limpiar células muertas (nullptr)
    cells_.erase(
        std::remove(cells_.begin(), cells_.end(), nullptr),
        cells_.end()
    );
    
    // 3. Agregar nuevas células
    for (auto& daughter : new_cells) {
        cells_.push_back(std::move(daughter));
    }
}
```

### 3. **Captura de Señales**

Las células emiten señales cuando ocurren eventos importantes:

```cpp
// Cuando célula se divide
void AgenticCell_v2::emitDivisionSignal() {
    std::unique_ptr<AgenticCell_v2> daughter = clone();
    auto signal = std::make_unique<CellDivisionSignal>(
        std::move(daughter)
    );
    signal_emitter_(std::move(signal));  // Tissue lo captura
}
```

El Tissue recibe el signal_emitter en construcción:

```cpp
// En Tissue
cell->setSignalEmitter(
    [this](std::unique_ptr<ISignal> signal) {
        // Procesar signal
        if (auto* division = dynamic_cast<CellDivisionSignal*>(signal.get())) {
            // Manejar división
        }
    }
);
```

### 4. **Interfaz Pública**

```cpp
class TissueV2 {
public:
    // Ciclo de vida
    void live();
    
    // Gestión de células
    void addCell(std::unique_ptr<ICell> cell);
    void clear();
    
    // Consultas
    std::size_t size() const;
    ICell* getCell(std::size_t idx);
    const ICell* getCell(std::size_t idx) const;
    
    // Filtros
    std::vector<ICell*> getLiveCells();
    std::vector<ICell*> getCellsByStage(CellLifeStage stage);
    
    // Identidad
    void setId(std::uint64_t id);
    std::uint64_t id() const;
};
```

### 5. **Asignación de IDs**

Cada célula obtiene un ID único cuando se agrega al Tissue:

```cpp
void TissueV2::addCell(std::unique_ptr<ICell> cell) {
    cell->setId(next_cell_id_++);  // ID único
    cells_.push_back(std::move(cell));
}
```

**Ventajas**:
- Trazabilidad de células
- Reproducibilidad (con seed determinístico)
- Debugging más fácil

### 6. **Manejo de Excepciones en Contexto**

```cpp
// Flujo: Tissue.live() → cell.live() → Excepción
void TissueV2::live() {
    for (auto& cell : cells_) {
        try {
            cell->live();
        } catch (const CellDeathException& e) {
            logger_->info("Cell {} died: {}", cell->id(), e.what());
            cell = nullptr;  // Marcar para eliminar
        } catch (const std::exception& e) {
            logger_->error("Unexpected exception in cell {}: {}", 
                         cell->id(), e.what());
            // Decidir: re-throw o continuar?
        }
    }
    // Limpiar nulos
    cells_.erase(
        std::remove(cells_.begin(), cells_.end(), nullptr),
        cells_.end()
    );
}
```

### 7. **Performance Considerations**

Para millones de células:

```cpp
// Optimización 1: Reservar espacio
TissueV2 tissue;
tissue.reserve(1000000);  // Pre-allocar para 1M células

// Optimización 2: Lazy evaluation de getCellsByStage
// No crear lista completa cada vez, usar iterador

// Optimización 3: Batch processing
// En lugar de live() célula por célula, procesar en lotes
for (size_t i = 0; i < cells_.size(); i += BATCH_SIZE) {
    // Procesar BATCH_SIZE células en paralelo (OpenMP)
}
```

### 8. **Tracking y Tracing**

El Tissue puede mantener estadísticas:

```cpp
struct TissueStats {
    std::size_t total_cells = 0;
    std::size_t dead_cells_this_tick = 0;
    std::size_t new_cells_this_tick = 0;
    std::size_t neoplastic_cells = 0;
    
    double avg_d1 = 0.0;
    double avg_d2 = 0.0;
};

TissueStats TissueV2::getStats() const {
    // Calcular estadísticas
}
```

### 9. **Ciclo Típico de Simulación**

```cpp
int main() {
    TissueV2 tissue;
    
    // Inicializar con células
    for (int i = 0; i < 100; i++) {
        auto cell = CellFactory_v2::createNormalCell(seed++);
        tissue.addCell(std::move(cell));
    }
    
    // Simulación
    for (int tick = 0; tick < 10000; tick++) {
        tissue.live();  // Una línea: maneja todo
        
        // Logging cada 100 ticks
        if (tick % 100 == 0) {
            std::cout << "Tick " << tick 
                      << ": " << tissue.size() << " cells\n";
        }
    }
    
    std::cout << "Final: " << tissue.size() << " cells\n";
}
```

### 10. **Dos Versiones: Tissue vs TissueV2**

| Aspecto | Tissue (v1) | TissueV2 (v2) |
|---------|-----------|--------------|
| Células | ICell genérico | ICell + AgenticCell_v2 específicamente |
| D1/D2 | No | Sí |
| CellLifeStage | No | Sí |
| getCellsByStage | No | Sí (filtra por stage) |
| Apoptosis | Básica | Bifásica (Phase 1-2) |
| Big Bang | No | Sí (neoplastic_division_rate) |

**Decisión**: Mantener ambas para compatibilidad:
- `Tissue`: Versión simple, legacy
- `TissueV2`: Versión moderna con D1+D2+CellLifeStage

## Consequences

✅ **Ventajas**:
- **Encapsulación**: Tissue maneja toda la complejidad de colección
- **Escalabilidad**: Works desde 1 hasta millones de células
- **Exception Safety**: Captura excepciones elegantemente
- **Signal-driven**: Arquitectura limpia event-based
- **Observable**: Fácil acceder a queries (size, getCellsByStage, etc)
- **Testeable**: Comportamiento predecible y reproducible
- **Extensible**: Nuevas queries/métodos sin cambiar core

⚠️ **Desventajas**:
- **Memory**: Vector crece dinámicamente, puede usar mucha RAM
- **Performance**: O(n) para each live(), puede ser lento con millones
- **Cleanup**: Limpiar células muertas (erase-remove) es O(n)
- **Signals**: Dynamic_cast en signal handling es O(1) pero ineficiente

## Alternatives Considered

1. **Linked List en lugar de Vector**:
   - ✅ O(1) removal
   - ❌ O(n) random access
   - ❌ Peor cache locality

2. **Spatial Indexing (Quad-tree)**:
   - Células organizadas espacialmente
   - ✅ Realista (células tienen posición)
   - ❌ Mucha más complejidad
   - ❌ Para esta simulación, no necesario

3. **Observer Pattern (células notifican cambios)**:
   - Actual: Try/catch en Tissue.live()
   - ✅ Más limpio (separación)
   - ❌ Overhead de observers
   - Elegido: Try/catch es más simple

4. **Task Queue (células son tareas en cola)**:
   - Células ejecutan cuando scheduled
   - ✅ Fácil paralelizar
   - ❌ Más complejo scheduling
   - Elegido: Simple iteration mejor

## Implementation Checklist

- ✅ TissueV2: contenedor vector<unique_ptr<ICell>>
- ✅ TissueV2::live() con try/catch
- ✅ Manejo de CellDeathException
- ✅ Asignación de IDs
- ✅ getLiveCells() / getCellsByStage()
- ⏳ Optimizaciones de performance (batch, parallelization)
- ⏳ Stats/tracing (TissueStats)
- ⏳ Benchmarking (cuánto escala?)
- ⏳ Memory profiling (RAM con millones de células)

## Related ADRs

- **ADR-0001**: Arquitectura de Capas (Tissue en Domain)
- **ADR-0002**: Agentic Cell Pattern (Tissue ejecuta live())
- **ADR-0004**: Sistema de Apoptosis (Tissue captura excepciones)
- **ADR-0005**: Big Bang Mode (Tissue maneja divisiones)

## References

- `src/domain/tissue/TissueV2.h`: Implementación principal
- `src/domain/tissue/Tissue.h`: Versión legacy
- `src/domain/signal/ISignal.h`: Interfaz de señales
- `src/domain/exception/CellDeathException.h`: Excepción
- `tests/TissueV2Test.cpp`: Tests de Tissue
- `src/application/scenarios/`: Ejemplos de uso

---

**Última actualización**: 2025-12-17  
**Implementado por**: Equipo de cellSim

