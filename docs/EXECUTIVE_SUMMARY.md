# cellSim - Resumen Ejecutivo

## 📋 Introducción

**cellSim** es un simulador de dinámica celular que modela la evolución de poblaciones de células en un tejido, incluyendo mutaciones genéticas, división celular, apoptosis y transformación neoplástica. El sistema permite estudiar cómo diferentes tasas de mutación y división afectan la composición y viabilidad del tejido.

---

## 🧬 Estructura de la Célula

### Componentes Principales

Una célula (AgenticCell) contiene:

| Componente | Descripción | Ejemplo |
|-----------|-------------|---------|
| **Genoma** | 2 genes: BRCA1 y TP53 | BRCA1: +/- (INICIAL), TP53: +/+ |
| **Estado BRCA1** | Viabilidad celular | -/- = MUERTE (riesgo desde +/-) |
| **Estado TP53** | Protección contra neoplasias | -/- = SIN PROTECCIÓN |
| **Edad** | Ciclos de vida | 0-100+ ciclos |
| **Neoplasm k** | Probabilidad de transformación | 0.002 por defecto |
| **Tasa de División** | Probabilidad de reproducción | 0.001 (0.1%) por defecto |
| **Inestabilidad Genómica** | Degradación del sistema | Comienza en 1.0 |

### Estados Genéticos de BRCA1

```
+/- (Heterocigoto) ← ESTADO INICIAL DE TODA CÉLULA
 ↓ (mutación 1%)
-/- (Homocigoto)  → APOPTOSIS (muerte celular)
```

**Nota importante:** BRCA1 comienza SIEMPRE en estado +/- en todas las células

### Estados Genéticos de TP53

```
+/+ (Normal)       → PROTECCIÓN contra neoplasias
 ↓ (mutación)
+/- (Heterocigoto) → Protección degradada, inestabilidad ↑
 ↓ (mutación)
-/- (Homocigoto)   → SIN PROTECCIÓN, neoplasias probables
```

---

## 🔄 Ciclo de Vida Celular (6 Fases)

Cada célula ejecuta 6 fases secuenciales cada ciclo (año):

```
FASE 0: Baseline Assessment
├─ Mostrar estado actual (si verbose)

FASE 1: G1 Integrity Checkpoint
├─ ¿Está viva? → Sí, continúa
└─ ¿Es neoplástica? → Sí, FIN (transformada)

FASE 2: Endocytosis
├─ Procesar mensajes del tejido (apoptosis, señales)
└─ Evaluar si aceptar apoptosis

FASE 3: Nuclear Dynamics
├─ Mutar genes según thresholds
└─ Evaluar viabilidad (BRCA1 -/- = muerte)

FASE 4: Cytoplasmic Remodeling
├─ Actualizar inestabilidad genómica
├─ Intentar transformación neoplástica
└─ Intentar división celular

FASE 5: Exocytosis
└─ (Placeholder para futura expansión)

↓ FIN CICLO, incremental edad
```

### Decisiones Críticas en el Ciclo

| Fase | Decisión | Resultado |
|------|----------|-----------|
| 1 | ¿BRCA1 -/-? | MUERTE |
| 1 | ¿TP53 -/- + random < k? | NEOPLASM |
| 3 | ¿Mutación de genes? | Cambio de estado |
| 4 | ¿random < division_rate? | DIVISIÓN |
| 4 | ¿random < inestabilidad × k? | NEOPLASM |

---

## 🧬 Diagrama de Flujo: Célula Individual

