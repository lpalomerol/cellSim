# Plan: Refactorización progresiva de AgenticCell - Eliminación de code smells

Vas a eliminar los code smells detectados en `AgenticCell`, comenzando por los más sencillos (magic strings, constantes) hasta llegar a los más complejos (estrategias, value objects). Este enfoque incremental minimiza riesgos y permite validar después de cada paso.

## Steps

### 1. Eliminar Magic Strings

**Complejidad**: Baja (30-45 min)

Crear constantes para nombres de genes (`"TP53"`, `"BRCA1"`) y namespaces para strings genéticos (`"+/+"`, `"+/-"`, `"-/-"`) en `Gene.h` y `AgenticCell.cpp`.

**Archivos afectados**:
- `src/domain/gene/Gene.h`
- `src/domain/cell/AgenticCell.cpp`
- `src/domain/cell/AgenticCell.h`

**Implementación**:

```cpp
// En Gene.h o en un nuevo archivo GeneConstants.h
namespace domain {
    namespace GeneNames {
        constexpr const char* TP53 = "TP53";
        constexpr const char* BRCA1 = "BRCA1";
    }
    
    namespace GeneticStatusStrings {
        constexpr const char* WILD_TYPE = "+/+";
        constexpr const char* HETEROZYGOUS = "+/-";
        constexpr const char* HOMOZYGOUS_RECESSIVE = "-/-";
        constexpr const char* UNKNOWN = "?";
    }
}
```

**Beneficios**:
- Elimina typos en comparaciones de strings
- Autocomplete en IDE
- Refactoring seguro con find-all-references
- Un solo lugar para cambiar representación

**Validación**:
- Compilar sin errores
- Ejecutar `./build/run_tests`
- Verificar que todos los tests pasan

---

### 2. Eliminar duplicación de umbral de apoptosis

**Complejidad**: Baja-Media (45 min - 1 hora)

Eliminar `apoptosis_threshold_` (10.0) y unificar con `d2_apoptosis_threshold_` (5.0) usando un multiplicador. Actualmente hay **dos variables para el mismo concepto**: cuándo D2 causa muerte o inmortalidad.

**Problema identificado**:
- `apoptosis_threshold_` (10.0) → Células neoplásicas evaden apoptosis
- `d2_apoptosis_threshold_` (5.0) → Células normales mueren por apoptosis
- Son el **mismo fenómeno biológico** con valores diferentes según contexto

**Archivos afectados**:
- `src/domain/cell/AgenticCell.h`
- `src/domain/cell/AgenticCell.cpp`
- Todos los archivos que crean `AgenticCell` (tests, factories)

**Implementación**:

```cpp
// En AgenticCell.h
class AgenticCell {
private:
    // ELIMINAR:
    // double apoptosis_threshold_ = 10.0;
    
    // MANTENER:
    double d2_apoptosis_threshold_ = 5.0;
    
    
    // ...
};

// En AgenticCell.cpp - Constructor
AgenticCell::AgenticCell(...
                         double d2_apoptosis_threshold)  // Eliminar: apoptosis_instability_threshold
    : d2_apoptosis_threshold_(d2_apoptosis_threshold) {
    // Eliminar: apoptosis_instability_threshold_
}

// En phase4_CytoplasmicRemodeling() - Cambiar lógica
void AgenticCell::phase4_CytoplasmicRemodeling() {
    // ...existing code...
    
    // ANTES:
    // if (is_neoplastic_ && !has_evaded_apoptosis_) {
    //     if (d2_immunosuppression_ >= apoptosis_threshold_) {
    //         has_evaded_apoptosis_ = true;
    //     }
    // }
    
    // DESPUÉS:
    if (is_neoplastic_ && !has_evaded_apoptosis_) {
        // Células neoplásicas necesitan 2x el umbral para evadir
        double evasion_threshold = d2_apoptosis_threshold_ * EVASION_MULTIPLIER;
        if (d2_immunosuppression_ >= evasion_threshold) {
            has_evaded_apoptosis_ = true;
            logger_->logCell("[Phase4] Cell evaded apoptosis (D2=" + 
                           std::to_string(d2_immunosuppression_) + 
                           " >= " + std::to_string(evasion_threshold) + ")");
        }
    }
    
    // ...existing code...
}
```

**Cambios en llamadas al constructor**:

```cpp
// ANTES (12 parámetros):
AgenticCell(..., apoptosis_instability_threshold, ..., d1_primer_threshold, d2_apoptosis_threshold);

// DESPUÉS (11 parámetros):
AgenticCell(..., d1_primer_threshold, d2_apoptosis_threshold);
```

**Beneficios**:
- Elimina duplicación conceptual: un solo parámetro configurable
- Relación explícita: evasión = 2× apoptosis base
- Escalado automático: si cambias umbral de apoptosis, evasión se ajusta proporcionalmente
- Menos confusión: está claro que es el mismo fenómeno biológico
- Preparación para Parameter Objects (Paso 3)

