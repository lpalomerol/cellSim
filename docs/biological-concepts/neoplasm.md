# ⚠️ Neoplasias y Progresión del Cáncer en cellSim

## Introducción

Una **neoplasia** es la transformación de una célula normal en una célula cancerosa. En cellSim, representa un cambio cualitativo en el estado celular que resulta de la acumulación de mutaciones genéticas, particularmente en genes supresores de tumores como **TP53** y genes de reparación del ADN como **BRCA1**.

---

## ¿Qué es la Neoplasia?

La neoplasia es un **evento estocástico** que ocurre cuando:

1. Una célula acumula suficientes mutaciones genéticas
2. TP53 pierde su protección (muta a +/- o -/-)
3. Un muestreo aleatorio resulta favorable para transformación
4. La célula cambia a estado `isNeoplastic() = true`

### Implementación

```cpp
// En AgenticCell
bool is_neoplastic_ = false;

void develop_neoplasm() {
    if (u01 < neoplasm_k_) {
        is_neoplastic_ = true;
        emitNeoplasmSignal();
    }
}

bool isNeoplastic() const {
    return is_neoplastic_;
}
```

---

## Modelo Biológico: De Mutaciones a Cáncer

### Biología Real

El cáncer es resultado de múltiples mutaciones acumuladas:

```
Célula Normal
    │
    ├─ Mutación 1 (BRCA1 dañado)
    │  └─ Reparación defectuosa
    │
    ├─ Mutación 2 (TP53 parcialmente dañado +/-)
    │  └─ Control de calidad debilitado
    │  └─ Cascada de instabilidad
    │
    ├─ Mutaciones 3-N (otras células, aprovechar inestabilidad)
    │  └─ Acumulación rápida
    │
    ▼
Célula Precancerosa
    │
    ├─ Evento final (muestreo estocástico)
    │
    ▼
Célula Cancerosa (Neoplasia)
```

### En cellSim

cellSim modela este proceso con:

- **Eventos determinísticos**: Mutaciones genéticas
- **Evento estocástico final**: Transformación neoplástica

---

## Rol Crítico de BRCA1 en cellSim

### Contexto Biológico

Las células en cellSim son **células de mama luminales** con **estado inicial BRCA1 +/-** (heterocigoto). Esto refleja:

- Portadores hereditarios de mutación BRCA1
- Mayor predisposición a cáncer de mama
- Pero viables inicialmente (una copia funcional)

### Mecanismo de Apoptosis

```cpp
// En AgenticCell::alive()
bool AgenticCell::alive() const {
    const Gene *brca1 = genome_.getGene("BRCA1");
    return brca1 && brca1->enabled();  // Muere si BRCA1 es -/-
}
```

**Implicación**: Si BRCA1 muta a **-/- (ambas copias no funcionales)**:
- ✅ Célula **muere por apoptosis**
- ❌ **No se transforma en neoplasia**
- ❌ **No progresa hacia cáncer**

### Estados de BRCA1 y Viabilidad

| Estado | Viabilidad | Evento |
|--------|-----------|--------|
| +/+ | ✅ Viable | Reparación normal |
| +/- | ✅ Viable | Estado inicial, reparación limitada |
| -/- | ❌ Muerte | Apoptosis inmediata |

### Implicación para el Multihit

```
El multihit clásico NO puede incluir BRCA1 -/-

Ruta viable:
  BRCA1 +/- → (no muta) → TP53 +/+ → TP53 +/- → TP53 -/- → Neoplasia

Ruta terminal:
  BRCA1 +/- → BRCA1 -/- → APOPTOSIS (fin)
```

---

## Eventos Carcinogénicos: BRCA1 y TP53

### BRCA1: Reparación del ADN

**Función**: Reparación de rupturas de doble hebra

**Importancia**:
- Sin BRCA1, mutaciones se acumulan más rápido
- Asociado con cáncer de mama hereditario
- En cellSim: su mutación incrementa inestabilidad

**Estados BRCA1**:

| Estado | Función | Efecto |
|--------|---------|--------|
| +/+ | Normal | Reparación funcional |
| +/- | Reducida | Capacidad de reparación limitada |
| -/- | Nula | Sin reparación, caos genómico |

### TP53: El Guardián del Genoma

**Función**: Detecta daño y previene transformación

**Importancia**:
- Más importante que BRCA1 para prevenir neoplasia
- Mutado en ~50% de cánceres humanos
- En cellSim: su presencia (+/+) bloquea transformación

