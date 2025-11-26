void Gene::mutate() {
    switch (state_) {
        case State::PlusPlus:
            state_ = State::PlusMinus;
            break;
        case State::PlusMinus:
            state_ = State::MinusMinus;
            break;
        case State::MinusMinus:
            // Absorbe el estado -/-
            break;
    }
}
```

---

## Recapitulación

| Concepto | Explicación |
|----------|-------------|
| **Gen** | Unidad de herencia con 3 estados posibles |
| **Estados** | +/+ (sano), +/- (parcial), -/- (dañado) |
| **Mutación** | Transición irreversible a estado más dañado |
| **Threshold** | Probabilidad base de mutación |
| **Inestabilidad** | Factor que aumenta mutabilidad |
| **TP53** | Regulador maestro de estabilidad |
| **BRCA1** | Gen de reparación del ADN |

---

## Siguiente Paso

👉 **Lee [Genomas](./genome.md)** para entender cómo estos genes se organizan en una colección coherente dentro de una célula.

---

## Referencias Cruzadas

- 🔗 [Genomas](./genome.md) - Cómo se agrupan genes
- 🔗 [Células](./cells.md) - Cómo usan genomas
- 🔗 [Neoplasias](./neoplasm.md) - Cómo mutaciones llevan a cáncer
# 🧪 Genes y Mutaciones en cellSim

## Introducción

Los **genes** son las unidades fundamentales de herencia en cellSim. Cada gen es una entidad discreta que puede cambiar de estado a través de **mutaciones**, afectando el comportamiento y la viabilidad de la célula.

---

## ¿Qué es un Gen?

En cellSim, un gen es representado por la clase `Gene` y contiene:

- **Nombre**: identificador único (ej. "TP53", "BRCA1")
- **Estado**: uno de tres valores posibles
- **Threshold de mutación**: probabilidad de cambio
- **Factor de inestabilidad**: modificador de mutabilidad

### Definición en Código

```cpp
class Gene {
public:
    enum class State { PlusPlus, PlusMinus, MinusMinus };
    
    explicit Gene(std::string name, 
                  State initial = State::PlusPlus, 
                  double mutation_threshold = 0.1, 
                  double mutation_instability_k = 0.0, 
                  bool verbose = false);
    
    const std::string& name() const;
    void mutate();
    void live(bool apply_instability);
    bool enabled() const;
};
```

---

## Estados Genéticos

Cada gen puede estar en uno de tres estados:

### 1. **+/+** (Homocigoto Dominante)
- **Significado biológico**: Ambas copias del gen son funcionales
- **Símbolo**: ✅ Sano
- **Comportamiento**: Máxima protección/funcionalidad
- **Ejemplo**: TP53 +/+ protege de neoplasia

### 2. **+/-** (Heterocigoto)
- **Significado biológico**: Una copia funcional, otra defectuosa
- **Símbolo**: ⚠️ Parcial
- **Comportamiento**: Funcionalidad reducida
- **Ejemplo**: TP53 +/- riesgo intermedio

### 3. **-/-** (Homocigoto Recesivo)
- **Significado biológico**: Ambas copias no funcionales
- **Símbolo**: ❌ No funcional
- **Comportamiento**: Pérdida total de funcionalidad
- **Ejemplo**: TP53 -/- sin protección

---

## Máquina de Estados: Transiciones de Mutación

Los genes mutagénico siguen un modelo determinista de transiciones:

```
       Mutación
         ────────────────►
     
     +/+ ────────► +/- ────────► -/-
      
     ✅         ⚠️            ❌
     
     Una mutación avanza el estado
     (no puede retroceder naturalmente)
```

### Tabla de Transiciones

| Estado Actual | Mutación Ocurre | Nuevo Estado |
|---------------|-----------------|--------------|
| +/+ | Sí | +/- |
| +/- | Sí | -/- |
| -/- | Sí | -/- (sin cambio) |

**Nota**: Una vez que un gen llega a **-/-**, adicionales mutaciones no lo cambian (es un estado absorbente).

---

## Umbrales de Mutación

### Concepto: `mutation_threshold`

El **threshold de mutación** es la probabilidad base de que un gen mute en cada tick (año de simulación).

```cpp
double mutation_threshold = 0.1;  // 10% de probabilidad por año
```

### Fórmula de Mutación

```
if (random() < mutation_threshold):
    gen.mutate()