**Validación**:
- Compilar sin errores
- Ejecutar `./build/run_tests`
- Verificar que células neoplásicas evaden apoptosis correctamente (D2 ≥ 10.0 con umbral base 5.0)
- Verificar que células normales mueren por apoptosis (D2 ≥ 5.0)
- Comprobar logs de evasión de apoptosis

---

### 3. Crear Parameter Objects

**Complejidad**: Media (1-2 horas)

Agrupar parámetros relacionados del constructor en `InstabilityConfig`, `DivisionConfig` y `ThresholdConfig` para reducir el Long Parameter List (11 parámetros → 3-4 objetos) en `AgenticCell.h`.

**Nota**: Este paso asume que ya completaste el Paso 2 (eliminación de `apoptosis_threshold_`).

**Archivos afectados**:
- `src/domain/cell/AgenticCell.h`
- `src/domain/cell/AgenticCell.cpp`
- Todos los archivos que crean `AgenticCell` (tests, factories, etc.)

**Implementación**:

```cpp
// En AgenticCell.h o nuevo archivo CellConfig.h
namespace domain {
    struct InstabilityConfig {
        double low_delta = 0.001;
        double high_delta = 0.003;
        // apoptosis_threshold ELIMINADO (ver Paso 2)
    };

    struct DivisionConfig {
        double base_rate = 0.001;
        double neoplastic_rate = 0.001;
        bool enable_big_bang = false;
    };

    struct ThresholdConfig {
        double d1_primer = 2.0;
        double d2_apoptosis = 5.0;           // Umbral único para apoptosis
        double neoplasm_k = 0.002;
    };

    // Constructor simplificado
    AgenticCell(
        std::unique_ptr<INoiseSource> noise,
        Genome genome,
        const InstabilityConfig& instability = {},
        const DivisionConfig& division = {},
        const ThresholdConfig& thresholds = {},
        const ports::ILoggerPtr& logger = nullptr
    );
}
```

**Uso en el código**:

```cpp
// Constructor
AgenticCell::AgenticCell(
    std::unique_ptr<INoiseSource> noise,
    Genome genome,
    const InstabilityConfig& instability,
    const DivisionConfig& division,
    const ThresholdConfig& thresholds,
    const ports::ILoggerPtr& logger)
    : noise_(std::move(noise)),
      genome_(std::move(genome)),
      low_delta_instability_(instability.low_delta),
      high_delta_instability_(instability.high_delta),
      division_rate_(division.base_rate),
      neoplastic_division_rate_(division.neoplastic_rate),
      enable_big_bang_mode_(division.enable_big_bang),
      d1_primer_threshold_(thresholds.d1_primer),
      d2_apoptosis_threshold_(thresholds.d2_apoptosis),
      base_neoplasm_k_(thresholds.neoplasm_k),
      logger_(logger ? logger : std::make_shared<adapters::NullLogger>()) {
    // ...
}

// Uso de evasion_multiplier en phase4
double evasion_threshold = d2_apoptosis_threshold_ * thresholds.evasion_multiplier;
// Nota: necesitarás guardar evasion_multiplier como campo si no es constante
```

**Beneficios**:
- Constructor más legible (6 parámetros vs 11)
- Configs reutilizables en tests
- Fácil añadir parámetros sin cambiar firma
- Inicialización por defecto más clara

**Validación**:
- Compilar sin errores
- Ejecutar `./build/run_tests`
- Verificar que constructores antiguos siguen funcionando (si mantienes overloads)
- Revisar escenarios de tests que usan configs específicas

---

### 4. Eliminar Primitive Obsession para estados genéticos

**Complejidad**: Media-Alta (2-3 horas)

Crear value object `GeneStatus` que encapsule el estado funcional del gen (+/+, +/-, -/-) con métodos semánticos basados en **funcionalidad** (enabled, partially_enabled, disabled), no en biología (zygosity).

**Problema actual**:
```cpp
// Comparaciones de strings propensas a errores
if (brca1->status() == "-/-") { /* logic */ }
if (tp53->status() == "+/-") { /* logic */ }
```

**Solución propuesta**:
```cpp
// Type-safe, semántico, autodocumentado
if (brca1->getStatus().isDisabled()) { /* logic */ }
if (tp53->getStatus().isPartiallyEnabled()) { /* logic */ }
```

**Archivos afectados**:
- `src/domain/gene/GeneStatus.h` (nuevo)
- `src/domain/gene/Gene.h` (actualizar para usar GeneStatus)
- `src/domain/gene/Gene.cpp` (actualizar implementación)
- `src/domain/cell/AgenticCell.cpp` (reemplazar comparaciones de strings)
- `src/domain/gene/Genome.cpp` (actualizar lógica)

