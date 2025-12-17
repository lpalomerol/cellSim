# ADR-0002: Patrón Agentic Cell

**Status**: Aceptada  
**Date**: 2025-12-17  
**Decision Makers**: Equipo de desarrollo  
**Supersedes**: Versiones anteriores de Cell (v1)

## Context

En sistemas de simulación biológica, las células son típicamente modeladas como entidades pasivas que reaccionan a eventos externos. Sin embargo, el comportamiento real de las células es proactivo:

- Las células toman decisiones basadas en su estado interno
- Las células pueden dividirse, morir o transformarse autónomamente
- Las células responden a señales pero también inician cambios
- Las células tienen "agencia" (capacidad de decisión)

El proyecto cellSim necesaba un modelo que capturara esta complejidad de forma testeable y mantenible.

## Decision

Se adopta el **Patrón Agentic Cell** donde:

### 1. **Las células son agentes autónomos**

```cpp
// La célula decide qué hacer en cada ciclo
void live() override;  // Llamado por el Tissue en cada tick
```

Cada célula:
- Ejecuta su lógica interna en `live()`
- Toma decisiones basadas en su estado genético y acumuladores
- Puede emitir señales (ej: division, death)
- No es simplemente un contenedor de datos

### 2. **Estado interno multidimensional (D1 + D2)**

```cpp
// D1: DNA Damage - Acumula durante divisiones erróneas
double d1_dna_damage_;

// D2: Immunosuppression - Resiste apoptosis extrínseca
double d2_immunosuppression_;
```

El modelo distingue:
- **D1 (DNA Damage)**: Inestabilidad acumulativa del ADN
- **D2 (Immunosuppression)**: Capacidad de evadir apoptosis

Estos acumuladores derivan estados emergentes:

```cpp
enum class CellLifeStage {
    DEAD,         // !alive()
    BASELINE,     // TP53 +/+ & BRCA1 +/- (normal, protegida por TP53, BRCA1 nativo heterozygous)
    UNSTABLE,     // TP53 +/- & BRCA1 +/- (débil protección, primera instabilidad)
    UNPROTECTED,  // TP53 -/- (vulnerable, sin protección de TP53)
    PRIMER,       // UNPROTECTED + D1 > 2.0 (pre-tumoral, detectable)
    TUMORAL       // is_neoplastic_ == true (transformada, neoplasma activo)
};
```

### 3. **Ciclo de vida basado en fases**

En cada `live()`, la célula progresa a través de fases internas:

```
Phase 0: Check alive (si muerte previa, exit)
Phase 1: Apoptosis/Necrosis (intrínseca por genética + mutaciones)
Phase 2: Endocytosis (apoptosis extrínseca por D2)
Phase 3: Senescence/Immortalization (decisión neoplasm)
Phase 4: Cytoplasmic Remodeling (actualizar D1, D2, posible división)
```

### 4. **Acumuladores con dinámicas diferentes**

**D1 Deltas** (según TP53):
- TP53 +/+: delta bajo (0.0001) - protegida
- TP53 +/-: delta medio (0.0001) - parcialmente protegida
- TP53 -/-: delta alto (0.0002) - vulnerable

**D2 Incremento**:
- Se incrementa en Phase 4
- Usualmente menor que D1 delta
- Representa evasión de mecanismos inmunológicos

### 5. **Interface claramente definida (ICell)**

```cpp
struct ICell {
    // Lifecycle
    virtual void live() = 0;
    virtual bool alive() const = 0;
    
    // Genetics
    virtual std::string getBRCA1Status() const = 0;
    virtual std::string getTP53Status() const = 0;
    
    // Neoplasia
    virtual bool isNeoplastic() const = 0;
    
    // Mutation
    virtual void mutateGene(const std::string& name) = 0;
    
    // Signaling
    virtual void setSignalEmitter(...) = 0;
    virtual void receiveMessage(std::unique_ptr<ISignal>) = 0;
};
```

### 6. **Señalización intra-tissue**

Las células pueden emitir señales:

```cpp
// Cuando una célula decide dividirse
std::unique_ptr<CellDivisionSignal> signal = 
    std::make_unique<CellDivisionSignal>(std::move(daughter_cell));

signal_emitter_(std::move(signal));  // El Tissue la captura
```

