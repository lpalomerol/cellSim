# ADR-0004: Sistema de Apoptosis

**Status**: Aceptada  
**Date**: 2025-12-17  
**Decision Makers**: Equipo de desarrollo  
**Relates to**: ADR-0002 (Agentic Cell), ADR-0003 (Mutaciones)

## Context

La apoptosis (muerte celular programada) es fundamental en la biología real:
- **Apoptosis intrínseca**: Muerte por daño interno (regulada por TP53)
- **Apoptosis extrínseca**: Muerte por señales externas (regulada por sistema inmune)

Sin un sistema de apoptosis, las células nunca morirían y la simulación sería irreal. Además, la resistencia a apoptosis es característica de las neoplasias.

El modelo debe capturar:
1. Protección TP53 contra apoptosis intrínseca
2. Evasión de apoptosis extrínseca vía D2 (immunosuppression)
3. Lanzamiento de `CellDeathException` cuando muere
4. Ambas vías deben integrarse naturalmente en `AgenticCell_v2.live()`

## Decision

Se adopta un **sistema de apoptosis bifásico** donde las células tienen dos oportunidades de muerte, cada una controlada por mecanismos distintos:

### 1. **Apoptosis Intrínseca (Phase 1)**

**Gatillo**: Estado genético de TP53

```cpp
// En Phase 1 de AgenticCell_v2::live()
std::string tp53_status = genome_.getGene("TP53")->status();

if (tp53_status == "+/+") {
    // Protección total: no apoptosis
    // (pero podría haber necrosis por envejecimiento)
} else if (tp53_status == "+/-") {
    // Protección parcial: probabilidad baja
    double random = noise_->nextDouble();
    if (random < 0.01) {  // 1% de apoptosis
        throw CellDeathException("TP53 +/- intrinsic apoptosis");
    }
} else { // TP53 -/-
    // Sin protección: probabilidad alta
    double random = noise_->nextDouble();
    if (random < 0.10) {  // 10% de apoptosis
        throw CellDeathException("TP53 -/- intrinsic apoptosis");
    }
}
```

**Lógica**:
- TP53 +/+: Guardián funcional → sin apoptosis intrínseca
- TP53 +/-: Guardián débil → apoptosis ocasional
- TP53 -/-: Sin guardián → apoptosis frecuente (pero evitable vía D2)

### 2. **Apoptosis Extrínseca (Phase 2)**

**Gatillo**: D2 (Immunosuppression) bajo + ruido

La apoptosis extrínseca es mediada por el sistema inmune. Las células con bajo D2 son visibles al sistema inmune y pueden ser eliminadas. Las células con alto D2 se "camuflan".

```cpp
// En Phase 2 de AgenticCell_v2::live()
double d2_apoptosis_threshold = 5.0;  // Parámetro configurable

if (d2_immunosuppression_ > d2_apoptosis_threshold) {
    // Célula está camuflada: NO puede sufrir apoptosis extrínseca
    // El sistema inmune no la detecta
} else {
    // Célula es visible: puede sufrir apoptosis
    double random = noise_->nextDouble();
    if (random < 0.05) {  // 5% de apoptosis extrínseca
        throw CellDeathException("D2-based extrinsic apoptosis");
    }
}
```

**Lógica**:
- D2 < 5.0: Célula visible → susceptible a apoptosis extrínseca
- D2 ≥ 5.0: Célula camuflada → resistente a apoptosis extrínseca

### 3. **Necrosis (Alternativa a Apoptosis)**

En raros casos, la célula puede morir por necrosis (muerte no programada):

```cpp
// En Phase 1-2
double age_threshold = 1000;  // Envejecimiento extremo
if (age_ > age_threshold) {
    throw CellDeathException("Necrosis by extreme aging");
}
```

### 4. **CellDeathException**

```cpp
class CellDeathException : public std::exception {
private:
    std::string reason_;  // ej: "TP53 -/- intrinsic apoptosis"
    
public:
    explicit CellDeathException(std::string reason);
    const char* what() const noexcept override;
};
```

La excepción es lanzada por la célula y capturada por el Tissue:

```cpp
// En TissueV2::live()
for (auto& cell : cells_) {
    try {
        cell->live();  // Célula ejecuta, puede lanzar excepción
    } catch (const CellDeathException& e) {
        // Célula muere: remover de tissue
        // Pero no borrar inmediatamente (mantener en traces)
        cell = nullptr;  // O marcar como dead
    }
}
```

### 5. **Fases de Apoptosis en Contexto**

En `AgenticCell_v2::live()` tenemos 5 fases:

```
Phase 0: Check alive?
    └─ Si !alive(), return
    
Phase 1: Intrinsic Apoptosis (TP53-based)
    ├─ TP53 +/+: No apoptosis
    ├─ TP53 +/-: Baja probabilidad (~1%)
    └─ TP53 -/-: Alta probabilidad (~10%)
    └─ Lanza CellDeathException si ocurre
    
Phase 2: Extrinsic Apoptosis (D2-based)
    ├─ D2 >= 5.0: No apoptosis (camuflada)
    ├─ D2 < 5.0: Susceptible (~5% probabilidad)
    └─ Lanza CellDeathException si ocurre
    
Phase 3: Senescence / Immortalization
    └─ Decidir si transformarse a neoplasm
    
Phase 4: Cytoplasm Remodeling
    ├─ Actualizar D1, D2
    └─ Division (si procede)
```

### 6. **Integración con Neoplasias**

Las células neoplásticas tienen resistencia apoptótica:

```cpp
// En Phase 1-2
if (is_neoplastic_) {
    // Las neoplasias casi no sufren apoptosis
    // O han alcanzado un estado donde la apoptosis es rara
    // (Esto puede variar según el tipo de neoplasia)
    if (random < 0.001) {  // 0.1% para neoplasias
        throw CellDeathException("Rare neoplastic apoptosis");
    }
} else {
    // Apoptosis normal para células no neoplásticas
}
```

### 7. **Parámetros Configurables**

```cpp
struct ApoptosisConfig {
    double intrinsic_probability_plus_plus = 0.0;      // TP53 +/+
    double intrinsic_probability_plus_minus = 0.01;    // TP53 +/-
    double intrinsic_probability_minus_minus = 0.10;   // TP53 -/-
    
    double extrinsic_probability = 0.05;               // D2 < threshold
    double d2_extrinsic_threshold = 5.0;               // D2 para evasión
    
    double neoplastic_apoptosis_probability = 0.001;   // Neoplasias rara vez mueren
};
```

## Consequences

✅ **Ventajas**:
- **Realismo biológico**: Dos vías de apoptosis reales
- **TP53 relevante**: La mutación de TP53 tiene impacto directo
- **D2 relevante**: La D2 es mecánica de evasión genuina
- **Emergencia**: Combinación de D1+D2 genera comportamientos complejos
- **Controlable**: Probabilidades ajustables por parámetro
- **Exception-based**: Modelado natural con excepciones

⚠️ **Desventajas**:
- **Complejidad**: Dos fases de apoptosis puede confundir
- **Parámetros**: Muchas probabilidades a ajustar
- **Performance**: Verificación en cada tick (pero es O(1))
- **Bugs subliminales**: Hard debuggear "por qué no muere esta célula"

## Alternatives Considered

1. **Apoptosis Única (Solo Intrínseca)**:
   - ❌ Menos realista
   - ❌ D2 no tiene uso
   - ✅ Más simple

2. **Apoptosis Continua (Acumulador que baja)**:
   - Células con "health" que baja cada tick
   - Cuando health=0, muere
   - ❌ Menos claro qué causa muerte
   - ❌ Difícil debuggear

3. **Sistema Inmune Explícito**:
   - Células del sistema inmune que "atacan"
   - ✅ Muy realista
   - ❌ Complejidad dramática
   - ❌ Performance (más agentes)

4. **Apoptosis Solo TP53, Sin D2**:
   - ❌ Neoplasias podrían evitar apoptosis solo por TP53 -/-
   - ❌ No captura el rol del sistema inmune

## Implementation Checklist

- ✅ Phase 1: Apoptosis intrínseca (TP53-based)
- ✅ Phase 2: Apoptosis extrínseca (D2-based)
- ✅ CellDeathException y lanzamiento
- ✅ TissueV2 captura excepciones
- ✅ Probabilities por genotype
- ⏳ Tests de cada rama (TP53 +/+, +/-, -/-)
- ⏳ Tests de cada rama (D2 < 5.0, >= 5.0)
- ⏳ Tests de integración (Phase 1 + Phase 2)
- ⏳ Validación de traces con múltiples escenarios

## Related ADRs

- **ADR-0002**: Agentic Cell Pattern (Phases 1-2)
- **ADR-0003**: Manejo de Mutaciones (TP53 estado)
- **ADR-0005**: Big Bang Mode (Neoplasias evasión)
- **ADR-0006**: Tissue Manager (Captura excepciones)

## References

- `src/domain/exception/CellDeathException.h`: Definición de excepción
- `src/domain/cell/AgenticCell_v2.h`: Phases 1-2
- `src/domain/tissue/TissueV2.h`: Captura de excepciones
- `tests/ApoptosisTest.cpp`: Tests de apoptosis
- `docs/APOPTOSIS-INTRINSECA-VS-EXTRINSECA.md`: Análisis detallado

---

**Última actualización**: 2025-12-17  
**Implementado por**: Equipo de cellSim