**Implementación de GeneStatus.h**:

```cpp
#pragma once
#include <string>

namespace domain {

    /// GeneStatus: Value object representing functional state of a gene
    /// Based on allele configuration: +/+ (enabled), +/- (partially_enabled), -/- (disabled)
    /// This encapsulates the genotype-to-phenotype mapping
    class GeneStatus {
    public:
        enum class State {
            ENABLED,              // +/+ (wildtype, fully functional)
            PARTIALLY_ENABLED,    // +/- (heterozygous, semi-functional)
            DISABLED,             // -/- (homozygous recessive, non-functional)
            UNKNOWN               // ? (invalid/unknown state)
        };

        /// Factory methods for clarity and type safety
        static GeneStatus enabled() { 
            return GeneStatus(State::ENABLED); 
        }
        static GeneStatus partiallyEnabled() { 
            return GeneStatus(State::PARTIALLY_ENABLED); 
        }
        static GeneStatus disabled() { 
            return GeneStatus(State::DISABLED); 
        }
        static GeneStatus unknown() { 
            return GeneStatus(State::UNKNOWN); 
        }

        /// Parse from string representation (for legacy compatibility and tests)
        static GeneStatus fromString(const std::string& str) {
            if (str == "+/+") return enabled();
            if (str == "+/-") return partiallyEnabled();
            if (str == "-/-") return disabled();
            return unknown();
        }

        /// Semantic query methods: more intuitive than checking biotype
        bool isEnabled() const { 
            return state_ == State::ENABLED; 
        }
        bool isPartiallyEnabled() const { 
            return state_ == State::PARTIALLY_ENABLED; 
        }
        bool isDisabled() const { 
            return state_ == State::DISABLED; 
        }
        bool isUnknown() const { 
            return state_ == State::UNKNOWN; 
        }

        /// Alternative: query if gene has any functional copy
        /// Useful for: "Can this gene still suppress tumors?"
        bool hasFunctionalCopy() const {
            return state_ == State::ENABLED || state_ == State::PARTIALLY_ENABLED;
        }

        /// Convert to string representation (for legacy output, logging, etc)
        std::string toString() const {
            switch (state_) {
                case State::ENABLED: return "+/+";
                case State::PARTIALLY_ENABLED: return "+/-";
                case State::DISABLED: return "-/-";
                case State::UNKNOWN: return "?";
                default: return "?";
            }
        }

        /// Equality and comparison operators
        bool operator==(const GeneStatus& other) const {
            return state_ == other.state_;
        }
        bool operator!=(const GeneStatus& other) const {
            return state_ != other.state_;
        }

        /// Mutate: transition to next less-functional state
        /// +/+ → +/- → -/- (mimics progressive loss of function)
        GeneStatus mutated() const {
            if (state_ == State::ENABLED) return partiallyEnabled();
            if (state_ == State::PARTIALLY_ENABLED) return disabled();
            return *this; // Already disabled, cannot mutate further
        }

    private:
        explicit GeneStatus(State s) : state_(s) {}
        State state_;
    };

} // namespace domain
```

**Cambios en Gene.h**:

```cpp
class Gene : public ports::ILoggeable {
public:
    // ...existing includes...
    #include "GeneStatus.h"

    explicit Gene(std::string name,
        GeneStatus initial = GeneStatus::enabled(),
        double mutation_threshold = 0.1,
        double mutation_instability_k = 0.0,
        ports::ILoggerPtr logger = nullptr
    );

    [[nodiscard]] const std::string& name() const;
    
    /// Get functional status of gene (NEW: semantic API)
    [[nodiscard]] GeneStatus getStatus() const;
    
    /// Legacy compatibility: return string representation
    [[nodiscard]] std::string status() const {
        return getStatus().toString();
    }

    void setState(GeneStatus status);
    void mutate();
    // ... resto de métodos ...

private:
    GeneStatus state_;  // Changed from enum State to GeneStatus
};
```

**Cambios en AgenticCell.cpp - Ejemplos de refactoring**:

```cpp
// ANTES: String comparisons (error-prone)
const Gene* brca1 = genome_.getGene(GeneNames::BRCA1);
if (brca1->status() == GeneticStatusStrings::HOMOZYGOUS_RECESSIVE) {
    return false;
}

// DESPUÉS: Semantic, type-safe methods
const Gene* brca1 = genome_.getGene(GeneNames::BRCA1);
if (brca1->getStatus().isDisabled()) {
    return false;
}

// ANTES
if (tp53_status == "+/-" && brca1_status == "+/-") {

// DESPUÉS
auto tp53_status = tp53->getStatus();
auto brca1_status = brca1->getStatus();
if (tp53_status.isPartiallyEnabled() && brca1_status.isPartiallyEnabled()) {

// ANTES: calculateInstabilityDeltas()
if (tp53_status == GeneticStatusStrings::HOMOZYGOUS_RECESSIVE) {
    delta_tp53 = high_delta_instability_;
}

// DESPUÉS
if (tp53_status.isDisabled()) {
    delta_tp53 = high_delta_instability_;
}
```