**Estados TP53**:

| Estado | Protección | Efecto |
|--------|-----------|--------|
| +/+ | Total | Previene transformación (100%) |
| +/- | Ninguna | Permite transformación normal |
| -/- | Ninguna | Permite transformación acelerada |

---

## Hipótesis del Multihit

### Concepto

El cáncer requiere **múltiples eventos mutacionales independientes**:

```
Célula Normal → Evento 1 → Evento 2 → Evento 3 → Cáncer

Probabilidad de Cáncer = P(E1) × P(E2) × P(E3) × ...
(asumiendo independencia)
```

### En cellSim

```
Año 0: Genoma normal
       BRCA1 +/- (inicial), TP53 +/+  ← Estado inicial

Año 5: BRCA1 muta hacia -/-
       BRCA1 -/-, TP53 +/+
       ─────────────────────────────────
       → APOPTOSIS (MUERTE CELULAR) ❌
       ─────────────────────────────────
       (La célula muere, no progresa a neoplasia)

ALTERNATIVA (Ruta válida):
Año 0: BRCA1 +/-, TP53 +/+

Año 5: TP53 muta a +/-
       BRCA1 +/-, TP53 +/-  ← Primer hit (inestabilidad)
       Cascada de inestabilidad comienza

Año 12: TP53 muta a -/-
        BRCA1 +/-, TP53 -/-  ← Segundo hit
        Inestabilidad aún más alta

Año 18: Evento final estocástico
        neoplasm_k se dispara → Transformación
        
        O: BRCA1 también muta a -/- → APOPTOSIS
```

### Matemática

```
Probabilidad de al menos una neoplasia en N años:

P(Neoplasia ≥ 1) = 1 - (1 - neoplasm_k)^N

Con neoplasm_k = 0.002:
- En 10 años: ~2%
- En 100 años: ~18%
- En 500 años: ~63%

Con TP53 dañado (inestabilidad):
- La cascada acelera futuras mutaciones
- Probabilidad se amplifica
```

---

## Muestreo Estocástico de Neoplasia

### Mecanismo

En cada tick, si TP53 **no protege**, se muestrea:

```cpp
void AgenticCell::develop_neoplasm() {
    // Generar número aleatorio entre 0 y 1
    double u01 = noise_->next().u01;
    
    // Comparar con threshold
    if (u01 < neoplasm_k_) {
        // Transformación ocurre
        is_neoplastic_ = true;
        
        // Emitir señal
        emitNeoplasmSignal();
    }
}
```

### Parámetro: `neoplasm_k`

```
neoplasm_k = probabilidad de transformación por año

Valores típicos:
├─ 0.001 (0.1%):  Muy raro, simulaciones muy largas
├─ 0.002 (0.2%):  Estándar cellSim
├─ 0.01 (1%):     Bastante probable
└─ 0.05 (5%):     Muy probable
```

### Ejemplo Concreto

```
Célula A: neoplasm_k = 0.002
  Año 1: random = 0.0031 > 0.002 → Sin cambio
  Año 2: random = 0.0015 < 0.002 → NEOPLASIA ✓

Célula B: neoplasm_k = 0.002
  Año 1: random = 0.0003 < 0.002 → NEOPLASIA ✓ (menos suerte!)
```

---

## Protección vs. Aceptación

### Escenario 1: Célula Protegida (TP53 +/+)

```cpp
// TP53 está sano
if (isNeoplasticProtected()) {  // ← true
    // NO se ejecuta develop_neoplasm()
    if (verbose_) {
        std::cout << "Cell " << id_ << " is TP53 protected" << std::endl;
    }
    return;  // Saltar evaluación de neoplasia
}
```

**Resultado**: Célula **nunca** se transforma en neoplasia

**Probabilidad de transformación**: 0%

### Escenario 2: Célula Vulnerable (TP53 +/- o -/-)

```cpp
// TP53 está dañado
if (!isNeoplasticProtected()) {  // ← true (dañado)
    // Ejecutar desarrollo de neoplasia
    develop_neoplasm();
    
    if (is_neoplastic_) {
        std::cout << "Cell " << id_ << " became NEOPLASTIC!" << std::endl;
    }
}
```

**Resultado**: Célula puede transformarse (estocásticamente)

**Probabilidad de transformación**: neoplasm_k (típicamente 0.002 por año)

### Tabla Comparativa