```

### Ejemplo

- TP53 con threshold = 0.15 (15%)
- Cada año: 15% de probabilidad de mutación
- En 100 años: ~~78% de probabilidad de al menos una mutación

---

## Inestabilidad Genómica: `mutation_instability_k`

### Concepto

El **factor de inestabilidad** (`mutation_instability_k`) es un **modificador** que aumenta la mutabilidad cuando el genoma está inestable.

```cpp
double mutation_instability_k = 0.1;  // 10% de aumento
```

### Cuándo se Aplica

Se aplica **solo cuando `Genome::isUnstable()` es verdadero**, es decir, cuando **TP53 no está en estado +/+**.

### Fórmula Modificada

```
threshold_efectivo = mutation_threshold + mutation_instability_k
                     (cuando el genoma está inestable)

threshold_efectivo = mutation_threshold
                     (cuando el genoma es estable)
```

### Ejemplo Concreto

```
BRCA1:
  - mutation_threshold = 0.10
  - mutation_instability_k = 0.05

Genoma estable (TP53 +/+):
  threshold_efectivo = 0.10

Genoma inestable (TP53 +/-):
  threshold_efectivo = 0.10 + 0.05 = 0.15

Genoma muy inestable (TP53 -/-):
  threshold_effectivo = 0.10 + 0.05 = 0.15
```

---

## Genes Clave: TP53 y BRCA1

### TP53: El "Guardián del Genoma"

**Biología**: Gen supresor de tumores que:
- Detecta daño genético
- Detiene el ciclo celular
- Induce apoptosis (muerte celular)
- Previene neoplasias

**En cellSim**:
- Protege de neoplasia cuando está en estado +/+
- Su mutación desencadena cascadas de cambios
- Es el regulador maestro de estabilidad genómica

**Estados críticos**:
- `TP53 +/+` → Genoma estable, sin inestabilidad añadida
- `TP53 +/-` → Genoma inestable (applies instability)
- `TP53 -/-` → Genoma muy inestable

### BRCA1: Gen de Reparación del ADN

**Biología**: Gen de reparación de ADN que:
- Repara roturas de doble hebra
- Está asociado con cáncer de mama hereditario
- Mutaciones heredadas aumentan riesgo de cáncer significativamente

**En cellSim**:
- **Estado inicial**: +/- (heterocigoto, células de mama luminal)
- **Si muta a -/-**: Apoptosis (muerte celular) - NO hay viabilidad sin reparación
- Implementación: `alive()` requiere que BRCA1 esté `enabled()` (+/+)
- Su rol es CRÍTICO: pérdida total (homocigoto) = muerte celular

---

## Ciclo de Vida de una Mutación

### Fase 1: Evaluación
```
¿Es hora de evaluar mutación?
(cada tick / año)
```

### Fase 2: Muestreo Estocástico
```
Generar número aleatorio (0-1)
Comparar con threshold_efectivo
```

### Fase 3: Aplicación
```
if (random < threshold):
    estado = siguiente_estado()
```

### Fase 4: Registro (si verbose)
```
Registrar mutación en trazas
Incluir estado anterior y nuevo
```

---

## Visualización: Máquina de Estados Detallada

```
┌──────────────────────────────────────────────────────────────┐
│                    CICLO DE VIDA DE UN GEN                   │
└──────────────────────────────────────────────────────────────┘

1. INICIO (cada tick)
   │
   ├─► ¿gene.live(apply_instability)?
   │
   ├─► Calcular threshold efectivo
   │   │
   │   ├─ if (apply_instability):
   │   │    threshold_eff = threshold + instability_k
   │   │
   │   └─ else:
   │      threshold_eff = threshold
   │
   ├─► Muestrear random()
   │
   ├─► Evaluar: random < threshold_eff?
   │   │
   │   ├─ SÍ ──► MUTACIÓN
   │   │         gen.mutate()
   │   │         ├─ +/+ ──► +/-
   │   │         ├─ +/- ──► -/-
   │   │         └─ -/- ──► -/- (sin cambio)
   │   │
   │   └─ NO ──► SIN CAMBIO
   │
   └─► FIN

2. RESULTADO
   │
   ├─► Gen actualizado a nuevo estado
   ├─► Trazas registradas (si verbose)
   └─► Listo para siguiente tick
```

---

## Ejemplo Práctico: Evolución de TP53

Simulemos la evolución de un gen TP53 en 10 años:

```
Configuración:
  - TP53: mutation_threshold = 0.15
  - TP53: mutation_instability_k = 0.10
  - Genoma: inicialmente estable (sin otras mutaciones)

Años 1-3: TP53 +/+, threshold = 0.15
  Año 1: random = 0.23 > 0.15 → Sin cambio
  Año 2: random = 0.08 < 0.15 → MUTACIÓN a +/-
  Año 3: random = 0.12 < 0.15 → Sigue +/-