**Cambios en Genome.cpp**:

```cpp
// ANTES
bool Genome::isUnstable() const {
    const Gene* tp53 = getGene(GeneNames::TP53);
    if (!tp53) return false;
    std::string s = tp53->status();
    return (s == GeneticStatusStrings::HETEROZYGOUS || 
            s == GeneticStatusStrings::HOMOZYGOUS_RECESSIVE);
}

// DESPUÉS
bool Genome::isUnstable() const {
    const Gene* tp53 = getGene(GeneNames::TP53);
    if (!tp53) return false;
    auto status = tp53->getStatus();
    return !status.isEnabled();  // Any non-wildtype is unstable
}
```

**Beneficios**:
- ✅ Type safety: imposible crear estados inválidos
- ✅ Semántica clara: `isDisabled()` vs `== "-/-"`
- ✅ Autocomplete: métodos en lugar de magic strings
- ✅ Extensible: puedes añadir más métodos (ej: `hasFunctionalCopy()`)
- ✅ Refactoring seguro: cambiar representación no afecta código que usa métodos
- ✅ Documentación viva: los nombres de métodos documentan la lógica

**Validación**:
- Compilar sin errores
- Ejecutar `./build/run_tests`
- Verificar que todos los tests pasan
- Validar que la semántica es consistente en todo el código

---

### 5. Extraer Feature Envy a Genome

**Complejidad**: Media (1-2 horas)

Mover lógica de viabilidad celular basada en genoma desde `alive()` a un método `Genome::isCellViable()` que encapsule las reglas BRCA1/TP53 en `Genome.h`.

**Archivos afectados**:
- `src/domain/gene/Genome.h`
- `src/domain/gene/Genome.cpp`
- `src/domain/cell/AgenticCell.cpp`

**Implementación**:

```cpp
// En Genome.h
class Genome {
public:
    // ...existing methods...
    
    /// Determina si el estado genómico permite que la célula sea viable
    /// Reglas:
    /// - BRCA1 ausente → NO viable
    /// - BRCA1 -/- + TP53 -/- → viable (no hay checkpoint)
    /// - BRCA1 -/- + TP53 funcional → NO viable (checkpoint mata la célula)
    /// - BRCA1 +/+ o +/- → viable
    [[nodiscard]] bool isCellViable() const;
    
    /// Retorna si el genoma indica protección neoplásica
    /// TP53 funcional (+/+ o +/-) puede eliminar células dañadas
    [[nodiscard]] bool hasNeoplasticProtection() const;
};

// En Genome.cpp
bool Genome::isCellViable() const {
    const Gene* brca1 = getGene(GeneNames::BRCA1);
    const Gene* tp53 = getGene(GeneNames::TP53);

    // BRCA1 missing → dead
    if (!brca1) {
        return false;
    }

    // BRCA1 -/- is lethal ONLY if TP53 is functional
    if (brca1->getStatus().isHomozygousRecessive()) {
        // TP53 -/- cannot kill the cell → alive
        if (tp53 && tp53->getStatus().isHomozygousRecessive()) {
            return true;
        }
        // TP53 functional → detects damage → dead
        return false;
    }

    // BRCA1 functional → viable
    return true;
}

bool Genome::hasNeoplasticProtection() const {
    const Gene* tp53 = getGene(GeneNames::TP53);
    if (!tp53) return false;
    return !tp53->getStatus().isHomozygousRecessive();
}

// En AgenticCell.cpp
bool AgenticCell::alive() const {
    // If cell has evaded apoptosis (is neoplastic), always alive
    if (has_evaded_apoptosis_) {
        return true;
    }
    
    // Delegate viability decision to genome
    return genome_.isCellViable();
}

bool AgenticCell::isNeoplasticProtected() const {
    return genome_.hasNeoplasticProtection();
}
```

**Beneficios**:
- Cohesión: lógica genómica en la clase Genome
- Testabilidad: puedes probar viabilidad sin crear AgenticCell
- Single Responsibility: AgenticCell no conoce reglas BRCA1/TP53
- Reutilización: otros componentes pueden consultar viabilidad genómica

**Validación**:
- Compilar sin errores
- Ejecutar `./build/run_tests`
- Verificar tests de viabilidad celular
- Validar que las reglas BRCA1/TP53 siguen funcionando
- Comprobar logs de muerte celular

---

### 6. Implementar Strategy para cálculo de deltas con Value Object

**Complejidad**: Media-Alta (2-3 horas)

