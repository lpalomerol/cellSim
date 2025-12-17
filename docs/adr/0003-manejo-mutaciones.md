# ADR-0003: Manejo de Mutaciones

**Status**: Aceptada  
**Date**: 2025-12-17  
**Decision Makers**: Equipo de desarrollo  
**Relates to**: ADR-0002 (Agentic Cell Pattern), ADR-0001 (Capas)

## Context

El proyecto cellSim necesita un sistema genético que:
- Represente genes reales (TP53, BRCA1)
- Permita mutaciones progresivas
- Afecte el comportamiento de las células (D1, D2)
- Sea testeable y determinístico

Las mutaciones son el motor del cambio biológico en la simulación. Sin un sistema de mutaciones claro, las células no pueden progresar a neoplasias.

## Decision

Se adopta un **sistema de mutaciones basado en genes discretos** con los siguientes componentes:

### 1. **Gene: Estados Discretos**

Cada gen tiene 3 estados posibles:

```cpp
enum class State {
    PlusPlus,    // +/+  Homocigoto wild-type (protegido)
    PlusMinus,   // +/-  Heterocigoto (intermedio)
    MinusMinus   // -/-  Homocigoto mutante (vulnerable)
};
```

**Ejemplos**:
- TP53 +/+: Protección máxima contra apoptosis
- TP53 +/-: Protección parcial
- TP53 -/-: Sin protección, vulnerable

### 2. **Gene: Mutación Probabilística**

Cada gen tiene:

```cpp
struct Gene {
    std::string name_;                    // ej: "TP53", "BRCA1"
    State state_;                         // +/+, +/-, -/-
    double mutation_threshold_;           // Umbral para mutar (0.0-1.0)
    double mutation_instability_k_;       // Factor de inestabilidad
    INoiseSource* noise_;                 // Generador de números aleatorios
};
```

**Proceso de mutación**:

```cpp
void Gene::live(bool apply_instability = true, double genomic_instability = 1.0) {
    // 1. Calcular umbral efectivo
    double effective_threshold = mutation_threshold_;
    if (apply_instability) {
        effective_threshold += mutation_instability_k_;  // Suma lineal
    }
    effective_threshold *= genomic_instability;        // Factor multiplicativo
    
    // 2. Generar número aleatorio [0,1)
    double random = noise_->nextDouble();
    
    // 3. Si random < effective_threshold, mutar
    if (random < effective_threshold) {
        mutate();  // +/+ → +/- → -/-
    }
}
```

Transiciones de estado (el gen siempre "degrada"):
- +/+ → +/- (pierde una copia)
- +/- → -/- (pierde la última copia)
- -/- → -/- (ya no puede mutar más)

### 3. **Genome: Colección de Genes**

```cpp
class Genome {
private:
    std::unordered_map<std::string, Gene> genes_;  // name → Gene
};
```

**Operaciones**:

```cpp
// Crear genoma default
Genome genome = Genome::makeDefaultGenome();
// Contiene: TP53 (+/+), BRCA1 (+/+)

// Clonar genoma
Genome clone = genome.clone();

// Hacer vivir todos los genes
genome.liveAllGenes(genomic_instability = 1.0);

// Acceder a un gene
const Gene* tp53 = genome.getGene("TP53");

// Mutar un gene específico
genome.mutate("TP53");  // +/+ → +/-
```

### 4. **Integración con AgenticCell_v2**

En cada `live()` (Phase 4: Cytoplasm Remodeling):

```cpp
void AgenticCell_v2::live() {
    // ... Phases 0-3 ...
    
    // Phase 4: Cytoplasm Remodeling
    // Actualizar D1 según TP53 status
    std::string tp53_status = genome_.getGene("TP53")->status();
    if (tp53_status == "+/+") {
        d1_dna_damage_ += low_delta_instability;      // 0.0001
    } else if (tp53_status == "+/-") {
        d1_dna_damage_ += low_delta_instability;      // 0.0001
    } else { // -/-
        d1_dna_damage_ += high_delta_instability;     // 0.0002
    }
    
    // Hacer vivir el genoma (avanza mutaciones)
    genome_.liveAllGenes(genomic_instability = d1_dna_damage_);
    
    // Actualizar D2
    d2_immunosuppression_ += 0.0001;  // Siempre crece
}
```

### 5. **Genes Clave**