Años 4-10: TP53 +/-, threshold = 0.25 (ahora inestable!)
           (0.15 base + 0.10 inestabilidad)
  Año 4: random = 0.18 < 0.25 → MUTACIÓN a -/-
  Año 5: random = 0.31 > 0.25 → Sin cambio
  ...
  Año 10: TP53 -/-, genoma altamente inestable
```

**Observación**: Una vez que TP53 muta a +/-, la inestabilidad se activa, acelerando futuras mutaciones.

---

## Código: Ciclo de Vida de un Gen

```cpp
// En Gene::live(bool apply_instability)
void Gene::live(bool apply_instability) {
    // 1. Calcular threshold efectivo
    double threshold = get_mutation_threshold(apply_instability);
    
    // 2. Muestrear ruido
    double u01 = noise_->next().u01;
    
    // 3. Evaluar mutación
    if (u01 < threshold) {
        mutate();
        if (verbose_) {
            std::cout << name_ << " mutated to " 
                      << status() << std::endl;
        }
    }
}

// Transición de estado
# 📚 Documentación de cellSim

Bienvenido a la documentación completa de **cellSim**, un simulador avanzado de evolución celular y progresión de neoplasias.

## 🧬 Conceptos Biológicos Fundamentales

Esta sección explora en profundidad los componentes biológicos del simulador y cómo interactúan entre sí.

### Comienza tu viaje aquí:

1. **[Genes y Mutaciones](./biological-concepts/genes.md)** 🧪
   - Entiende cómo funcionan los genes en cellSim
   - Estados genéticos y transiciones de mutación
   - Genes clave: TP53 y BRCA1

2. **[Genomas](./biological-concepts/genome.md)** 🔬
   - La colección completa de genes de una célula
   - Estabilidad genómica y detección de instabilidad
   - Clonación y ciclo de vida

3. **[Células](./biological-concepts/cells.md)** 🟢
   - AgenticCell: el modelo de célula
   - Ciclo de vida y evaluación de viabilidad
   - Progresión hacia neoplasia

4. **[Tejidos](./biological-concepts/tissue.md)** 🧠
   - Colecciones de células interactuando
   - Gestión de identidades y rastreo
   - Fases de ejecución

5. **[Neoplasias](./biological-concepts/neoplasm.md)** ⚠️
   - Progresión hacia cáncer
   - Hipótesis del multihit
   - Modelos estocásticos

6. **[Interacciones Celulares](./biological-concepts/interactions.md)** 📡
   - Comunicación mediante señales
   - Broadcasts y mensajes dirigidos
   - Rastreo de eventos

---

## 🗺️ Mapa Conceptual

```
┌─────────────────────────────────────────────────────────┐
│                   SIMULADOR cellSim                      │
└────────────┬────────────────────────────────┬────────────┘
             │                                │
        ┌────▼────┐                    ┌─────▼─────┐
        │  GENES  │                    │  GENOMAS  │
        │ TP53,   │                    │ Colección │
        │ BRCA1   │                    │ de Genes  │
        └────┬────┘                    └─────┬─────┘
             │                               │
             └───────────┬───────────────────┘
                         │
                    ┌────▼────────┐
                    │   CÉLULAS   │
                    │ AgenticCell │
                    └────┬────────┘
                         │
                    ┌────▼────────┐
                    │  TEJIDOS    │
                    │ Colecciones │
                    └────┬────────┘
                         │
        ┌────────────┬────▼────────┬──────────────┐
        │            │             │              │
    ┌───▼──┐   ┌────▼────┐  ┌────▼──┐    ┌────▼────┐
    │SEÑALES│   │NEOPLASIA│  │MUTACIÓN│   │MUERTE   │
    └───────┘   └─────────┘  └────────┘   └─────────┘
```

---

## 🚀 Inicio Rápido

Para ver el proyecto en acción:

```bash
git clone https://github.com/lpalomerol/cellSim.git
cd cellSim
cmake -B build
cmake --build build
./build/cellSim
```

---

## 📖 Estructura de Documentación

```
docs/
├── README.md (estás aquí)
└── biological-concepts/
    ├── README.md          (índice completo)
    ├── genes.md           (§1)
    ├── genome.md          (§2)
    ├── cells.md           (§3)
    ├── tissue.md          (§4)
    ├── neoplasm.md        (§5)
    └── interactions.md    (§6)
```

---

## 🔗 Referencias Cruzadas

Cada página incluye enlaces a conceptos relacionados para facilitar la navegación.

---

## ℹ️ Nota sobre el Proyecto

- **Lenguaje**: C++17
- **Framework de tests**: GoogleTest
- **Build system**: CMake
- **Patrón de arquitectura**: Domain-Driven Design (DDD)

---

**¡Comienza leyendo [Genes y Mutaciones](./biological-concepts/genes.md)!** 🧬