Crear `InstabilityDeltas` (value object), `IInstabilityDeltaStrategy` con implementación `GenomicInstabilityDeltaStrategy` para extraer `calculateInstabilityDeltas()` y permitir modelos intercambiables en `AgenticCell.h`.

**Estructura de directorios**:
```
src/domain/cell/
├── model/
│   └── InstabilityDeltas.h  (nuevo - value object)
└── strategies/
    ├── IInstabilityDeltaStrategy.h  (nuevo - interfaz)
    ├── GenomicInstabilityDeltaStrategy.h  (nuevo)
    └── GenomicInstabilityDeltaStrategy.cpp  (nuevo)
```

**Nota**: Si el directorio `src/domain/cell/model/` no existe, créalo primero:
```bash
mkdir -p src/domain/cell/model
```

**Archivos afectados**:
- `src/domain/cell/model/InstabilityDeltas.h` (nuevo - value object)
- `src/domain/cell/strategies/IInstabilityDeltaStrategy.h` (nuevo)
- `src/domain/cell/strategies/GenomicInstabilityDeltaStrategy.h` (nuevo)
- `src/domain/cell/strategies/GenomicInstabilityDeltaStrategy.cpp` (nuevo)
- `src/domain/cell/AgenticCell.h`
- `src/domain/cell/AgenticCell.cpp`

**Implementación**:

```cpp
// InstabilityDeltas.h (Value Object)
#pragma once
#include <string>

namespace domain {
    /// Value object representing DNA damage and immunosuppression increments
    /// Encapsulates the pair of deltas calculated by instability strategies
    /// 
    /// Benefits over std::pair<double, double>:
    /// - Self-documenting: d1() and d2() vs first/second
    /// - Type-safe: cannot confuse order
    /// - Extensible: easy to add mutation_rate or other metrics
    /// - Helper methods: totalInstability(), isSignificant(), etc.
    class InstabilityDeltas {
    public:
        /// Factory: create from individual delta values
        static InstabilityDeltas create(double d1, double d2) {
            return InstabilityDeltas(d1, d2);
        }

        /// Getters
        [[nodiscard]] double d1() const { return d1_; }
        [[nodiscard]] double d2() const { return d2_; }

        /// Convenience: apply deltas to current values
        [[nodiscard]] double applyToD1(double current_d1) const {
            return current_d1 + d1_;
        }

        [[nodiscard]] double applyToD2(double current_d2) const {
            return current_d2 + d2_;
        }

        /// Total instability increase (sum of both)
        [[nodiscard]] double totalInstability() const {
            return d1_ + d2_;
        }

        /// Check if deltas are significant (above threshold)
        [[nodiscard]] bool isSignificant(double threshold = 0.001) const {
            return d1_ > threshold || d2_ > threshold;
        }

        /// Equality
        [[nodiscard]] bool operator==(const InstabilityDeltas& other) const {
            return d1_ == other.d1_ && d2_ == other.d2_;
        }

        [[nodiscard]] bool operator!=(const InstabilityDeltas& other) const {
            return !(*this == other);
        }

        /// String representation (for logging)
        [[nodiscard]] std::string toString() const {
            return "InstabilityDeltas{D1=" + std::to_string(d1_) +
                   ", D2=" + std::to_string(d2_) + "}";
        }

    private:
        explicit InstabilityDeltas(double d1, double d2)
            : d1_(d1), d2_(d2) {}

        double d1_;  // DNA damage delta
        double d2_;  // Immunosuppression delta
    };

} // namespace domain

// IInstabilityDeltaStrategy.h
#pragma once
#include "../model/InstabilityDeltas.h"

namespace domain {
    class AgenticCell;
    
    /// Interface para estrategias de cálculo de deltas de inestabilidad
    /// Permite inyectar diferentes modelos matemáticos de progresión tumoral
    class IInstabilityDeltaStrategy {
    public:
        virtual ~IInstabilityDeltaStrategy() = default;
        
        /// Calcula los incrementos de D1 (daño DNA) y D2 (inmunosupresión)
        /// @param cell Célula para la cual calcular los deltas
        /// @return InstabilityDeltas value object con d1 y d2
        virtual InstabilityDeltas calculateDeltas(const AgenticCell& cell) const = 0;
    };
}

// GenomicInstabilityDeltaStrategy.h
#pragma once
#include "IInstabilityDeltaStrategy.h"
#include "../AgenticCell.h"

namespace domain {
    /// Estrategia por defecto: calcula deltas basándose en estado de TP53 y BRCA1
    /// 
    /// Modelo:
    /// - D1 (DNA damage) depende solo de TP53
    /// - D2 (Immunosuppression) depende de TP53 + BRCA1 (aditivo)
    /// 
    /// Parámetros configurables:
    /// - low_delta: incremento para heterocigotos (+/-)
    /// - high_delta: incremento para homocigotos recesivos (-/-)
    class GenomicInstabilityDeltaStrategy : public IInstabilityDeltaStrategy {
    public:
        GenomicInstabilityDeltaStrategy(double low_delta, double high_delta)
            : low_delta_(low_delta), high_delta_(high_delta) {}

        InstabilityDeltas calculateDeltas(const AgenticCell& cell) const override;

    private:
        double low_delta_;
        double high_delta_;
        
        double calculateGeneDelta(const std::string& gene_status) const;
    };
}

// GenomicInstabilityDeltaStrategy.cpp
#include "GenomicInstabilityDeltaStrategy.h"
#include "../../gene/Gene.h"

namespace domain {
    InstabilityDeltas GenomicInstabilityDeltaStrategy::calculateDeltas(
        const AgenticCell& cell) const {
        
        std::string tp53_status = cell.getTP53();
        std::string brca1_status = cell.getBRCA1();

        // Delta TP53: affects D1 directly and contributes to D2
        double delta_tp53 = calculateGeneDelta(tp53_status);

        // Delta BRCA1: contributes only to D2
        double delta_brca1 = 0.0;
        if (brca1_status == "+/-") {
            delta_brca1 = low_delta_;
        } else if (brca1_status == "-/-") {
            delta_brca1 = 2 * high_delta_;
        }

        // D1 (DNA damage) = Δ(TP53) only
        // D2 (Immunosuppression) = Δ(TP53) + Δ(BRCA1)
        double delta_d1 = delta_tp53;
        double delta_d2 = delta_tp53 + delta_brca1;

        return InstabilityDeltas::create(delta_d1, delta_d2);
    }

    double GenomicInstabilityDeltaStrategy::calculateGeneDelta(
        const std::string& gene_status) const {
        
        if (gene_status == "+/+") {
            return 0.0;
        } else if (gene_status == "+/-") {
            return low_delta_;
        } else if (gene_status == "-/-") {
            return high_delta_;
        }
        return 0.0;
    }
}

// En AgenticCell.h
class AgenticCell {
private:
    std::unique_ptr<IInstabilityDeltaStrategy> delta_strategy_;
    
public:
    AgenticCell(...,
                std::unique_ptr<IInstabilityDeltaStrategy> delta_strategy = nullptr);
    
    // Exponer getters para que estrategia pueda acceder
    [[nodiscard]] const Genome& getGenome() const { return genome_; }
    [[nodiscard]] bool hasEvadedApoptosis() const { return has_evaded_apoptosis_; }
};

// En AgenticCell.cpp (constructor)
AgenticCell::AgenticCell(..., std::unique_ptr<IInstabilityDeltaStrategy> delta_strategy)
    : delta_strategy_(delta_strategy
        ? std::move(delta_strategy)
        : std::make_unique<GenomicInstabilityDeltaStrategy>(
            low_delta_instability, high_delta_instability)) {
    // ...
}

// En phase4_CytoplasmicRemodeling
void AgenticCell::phase4_CytoplasmicRemodeling() {
    // Calculate instability deltas using strategy (returns value object)
    InstabilityDeltas deltas = delta_strategy_->calculateDeltas(*this);
    
    // Apply deltas (self-documenting)
    d1_dna_damage_ = deltas.applyToD1(d1_dna_damage_);
    d2_immunosuppression_ = deltas.applyToD2(d2_immunosuppression_);
    
    // Optional: log with toString()
    if (logger_) {
        logger_->logCell("[Phase4] Applied " + deltas.toString());
    }
    
    // ... resto del código
}
```