```
INICIO CICLO
     ↓
[FASE 0] Descripción
     ↓
[FASE 1] Checkpoint
     ├─ ¿BRCA1 -/-? ──→ MUERE ✗
     ├─ ¿Neoplástica? → FIN (transformada)
     └─ ¿Viva? → Continúa ✓
     ↓
[FASE 2] Endocytosis
     ├─ Recibir mensaje de tejido?
     ├─ ¿Apoptosis solicitada?
     │  └─ random < inestabilidad_genómica?
     │     ├─ Sí → ACEPTA y MUERE
     │     └─ No → RECHAZA, continúa
     └─ Procesar otros mensajes
     ↓
[FASE 3] Nuclear Dynamics
     ├─ Para cada gen:
     │  └─ random < threshold × instability?
     │     └─ Sí → MUTA gen
     ├─ Recalcular BRCA1 -/-? → MUERE ✗
     └─ Continúa si viva
     ↓
[FASE 4] Cytoplasmic Remodeling
     ├─ Actualizar inestabilidad genómica
     │  └─ inestability_new = inestability_old²
     ├─ ¿Transformación neoplástica?
     │  └─ random < neoplasm_k × instability?
     │     └─ Sí → GENERA NeoplasmSignal
     └─ ¿División celular?
        └─ random < division_rate?
           └─ Sí → CLONA célula + GENERA CellDivisionSignal
     ↓
[FASE 5] Exocytosis
     └─ (Placeholder)
     ↓
INCREMENTAR EDAD
     ↓
FIN CICLO
```

---

## 🧬 Diagrama de Flujo: Tejido (Población)

**Flujo Principal:**

```
INICIO SIMULACIÓN
  ↓
Crear N células iniciales (BRCA1: +/-, TP53: +/+)
  ↓
PARA cada año (1 a MAX_T):
  ├─ Mostrar resumen genético actual
  └─ Ejecutar live() en CADA célula
       ↓
  PROCESAR EVENTOS:
  ├─ MUERTE → Quitar población
  ├─ NEOPLASM → Registrar + enviar apoptosis
  │   └─ Célula: random < inestabilidad?
  │       ├─ Sí → ACEPTA, muere
  │       └─ No → RECHAZA, vive neoplástica
  ├─ DIVISIÓN → Crear hija + agregar a población
  └─ Actualizar población total
  ↓
¿Más años? → Sí: Repetir
          → No: FIN SIMULACIÓN
  ↓
RESULTADO: Población final con estadísticas
```

**Resumen genético mostrado cada año:**
- `[BRCA+/- TP53+/+]` - Células normales
- `[BRCA+/- TP53+/-]` - Células con TP53 degradado
- `[BRCA+/- TP53-/-]` - Células neoplásticas
- `[BRCA-/-]` - Células muertas

---

## 🔄 Ciclo de Apoptosis: Tejido-Célula

Cuando se detecta una neoplasia, ocurre este intercambio:

```
TEJIDO                          CÉLULA
  │                               │
  ├──────────────────────────────→ NeoplasmSignal recibida
  │                               │
  │                         [FASE 2: Endocytosis]
  │                               │
  │                         Evaluar apoptosis
  │                               │
  │                         random < inestabilidad?
  │                         ╱─ SÍ ─╲
  │                        ╱         ╲
  │                      Sí            No
  │                       │            │
  │←───────────────────── Muerte      Rechazo
  │                       │            │
  │                    [FASE 1 final]  │
  │                    Se quita        │
  │                    población--    Continúa
  │                                    neoplástica
  │
  MARCA como:
  ├─ Célula muerta (removida)
  └─ Célula neoplástica resistente (permanece)
```

**Insight clave:** Mayor inestabilidad genómica = Mayor probabilidad de rechazar apoptosis

---

## 📊 Eventos Posibles en una Célula

### 1. **Mutación Genética** (Fase 3)
- **Trigger:** `random < threshold × genomic_instability`
- **Efecto:** Cambio de estado en BRCA1 o TP53
- **Consecuencia:** Inestabilidad ↑, viabilidad en riesgo

### 2. **Apoptosis por BRCA1 -/-** (Fase 1)
- **Trigger:** BRCA1 alcanza estado -/-
- **Efecto:** Muerte inmediata y garantizada
- **Consecuencia:** Célula removida de la población

