# 🧬 Conceptos Biológicos - cellSim

Índice completo de la documentación de conceptos biológicos en cellSim.

## 📚 Secciones Principales

### 1️⃣ [Genes y Mutaciones](./genes.md)
- Definición de genes en cellSim
- Estados genéticos: +/+, +/-, -/-
- Máquina de estados de mutación
- Thresholds y umbrales de mutación
- Inestabilidad genómica
- Genes específicos: TP53 y BRCA1

**Conceptos clave**: `Gene`, `State`, `mutation_threshold`, `mutation_instability_k`

---

### 2️⃣ [Genomas](./genome.md)
- Composición de un genoma
- Relación con los genes
- Ciclo de vida del genoma
- Clonación genómica
- Detección de instabilidad
- TP53 como regulador maestro
- Factores multiplicativos de inestabilidad

**Conceptos clave**: `Genome`, `liveAllGenes()`, `isUnstable()`, `genomic_instability`

---

### 3️⃣ [Células](./cells.md)
- ¿Qué es AgenticCell?
- Identificación y edad de la célula
- Estados: viva, muerta, neoplástica
- Fases del ciclo de vida
- Protección por TP53
- Ruido estocástico
- Evaluación de viabilidad

**Conceptos clave**: `AgenticCell`, `live()`, `isNeoplastic()`, `neoplasm_k`

---

### 4️⃣ [Tejidos](./tissue.md)
- Definición de tejido
- Colección de células
- Gestión de identidades
- Fases de ejecución
- Integración de señales
- Rastreo genético
- Thread-safety

**Conceptos clave**: `Tissue`, `addCell()`, `live()`, `GeneticTrackingData`

---

### 5️⃣ [Neoplasias](./neoplasm.md)
- Progresión hacia cáncer
- Modelo biológico: mutaciones → cáncer
- Eventos carcinogénicos
- Hipótesis del multihit
- Muestreo estocástico
- Protección vs. aceptación
- Señales de neoplasia

**Conceptos clave**: `isNeoplastic()`, `neoplasm_k`, `NeoplasmSignal`, `NeoplasticException`

---

### 6️⃣ [Interacciones Celulares](./interactions.md)
- Comunicación mediante señales
- Tipos de señales
- Broadcasts vs. mensajes dirigidos
- Cola de mensajes
- Emisores y listeners
- Rastreo de eventos
- Integridad de comunicación

**Conceptos clave**: `ISignal`, `setSignalEmitter()`, `receiveMessage()`, `targetIds()`

---

## 🗺️ Flujo de Lectura Recomendado

```
Inicio
  │
  ▼
1. Genes (bloques básicos)
  │
  ▼
2. Genomas (colecciones)
  │
  ▼
3. Células (entidades vivas)
  │
  ▼
4. Tejidos (sistemas)
  │
  ├─► 5. Neoplasias (patología)
  │
  └─► 6. Interacciones (comunicación)
```

---

## 🔍 Búsqueda Rápida por Concepto

| Concepto | Página |
|----------|--------|
| Mutación | [Genes](./genes.md) |
| Inestabilidad | [Genomas](./genome.md) |
| Ciclo de vida | [Células](./cells.md) |
| Colecciones | [Tejidos](./tissue.md) |
| Cáncer | [Neoplasias](./neoplasm.md) |
| Señales | [Interacciones](./interactions.md) |
| TP53 | [Genes](./genes.md), [Genomas](./genome.md), [Células](./cells.md) |
| BRCA1 | [Genes](./genes.md), [Neoplasias](./neoplasm.md) |

---

## 💡 Consejos para la Lectura

- 📖 **Si eres nuevo**: comienza con la sección 1 (Genes)
- 🔧 **Si eres desarrollador**: salta a [Células](./cells.md)
- 🏥 **Si te interesa la biología**: enfócate en [Neoplasias](./neoplasm.md)
- 📡 **Si quieres entender comunicación**: lee [Interacciones](./interactions.md)

---

## 🎯 Objetivos de Aprendizaje

Al terminar esta documentación, comprenderás:

✅ Cómo funcionan los genes y sus mutaciones  
✅ La estructura y dinámica de los genomas  
✅ El ciclo de vida completo de una célula  
✅ Cómo se organizan las células en tejidos  
✅ La progresión biológica hacia neoplasias  
✅ Cómo se comunican las células mediante señales  

---

**¡Vamos a comenzar! Abre [Genes y Mutaciones](./genes.md)** 🚀