**Estrategias alternativas posibles**:
- `AgeBasedDeltaStrategy`: considera edad celular
- `StochasticDeltaStrategy`: añade ruido aleatorio
- `CompoundDeltaStrategy`: combina múltiples estrategias

**Beneficios de combinar Value Object + Strategy Pattern**:

Esta refactorización aplica **dos patrones complementarios**:

1. **Value Object `InstabilityDeltas`** (mejora el *qué* retorna la estrategia):
   - **Semántica clara**: `deltas.d1()` y `deltas.d2()` vs `pair.first` y `pair.second`
   - **Type-safe**: imposible confundir el orden de los valores
   - **Extensibilidad**: fácil añadir `mutation_rate_` u otras métricas sin romper código existente
   - **Métodos helper**: `totalInstability()`, `isSignificant()`, `toString()` para logging
   - **Testeable**: asserts claros en tests (`ASSERT_EQ(deltas.d1(), expected)`)
   - **Autodocumentado**: el código se lee como lenguaje natural

2. **Strategy Pattern** (mejora el *cómo* se calculan los deltas):
   - **Open/Closed**: añadir modelos sin modificar AgenticCell
   - **Testing**: mockear estrategia para tests deterministas
   - **Calibración**: ajustar parámetros sin recompilar
   - **Experimentación**: comparar modelos biológicos fácilmente
   - **Dependency Inversion**: AgenticCell depende de abstracción, no de implementación