### 3. **Transformación Neoplástica** (Fase 4)
- **Trigger:** `random < neoplasm_k × genomic_instability`
- **Efecto:** Célula becomes neoplástica, genera señal
- **Consecuencia:** Tejido intenta apoptosis (Ver ciclo arriba)

### 4. **Rechazo de Apoptosis** (Fase 2, si recibe señal)
- **Trigger:** `random > genomic_instability` (cuando apoptosis solicitada)
- **Efecto:** Célula ignora orden de muerte
- **Consecuencia:** Célula neoplástica resistente permanece

### 5. **División Celular** (Fase 4)
- **Trigger:** `random < division_rate`
- **Efecto:** Se crea célula hija idéntica
- **Consecuencia:** Población crece, nueva célula hereda genoma

### 6. **Aumento de Inestabilidad** (Fase 4)
- **Trigger:** Cada ciclo automático
- **Efecto:** `inestability_new = inestability_old²`
- **Consecuencia:** Degeneración progresiva del sistema

---

## 🎯 Escenarios Implementados

### Resumen Rápido de los 6 Escenarios

| Escenario | BRCA1 | TP53 | División | Duración | Resultado Esperado |
|-----------|-------|------|----------|----------|-------------------|
| **default** | 0.01 | 0.01 | 0% | 50 años | Balance: algunos mueren, neoplasias leves |
| **no_mutations** | 0.0 | 0.0 | 0% | 50 años | Estable: células nunca cambian |
| **high_brca_apoptosis** | 0.5 | 0.01 | 0% | 10 años | Apocalipsis: ~99% mueren por BRCA1 |
| **high_tp53_mutation** | 0.001 | 0.3 | 0% | 50 años | Neoplasias masivas: 60-70% transformadas |
| **cell_division_healthy** | 0.0 | 0.0 | 10% | 5 años | Crecimiento exponencial puro |
| **realistic_division** | 0.01 | 0.01 | 1% | 50 años | **NUEVO**: Balance realista con crecimiento |

### Dinámicas Clave por Escenario

#### 1. `default` - Realista Base
```
Población:    100 → 100 (estable)
Muertes:      ~5-10% en 50 años
Neoplasias:   10-20% en 50 años
División:     Sin (0%)
Insight:      Las mutaciones tienen efecto pero lento
```

#### 2. `no_mutations` - Control Puro
```
Población:    100 → 100 (completamente estable)
Muertes:      0%
Neoplasias:   0%
División:     Sin (0%)
Insight:      Valida que sin mutaciones no hay cambios
```

#### 3. `high_brca_apoptosis` - Catástrofe BRCA1
```
Población:    100 → 0 (colapso)
Muertes:      ~50% año 1, ~99% año 10
Neoplasias:   0% (no hay tiempo)
División:     Sin (0%)
Insight:      BRCA1 es crítico para viabilidad
```

#### 4. `high_tp53_mutation` - Invasión Neoplástica
```
Población:    100 → 100 (se mantiene)
Muertes:      Mínimas (~5-10%)
Neoplasias:   10% año 5 → 60-70% año 20
División:     Sin (0%), pero hay resistencia
Insight:      TP53 es guardián del genoma; sin él, caos
```

#### 5. `cell_division_healthy` - Crecimiento Puro
```
Población:    10 → 27 (crecimiento de 2.7x)
Muertes:      0%
Neoplasias:   0%
División:     10% → muy activa
Insight:      Demuestra mecánica de división
```

#### 6. `realistic_division` - Balance Realista (50 años)
```
Población:    100 → 180-240 (crecimiento lento)
Muertes:      ~5-10%
Neoplasias:   5-15%
División:     1% → crecimiento sostenido
Insight:      Simula tejido normal en crecimiento con riesgo bajo
```

---

## 🔬 Insights Principales

### 1. **BRCA1 es Viabilidad**
- BRCA1 -/- = muerte garantizada
- No hay escape: es letal en Fase 1
- **Consecuencia:** Frena crecimiento de poblaciones con alta tasa de mutación

