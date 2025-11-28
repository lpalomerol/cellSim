└─ INMUNOSUPRESIÓN: MÁXIMA 🔴
   ├─ Sistema inmune severamente deprimido
   ├─ Vigilancia: prácticamente ausente
   └─ Evasión inmune: altamente probable
## Introducción

### Visualización: Tránsito de TP53

```
        TP53 +/+              TP53 +/-              TP53 -/-
        (SALVAJE)            (HETERO)             (KNOCKOUT)

        🟢 Segura            🟡 Degradada          🔴 Peligrosa
        
        No tumores ✅        No tumores ✅         Tumores ✅
        Baja mut ✅          Med mut ⚠️            Alta mut ❌
        Inmune ok ✅         Inmune ⚠️             Inmune ❌

        Estable              En transición         Caótica
```

### Tabla Comparativa

| Característica | TP53 +/+ | TP53 +/- | TP53 -/- |
|---|---|---|---|
| **Protección** | Completa ✅ | Degradada ⚠️ | Nula ❌ |
| **Permite tumor** | NO | NO | SÍ |
| **Inestabilidad** | Base 🟢 | Media 🟡 | Alta 🔴 |
| **Inmunosupresión** | Base 🟢 | Media 🟡 | Alta 🔴 |
| **Mutaciones/año** | Normal | ↑ 1.5-2x | ↑↑↑ 3-5x |
| **Riesgo oncogénico** | Bajo | Medio | Muy alto |

La **célula** es la unidad funcional viva del simulador. Es una entidad autónoma con su propio genoma, identidad, edad y estado. Las células pueden vivir, mutar, reproducirse, morir o transformarse en neoplasias.

---

## ¿Qué es AgenticCell?

`AgenticCell` es la implementación de una célula en cellSim. Es la clase que implementa la interfaz `ICell`:

```cpp
class AgenticCell final : public ICell {
public:
    AgenticCell(std::unique_ptr<INoiseSource> noise,
                Genome genome,
                double neoplasm_k = 0.002,
                double low_delta_instability = 0.0001,
                double high_delta_instability = 0.0002,
                bool verbose = false);
    
    void live() override;
    bool alive() const override;
    bool isNeoplastic() const override;
    
    // Interfaz de comunicación
    void setSignalEmitter(std::function<void(std::unique_ptr<ISignal>)>) override;
    void receiveMessage(std::unique_ptr<ISignal> signal) override;
};
```

### Características Clave

✅ Ciclo de vida completo (nacimiento, mutación, muerte)  
✅ Genoma independiente  
✅ Identificación única  
✅ Edad y trazabilidad  
✅ Comunicación mediante señales  
✅ Muestreo estocástico de eventos  
✅ Ruido inyectable para reproducibilidad  

---

## Identidad de la Célula

### ID Único

Cada célula tiene un identificador único asignado por el tejido:

```cpp
// Asignación (por el tejido)
cell->setId(42);

// Lectura
std::uint64_t cell_id = cell->id();  // 42
```

### Edad de la Célula

La edad se incrementa en cada tick:

```cpp
std::uint64_t age = cell->getAge();  // Número de años vividos
```

### Semilla del Ruido

Cada célula tiene una fuente de ruido con semilla reproducible:

```cpp
std::uint64_t seed = cell->getSeed();
```

---

## Estados Celulares

Una célula puede estar en uno de estos estados:

### 1. **Viva** ✅
```
Célula normal, ejecutando ciclo de vida
├─ Genoma activo
├─ Mutar normalmente
├─ Envejecer
└─ Comunicarse con otras células
```

### 2. **Muerta** ❌
```
Célula ha sido destruida
├─ No muta más
├─ No comunica
└─ Puede ser removida del tejido
```

### 3. **Neoplástica** ⚠️
```
Célula cancerosa / transformada
├─ Sigue viviendo
├─ Emite señales de neoplasia
├─ Puede afectar a otras células
└─ Es rastreada por el tejido
```

---

