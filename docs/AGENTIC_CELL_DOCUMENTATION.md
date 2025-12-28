# 🧬 Agentic Cell: Documentación Completa del Proceso de Evolución Celular

**Versión:** 2.0  
**Fecha:** Diciembre 2025  
**Estado:** ✅ Documentado  
**Audiencia:** Biólogos, Matemáticos, Bioinformáticos

---

## 📋 TL;DR - Resumen Ejecutivo

> **Para lectores con poco tiempo: Esta es la esencia del modelo en 2 minutos.**

### ¿Qué es Agentic Cell?

Un modelo de simulación donde las células son **agentes autónomos** que evolucionan desde un estado sano hasta potencialmente convertirse en tumorales. Cada célula tiene:

- **4 atributos internos**: BRCA1, TP53, D1, D2
- **6 estados posibles**: DEAD, BASELINE, UNSTABLE, UNPROTECTED, PRIMER, TUMORAL

### La Historia en 30 Segundos

1. **Célula sana** (BASELINE): BRCA1 +/-, TP53 +/+ → Protegida, estable
2. **Primera mutación** (UNSTABLE): TP53 +/- → Acumula mutaciones, D1/D2 crecen lentamente
3. **Sin protección** (UNPROTECTED): TP53 -/- → Vulnerable, D1/D2 crecen rápidamente
4. **Pre-tumoral** (PRIMER): D1 > umbral → Detectable, el tejido intenta eliminarla
5. **Tumoral** (TUMORAL): Si D2 alto → Evade apoptosis, se vuelve neoplásica

### Variables Clave

| Variable | Significado | Crece con |
|----------|-------------|-----------|
| **D1** | Inestabilidad genómica (daño ADN) | Edad + Estado mutacional TP53 |
| **D2** | Resistencia inmune (evasión apoptosis) | Edad + Estado mutacional TP53 + BRCA1 |

### Reglas de Oro

- ✅ **BRCA1 -/- = MUERTE** (apoptosis intrínseca inmediata)
- ✅ **TP53 -/- = VULNERABLE** (permite transformación neoplásica)
- ✅ **D1 alto + D2 bajo = APOPTOSIS EXTRÍNSECA** (el tejido mata la célula)
- ✅ **D1 alto + D2 alto = TUMOR** (la célula evade y se transforma)