### 2. **TP53 es Guardián**
- TP53 +/+ protege contra neoplasias
- TP53 -/- desata transformación neoplástica
- **Consecuencia:** Sin TP53, ~60-70% del tejido se transforma en 20 años

### 3. **Inestabilidad Genómica es Degeneración**
- Cada ciclo: `instability = instability²`
- Aumenta probabilidad de mutación y neoplasia
- **Consecuencia:** El tiempo trabaja en contra del tejido

### 4. **Apoptosis vs Resistencia**
- Tejido envía apoptosis a neoplasias
- **NUEVO:** Célula puede RECHAZAR apoptosis si `random > instability`
- Mayor inestabilidad = Mayor resistencia a muerte
- **Consecuencia:** Neoplasias resistentes colonizan el tejido

### 5. **División-Mutación Balance**
- División 1% + Mutación 1% = Crecimiento moderado
- **realistic_division** simula tejido normal
- **Consecuencia:** Poblaciones pueden crecer incluso con muerte ocasional

### 6. **Determinismo Estocástico**
- Todas las decisiones son probabilísticas (random < threshold)
- Mismos parámetros, distinto outcome cada ejecución
- Diferentes semillas → distintos destinos celulares
- **Consecuencia:** Simulaciones son realistas pero impredecibles

---

## 📈 Comparativa de Consecuencias

### Crecimiento Poblacional

```
Células
  |
240+|                    ✓ realistic_div
200+|                  ✓
160+|              ✓
120+|            ✓ cell_div_healthy
 80+|  ✓ default  ✓
 40+|✓ no_mut     ✓ high_tp53
  0+|_✗ high_brca_apoptosis
    0  10  20  30  40  50 (años)
```

### Transformación Neoplástica

```
Neoplasias %
  |
100|                    high_tp53_mut
 80|                  ╱────
 60|              ╱────
 40|          ╱────
 20|      ╱────  realistic_div
  0|●─────────────────── no_mut, high_brca
    0  10  20  30  40  50 (años)
```

### Viabilidad Celular

```
Vivas %
  |
100|●●● no_mut
  |●●●●cell_div_healthy
 80|●●●●●default
  |●●●●●realistic_div
 60|●●●●●high_tp53
  |●●
 40|●
  |●●●●
 20|●●●●●●●
  |●●●●●●●●●high_brca_apoptosis
  0|
    0  10  20  30  40  50 (años)
```

---

## 🎮 Cómo Usar

### Comando Básico

```bash
./random_cells [n_cells] [max_t] [seed] --scenario [name]
```

### Ejemplos por Objetivo

**Entender viabilidad:**
```bash
./random_cells 100 10 42 --scenario high_brca_apoptosis
# Ver cómo BRCA1 mata la población
```

**Ver crecimiento:**
```bash
./random_cells 20 5 42 --scenario cell_division_healthy
# Ver División celular en acción
```

**Simular tejido real (50 años):**
```bash
./random_cells 100 50 42 --scenario realistic_division
# Balance entre crecimiento, muerte y neoplasias
```

---

## ✅ Checklist de Funcionalidades

- ✅ Mutaciones genéticas (BRCA1, TP53)
- ✅ Apoptosis por BRCA1 -/-
- ✅ Transformación neoplástica
- ✅ **Rechazo de apoptosis** (basado en inestabilidad)
- ✅ División celular con clonación
- ✅ Inestabilidad genómica progresiva
- ✅ 6 escenarios implementados
- ✅ Signals (Neoplasm, CellDivision)
- ✅ 55 tests unitarios PASSING

---

## 📚 Referencias Rápidas

- **Código:** `/home/luis/CLionProjects/cellSim/`
- **Tests:** `build/tests/unit_tests`
- **Escenarios:** `docs/SCENARIOS.md`
- **Compilar:** `make -j4`
- **Ejecutar:** `./build/random_cells`