## Ciclo de Vida de la Célula: Método `live()`

El ciclo de vida se ejecuta en cada tick (año). Tiene **3 fases principales**:

```cpp
void AgenticCell::live() override {
    // FASE 1: Avanzar genes
    genome_.liveAllGenes();
    
    // FASE 2: Evaluar muerte
    // (lógica de viabilidad)
    if (!shouldContinueLiving()) {
        is_alive_ = false;
        return;
    }
    
    // FASE 3: Evaluar neoplasia
    // (si TP53 protege, salta esta fase)
    if (!isNeoplasticProtected()) {
        develop_neoplasm();
    } else {
        // Registrar protección
    }
    
    // Incrementar edad
    age_++;
}
```

### Visualización del Ciclo

```
┌─────────────────────────────────────────────┐
│      CICLO DE VIDA: live()                  │
└─────────────────────────────────────────────┘

INICIO (Célula viva)
│
├─────► FASE 1: MUTACIÓN GENÓMICA
│       │
│       ├─ genome_.liveAllGenes()
│       │  ├─ Detectar inestabilidad (¿TP53 dañado?)
│       │  ├─ Para cada gen: evaluar mutación
│       │  └─ Actualizar estado genómico
│       │
│       └─ Si TP53 se daña → Cascada de inestabilidad
│
├─────► FASE 2: EVALUACIÓN DE VIABILIDAD (Punto crítico: BRCA1)
│       │
│       ├─ ¿BRCA1 está enabled()?
│       │  (es decir, ¿BRCA1 es +/- ? ← ÚNICO estado viable)
│       │
│       ├─ SÍ (BRCA1 +/-) ──► Célula VIABLE ✅
│       │  │
│       │  └─ Continuar al siguiente paso
│       │
│       └─ NO (BRCA1 -/-) ──► APOPTOSIS
│           │
│           └─ Célula MUERE
│               is_alive_ = false
│               throw CellDeathException
│               Retornar (fin del ciclo)
│
├─────► FASE 3: EVALUACIÓN DE NEOPLASIA
│       │
│       ├─ ¿TP53 es -/- ?
│       │  (REGLA CLAVE: solo -/- permite tumores)
│       │
│       ├─ NO (TP53 +/+ o +/-) ──► PROTEGIDA
│       │  │
│       │  ├─ Si TP53 +/-: aumenta inestabilidad e inmunosupresión
│       │  │
│       │  └─ SIN transformación neoplástica
│       │     No ejecutar develop_neoplasm()
│       │
│       └─ SÍ (TP53 -/-) ──► VULNERABLE
│           │
│           ├─ develop_neoplasm()
│           │
│           ├─ Muestrear random(0-1)
│           ├─ Comparar con neoplasm_k
│           │
│           ├─ random < neoplasm_k?
│           │  ├─ SÍ ──► NEOPLASIA DESARROLLADA
│           │  │       is_neoplastic_ = true
│           │  │       Emitir signal
│           │  │
│           │  └─ NO ──► Sin cambio (pero inestable)
│
├─────► INCREMENTAR EDAD
│       age_++
│
└─────► FIN DE CICLO
        Célula lista para siguiente tick
```

---

## Fase 1: Ciclo Genómico

### Ejecución

```cpp
genome_.liveAllGenes(genomic_instability);
```

### Efectos

- Cada gen evalúa mutación
- Threshold base + factor de inestabilidad si es necesario
- Cambios de estado acumulativos
- Cascada potencial de mutaciones

### Ejemplo

```
Antes:  TP53[+/+]  BRCA1[+/-]
↓ Ciclo 1
Después: TP53[+/+]  BRCA1[+/-]  (sin cambios)

Antes:  TP53[+/+]  BRCA1[+/-]
↓ Ciclo 2
Después: TP53[+/-]  BRCA1[+/-]  (TP53 mutó!)

Antes:  TP53[+/-]  BRCA1[+/-]
↓ Ciclo 3
Después: TP53[+/-]  BRCA1[-/-]  (BRCA1 mutó!)
         → CÉLULA MUERE (apoptosis en Fase 2)
```