El Tissue:
- Recolecta señales de todas sus células
- Procesa divisiones, muertes, etc.
- Mantiene consistencia del estado

## Implementation Details

### AgenticCell_v2 (Versión actual)

```cpp
class AgenticCell_v2 final : public ICell, public ports::ILoggeable {
public:
    // Constructor con parámetros configurables
    AgenticCell_v2(
        std::unique_ptr<INoiseSource> noise,
        Genome genome,
        double neoplasm_k = 0.002,
        double low_delta_instability = 0.0001,
        double high_delta_instability = 0.0002,
        // ... más parámetros
    );
    
    void live() override;  // La lógica agentic
};
```

### Flujo de decisión en `live()`

```
Phase 0: Check if alive()
├─ NO → return (célula ya muerta)
└─ SÍ → continue

Phase 1: Check intrinsic apoptosis
├─ TP53 +/+: nunca (protegida)
├─ TP53 +/-: check probabilidad baja
└─ TP53 -/-: check probabilidad alta

Phase 2: Check extrinsic apoptosis (D2-based)
├─ if (D2 > d2_apoptosis_threshold) → avoid apoptosis
└─ else → puede apoptosis extrínseca

Phase 3: Neoplasm decision
├─ Check transformation criteria (TP53, BRCA1, edad)
└─ if meets criteria → set is_neoplastic_ = true

Phase 4: Cytoplasmic Remodeling
├─ Update D1 (por TP53 status)
├─ Update D2 (immunosuppression)
└─ Decision de division (si no es apoptosis/necrosis)
```

## Consequences

✅ **Ventajas**:
- **Agencia clara**: Las células toman decisiones propias
- **Emergencia**: Comportamientos complejos emergen de decisiones locales
- **Testeable**: Cada célula puede probarse independientemente
- **Documentable**: El flujo de decisión es explícito y rastreable
- **Flexible**: Fácil de añadir nuevas fases o criterios
- **Realista**: Refleja mejor el comportamiento real de células

⚠️ **Desventajas**:
- Mayor complejidad cognitiva que modelos pasivos
- Más parámetros a ajustar (deltas, thresholds, tasas)
- Debugging más complicado (necesita traces detalladas)
- Performance: O(n) por célula por tick (manejable para millones)

## Alternatives Considered

1. **Modelo pasivo (Event-driven)**:
   - Células solo reaccionan a eventos externos
   - ❌ No captura toma de decisiones propia de células
   - ❌ Menos realista biológicamente

2. **Máquina de estados (FSM)**:
   - Cada célula en un estado discreto
   - Transiciones explícitas entre estados
   - ❌ Menos flexible para cambios graduales (D1, D2)
   - ❌ Explosión de estados

3. **Cellular Automata (CA)**:
   - Reglas simples, vecindad local
   - Más simple que Agentic
   - ❌ Difícil modelar genética compleja
   - ❌ No captura agencia individual

4. **Agent-Based Modeling (ABM) clásico**:
   - Parecido a Agentic pero menos estructurado
   - ❌ Sin guardarriles de fases y ciclo de vida
   - Podría escalar a algo parecido a Agentic

## Related ADRs

- **ADR-0001**: Arquitectura de Capas (donde vive AgenticCell_v2)
- **ADR-0003**: Manejo de Mutaciones (genética que impulsa decisiones)
- **ADR-0004**: Sistema de Apoptosis (fases 1-2 del ciclo)
- **ADR-0005**: Big Bang Mode (división neoplástica acelerada)

## References

- `src/domain/cell/AgenticCell_v2.h`: Implementación principal
- `src/domain/cell/CellLifeStage.h`: Estados derivados
- `src/domain/ports/ICell.h`: Interfaz agentic
- `GUIDE-agenticCellRefactor.md`: Contexto del refactor
- `docs/diagrams.md`: Diagramas de flujo

## Implementation Checklist

- ✅ AgenticCell_v2 con D1 + D2
- ✅ Fases de ciclo de vida
- ✅ CellLifeStage enum derivado
- ✅ Sistema de señalización (CellDivisionSignal)
- ✅ Integración con Tissue
- ⏳ Tests exhaustivos de todas las fases
- ⏳ Benchmarking de performance con millones de células

---

**Última actualización**: 2025-12-17  
**Implementado por**: Equipo de cellSim