> ⚙️ **NOTA DE CONFIGURABILIDAD**: Todos los procesos del modelo son **completamente configurables**. Los parámetros de D1 y D2 (deltas, umbrales, valores máximos), las tasas de mutación, división celular y transformación neoplásica pueden ajustarse según las necesidades del escenario de simulación. Ver la sección [Parámetros Configurables](#parámetros-configurables) para más detalles.

---

## 📚 Índice de Contenidos

1. [Introducción](#1-introducción)
2. [Estructura de la Célula](#2-estructura-de-la-célula)
3. [Estados Celulares (CellLifeStage)](#3-estados-celulares-celllifestage)
4. [Evolución Celular: De Sana a Tumoral](#4-evolución-celular-de-sana-a-tumoral)
5. [D1 y D2: Inestabilidad Genómica y Resistencia Inmune](#5-d1-y-d2-inestabilidad-genómica-y-resistencia-inmune)
6. [Modelo Matemático](#6-modelo-matemático)
7. [Ciclo de Vida Celular (6 Fases)](#7-ciclo-de-vida-celular-6-fases)
8. [Escenarios y Validación](#8-escenarios-y-validación)
9. [Parámetros Configurables](#9-parámetros-configurables)
10. [Referencias y Código Fuente](#10-referencias-y-código-fuente)

---

## 1. Introducción

### 1.1 Contexto Biológico

El cáncer es una enfermedad caracterizada por la **pérdida progresiva de mecanismos de control celular**. En condiciones normales, las células tienen sistemas de protección:

- **Reparación del ADN**: Genes como BRCA1 corrigen errores en la replicación
- **Supresión tumoral**: Genes como TP53 ("guardián del genoma") detectan daño y activan apoptosis
- **Vigilancia inmune**: El sistema inmune detecta y elimina células anómalas

Cuando estos sistemas fallan, las células pueden acumular mutaciones, evitar la muerte programada y eventualmente transformarse en **células tumorales**.

### 1.2 El Modelo Agentic Cell

**Agentic Cell** simula este proceso mediante un modelo basado en agentes donde:

1. Cada célula es un **agente autónomo** que toma decisiones
2. Las células tienen un **estado interno** con 4 atributos (BRCA1, TP53, D1, D2)
3. El estado evoluciona según **reglas probabilísticas** y **umbrales**
4. Las células pueden **dividirse, mutar, morir o transformarse**

### 1.3 Objetivos del Modelo

- **Para biólogos**: Comprender cómo diferentes tasas de mutación afectan la progresión tumoral
- **Para matemáticos**: Estudiar dinámicas poblacionales y modelos estocásticos
- **Para bioinformáticos**: Validar hipótesis computacionalmente antes de experimentos

---

## 2. Estructura de la Célula

### 2.1 Los 4 Atributos Internos

Cada célula tiene 4 atributos que determinan su comportamiento:

```
+---------------------------------------------------------------+
|                    ESTRUCTURA DE LA CÉLULA                     |
+---------------------------------------------------------------+

                    +-------------------------+
                    |        CÉLULA           |
                    +-------------------------+
                    | BRCA1: +/- o -/-        |  <-- Gen de reparación ADN
                    | TP53:  +/+ o +/- o -/-  |  <-- Supresor tumoral
                    | D1:    [0, 999]         |  <-- Inestabilidad genómica
                    | D2:    [0, 999]         |  <-- Resistencia inmune
                    +-------------------------+
```

#### 2.1.1 Gen BRCA1 (Breast Cancer Susceptibility Gene 1)

| Estado | Significado | Consecuencia |
|--------|-------------|--------------|
| **+/-** | Heterocigoto (1 copia funcional) | ✅ Célula viable, reparación parcial |
| **-/-** | Homocigoto recesivo (0 copias funcionales) | ❌ **MUERTE inmediata** (apoptosis intrínseca) |

> **Nota sobre el modelo**: En este modelo de simulación, BRCA1 inicia como +/- (heterocigoto) porque se simula una población con predisposición hereditaria al cáncer de mama (portadores de mutación germinal en BRCA1). En células normales sin predisposición hereditaria, BRCA1 sería +/+.

#### 2.1.2 Gen TP53 (Tumor Protein 53 - "Guardián del Genoma")

| Estado | Significado | Protección | Efecto en D1/D2 |
|--------|-------------|------------|-----------------|
| **+/+** | Homocigoto dominante (2 copias funcionales) | ✅ **MÁXIMA** | δ = 0 (no crece) |
| **+/-** | Heterocigoto (1 copia funcional) | ⚠️ **PARCIAL** | δ = low_delta |
| **-/-** | Homocigoto recesivo (0 copias funcionales) | ❌ **SIN PROTECCIÓN** | δ = high_delta |

#### 2.1.3 D1 - Inestabilidad Genómica (DNA Damage)

D1 representa la **acumulación de daño en el ADN** de la célula:

- **Significado biológico**: Errores de replicación, roturas de cadena doble, mutaciones puntuales
- **Rango**: [1.0, 999.0] (saturación máxima)
- **Efecto**: Cuando D1 supera un umbral, la célula entra en estado PRIMER

#### 2.1.4 D2 - Resistencia Inmune (Immunosuppression)

D2 representa la **capacidad de la célula para evadir la respuesta inmune**:

- **Significado biológico**: Expresión de ligandos inhibitorios (ej: PD-L1), secreción de citoquinas inmunosupresoras
- **Rango**: [1.0, 999.0] (saturación máxima)
- **Efecto**: Cuando D2 supera un umbral, la célula puede resistir la apoptosis extrínseca

### 2.2 Diagrama de la Célula

```
+------------------------------------------------------------------------+
|                          CÉLULA AGÉNTICA                                |
+------------------------------------------------------------------------+
|                                                                         |
|   +----------------+           +----------------------------------+     |
|   |    GENOMA      |           |  ACUMULADORES DE INESTABILIDAD   |     |
|   +----------------+           +----------------------------------+     |
|   | BRCA1: +/-     |<--------->| D1: Inestabilidad Genómica       |     |
|   | TP53:  +/+     |<--------->|     (crece con edad + mutaciones)|     |
|   +----------------+           |                                  |     |
|          |                     | D2: Resistencia Inmune           |     |
|          v                     |     (crece con edad + mutaciones)|     |
|   +----------------+           +----------------------------------+     |
|   |  MUTACIONES    |                          |                         |
|   |  (aleatorias)  |                          v                         |
|   +----------------+           +----------------------------------+     |
|                                | ESTADO DERIVADO (CellLifeStage)  |     |
|                                | - BASELINE / UNSTABLE            |     |
|                                | - UNPROTECTED / PRIMER / TUMORAL |     |
|                                +----------------------------------+     |
+------------------------------------------------------------------------+
```

---

## 3. Estados Celulares (CellLifeStage)

### 3.1 Los 6 Estados Posibles

El estado de la célula se **deriva en tiempo real** de sus atributos internos:

```
+---------------------------------------------------------------+
|              ESTADOS CELULARES (CellLifeStage)                 |
+---------------------------------------------------------------+

    +----------+      +----------+      +-------------+
    |   DEAD   |      | BASELINE | ---> |  UNSTABLE   |
    | (Muerta) |      |  (Sana)  |      | (Inestable) |
    +----------+      +----------+      +-------------+
                           |                   |
                           |                   v
                      TP53 +/+            TP53 +/-
                           |                   |
                           |                   |
                           +--------+----------+
                                    |
                                    v
                           +-----------------+
                           |   UNPROTECTED   |
                           | (Sin protección)|
                           |    TP53 -/-     |
                           +--------+--------+
                                    |
                              D1 > umbral
                                    v
                           +-----------------+
                           |     PRIMER      |
                           |  (Pre-tumoral)  |
                           |   Detectable    |
                           +--------+--------+
                                    |
                           D2 > umbral (evade)
                                    v
                           +-----------------+
                           |    TUMORAL      |
                           |  (Neoplásica)   |
                           |    Inmortal     |
                           +-----------------+
```

### 3.2 Tabla de Estados

| Estado | Condición | BRCA1 | TP53 | D1 | D2 | Descripción |
|--------|-----------|-------|------|----|----|-------------|
| **DEAD** | !alive() | - | - | - | - | Célula muerta (eliminada del tejido) |
| **BASELINE** | TP53 +/+ | +/- | +/+ | bajo | bajo | Célula sana, máxima protección |
| **UNSTABLE** | TP53 +/- | +/- | +/- | crece | crece | Primera inestabilidad, aún protegida |
| **UNPROTECTED** | TP53 -/- | +/- o -/- | -/- | crece | crece | Sin protección TP53, vulnerable |
| **PRIMER** | UNPROTECTED + D1 > 2.0 | +/- o -/- | -/- | alto | variable | Pre-tumoral, detectable por tejido |
| **TUMORAL** | is_neoplastic = true | +/- o -/- | -/- | alto | alto | Transformada, neoplásica, inmortal |

### 3.3 Reglas de Transición

```python
# Pseudocódigo de derivación de estado
def get_cell_life_stage(cell):
    if not cell.alive():
        return DEAD
    
    if cell.is_neoplastic:
        return TUMORAL
    
    if cell.TP53 == "-/-":
        if cell.D1 > d1_primer_threshold:
            return PRIMER
        else:
            return UNPROTECTED
    
    if cell.TP53 == "+/-":
        return UNSTABLE
    
    if cell.TP53 == "+/+":
        return BASELINE
```

---

## 4. Evolución Celular: De Sana a Tumoral

### 4.1 Diagrama de Evolución Completo

```
+-------------------------------------------------------------------------+
|            EVOLUCIÓN CELULAR: DE SANA A TUMORAL                          |
+-------------------------------------------------------------------------+

TIEMPO ----------------------------------------------------------------------->

        +----------------------------------------------------------------------+
        |  CÉLULA SANA (BASELINE)                                              |
        |  BRCA1: +/-    TP53: +/+    D1: 1.0    D2: 1.0                       |
        |  [OK] Protegida por TP53    [OK] Sin acumulación de daño             |
        +----------------------------------+-----------------------------------+
                                           |
                              +------------+------------+
                              | MUTACIÓN TP53 (+/+ -> +/-)|
                              | Probabilidad: ~1% /ciclo  |
                              +------------+------------+
                                           v
        +----------------------------------------------------------------------+
        |  CÉLULA INESTABLE (UNSTABLE)                                         |
        |  BRCA1: +/-    TP53: +/-    D1: 1.0+d    D2: 1.0+d                   |
        |  [!] Protección parcial    [!] D1/D2 crecen con low_delta            |
        +----------------------------------+-----------------------------------+
                                           |
                              +------------+------------+
                              | MUTACIÓN TP53 (+/- -> -/-)|
                              | Probabilidad: ~1% /ciclo  |
                              +------------+------------+
                                           v
        +----------------------------------------------------------------------+
        |  CÉLULA SIN PROTECCIÓN (UNPROTECTED)                                 |
        |  BRCA1: +/-    TP53: -/-    D1: crece    D2: crece                   |
        |  [X] Sin protección TP53    [X] D1/D2 crecen con high_delta          |
        |  [!] Vulnerable a transformación neoplásica                          |
        +----------------------------------+-----------------------------------+
                                           |
                              +------------+------------+
                              | D1 > d1_primer_threshold  |
                              | (típicamente D1 > 2.0)    |
                              +------------+------------+
                                           v
        +----------------------------------------------------------------------+
        |  CÉLULA PRE-TUMORAL (PRIMER)                                         |
        |  BRCA1: +/-    TP53: -/-    D1: alto    D2: variable                 |
        |  [?] Detectable por el tejido                                        |
        |  [!] El tejido intenta inducir apoptosis extrínseca                  |
        +------------------+---------------------------+-----------------------+
                           |                           |
          +----------------+---------------+  +--------+------------------------+
          | D2 < d2_apoptosis_threshold    |  | D2 > d2_apoptosis_threshold     |
          | (D2 bajo = no evade)           |  | (D2 alto = evade apoptosis)     |
          +----------------+---------------+  +--------+------------------------+
                           v                           v
        +------------------------------+  +------------------------------------+
        |        APOPTOSIS             |  |     CÉLULA TUMORAL (TUMORAL)       |
        |  [X] Célula muere            |  |  BRCA1: +/-    TP53: -/-           |
        |  (apoptosis extrínseca)      |  |  D1: alto    D2: alto              |
        |                              |  |  [!] Neoplásica    [!] Inmortal    |
        +------------------------------+  |  [!] Puede dividirse (Big Bang)    |
                                          +------------------------------------+
```

### 4.2 Caminos Posibles

La célula puede tomar diferentes caminos según las mutaciones que acumule:

#### Camino 1: Muerte por BRCA1

```
BASELINE (BRCA1 +/-, TP53 +/+)
    |
    +-- MUTACIÓN BRCA1 (+/- -> -/-)
    |
    v
MUERTE INMEDIATA (apoptosis intrínseca en Fase 1)
```

> **Nota**: Si BRCA1 muta a -/- antes que TP53, la célula muere inmediatamente. BRCA1 -/- es incompatible con la vida celular.

#### Camino 2: Apoptosis Extrínseca

```
BASELINE -> UNSTABLE -> UNPROTECTED -> PRIMER
    |
    +-- D2 bajo (< umbral)
    +-- Tejido detecta célula anómala
    +-- Envía señal de apoptosis
    |
    v
MUERTE (apoptosis extrínseca en Fase 2)
```

#### Camino 3: Transformación Tumoral (Big Bang)

```
BASELINE -> UNSTABLE -> UNPROTECTED -> PRIMER
    |
    +-- D1 alto + D2 alto
    +-- Célula evade apoptosis extrínseca
    +-- Transformación neoplásica
    |
    v
TUMORAL (inmortal, puede dividirse)
```

### 4.3 Comportamiento según D1 y D2

La siguiente tabla resume qué sucede en el estado UNPROTECTED (TP53 -/-):

| D1 | D2 | Estado Resultante | Descripción |
|----|----|--------------------|-------------|
| Bajo | Bajo | UNPROTECTED | Sigue acumulando mutaciones, no detectable |
| **Alto** | Bajo | PRIMER → APOPTOSIS | Detectable, pero no evade → muere |
| Bajo | Alto | UNPROTECTED | Sigue acumulando, evade pero sin D1 suficiente |
| **Alto** | **Alto** | PRIMER → **TUMORAL** | Detectable, evade apoptosis → se transforma |

---

## 5. D1 y D2: Inestabilidad Genómica y Resistencia Inmune

### 5.1 Concepto Biológico

#### D1: Inestabilidad Genómica (DNA Damage)

D1 modela la **acumulación de daño en el ADN** que ocurre naturalmente con:

- **Edad**: Cada ciclo celular introduce errores de replicación
- **Mutaciones**: La pérdida de TP53 acelera la acumulación
- **Estrés**: Factores externos (radiación, químicos) también contribuyen

En el modelo, D1 representa:
- Roturas de doble cadena no reparadas
- Mutaciones puntuales acumuladas
- Inestabilidad cromosómica

#### D2: Resistencia Inmune (Immunosuppression)

D2 modela la **capacidad de evasión del sistema inmune** que desarrollan las células pre-tumorales:

- **Expresión de inhibidores**: Moléculas como PD-L1 que inhiben células T
- **Secreción de citoquinas**: Moléculas inmunosupresoras en el microambiente
- **Pérdida de antígenos**: Reducción de señales que alertan al sistema inmune

En el modelo, D2 representa la capacidad de la célula para **resistir la apoptosis extrínseca** inducida por el tejido o el sistema inmune.

### 5.2 Cómo Crecen D1 y D2

Los acumuladores D1 y D2 crecen en **cada ciclo celular** según la siguiente lógica:

```
D1(t+1) = D1(t) + δ_D1(TP53_status, age)
D2(t+1) = D2(t) + δ_D2(TP53_status, BRCA1_status, age)
```

Donde los deltas (δ) dependen del estado mutacional:

#### Estrategia de Cálculo de Deltas

```cpp
// Implementación en GenomicInstabilityDeltaStrategy.cpp

// D1 (DNA damage) = Δ(TP53) + factor_edad
δ_D1 = δ_TP53 + age × 0.00001

// D2 (Immunosuppression) = Δ(TP53) + Δ(BRCA1) + factor_edad
δ_D2 = δ_TP53 + δ_BRCA1 + age × 0.00001
```

#### Valores de Delta por Estado

| Gen | Estado | Delta | Descripción |
|-----|--------|-------|-------------|
| **TP53** | +/+ | 0.0 | Reparación eficiente, sin incremento |
| **TP53** | +/- | low_delta (0.001) | Reparación parcial, incremento moderado |
| **TP53** | -/- | high_delta (0.003) | Sin reparación, incremento alto |
| **BRCA1** | +/- | low_delta (0.001) | Contribuye solo a D2 |
| **BRCA1** | -/- | 2 × high_delta | Contribución severa a D2 (antes de morir) |

### 5.3 Diagrama de Acumulación

```
+-------------------------------------------------------------------------+
|         DINÁMICA DE ACUMULACIÓN DE D1 Y D2                               |
+-------------------------------------------------------------------------+

D1, D2
   |
   |                                           /// TP53 -/- (high_delta)
   |                                       ///
   |                                    ///
   |                                 ///
   |                              ///
   |                           ///
   |                        ///
   |                     ///
   |                  ///  ------ TP53 +/- (low_delta)
   |               ///  ------
   |            ///  ------
   |         ///  ------
   |      /// ------
   |   ///------  =============== TP53 +/+ (delta = 0)
   |///===========================================================
   +----------------------------------------------------------------- Tiempo
        0        10        20        30        40        50

LEYENDA:
  === TP53 +/+ : D1/D2 estables (solo crece mínimamente con edad)
  --- TP53 +/- : D1/D2 crecen moderadamente
  /// TP53 -/- : D1/D2 crecen rápidamente
```

### 5.4 Tabla de Incrementos por Ciclo

| Escenario | TP53 | BRCA1 | δ_D1 | δ_D2 | Descripción |
|-----------|------|-------|------|------|-------------|
| Célula sana | +/+ | +/- | 0.0 + age×0.00001 | 0.0 + low + age×0.00001 | Estable |
| Primera mutación | +/- | +/- | low + age×0.00001 | low + low + age×0.00001 | Crece lento |
| Sin protección | -/- | +/- | high + age×0.00001 | high + low + age×0.00001 | Crece rápido |
| Máximo riesgo | -/- | -/- | high + age×0.00001 | high + 2×high + age×0.00001 | BRCA1 -/- → Muerte |

> **Nota**: BRCA1 -/- causa muerte inmediata, por lo que el escenario "máximo riesgo" es teórico.

---

## 6. Modelo Matemático

### 6.1 Fórmulas de Evolución

#### Evolución de D1 (Inestabilidad Genómica)

```
D1(t+1) = min(D1(t) + δ_D1, max_D1)

donde:
  δ_D1 = δ_TP53 + (age × 0.00001)
  
  δ_TP53 = {
    0.0          si TP53 = +/+
    low_delta    si TP53 = +/-  (típicamente 0.001)
    high_delta   si TP53 = -/-  (típicamente 0.003)
  }
  
  max_D1 = 999.0 (saturación)
```

#### Evolución de D2 (Resistencia Inmune)

```
D2(t+1) = min(D2(t) + δ_D2, max_D2)

donde:
  δ_D2 = δ_TP53 + δ_BRCA1 + (age × 0.00001)
  
  δ_TP53 = (igual que arriba)
  
  δ_BRCA1 = {
    0.0            si BRCA1 = +/+ (en modelo, BRCA1 inicia como +/-)
    low_delta      si BRCA1 = +/-  (típicamente 0.001)
    2×high_delta   si BRCA1 = -/-  (pero esto causa muerte)
  }
  
  max_D2 = 999.0 (saturación)
```

### 6.2 Probabilidades de Mutación

```
P(mutación_gen) = threshold_gen × (1 + k_gen) × genomic_instability

donde:
  threshold_gen = tasa base de mutación (ej: 0.01 = 1% por ciclo)
  k_gen = coeficiente de aceleración (puede ser 0)
  genomic_instability = función de D1 (típicamente D1)
```

#### Transiciones de TP53

```
P(TP53: +/+ → +/-) = μ_TP53 × D1    (primera mutación)
P(TP53: +/- → -/-) = μ_TP53 × D1    (segunda mutación)
```

#### Transiciones de BRCA1

```
P(BRCA1: +/- → -/-) = μ_BRCA1 × D1  (mutación letal)
```

### 6.3 Umbrales de Decisión

| Parámetro | Valor Típico | Decisión |
|-----------|--------------|----------|
| `d1_primer_threshold` | 2.0 | Si D1 > 2.0 y TP53 = -/- → Estado PRIMER |
| `d2_apoptosis_threshold` | 5.0 | Si D2 > 5.0 → Evade apoptosis extrínseca |
| `neoplasm_k` | 0.002 | Probabilidad base de transformación neoplásica |

### 6.4 Ejemplo Numérico

```
Célula inicial:
  BRCA1 = +/-, TP53 = +/+, D1 = 1.0, D2 = 1.0, age = 0

Después de 100 ciclos (TP53 = +/+, age promedio = 50):
  # Nota: age promedio durante 100 ciclos = 50 (promedio de 0 a 100)
  D1 = 1.0 + 100 × (0.0 + 0.0005) = 1.05  (casi sin cambio, solo factor edad)
  D2 = 1.0 + 100 × (0.0 + 0.001 + 0.0005) = 1.15

Después de mutación TP53 (+/+ → +/-):
  D1 = 1.05 + ciclos × (0.001 + age×0.00001)
  D2 = 1.15 + ciclos × (0.001 + 0.001 + age×0.00001)

Después de 100 ciclos adicionales (TP53 = +/-):
  D1 ≈ 1.05 + 100 × 0.001 = 1.15
  D2 ≈ 1.15 + 100 × 0.002 = 1.35

Después de segunda mutación TP53 (+/- → -/-):
  D1 crece ahora con high_delta (0.003)
  D2 crece ahora con high_delta (0.003)
  
Después de 100 ciclos más (TP53 = -/-):
  D1 ≈ 1.1005 + 100 × 0.003 = 1.4005
  D2 ≈ 1.305 + 100 × 0.004 = 1.705
  
  Si D1 > 2.0 → PRIMER
  Si D2 > 5.0 y D1 > 2.0 → puede transitar a TUMORAL
```

---

## 7. Ciclo de Vida Celular (6 Fases)

### 7.1 Resumen del Ciclo

Cada célula ejecuta **6 fases secuenciales** en cada tick de simulación:

```
+-------------------------------------------------------------------------+
|                    CICLO DE VIDA CELULAR (6 FASES)                       |
+-------------------------------------------------------------------------+

  +-------------+   +-------------+   +-------------+
  |   FASE 0    |   |   FASE 1    |   |   FASE 2    |
  |  Baseline   |-->|  G1 Check   |-->| Endocytosis |
  |  Assessment |   |  (BRCA1)    |   | (Apoptosis) |
  +-------------+   +-------------+   +-------------+
                          |                  |
                    BRCA1 -/- ?         D2 < umbral?
                          v                  v
                      MUERTE             MUERTE
                    (intrínseca)       (extrínseca)

                    (si sobrevive)    (si sobrevive)
                          v                  v
                    +-------------+   +-------------+   +-------------+
                    |   FASE 3    |   |   FASE 4    |   |   FASE 5    |
                    |  Nuclear    |-->| Remodeling  |-->| Exocytosis  |
                    |  Dynamics   |   |  (D1, D2)   |   | (Señales)   |
                    +-------------+   +-------------+   +-------------+
                          |                  |                  |
                     Mutaciones         Actualiza         Emite señales
                    (BRCA1, TP53)       D1 y D2          al tejido
                                             |
                                    Division? Transformacion?
                                             v
                                    age++ -> Siguiente ciclo
```

### 7.2 Detalle de Cada Fase

#### Fase 0: Baseline Assessment

- **Propósito**: Verificar si la célula está viva
- **Acción**: Si `alive() == false`, termina el ciclo
- **Resultado**: Continúa a Fase 1 o termina

#### Fase 1: G1 Integrity Checkpoint

- **Propósito**: Verificar integridad de BRCA1
- **Acción**: Si `BRCA1 == "-/-"`, la célula **muere inmediatamente**
- **Resultado**: Lanza `CellDeathException` (apoptosis intrínseca) o continúa

```cpp
// Pseudocódigo
if (BRCA1 == "-/-") {
    throw CellDeathException("BRCA1 knockout");
}
```

#### Fase 2: Endocytosis (Checkpoint de Apoptosis)

- **Propósito**: Procesar señales de apoptosis del tejido
- **Acción**: Si recibe señal de apoptosis, evalúa D2
- **Decisión**:
  - Si `D2 < d2_apoptosis_threshold` → **Acepta apoptosis** → MUERTE
  - Si `D2 >= d2_apoptosis_threshold` → **Evade apoptosis** → Sobrevive (inmortalización)

```cpp
// Pseudocódigo
if (recibe_senal_apoptosis) {
    if (D2 < d2_apoptosis_threshold) {
        die();  // Apoptosis extrínseca
    } else {
        has_evaded_apoptosis = true;  // Inmortalización
    }
}
```

#### Fase 3: Nuclear Dynamics (Mutaciones)

- **Propósito**: Aplicar mutaciones estocásticas
- **Acción**: Evalúa probabilidad de mutación para cada gen
- **Resultado**: Genes pueden transitar:
  - BRCA1: +/- → -/- (letal en próximo ciclo)
  - TP53: +/+ → +/- → -/- (progresivo)

```cpp
// Pseudocódigo
P_mut = threshold × D1;
if (random() < P_mut) {
    mutar_gen();
}
```

#### Fase 4: Cytoplasmic Remodeling

- **Propósito**: Actualizar D1 y D2, decidir transformación/división
- **Acciones**:
  1. Calcular deltas según estado genético
  2. Actualizar D1 y D2
  3. Evaluar transformación neoplásica (si TP53 -/- y D1 alto y D2 alto)
  4. Evaluar división celular (si no neoplásica)

```cpp
// Pseudocódigo
deltas = calculateInstabilityDeltas();  // Usa estrategia
D1 = min(D1 + deltas.d1, max_D1);
D2 = min(D2 + deltas.d2, max_D2);

if (TP53 == "-/-" && D1 > d1_primer && D2 > d2_apoptosis) {
    transformar_neoplasica();
}

if (!is_neoplastic && random() < division_rate) {
    crear_hija();
}
```

#### Fase 5: Exocytosis

- **Propósito**: Emitir señales al tejido
- **Acción**: Si es neoplásica, emite `NeoplasmSignal`
- **Resultado**: El tejido puede responder (ej: enviar apoptosis)

```cpp
// Pseudocódigo
if (is_neoplastic) {
    emit(NeoplasmSignal);
}
age++;
```

---

## 8. Escenarios y Validación

### 8.1 Escenarios de Validación Implementados

El modelo incluye 14 escenarios de validación divididos en grupos:

#### Controles Basales (Escenarios 01-02)

| Escenario | Descripción | Resultado Esperado |
|-----------|-------------|-------------------|
| 01 | Sin mutaciones, sin división | Población estable (1000) |
| 02 | Sin mutaciones, división 15% | Crecimiento exponencial (~5000) |

#### Controles Paramétricos (Escenarios 03-05)

| Escenario | Descripción | Resultado Esperado |
|-----------|-------------|-------------------|
| 03 | BRCA1 muy mutágeno (20%) | Extinción total |
| 04 | TP53 muy mutágeno (10%) | 98% neoplásticas |
| 05 | TP53 + alta inestabilidad | Mayor resistencia |

#### Escenarios Realistas (Escenarios 06-09)

| Escenario | Descripción | Resultado Esperado |
|-----------|-------------|-------------------|
| 06 | Parámetros realistas base | 20% neoplásticas |
| 07 | TP53 bajo (protector) | 2.5% neoplásticas |
| 08 | TP53 + inestabilidad alta | 46% neoplásticas |
| 09 | **Balanceado (recomendado)** | 4.7% neoplásticas |

#### Modo Big Bang (Escenarios 10-14)

| Escenario | Descripción | Resultado Esperado |
|-----------|-------------|-------------------|
| 10 | Big Bang bajo umbral | 92% neoplásticas (año 80) |
| 11 | Big Bang moderado | 85%+ neoplásticas |
| 12 | Big Bang agresivo | 95%+ neoplásticas |
| 13 | Inestabilidad extrema | 99%+ neoplásticas |
| 14 | Control reproducible | Validación cruzada |

### 8.2 Fases Temporales del Big Bang

El modelo Big Bang simula la **transformación masiva neoplástica** con 4 fases:

```
FASE 1: LATENCIA (Años 0-30)
  • Población: ~1000 células protegidas
  • Neoplásticas: 0-10%
  • D1/D2: Bajos
  • Apoptosis: Activa

FASE 2: ACELERACIÓN (Años 30-50)
  • Población: 600-1000 células
  • Neoplásticas: 10-50%
  • D1/D2: Creciendo exponencialmente
  • Apoptosis: Débil

FASE 3: CRÍTICA - "BIG BANG" (Años 50-70)
  ⚠️ PUNTO DE RUPTURA
  • Transformación MASIVA simultánea
  • Neoplásticas: 50-90%
  • D1/D2: Extremos (>50-100)
  • Apoptosis: Evasiva

FASE 4: DOMINIO (Años 70-80)
  • Neoplásticas: 90-99%
  • Inmortales: >99%
  • Crecimiento: Exponencial descontrolado
```

---

## 9. Parámetros Configurables

> ⚙️ **IMPORTANTE**: El modelo Agentic Cell es **completamente configurable**. Todos los parámetros pueden ajustarse para simular diferentes escenarios biológicos, desde poblaciones normales hasta casos de alta predisposición genética.

### 9.1 Configuración de Inestabilidad (InstabilityConfig)

Controla cómo crecen los acumuladores D1 y D2:

| Parámetro | Valor por Defecto | Descripción | Rango Típico |
|-----------|-------------------|-------------|--------------|
| `low_delta` | 0.001 | Incremento de D1/D2 para mutaciones heterocigotas (TP53 +/-) | 0.0001 - 0.01 |
| `high_delta` | 0.003 | Incremento de D1/D2 para mutaciones homocigotas (TP53 -/-) | 0.001 - 0.05 |
| `max_d1` | 999.0 | Valor máximo de saturación para D1 | 100.0 - 999.0 |
| `max_d2` | 999.0 | Valor máximo de saturación para D2 | 100.0 - 999.0 |

**Ejemplo de uso:**
```cpp
InstabilityConfig config;
config.low_delta = 0.002;   // Incremento más rápido
config.high_delta = 0.01;   // Inestabilidad severa
config.max_d1 = 500.0;      // Saturación más temprana
```

### 9.2 Configuración de División (DivisionConfig)

Controla las tasas de reproducción celular:

| Parámetro | Valor por Defecto | Descripción | Rango Típico |
|-----------|-------------------|-------------|--------------|
| `base_rate` | 0.001 | Probabilidad de división por ciclo (células normales) | 0.0001 - 0.15 |
| `neoplastic_rate` | 0.001 | Probabilidad de división por ciclo (células neoplásicas) | 0.001 - 0.25 |
| `enable_big_bang` | false | Modo Big Bang: división acelerada de neoplásicas | true/false |

### 9.3 Configuración de Umbrales (ThresholdConfig)

Controla los puntos de transición entre estados:

| Parámetro | Valor por Defecto | Descripción | Rango Típico |
|-----------|-------------------|-------------|--------------|
| `d1_primer` | 2.0 | Umbral de D1 para entrar en estado PRIMER | 1.0 - 10.0 |
| `d2_apoptosis` | 5.0 | Umbral de D2 para resistir apoptosis extrínseca | 3.0 - 20.0 |
| `neoplasm_k` | 0.002 | Probabilidad base de transformación neoplásica | 0.001 - 0.1 |

### 9.4 Tasas de Mutación

Las tasas de mutación para cada gen también son configurables:

| Gen | Parámetro | Valor por Defecto | Descripción |
|-----|-----------|-------------------|-------------|
| BRCA1 | `BRCA1_threshold` | 0.05 (5%) | Probabilidad de mutación +/- → -/- por ciclo |
| TP53 | `TP53_threshold` | 0.01 (1%) | Probabilidad de mutación por ciclo |

### 9.5 Cómo Ajustar los Parámetros

Los parámetros se pueden ajustar de tres formas:

1. **En código**: Modificando los structs de configuración al crear células
2. **Por escenario**: Usando `SimulationConfig` para cargar configuraciones predefinidas
3. **En tiempo de ejecución**: Algunos parámetros pueden modificarse dinámicamente

**Ejemplo de configuración completa:**
```cpp
// Configurar una célula con parámetros personalizados
InstabilityConfig instab{
    .low_delta = 0.002,
    .high_delta = 0.008,
    .max_d1 = 999.0,
    .max_d2 = 999.0
};

DivisionConfig div{
    .base_rate = 0.05,
    .neoplastic_rate = 0.10,
    .enable_big_bang = true
};

ThresholdConfig thresh{
    .d1_primer = 3.0,
    .d2_apoptosis = 8.0,
    .neoplasm_k = 0.01
};

auto cell = std::make_unique<AgenticCell>(
    noise, genome, instab, div, thresh, logger
);
```

### 9.6 Efectos de Modificar D1 y D2

La siguiente tabla resume el efecto de modificar los parámetros de D1 y D2:

| Cambio | Efecto en Simulación | Interpretación Biológica |
|--------|---------------------|--------------------------|
| ↑ `low_delta` | Células acumulan inestabilidad más rápido en TP53 +/- | Mayor sensibilidad a haploinsuficiencia |
| ↑ `high_delta` | Células en TP53 -/- progresan más rápido a PRIMER | Mayor agresividad tumoral |
| ↓ `d1_primer` | Células entran en PRIMER con menos daño | Detección más temprana por el tejido |
| ↑ `d2_apoptosis` | Células necesitan más resistencia para evadir | Sistema inmune más efectivo |
| ↑ `neoplasm_k` | Mayor probabilidad de transformación | Microambiente pro-tumoral |

---

## 10. Referencias y Código Fuente

### 10.1 Archivos Clave del Código

| Archivo | Descripción |
|---------|-------------|
| `src/domain/cell/AgenticCell.h` | Implementación principal de la célula |
| `src/domain/cell/CellLifeStage.h` | Enum con los 6 estados celulares |
| `src/domain/cell/CellConfig.h` | Configuración de instabilidad, división, umbrales |
| `src/domain/cell/strategies/GenomicInstabilityDeltaStrategy.cpp` | Estrategia de cálculo de D1/D2 |
| `src/domain/cell/model/InstabilityDeltas.h` | Value object para deltas D1/D2 |
| `src/domain/gene/Genome.h` | Genoma con BRCA1 y TP53 |

### 10.2 Documentación Adicional

| Documento | Descripción |
|-----------|-------------|
| `docs/README.md` | Documentación general del modelo biológico |
| `docs/BIG_BANG_MODE.md` | Documentación del modo Big Bang |
| `docs/diagrams.md` | Diagramas FSM y ciclo de vida |
| `docs/adr/0002-agentic-cell-pattern.md` | Decisión arquitectónica del patrón Agentic |
| `BIG_BANG_SUMMARY.md` | Resumen ejecutivo del Big Bang |
| `INDEX.md` | Índice de validación con 14 escenarios |

### 10.3 Diagramas Originales

Los diagramas originales de la discusión del modelo se encuentran en:

```
docs/diagrams_luis/
+-- celula.png                # Estructura de la célula
+-- estado_1.png              # Estados iniciales
+-- estado_2_base.png         # Estados con duplicidades
+-- estado_2_simplificado.png # Estados simplificados
+-- resumen.txt               # Notas del meeting
```

### 10.4 Referencias Biológicas

> 📚 **Nota sobre las referencias**: Las siguientes referencias son académicas y verificadas. Corresponden a artículos publicados en revistas científicas de alto impacto (Science, Nature, Cell, British Journal of Cancer). Se recomienda consultar las fuentes originales para profundizar en los conceptos biológicos.

1. **Tomasetti C., Li L., Vogelstein B. (2017)**: Stem cell divisions, somatic mutations, cancer etiology, and cancer prevention. *Science*, 355(6331), 1330-1334. DOI: 10.1126/science.aaf9011
2. **Tomasetti C., Vogelstein B. (2015)**: Variation in cancer risk among tissues can be explained by the number of stem cell divisions. *Science*, 347(6217), 78-81. DOI: 10.1126/science.1260825
3. **Armitage P., Doll R. (1954)**: The age distribution of cancer and a multi-stage theory of carcinogenesis. *British Journal of Cancer*, 8(1), 1-12. DOI: 10.1038/bjc.1954.1
4. **Venkitaraman A.R. (2002)**: Cancer susceptibility and the functions of BRCA1 and BRCA2. *Cell*, 108(2), 171-182. DOI: 10.1016/s0092-8674(02)00615-3
5. **Vogelstein B., Lane D., Levine A.J. (2000)**: Surfing the p53 network. *Nature*, 408(6810), 307-310. DOI: 10.1038/35042675

---

## 📞 Contacto y Soporte

**Repositorio**: cellSim  
**Última actualización**: Diciembre 2025  
**Autor**: Equipo cellSim

---

## Apéndice A: Glosario

| Término | Definición |
|---------|-----------|
| **Agente** | Entidad autónoma que toma decisiones basadas en su estado interno |
| **Apoptosis** | Muerte celular programada |
| **Apoptosis intrínseca** | Muerte activada por defectos internos (ej: BRCA1 -/-) |
| **Apoptosis extrínseca** | Muerte inducida por señales externas (ej: sistema inmune) |
| **BRCA1** | Gen supresor tumoral que repara ADN |
| **D1** | Contador de inestabilidad genómica (DNA damage) |
| **D2** | Contador de resistencia inmune (immunosuppression) |
| **Delta (δ)** | Incremento por ciclo de D1 o D2 |
| **Homocigoto** | Dos copias iguales de un gen (-/- o +/+) |
| **Heterocigoto** | Una copia de cada variante de un gen (+/-) |
| **Neoplasia** | Crecimiento celular anormal (tumor) |
| **TP53** | Gen supresor tumoral ("guardián del genoma") |

---

**Fin del documento**