**Sinergia entre ambos patrones**:
```cpp
// Strategy define el ALGORITMO
class IInstabilityDeltaStrategy {
    // Value Object encapsula el RESULTADO
    virtual InstabilityDeltas calculateDeltas(...) const = 0;
};

// Uso: claro, seguro y flexible
InstabilityDeltas deltas = strategy->calculateDeltas(cell);  // Strategy
d1_dna_damage_ = deltas.applyToD1(d1_dna_damage_);          // Value Object
```

**Validación**:
- Compilar sin errores
- Ejecutar `./build/run_tests`
- Verificar que deltas D1/D2 se calculan correctamente con diferentes genotipos
- Validar que `InstabilityDeltas` funciona correctamente:
  - `deltas.d1()` y `deltas.d2()` retornan valores esperados
  - `deltas.totalInstability()` suma correctamente
  - `deltas.isSignificant()` detecta umbrales
  - `deltas.toString()` genera log legible
- Comprobar logs de actualización de instabilidad con formato autodocumentado
- Validar que la inyección de estrategia funciona con default y custom strategies

**Tests sugeridos**:
```cpp
// Test del Value Object
TEST(InstabilityDeltasTest, CreateAndAccess) {
    auto deltas = InstabilityDeltas::create(0.5, 1.2);
    ASSERT_EQ(deltas.d1(), 0.5);
    ASSERT_EQ(deltas.d2(), 1.2);
    ASSERT_EQ(deltas.totalInstability(), 1.7);
    ASSERT_TRUE(deltas.isSignificant());
}

// Test de la Strategy
TEST(GenomicInstabilityDeltaStrategyTest, CalculateDeltas_TP53Heterozygous) {
    // Setup
    Genome genome = GenomeFactory::createWithMutations({{GeneNames::TP53, "+/-"}});
    AgenticCell cell(/* ... */, std::move(genome));
    GenomicInstabilityDeltaStrategy strategy(0.1, 0.2);
    
    // Execute
    InstabilityDeltas deltas = strategy.calculateDeltas(cell);
    
    // Verify
    ASSERT_EQ(deltas.d1(), 0.1);  // TP53 +/- contributes low_delta to D1
    ASSERT_EQ(deltas.d2(), 0.1);  // TP53 +/- contributes low_delta to D2
}
```

---

### 7. Implementar Strategy para viabilidad celular

**Complejidad**: Media (1.5-2 horas)

Crear `IViabilityStrategy` con implementación `GenomicViabilityStrategy` para extraer método `alive()` y desacoplar reglas de supervivencia en `AgenticCell.h`.

**Archivos afectados**:
- `src/domain/cell/strategies/IViabilityStrategy.h` (nuevo)
- `src/domain/cell/strategies/GenomicViabilityStrategy.h` (nuevo)
- `src/domain/cell/strategies/GenomicViabilityStrategy.cpp` (nuevo)
- `src/domain/cell/AgenticCell.h`
- `src/domain/cell/AgenticCell.cpp`

**Implementación**:

```cpp
// IViabilityStrategy.h
#pragma once

namespace domain {
    class AgenticCell;
    
    /// Interface para estrategias de viabilidad celular
    /// Permite inyectar diferentes criterios de supervivencia
    class IViabilityStrategy {
    public:
        virtual ~IViabilityStrategy() = default;
        
        /// Determina si la célula está viva según criterios específicos
        /// @param cell Célula a evaluar
        /// @return true si la célula es viable, false en caso contrario
        virtual bool isAlive(const AgenticCell& cell) const = 0;
    };
}

// GenomicViabilityStrategy.h
#pragma once
#include "IViabilityStrategy.h"

namespace domain {
    /// Estrategia por defecto: viabilidad basada en estado genómico
    /// 
    /// Reglas:
    /// - Células que evadieron apoptosis → siempre vivas (inmortales)
    /// - Viabilidad determinada por Genome::isCellViable()
    class GenomicViabilityStrategy : public IViabilityStrategy {
    public:
        GenomicViabilityStrategy() = default;
        
        bool isAlive(const AgenticCell& cell) const override;
    };
}

// GenomicViabilityStrategy.cpp
#include "GenomicViabilityStrategy.h"
#include "../AgenticCell.h"

namespace domain {
    bool GenomicViabilityStrategy::isAlive(const AgenticCell& cell) const {
        // If cell has evaded apoptosis (is neoplastic), always alive
        if (cell.hasEvadedApoptosis()) {
            return true;
        }
        
        // Delegate to genome viability check
        return cell.getGenome().isCellViable();
    }
}

// En AgenticCell.h
class AgenticCell {
private:
    std::unique_ptr<IViabilityStrategy> viability_strategy_;
    std::unique_ptr<IInstabilityDeltaStrategy> delta_strategy_;
    
public:
    AgenticCell(...,
                std::unique_ptr<IViabilityStrategy> viability_strategy = nullptr,
                std::unique_ptr<IInstabilityDeltaStrategy> delta_strategy = nullptr);
    
    bool alive() const override {
        return viability_strategy_->isAlive(*this);
    }
};

// En AgenticCell.cpp (constructor)
AgenticCell::AgenticCell(...,
                         std::unique_ptr<IViabilityStrategy> viability_strategy,
                         std::unique_ptr<IInstabilityDeltaStrategy> delta_strategy)
    : viability_strategy_(viability_strategy
        ? std::move(viability_strategy)
        : std::make_unique<GenomicViabilityStrategy>()),
      delta_strategy_(delta_strategy
        ? std::move(delta_strategy)
        : std::make_unique<GenomicInstabilityDeltaStrategy>(
            low_delta_instability, high_delta_instability)) {
    // ...
}
```