| Propiedad | Protegida | Vulnerable |
|-----------|-----------|-----------|
| Estado TP53 | +/+ | +/- o -/- |
| ¿Muestreo neoplasia? | ❌ NO | ✅ SÍ |
| Prob. transformación | 0% | neoplasm_k |
| Resultado final | Sana (siempre) | Puede ser neoplasia |

---

## Señales de Neoplasia

### Tipo: `NeoplasmSignal`

```cpp
class NeoplasmSignal : public ISignal {
public:
    Type type() const noexcept override {
        return Type::Neoplasm;
    }
    
    uint64_t sourceId() const noexcept override {
        return source_cell_id_;
    }
    
    const std::string& message() const noexcept override {
        return message_;
    }
    
    const std::vector<uint64_t>& targetIds() const noexcept override {
        return target_ids_;  // Vacío = broadcast
    }
};
```

### Emisión

```cpp
// En AgenticCell::develop_neoplasm()
if (u01 < neoplasm_k_) {
    is_neoplastic_ = true;
    
    // Crear y emitir señal
    auto signal = std::make_unique<NeoplasmSignal>(
        id_,  // Identificador de la célula
        "Cell " + std::to_string(id_) + " became neoplastic"
    );
    
    signal_emitter_(std::move(signal));
}
```

### Recepción

```cpp
// Otras células reciben la señal
void AgenticCell::receiveMessage(std::unique_ptr<ISignal> signal) {
    if (signal->type() == ISignal::Type::Neoplasm) {
        // Procesar: podría incluir reacciones inmunológicas
        // En versión simple: solo log
        incoming_messages_.push(std::move(signal));
    }
}
```

---

## Rastreo de Neoplasmas

### Rastreo Global

El tejido mantiene un registro:

```cpp
std::set<std::uint64_t> identified_neoplasms_;

// Cuando una célula se transforma
if (cell->isNeoplastic()) {
    identified_neoplasms_.insert(cell->id());
}

// Acceder
auto neoplasms = tissue.getIdentifiedNeoplasms();
std::cout << "Total neoplasms: " << neoplasms.size() << std::endl;
```

### Listener

```cpp
tissue.setNeoplasmListener([](uint64_t cell_id, const std::string& msg) {
    std::cout << "ALERT: Cell " << cell_id << " " << msg << std::endl;
});

// Se dispara cuando una célula se transforma
// ALERT: Cell 42 Cell became neoplastic
```

---

## Cascada de Transformación

### Secuencia Típica

```
Año 0: BRCA1[+/+] TP53[+/+]  ← Genoma sano

Año 5: BRCA1[+/-] TP53[+/+]  ← Primera mutación
       (reparación débil)

Año 8: BRCA1[+/-] TP53[+/-]  ← Segunda mutación
       (TP53 dañado!)
       
       *** PUNTO DE NO RETORNO ***
       Cascada de inestabilidad activada
       threshold = threshold_base + instability_k

Año 9: BRCA1[-/-] TP53[+/-]  ← Cascada: BRCA1 muta rápidamente

Año 10: BRCA1[-/-] TP53[-/-]  ← Cascada: TP53 se daña completamente
        (genoma ahora super-inestable)

Año 11-15: Múltiples mutaciones más (en genes ficticios adicionales)
           Acumulación sin control

Año 18: EVENTO FINAL ESTOCÁSTICO
        random(0,1) < neoplasm_k
        ✓ SÍ: Transformación a NEOPLASIA
        Cell.isNeoplastic() = true
        Emitir NeoplasmSignal
        Tejido registra en identified_neoplasms_
```

---

## Visualización: Árbol de Progresión

```
┌──────────────────────────────────────────────────────┐
│           PROGRESIÓN A NEOPLASIA                     │
└──────────────────────────────────────────────────────┘

Célula Normal
    │
    ├─► Mutación BRCA1 (5-10% por año)
    │   │
    │   └─► BRCA1 +/- (reparación débil)
    │       │
    │       ├─► Mutaciones adicionales aceleradas
    │       │   (ya sin reparación)
    │       │
    │       └─► Mutación TP53
    │           │
    │           └─► TP53 +/- (PUNTO CRÍTICO)
    │               │
    │               │ *** INESTABILIDAD ACTIVA ***
    │               │ Cascada de mutaciones
    │               │ threshold_eff += instability_k
    │               │
    │               └─► Múltiples mutaciones simultáneas
    │                   │
    │                   ├─► BRCA1 muta a -/-
    │                   ├─► TP53 muta a -/-
    │                   └─► Otros genes mutan
    │                       │
    │                       └─► Genoma altamente caótico
    │                           │
    │                           ├─► Muestreo estocástico
    │                           │   random < neoplasm_k?
    │                           │
    │                           ├─ SÍ (desafortunado)
    │                           │  └─► TRANSFORMACIÓN
    │                           │      ✓ isNeoplastic = true
    │                           │      ✓ Emitir signal
    │                           │      ✓ Registrar en tejido
    │                           │
    │                           └─ NO (afortunado)
    │                              └─► Sigue siendo célula
    │                                  normal (pero caótica)
    │
    ▼
Neoplasia / Célula Cancerosa
```