**TP53 (Guardián del Genoma)**:
- Controla apoptosis intrínseca
- TP53 +/+: Apoptosis fácil (protección)
- TP53 +/-: Apoptosis débil
- TP53 -/-: Sin apoptosis intrínseca (vulnerable)
- Default: +/+

**BRCA1 (Reparación de ADN)**:
- Afecta tasa de mutaciones de TP53
- BRCA1 +/- (NATIVO): Reparación normal (estado base en células reales)
- BRCA1 -/-: Mala reparación (alta mutación de TP53, muy raro)
- ⚠️ NOTA: BRCA1 nativo nunca es +/+ (wild-type), siempre +/- porque es supresor tumoral
- Default: +/- (no +/+)

### 6. **Fuentes de Ruido (INoiseSource)**

Las mutaciones requieren aleatoriedad determinística:

```cpp
struct INoiseSource {
    virtual double nextDouble() = 0;  // Retorna [0,1)
};
```

Implementaciones:
- **NormalDistributionNoise**: Distribución gaussiana
- **UniformNoise**: Distribución uniforme
- **TestNoise**: Determinístico para tests

### 7. **Factory Pattern**

```cpp
// Crear genoma con genes específicos
Genome genome = GenomeFactory::createGenomeWithTP53AndBRCA1(
    tp53_state,
    brca1_state,
    logger
);
```

## Consequences

✅ **Ventajas**:
- **Transparencia**: Estados de genes explícitos y observables
- **Realismo**: Refleja herencia genética real (2 alelos)
- **Determinismo**: Reproducible con seed de ruido
- **Testeable**: Cada gene puede probarse aisladamente
- **Flexible**: Fácil añadir nuevos genes
- **Gradual**: Las mutaciones ocurren lentamente (no todo de repente)

⚠️ **Desventajas**:
- **Complejidad**: 3 estados × múltiples genes = combinaciones
- **Performance**: Cada cell hace `liveAllGenes()` cada tick
- **Parámetros**: Muchos thresholds a ajustar
- **No es realista en 100%**: Genes reales tienen >2 alelos

## Alternatives Considered

1. **Modelo continuo (genes como doubles)**:
   - Genes son valores 0.0-1.0 "fitness"
   - ❌ Menos realista biológicamente
   - ❌ Difícil definir umbrales claros
   - ✅ Más simple computacionalmente

2. **Sistema de alelos múltiples**:
   - Más de 2 alelos por gen
   - ✅ Más realista
   - ❌ Más complejidad
   - ❌ Menos testeable

3. **Redes genéticas (GRN)**:
   - Genes regulan otros genes
   - ✅ Muy realista
   - ❌ Mucho más complejo
   - ❌ Difícil de debuggear

4. **Sin mutaciones (estático)**:
   - Todos los genes fijos
   - ❌ No captura evolución
   - ❌ Células nunca mutan a neoplasia

## Implementation Checklist

- ✅ Gene: 3 estados (PlusPlus, PlusMinus, MinusMinus)
- ✅ Gene::live() con mutación probabilística
- ✅ Genome: contenedor de genes
- ✅ Integración con AgenticCell_v2 (Phase 4)
- ✅ TP53 y BRCA1 como genes default
- ✅ GenomeFactory para creación
- ✅ INoiseSource para aleatoriedad
- ⏳ Tests exhaustivos de todas las transiciones
- ⏳ Benchmarking de stability de mutaciones

## Related ADRs

- **ADR-0001**: Arquitectura de Capas (Genome en Domain)
- **ADR-0002**: Agentic Cell Pattern (Phase 4 integra mutaciones)
- **ADR-0004**: Sistema de Apoptosis (TP53 controla apoptosis)
- **ADR-0005**: Big Bang Mode (Neoplasia resultante de mutaciones)

## References

- `src/domain/gene/Gene.h`: Implementación individual del gene
- `src/domain/gene/Genome.h`: Contenedor de genes
- `src/domain/gene/GenomeFactory.h`: Factory para crear genomas
- `src/domain/cell/AgenticCell_v2.h`: Integración con Phase 4
- `tests/GenomeFactoryTest.cpp`: Tests
- `tests/GenomeMutateTest.cpp`: Tests de mutaciones

---

**Última actualización**: 2025-12-17  
**Implementado por**: Equipo de cellSim