---

## Fase 2: Evaluación de Viabilidad

### Concepto

Determinar si la célula puede seguir viva con su genoma actual.

### Lógica en cellSim

```cpp
bool AgenticCell::alive() const {
    const Gene *brca1 = genome_.getGene("BRCA1");
    return brca1 && brca1->enabled();  // Viabilidad depende de BRCA1
}
```

**BRCA1 es ESENCIAL para viabilidad**:
- `BRCA1 +/-` → ✅ Viable (único estado posible vivo)
- `BRCA1 -/-` → ❌ Apoptosis (muerte celular)

### Razón Biológica

Sin gen de reparación del ADN funcional (BRCA1 -/-), la acumulación de daño es letal. La célula entra en apoptosis automáticamente.

### Ejemplo

```
Escenario 1: BRCA1 +/- (estado inicial)
  alive() = true ✅

Escenario 2: BRCA1 muta a -/-
  alive() = false ❌ 
  → CellDeathException
  → Célula muere (apoptosis)
```

---

## Fase 3: Evaluación de Neoplasia

### Concepto

Determinar si la célula se transforma en una célula cancerosa.

### Protección por TP53

```cpp
bool AgenticCell::isNeoplasticProtected() const {
    const Gene* tp53 = genome_.getGene("TP53");
    if (!tp53) return false;
    
    // Solo protege si está en estado +/+
    return tp53->enabled();
}
```

### Tabla: Protección (Corrección según feedback del jefe)

| Estado TP53 | ¿Puede formar tumor? | Inestabilidad | Inmunosupresión | Acción |
|---|---|---|---|---|
| +/+ | ❌ NO | Base | Base | No muestrear neoplasia |
| +/- | ❌ NO | Aumenta | Aumenta | No muestrear neoplasia, pero con degradación |
| -/- | ✅ SÍ | Aumenta aún más | Aumenta aún más | Muestrear neoplasia activamente |

**Regla clave**: Una célula **NO puede formar tumor hasta que TP53 sea -/-**. Los estados TP53 +/? (incluyendo +/-) NO permiten transformación neoplástica, pero SÍ aumentan inestabilidad genómica e inmunosupresión.

### Muestreo de Neoplasia

```cpp
void AgenticCell::develop_neoplasm() {
    double u01 = noise_->next().u01;
    
    if (u01 < neoplasm_k_) {
        is_neoplastic_ = true;
        // Emitir señal
        emitNeoplasmSignal();
    }
}
```

### Parámetro: `neoplasm_k`

Umbral de transformación neoplástica:

```
neoplasm_k = 0.002  (0.2% por año)
             0.01   (1% por año)
             0.05   (5% por año)
```

Valores típicos en cellSim: **0.002 a 0.01**

---

## Protección TP53 Detallada

### Biología

TP53 es el "guardián del genoma":
- Detecta daño en el ADN
- Detiene el ciclo celular
- Induce apoptosis o reparación
- En cellSim: controla capacidad de formación de tumores

### Regla Fundamental (Feedback del Jefe)

**Una célula NO puede formar tumor a menos que TP53 sea -/-.**

Esto significa:
- TP53 +/+ → Protección completa ✅ 
- TP53 +/- → Protección presente (aunque degradada) ✅ 
- TP53 -/- → SIN protección, tumores posibles ❌ 

Sin embargo, la pérdida de función de TP53 genera dos efectos secundarios graves:
1. **Inestabilidad genómica**: aumenta con +/- y aún más con -/-
2. **Inmunosupresión**: aumenta progresivamente

### Implementación

```cpp
if (isNeoplasticProtected()) {
    // TP53 está +/+, célula es resistente a neoplasia
    if (verbose_) {
        std::cout << "Cell " << id_ << " is protected by TP53" 
                  << std::endl;
    }
    // No ejecutar develop_neoplasm()
    return;
}

// Si llegamos aquí, TP53 está dañado o ausente
// Nota: TP53 +/- TAMBIÉN está protegido en cellSim
// (en versiones futuras: implementar grados de protección)
develop_neoplasm();
```