---

## Código: Simulación de Progresión

```cpp
#include "domain/tissue/Tissue.h"
#include "domain/cell/AgenticCell.h"

// Crear tejido
Tissue tissue(true);

// Crear célula
auto genome = Genome::makeDefaultGenome();
auto noise = std::make_unique<RandomNoise>(42);
auto cell = std::make_unique<AgenticCell>(
    std::move(noise),
    genome,
    0.002  // neoplasm_k
);

tissue.addCell(std::move(cell));

// Registrar transformaciones
tissue.setNeoplasmListener([](uint64_t id, const std::string& msg) {
    std::cout << ">>> NEOPLASM DETECTED: Cell " << id << " <<<" << std::endl;
});

// Simular
for (int year = 0; year < 100; ++year) {
    tissue.live();
    
    // Consultar estado
    auto neoplasms = tissue.getIdentifiedNeoplasms();
    if (!neoplasms.empty()) {
        std::cout << "Year " << year << ": "
                  << neoplasms.size() << " neoplasm(s)" << std::endl;
    }
}

// Estadísticas finales
std::cout << "\nFinal Statistics:\n";
std::cout << "Total neoplasms: " 
          << tissue.getIdentifiedNeoplasms().size() << std::endl;
```

---

## Factores que Aceleran Transformación

### 1. Mutación Temprana de TP53

```
Sin TP53 dañado:
├─ Neoplasia rara
└─ Protección funciona

Con TP53 dañado tempranamente:
├─ Cascada de inestabilidad
├─ Múltiples mutaciones rápidas
└─ Mayor probabilidad de transformación
```

### 2. Inestabilidad Genómica Multiplicativa

```
factor genomic_instability amplifica:
├─ threshold_eff = threshold × factor + instability_k
└─ Resultado: mutaciones más frecuentes
```

### 3. Mayor neoplasm_k

```
neoplasm_k bajo (0.001):
├─ Transformación muy rara
├─ Simulaciones largas necesarias

neoplasm_k alto (0.05):
├─ Transformación muy probable
├─ Simulaciones pueden terminar rápido
```

---

## Recapitulación

| Concepto | Explicación |
|----------|-------------|
| **Neoplasia** | Transformación a célula cancerosa |
| **BRCA1** | Gen de reparación del ADN |
| **TP53** | Gen protector maestro |
| **Multihit** | Múltiples mutaciones necesarias |
| **Muestreo** | Evento estocástico final |
| **Protección** | TP53 +/+ bloquea transformación |
| **Señal** | NeoplasmSignal emitida |
| **Rastreo** | Tejido registra transformaciones |

---

## Diagrama: Estados Celulares

```
NORMAL                  PROTEGIDA               VULNERABLE
(Sana)                  (TP53 +/+)              (TP53 +/- o -/-)
│                       │                       │
├─ TP53 +/+             ├─ Nunca muestrea      ├─ Muestrea cada año
├─ No transforma        ├─ Nunca neoplasia     ├─ random < neoplasm_k?
└─ Vive indefinido       └─ Vive indefinido     │
                                                ├─ SÍ: NEOPLASIA
                                                └─ NO: Sigue normal
```

---

## Siguiente Paso

👉 **Lee [Interacciones Celulares](./interactions.md)** para entender cómo los eventos de neoplasia son comunicados entre células mediante señales.

---

## Referencias Cruzadas

- 🔗 [Genes](./genes.md) - Base de mutaciones
- 🔗 [Genomas](./genome.md) - Cómo se amplifican mutaciones
- 🔗 [Células](./cells.md) - Dónde ocurre transformación
- 🔗 [Tejidos](./tissue.md) - Rastreo de neoplasias
- 🔗 [Interacciones](./interactions.md) - Comunicación de eventos

