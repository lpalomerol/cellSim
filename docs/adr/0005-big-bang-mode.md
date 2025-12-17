# ADR-0005: Big Bang Mode

**Status**: Aceptada  
**Date**: 2025-12-17  
**Decision Makers**: Equipo de desarrollo  
**Relates to**: ADR-0002 (Agentic Cell), ADR-0003 (Mutaciones), ADR-0004 (Apoptosis)

## Context

En la biología real, las neoplasias (tumores) tienen características específicas que las distinguen de células normales:
- **Inmortalidad**: No responden a apoptosis
- **Divisiones aceleradas**: Se dividen mucho más frecuentemente
- **Descontrol**: Escapan a regulación normal

El proyecto cellSim necesaba un mecanismo para modelar este comportamiento "explosivo" de neoplasias. Sin él, las neoplasias se comportan igual que células normales.

**Big Bang Mode** es el nombre dado a la capacidad de las células neoplásticas de dividirse a una tasa acelerada.

## Decision

Se adopta un **mecanismo de división acelerada para neoplasias** donde:

### 1. **Parámetros de División**

Cada célula tiene dos tasas de división:

```cpp
double division_rate;              // Tasa normal (ej: 0.001 = 0.1%)
double neoplastic_division_rate;   // Tasa neoplástica (ej: 0.01 = 1%)
bool enable_big_bang_mode;         // Flag global para activar/desactivar
```

**Interpretación**:
- `division_rate`: Probabilidad de división en cada tick para células normales
- `neoplastic_division_rate`: Probabilidad para neoplasias cuando Big Bang está activado
- `enable_big_bang_mode`: Switch global (configurable en SimulationConfig)

### 2. **Lógica en Phase 4 (Cytoplasm Remodeling)**

```cpp
// En Phase 4 de AgenticCell_v2::live()

// Decidir si dividirse
double division_probability;
if (is_neoplastic_ && enable_big_bang_mode_) {
    // Neoplasia con Big Bang activado: usar tasa acelerada
    division_probability = neoplastic_division_rate_;  // Ej: 0.01 (1%)
} else {
    // Célula normal o Big Bang desactivado: tasa normal
    division_probability = division_rate_;  // Ej: 0.001 (0.1%)
}

// Generar número aleatorio
double random = noise_->nextDouble();
if (random < division_probability) {
    // Crear célula hija
    std::unique_ptr<AgenticCell_v2> daughter = clone();
    
    // Emitir señal de división
    auto signal = std::make_unique<CellDivisionSignal>(
        std::move(daughter)
    );
    signal_emitter_(std::move(signal));
    
    // Célula madre se marca como necesitando actualización de edad
    // (Puede dividirse máximo 1 vez por tick)
}
```

### 3. **Configuración Global**

```cpp
// En SimulationConfig
struct SimulationConfig {
    bool enable_big_bang_mode = false;
    
    double normal_division_rate = 0.001;        // 0.1%
    double neoplastic_division_rate = 0.01;     // 1.0% (10x más rápido)
};
```

**Uso**:

```cpp
SimulationConfig config;
config.enable_big_bang_mode = true;  // Activar Big Bang
config.neoplastic_division_rate = 0.05;  // 5% de división neoplástica

// Las células neoplásticas crearán ~50 hijas en 1000 ticks
```

### 4. **Efecto en Simulación**

**Sin Big Bang (enable_big_bang_mode=false)**:

```
Tick 0:    1 célula normal
Tick 100:  ~100 células (crecimiento lento)
Tick 1000: ~1000 células

Si surge 1 neoplasia:
Tick 1010: La neoplasia se divide lentamente como normal
           Crece a tasa normal
```

**Con Big Bang (enable_big_bang_mode=true)**:

```
Tick 0:    1 célula normal
Tick 100:  ~100 células

Si surge 1 neoplasia (TP53 -/-, transformada):
Tick 1010: Neoplasia comienza a dividirse rápido
Tick 1020: ~2-10 neoplasias
Tick 1030: ~20-100 neoplasias (crecimiento exponencial)
Tick 1050: Puede dominar todo el tissue
           "Big Bang" - explosión de neoplasias
```

### 5. **Importancia Biológica**

En la realidad:
- Células normales se dividen ~1 vez cada 24 horas
- Células cancerosas se dividen ~1 vez cada 12-48 horas (2-10x más rápido)
- Sin control de apoptosis + divisiones aceleradas = tumor
- Con apoptosis normal + divisiones aceleradas = muerte (células defectuosas)

**El Big Bang captura**: La combinación de (1) evasión de apoptosis + (2) divisiones rápidas = tumor que domina.

### 6. **Interacción con Apoptosis (ADR-0004)**

Para que un Big Bang sea realista, deben ocurrir AMBAS condiciones:

```
Célula Normal:
  - D1 bajo, D2 bajo
  - TP53 +/+
  - Apoptosis fácil (Phase 1-2)
  - División lenta (0.1%)
  └─ Resultado: Crece lentamente, muere ocasionalmente

Célula Neoplástica (sin Big Bang):
  - D1 alto, D2 muy alto
  - TP53 -/-
  - Apoptosis rara (~0.1%)
  - División lenta (0.1%)
  └─ Resultado: Crece muy lentamente (apoptosis rara compensa)

Célula Neoplástica (CON Big Bang):
  - D1 alto, D2 muy alto
  - TP53 -/-
  - Apoptosis rara (~0.1%)
  - División RÁPIDA (1%)
  └─ Resultado: Explosión exponencial (Big Bang!)
```

### 7. **Parámetros Ajustables**

```cpp
struct BigBangConfig {
    bool enable = false;
    
    double normal_division_rate = 0.001;
    double neoplastic_division_rate = 0.01;
    
    // Opcional: tasas según tipo de neoplasia
    double brca1_mutant_division_rate = 0.015;
    double tp53_mutant_division_rate = 0.02;
};
```

## Consequences

✅ **Ventajas**:
- **Realismo biológico**: Captura comportamiento de tumores reales
- **Observable**: Fácil ver cuándo surge Big Bang
- **Controlable**: Flag global para activar/desactivar
- **Escalable**: Works para 1 célula o 1 millón
- **Testeable**: Probabilidades reproducibles con seeds
- **Tuneable**: Tasas ajustables por escenario

⚠️ **Desventajas**:
- **Performance**: Con Big Bang, pueden surgir millones de células
- **Memory**: Simulación puede requerir mucha RAM
- **Realismo limitado**: Usa probabilidades simples, no biología real
- **Parámetros**: Tasas de división deben ser cuidadosamente elegidas

## Alternatives Considered

1. **Sin Big Bang (solo división normal)**:
   - ❌ Neoplasias no son visiblemente diferentes
   - ❌ Menos interesante biológicamente
   - ✅ Más simple

2. **Tasa determinística (siempre se divide)**:
   - Neoplasias se dividen determinísticamente cada N ticks
   - ❌ Menos realista (biología es probabilística)
   - ❌ Difícil calibrar

3. **Big Bang como fase (on/off brusco)**:
   - Neoplasias entran en "Big Bang phase" cuando se detectan
   - ❌ Menos gradual
   - ❌ Cambio artificial

4. **Aceleración gradual (D1/D2 → division_rate)**:
   - La tasa de división escala continuamente con D1
   - ✅ Muy realista
   - ❌ Más complicado calibrar
   - ❌ Difícil de debuggear

## Implementation Checklist

- ✅ Parámetro `enable_big_bang_mode_` en AgenticCell_v2
- ✅ Parámetro `neoplastic_division_rate_` en AgenticCell_v2
- ✅ Lógica en Phase 4: seleccionar tasa según neoplasticidad
- ✅ Flag global en SimulationConfig
- ✅ CellFactory_v2 recibe parámetros
- ⏳ Tests de crecimiento exponencial
- ⏳ Benchmarks de performance (millones de células)
- ⏳ Visualización de Big Bang en traces

## Related ADRs

- **ADR-0002**: Agentic Cell Pattern (Phase 4 implementa división)
- **ADR-0004**: Sistema de Apoptosis (combinación genera Big Bang realista)
- **ADR-0005**: Este ADR
- **ADR-0006**: Tissue Manager (gestiona explosión de células)

## References

- `src/domain/cell/AgenticCell_v2.h`: Implementación Phase 4 con Big Bang
- `src/application/config/SimulationConfig.h`: Flag enable_big_bang_mode
- `src/domain/signal/CellDivisionSignal.h`: Señal de división
- `tests/AgenticCellTest.cpp`: Tests con Big Bang
- `docs/BIG_BANG_MODE.md`: Documentación detallada

## Scenarios Que Demuestran Big Bang

### Scenario 1: Sin Big Bang (Baseline)
```cpp
SimulationConfig config;
config.enable_big_bang_mode = false;

// Resultado: Crecimiento lento y controlado
// A los 1000 ticks: ~1000 células
// Neoplasias pueden surgir pero no dominan
```

### Scenario 2: Con Big Bang (Big Bang)
```cpp
SimulationConfig config;
config.enable_big_bang_mode = true;
config.neoplastic_division_rate = 0.01;

// Resultado: Si surge neoplasia, explosión exponencial
// A los 1000 ticks: Millones de células neoplásticas
// Tissue puede saturarse (límite de memoria)
```

---

**Última actualización**: 2025-12-17  
**Implementado por**: Equipo de cellSim