### Modelo de Degradación: 3 Niveles

```
┌─────────────────────────────────────────────────────┐
│   NIVELES DE TP53 Y SUS EFECTOS                    │
└─────────────────────────────────────────────────────┘

NIVEL 1: TP53 +/+
│
├─ PROTECCIÓN: COMPLETA ✅
│  ├─ Detecta daño genético eficientemente
│  ├─ Detiene ciclo celular
│  ├─ Puede inducir reparación o apoptosis
│  └─ NO permite transformación neoplástica
│
├─ INESTABILIDAD GENÓMICA: BASE 🟢
│  ├─ Mutaciones en otros genes: tasa normal
│  └─ Cascadas de mutación: mínimas
│
└─ INMUNOSUPRESIÓN: BASE 🟢
   ├─ Sistema inmune activo
   └─ Vigilancia normal de células anormales

───────────────────────────────────────────────────────

NIVEL 2: TP53 +/-
│
├─ PROTECCIÓN: DEGRADADA ⚠️
│  ├─ Detecta daño, pero con retardo
│  ├─ Respuesta celular debilitada
│  ├─ Capacidad de reparación reducida
│  └─ NO permite transformación neoplástica (aún protege)
│
├─ INESTABILIDAD GENÓMICA: AUMENTA 🟡
│  ├─ Mutaciones en otros genes: frecuencia mayor
│  ├─ Cascadas de mutación: más probable
│  └─ Knockdown de TP53: baja expresión
│
└─ INMUNOSUPRESIÓN: AUMENTA 🟡
   ├─ Sistema inmune parcialmente deprimido
   ├─ Vigilancia reducida de células anormales
   └─ Primeros signos de evasión inmune

───────────────────────────────────────────────────────

NIVEL 3: TP53 -/-
│
├─ PROTECCIÓN: NULA ❌
│  ├─ No detecta daño
│  ├─ No detiene ciclo celular
│  ├─ No induce apoptosis
│  └─ ✅ PERMITE transformación neoplástica
│
├─ INESTABILIDAD GENÓMICA: MÁXIMA 🔴
│  ├─ Mutaciones: tasa muy alta
│  ├─ Cascadas de mutación: frecuentes y graves
│  ├─ Inestabilidad cromosómica
│  └─ Acumulación rápida de anomalías genómicas
│
├─► Alto riesgo de transformación
└─► Alto riesgo ❌
```

---

## Ruido Estocástico

### Concepto

Las decisiones de la célula no son determinísticas. Utilizan muestreo aleatorio:

```
Random u01 en [0, 1)
├─ Mutación: u01 < threshold?
├─ Neoplasia: u01 < neoplasm_k?
└─ Otros eventos estocásticos
```

### Inyección de Ruido

```cpp
// Al crear la célula
std::unique_ptr<INoiseSource> noise = 
    std::make_unique<RandomNoise>(seed);

AgenticCell cell(std::move(noise), genome);

// O con ruido fijo (reproducible)
std::unique_ptr<INoiseSource> fixed_noise = 
    std::make_unique<FixedNoise>();
```

### Reproducibilidad

```cpp
// Dos células con mismo genoma y misma semilla
// producirán idénticas secuencias de eventos

uint64_t seed = 12345;
AgenticCell cell1(
    std::make_unique<RandomNoise>(seed),
    genome.clone()
);

AgenticCell cell2(
    std::make_unique<RandomNoise>(seed),
    genome.clone()
);

// cell1 y cell2 tendrán evoluciones idénticas
```

---

## Interfaz de Comunicación

### Emisión de Señales

```cpp
// Configurar emisor de señales
auto emitter = [&tissue](std::unique_ptr<ISignal> signal) {
    tissue.broadcastSignal(std::move(signal));
};

cell->setSignalEmitter(emitter);

// La célula puede emitir señales durante live()
```