**Estrategias alternativas posibles**:
- `ImmortalViabilityStrategy`: siempre viva (para tests)
- `TimeBasedViabilityStrategy`: muerte por senescencia
- `ResourceBasedViabilityStrategy`: considera nutrientes

**Beneficios**:
- Separación de responsabilidades clara
- Testing: inyectar estrategia mock
- Flexibilidad: cambiar criterios sin modificar célula
- Consistencia: mismo patrón que delta strategy

**Validación**:
- Compilar sin errores
- Ejecutar `./build/run_tests`
- Verificar todos los tests de viabilidad pasan
- Validar escenarios de muerte celular
- Comprobar integración con phase1_G1IntegrityCheckpoint

---

## Further Considerations

### Refactorings adicionales (después de completar los 7 pasos)

1. **Encapsular estado mutable en CellularState**
   
   Después de las estrategias, considerar encapsular `d1_dna_damage_`, `d2_immunosuppression_`, `age_`, `is_neoplastic_` en una clase `CellularState` para:
   - Reducir campos mutables directos en AgenticCell
   - Facilitar snapshot/restore para tests
   - Mejorar razonamiento sobre transiciones de estado
   
   ```cpp
   class CellularState {
   public:
       double d1_damage;
       double d2_immunosuppression;
       int age;
       bool is_neoplastic;
       bool has_evaded_apoptosis;
       
       CellLifeStage deriveLifeStage(const Genome& genome, 
                                      double d1_threshold, 
                                      double d2_threshold) const;
   };
   ```

2. **Añadir comportamiento a Gene**
   
   Después del paso 4 (GeneticStatus), considerar añadir métodos de dominio:
   
   ```cpp
   class Gene {
   public:
       bool isLethalWhenLost() const;
       bool isTumorSuppressor() const;
       bool requiresCooperativeInactivation(const Gene& other) const;
   };
   ```

3. **State Pattern para ciclo celular**
   
   Si las fases se vuelven más complejas, considerar:
   
   ```cpp
   class ICellPhaseState {
   public:
       virtual void execute(AgenticCell& cell) = 0;
       virtual std::unique_ptr<ICellPhaseState> nextPhase() = 0;
   };
   ```

4. **Builder Pattern para construcción de células**
   
   Si la construcción se vuelve compleja después de Parameter Objects:
   
   ```cpp
   class AgenticCellBuilder {
   public:
       AgenticCellBuilder& withGenome(Genome genome);
       AgenticCellBuilder& withInstabilityConfig(InstabilityConfig config);
       AgenticCellBuilder& withViabilityStrategy(std::unique_ptr<IViabilityStrategy> strategy);
       std::unique_ptr<AgenticCell> build();
   };
   ```

### Testing entre pasos

**Después de cada paso**, ejecutar:

```bash
cd /home/luis/CLionProjects/cellSim/build
cmake --build . --target run_tests
./run_tests
```

**Tests críticos a verificar**:
- `CellLifeStageTransitionTest`: Transiciones entre estados
- `NeoplasticImmortalityTest`: Evasión de apoptosis
- `CellDivisionInstabilityPropagationTest`: Herencia de inestabilidad
- `SimulationTissueV2IntegrationTest`: Integración completa

### Documentación

Después de cada paso, actualizar:
- Comentarios de clase en headers
- README técnico si existe
- Diagramas UML si hay documentation

### Métricas de éxito

Al finalizar los 7 pasos:
- ✅ Reducción de parámetros del constructor: 12 → 6 (o 4-6 con Parameter Objects)
- ✅ Eliminación de magic strings: 100%
- ✅ Eliminación de duplicación de umbrales: `apoptosis_threshold_` unificado
- ✅ Comparaciones tipo-safe: strings → value objects
- ✅ Estrategias inyectables: 2 (viability + delta)
- ✅ Tests pasando: 100%
- ✅ Cobertura mantenida o mejorada