### Recepción de Mensajes

```cpp
// Tejido envía mensaje a célula
auto message = std::make_unique<NeoplasmSignal>(
    source_id,
    "Neighbor became neoplastic"
);

cell->receiveMessage(std::move(message));

// La célula procesa en siguiente live()
```

---

## Propiedades de Consulta

### Estado Genómico

```cpp
// Obtener genes específicos
std::string tp53_status = cell->getTP53();  // "+/+", "+/-", "-/-"
std::string brca1_status = cell->getBRCA1();

// Interfaz IGeneticProfile
std::string tp53_via_profile = cell->getTP53Status();
```

### Identidad y Edad

```cpp
std::uint64_t id = cell->id();
std::uint64_t age = cell->getAge();
std::uint64_t seed = cell->getSeed();
```

### Estado de Viabilidad

```cpp
bool viva = cell->alive();
bool neoplastica = cell->isNeoplastic();
bool protegida = cell->isNeoplasticProtected();
```

### Información

```cpp
cell->details();  // Imprime estado completo
```

---

## Código: Ejemplo de Célula

```cpp
#include "domain/cell/AgenticCell.h"
#include "domain/gene/GenomeFactory.h"
#include "domain/adapters/RandomNoise.h"

// Crear genoma por defecto
Genome genome = Genome::makeDefaultGenome();

// Crear fuente de ruido
auto noise = std::make_unique<RandomNoise>(42);

// Crear célula
AgenticCell cell(
    std::move(noise),
    genome,
    0.002,  // neoplasm_k
    0.0001, // low_delta_instability
    0.0002  // high_delta_instability
);

// Asignar ID
cell.setId(1);

// Simular 100 años
for (int year = 0; year < 100; ++year) {
    cell.live();
    
    if (cell.isNeoplastic()) {
        std::cout << "Cell became neoplastic at year " 
                  << year << std::endl;
        break;
    }
}

// Consultar estado final
std::cout << "Final age: " << cell.getAge() << std::endl;
cell.details();
```

---

## Recapitulación

| Concepto | Explicación |
|----------|-------------|
| **AgenticCell** | Implementación de célula viva |
| **ID** | Identificador único asignado por tejido |
| **Edad** | Número de ticks/años vividos |
| **Fase 1** | Ciclo genómico (liveAllGenes) |
| **Fase 2** | Evaluación de viabilidad |
| **Fase 3** | Evaluación de neoplasia |
| **Protección** | TP53 +/+ previene transformación |
| **Muestreo** | Ruido estocástico en decisiones |

---

## Diagrama: Máquina de Estados Celular

```
┌──────────────┐
│    NACIDA    │
└──────┬───────┘
       │
       ▼
    VIVA ◄────────────────┐
    │                    │
    ├─► live()           │
    │   ├─ Fase 1        │
    │   ├─ Fase 2        │
    │   ├─ Fase 3        │
    │   └─ age++         │
    │                    │
    ├─ ¿Sigue viva?      │
    │   ├─ SÍ ──────────┘
    │   │
    │   └─ NO
    │       │
    │       ▼
    │     MUERTA
    │
    ├─ ¿Neoplástica?
    │   │
    │   ├─ SÍ ──► NEOPLÁSTICA (sigue viva, pero transformada)
    │   │
    │   └─ NO ──► Vuelve a ciclo (bucle)
```

---

## Siguiente Paso

👉 **Lee [Tejidos](./tissue.md)** para entender cómo múltiples células se organizan y coordinan en un tejido completo.

---

## Referencias Cruzadas

- 🔗 [Genomas](./genome.md) - Lo que contiene cada célula
- 🔗 [Tejidos](./tissue.md) - Dónde viven las células
- 🔗 [Neoplasias](./neoplasm.md) - Transformación de células
- 🔗 [Interacciones](./interactions.md) - Comunicación celular

