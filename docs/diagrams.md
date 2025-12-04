# 📊 Diagramas del Simulador cellSim

## 1️⃣ FSM DE GENES (Finite State Machine)

### 🧬 Gen BRCA1: Modelo de Viabilidad

```
┌─────────────────────────────────────────────────────────────────┐
│                     FSM BRCA1 - VIABILIDAD                      │
└─────────────────────────────────────────────────────────────────┘

                    P(mutación) = threshold × genomic_instability
                                        ↓
        ┌──────────────┐          ┌──────────────┐
        │              │ ────────→ │              │
        │     +/-      │   μ₁      │     -/-      │
        │   (vivo)     │ ────────→ │   (MUERE)    │
        │              │           │              │
        └──────────────┘           └──────────────┘
         (INICIAL)
              ↑
         NUNCA RETORNA
         (irreversible)


Matemática:
──────────
P(+/- → -/-) = μ₁ = threshold_BRCA1 × (1 + k_BRCA1) × genomic_instability

Estados:
  • +/- : Heterocigoto (BRCA1 funcional, VIVO)
  • -/- : Homocigoto recesivo (BRCA1 deficiente, MUERTE en Fase 1)

Transición:
  • Unidireccional e irreversible
  • Generada por evento aleatorio en Fase 3 (Nuclear Dynamics)
  • Una vez en -/-, muerte garantizada en siguiente Fase 1
```

---

### 🛡️ Gen TP53: Modelo de Guardián Tumoral

```
┌─────────────────────────────────────────────────────────────────┐
│                   FSM TP53 - GUARDIÁN TUMORAL                   │
└─────────────────────────────────────────────────────────────────┘

                    P(mutación) = threshold × genomic_instability
                                        ↓
    ┌──────────────┐          ┌──────────────┐          ┌──────────────┐
    │              │ ────────→ │              │ ────────→ │              │
    │     +/+      │   μ₁      │     +/-      │   μ₂     │     -/-      │
    │✅ PROTEGIDO  │ ────────→ │✅ PROTEGIDO  │ ────────→ │❌ VULNERABLE │
    │ inest=base   │           │ inest=base   │           │ inest=base   │
    │              │           │  +0.5 (bajo) │           │  +1.0 (alto) │
    └──────────────┘           └──────────────┘           └──────────────┘
     (INICIAL)                (aumento moderado)      (aumento severo)


Matemática:
──────────
P(+/+ → +/-) = μ₁ = threshold_TP53 × (1 + k_TP53) × genomic_instability
P(+/- → -/-) = μ₂ = threshold_TP53 × (1 + k_TP53) × genomic_instability

Transformación Neoplástica (si TP53 = -/-):
  P(transformación) = neoplasm_k (parámetro configurable, default: 0.05 = 5%)
  Solo permite transformación si TP53 status = "-/-" (no protegido)

Inestabilidad genómica progresiva (configurables):
  • TP53 +/+ → inestabilidad = base (1.0)
  • TP53 +/- → inestabilidad += low_delta_instability (default: 0.5)
  • TP53 -/- → inestabilidad += high_delta_instability (default: 1.0)
  Nota: Estos valores pueden variar según el escenario (ver tabla de validación)

Estados de Protección contra Neoplasia:
  • +/+ : ✅ PROTEGIDO (TP53 totalmente funcional)
  • +/- : ✅ PROTEGIDO (TP53 parcialmente funcional, pero sigue siendo funcional)
  • -/- : ❌ NO PROTEGIDO (TP53 deficiente, vulnerable a transformación neoplástica)

Consecuencias:
  • TP53 +/+ → Rechaza transformación neoplástica, inestabilidad baja
  • TP53 +/- → Rechaza transformación neoplástica, pero INESTABILIDAD AUMENTA (aumento moderado: +0.5)
  • TP53 -/- → PERMITE transformación neoplástica, inestabilidad aumenta más (aumento severo: +1.0)
```

---

### 🔗 FSM Combinado: BRCA1 + TP53 (Protección vs Inestabilidad)

```
┌─────────────────────────────────────────────────────────────────┐
│         FSM COMBINADO - 2D STATE SPACE (PROTECCIÓN)             │
└─────────────────────────────────────────────────────────────────┘
```

#### TP53 STATE - BRCA1 STATE MATRIX:

| BRCA1 STATE \ TP53 STATE | +/+ | +/- | -/- |
|---|---|---|---|
| **+/-** | ✅ VIVO<br>Seguro<br>inest=1.0 | ✅ VIVO<br>Protegido<br>inest=1.0+0.5 | ❌ VIVO<br>Vulnerable<br>inest=1.0+1.0 |
| **-/-** | ✗ MUERTE<br>(Fase1) | ✗ MUERTE<br>(Fase1) | ✗ MUERTE<br>(Fase1) |

#### Tabla de Transiciones (por ciclo) - ESCENARIO DEFAULT:

| De Estado | Evento | A Estado | P(trans) | Protección | Resultado |
|---|---|---|---|---|---|
| **(+/-, +/+)** | BRCA1 mutación | (-/-, +/+) | 0.0099 | ✅ → ❌ | ✗ MUERTE (Fase 1) |
| | TP53 mutación | (+/-, +/-) | 0.0100 | ✅ → ✅ | VIVO (degradado) |
| | Ambas mutaciones | (-/-, +/-) | 0.0001 | ✅ → ✅ | ✗ MUERTE (Fase 1) |
| | Sin mutación | (+/-, +/+) | 0.9801 | ✅ → ✅ | VIVO (seguro) |
| **(+/-, +/-)** | BRCA1 mutación | (-/-, +/-) | 0.0099 | ✅ → ✅ | ✗ MUERTE (Fase 1) |
| | TP53 mutación | (+/-, -/-) | 0.0100 | ✅ → ❌ | VIVO (vulnerable) |
| | Ambas mutaciones | (-/-, -/-) | 0.0001 | ✅ → ❌ | ✗ MUERTE (Fase 1) |
| | Sin mutación | (+/-, +/-) | 0.9801 | ✅ → ✅ | VIVO (degradado) |
| **(+/-, -/-)** | BRCA1 mutación | (-/-, -/-) | 0.0100 | ❌ → ❌ | ✗ MUERTE (Fase 1) |
| | Sin cambio | (+/-, -/-) | 0.9900 | ❌ → ❌ | VIVO (vulnerable) |

#### PUNTOS CLAVE:
1. ✅ PROTECCIÓN: status ≠ "-/-" (tanto +/+ como +/-)
2. 📈 INESTABILIDAD: Aumenta con TP53 +/-, aumenta más con TP53 -/-
3. ❌ VULNERABLE: Solo TP53 -/- permite transformación neoplástica
4. 🔄 CICLO: Por cada ciclo, ~1% de probabilidad de mutación de cada gen

---

## 2️⃣ CICLO DE VIDA CELULAR (6 Fases)

```
┌─────────────────────────────────────────────────────────────────┐
│                 CICLO DE VIDA - 6 FASES CELULARES                │
└─────────────────────────────────────────────────────────────────┘

                         ┌──────────────┐
                         │  FASE 0:     │
                         │  Baseline    │
                         │ Assessment   │
                         └──────┬───────┘
                                │
                                ↓
                    ┌──────────────────────┐
                    │  FASE 1:             │
                    │  G1 Integrity        │
                    │  Checkpoint          │
                    └──────────┬───────────┘
                               │
                    ┌──────────┴──────────┐
                    │                     │
                    ↓                     ↓
            [VIVO: BRCA1 ≠ -/-]   [MUERTE: BRCA1 = -/-]
                    │                     │
                    ↓                     ✗ EXCEPECIÓN
                    │                       CellDeathException
                    │
                    ↓
        ┌──────────────────────┐
        │  FASE 2:             │
        │  Endocytosis         │
        │  (Recibe mensajes)   │
        └──────────┬───────────┘
                   │
        ┌──────────┴──────────────────────┐
        │                                  │
        ↓                                  ↓
    [Recibe Apoptosis]        [Recibe otras señales]
        │                              │
   ┌────┴─────────────┐               │
   │                  │               │
   ↓                  ↓               │
[Acepta]         [Rechaza*]           │
  │                  │                │
  ↓                  ↓                │
MUERTE          CONTINÚA              │
  │                  │                │
  │    ┌─────────────┴────────────────┤
  │    │                              │
  │    ↓                              │
  │ ┌──────────────────────┐          │
  │ │  FASE 3:             │          │
  │ │  Nuclear Dynamics    │          │
  │ │  (Mutaciones)        │          │
  │ └──────────┬───────────┘          │
  │            │                      │
  │            ↓                      │
  │    [Genes mutan según         [Genes mutan según
  │     threshold + instability]   threshold + instability]
  │            │                      │
  └────────────┴──────────────────────┤
               │                      │
               ↓                      ↓
        ┌──────────────────────┐
        │  FASE 4:             │
        │  Cytoplasmic         │
        │  Remodeling          │
        └──────────┬───────────┘
                   │
        ┌──────────┴──────────────────────────┐
        │                                     │
        ↓                                     ↓
    [División?]                    [Transformación?]
   P(div)=x%                      neoplasm_k > threshold
        │                                     │
   ┌────┴──────────┐                   ┌──────┴────────┐
   │               │                   │               │
   ↓               ↓                   ↓               ↓
  SÍ              NO              TP53 ≠ -/-      TP53 = -/-
   │               │            (protegido)     (no protegido)
   ↓               ↓                   │               │
 CLONA         CONTINÚA         RECHAZA          TRANSFORMA
 HIJA           (sin div)     (protegido)      (neoplástica)
   │               │           tanto +/+             │
   │               │           como +/-)             │
   │               │                   │             ↓
   │               │                   │        EMITE SEÑAL
   │               │                   │        NeoplasmSignal
   │               │                   │             │
   │               └───────────────────┴─────────────┘
   │                                   │
   │                                   ↓
   │                    ┌──────────────────────┐
   │                    │  FASE 5:             │
   │                    │  Exocytosis          │
   │                    │  (Emite señales)     │
   │                    └──────────┬───────────┘
   │                               │
   └───────────────────────────────┤
                                   │
                                   ↓
                            ┌──────────────┐
                            │  CICLO        │
                            │  COMPLETO     │
                            │  (age++)      │
                            └──────────────┘
                                   │
                                   ↓
                          [Siguiente ciclo]


* Rechazo de apoptosis: Si genomic_instability > apoptosis_instability_threshold

---
```

## 2️⃣.5️⃣ UMBRAL DE EVASIÓN DE APOPTOSIS (CRÍTICO)

```
┌─────────────────────────────────────────────────────────────────┐
│        EVASIÓN DE APOPTOSIS - UMBRAL DE INESTABILIDAD            │
└─────────────────────────────────────────────────────────────────┘

PARÁMETRO GLOBAL: apoptosis_instability_threshold = 10.0

┌─────────────────────────────────────────┐
│  Fase 2: Endocitosis (Recibe Apoptosis)  │
└──────────────────┬──────────────────────┘
                   │
            ┌──────┴──────┐
            │             │
            ↓             ↓
   Si genomic_inst ≤ 10.0    Si genomic_inst > 10.0
            │                        │
            ↓                        ↓
    ✅ ACEPTA APOPTOSIS      ❌ EVASIÓN APOPTOSIS
            │                        │
            ↓                        ↓
        ✗ MUERTE                 VIVE → INMORTAL
      (Programada)            (se vuelve resistente)
      (célula normal)          (neoplástica)
```

#### VALIDACIÓN EXPERIMENTAL (Escenarios):

| Escenario | TP53 | high_delta | Descripción | Resultado |
|---|---|---|---|---|
| **04** | 0.1 | 1.0 | Año 0: inestability=1.0 (<10) ✅ apoptosis activa<br>Año 3-5: inestability~8-66 (>10) ❌ evasión | 100% neoplásticas inmortales |
| **07** | 0.005 | 1.0 | Crece lentamente, muchas células mantienen inestability<10 | Solo 2.5% neoplásticas (mejor control) |
| **09** | 0.01 | 1.0 | Balance intermedio | 4.7% neoplásticas (realista) |

**IMPLICACIÓN:**
La inestabilidad genómica progresiva (instability = instability² + delta) hace que:
1. Células con TP53 -/- rápidamente superen threshold (evasión)
2. Células con TP53 +/- aumentan más lentamente (apoptosis aún activa)
3. Células con TP53 +/+ casi nunca superen threshold (máxima protección)

---

## 2️⃣.6️⃣ EVOLUCIÓN EXPONENCIAL DE INESTABILIDAD GENÓMICA

```
┌─────────────────────────────────────────────────────────────────┐
│     DINÁMICA: inestability = inestability² + delta_progressivo   │
└─────────────────────────────────────────────────────────────────┘

FÓRMULA MATEMÁTICA:
───────────────────
I(t+1) = I(t)² + δ(TP53_status)

Donde:
  I(t) = inestabilidad en ciclo t
  δ = delta configurado por TP53 status:
      • TP53 +/+ → δ = 0.0
      • TP53 +/- → δ = low_delta (default: 0.5)
      • TP53 -/- → δ = high_delta (default: 1.0)
```

#### EJEMPLO: Escenario 04 con TP53 -/- (high_delta=1.0)

| Ciclo | Inestability | Cálculo | Estado vs Threshold (10.0) |
|---|---|---|---|
| 0 | 1.0000 | inicial | 1.0 < 10.0 ✅ Apoptosis activa |
| 1 | 1.0000²+1.0 | 1.0+1.0 = 2.0 | 2.0 < 10.0 ✅ Apoptosis activa |
| 2 | 2.0000²+1.0 | 4.0+1.0 = 5.0 | 5.0 < 10.0 ✅ Apoptosis activa |
| 3 | 5.0000²+1.0 | 25.0+1.0 = 26.0 | 26.0 > 10.0 ❌ EVASIÓN APOPTOSIS |
| 4 | 26.0000²+1.0 | 676.0+1.0 = 677.0 | 677.0 > 10.0 ❌ INMORTAL GARANTIZADO |
| 5 | SATURATION | muy alto | ❌ MUERE CÉLULA DE TODAS FORMAS |

**Nota:** Una vez que instability > 10.0, la célula neoplástica se vuelve INMORTAL

#### COMPARACIÓN DE VELOCIDADES:

| Escenario | TP53 Status | Delta | Dinámica |
|---|---|---|---|
| **04** | -/- | 1.0 | Ciclo 0: 1.0 → Ciclo 1: 2.0 → Ciclo 2: 5.0 → **Ciclo 3: 26.0** ❌ EVASIÓN RÁPIDA |
| **05** | -/- | 0.5 | Ciclo 0: 1.0 → Ciclo 1: 1.25 → Ciclo 2: 2.0625 → Ciclo 3: 4.75 → **Ciclo 4: 23.06** ❌ EVASIÓN MÁS LENTA |
| **07/09** | +/- | 1.0 | Ciclo 0: 1.0 → Ciclo 1: 1.5 → Ciclo 2: 2.75 → Ciclo 3: 8.06 → **Ciclo 4: 65.04** ❌ EVASIÓN AÚN MÁS LENTA |

#### VALIDACIÓN EXPERIMENTAL - VELOCIDAD DE EVASIÓN:

| Escenario | TP53 | Delta | Población (año 50) | % Inmortales | Observación |
|---|---|---|---|---|---|
| **04** | -/- | 1.0 | 462 vivas | 100% | RÁPIDA EVASIÓN |
| **05** | -/- | 0.5 | 573 vivas | 99.5% | EVASIÓN UN POCO MÁS LENTA |
| **07** | +/- | 1.0 | 484 vivas | 100% | PERO SIN MUTACIONES TP53 = PROTECCIÓN |
| **09** | +/- | 1.0 | 362 vivas | 94% | CON MUTACIONES TP53 = ALGUNAS EVASIONES |

---

## 3️⃣ DINÁMICA DE POBLACIÓN (Evolución temporal)

```
┌─────────────────────────────────────────────────────────────────┐
│           DINÁMICA DE POBLACIÓN - 6 ESCENARIOS                   │
└─────────────────────────────────────────────────────────────────┘

ESCENARIO 1: DEFAULT (μ_BRCA1=0.01, μ_TP53=0.01, div=0%)
───────────────────────────────────────────────────────

Población
    │
 50 │         ╱╲╱╲╱╲
    │        ╱  ╲  ╲ ╲
 40 │───────╱────╲──╲─╲──────────── VIVOS (blue)
    │      ╱      ╲  ╲ ╲
 30 │     ╱        ╲  ╲ ╲
    │    ╱          ╲  ╲ ╲
 20 │───╱────────────╲──╲─╲─────── NEOPLÁSTICAS (red)
    │  ╱              ╲  ╲ ╲
 10 │ ╱                ╲  ╲ ╲
    │╱                  ╲  ╲ ╲──── MUERTAS (gray)
  0 └─────┬─────┬─────┬─────┬──── Tiempo (años)
    0    10    20    30    40    50
    
Balance: población se mantiene con mutaciones graduales


ESCENARIO 2: NO_MUTATIONS (μ=0.0, div=0%)
─────────────────────────────────────

Población
    │
 50 │ ████████████████████████ VIVOS (blue)
    │ ████████████████████████
 40 │ ████████████████████████
    │ ████████████████████████
    │
  0 └─────┬─────┬─────┬─────┬──── Tiempo (años)
    0    10    20    30    40    50

Estable: población estática, sin cambios


ESCENARIO 3: HIGH_BRCA_APOPTOSIS (μ_BRCA1=0.5, μ_TP53=0.01, div=0%)
────────────────────────────────────────────────────────────────────

Población
    │
 50 │ ██ VIVOS (blue)
    │ ██╲
 40 │ ██ ╲
    │ ██  ╲
 30 │ ██   ╲
    │ ██    ╲
 20 │ ██     ╲
    │ ██      ╲ ████ NEOPLÁSTICAS (red)
 10 │ ██       ╲████╲
    │ ██        ████ ╲
  0 │ ██████████████ ╲████████ MUERTAS (gray)
    └─────┬─────┬─────┬─────┬──── Tiempo (años)
     0     2     4     6     8    10

Colapso: 99% de muertes en ~10 años


ESCENARIO 4: HIGH_TP53_MUTATION (μ_BRCA1=0.001, μ_TP53=0.3, div=0%)
──────────────────────────────────────────────────────────────────

Población
    │
 50 │ ▓▓▓▓  VIVOS (blue)
    │ ▓▓▓▓╲ ████ NEOPLÁSTICAS (red)
 40 │ ▓▓▓▓ ╲████╱
    │ ▓▓▓▓  ████╱
 30 │ ▓▓▓▓  ████╱
    │ ▓▓▓▓  ████
 20 │ ▓▓▓▓  ████
    │ ▓▓▓▓  ████ ▒▒▒▒ MUERTAS (gray)
 10 │ ▓▓▓▓  ████ ▒▒▒▒
    │ ▓▓▓▓  ████ ▒▒▒▒
  0 │ ▓▓▓▓████████▒▒▒▒ Tiempo (años)
    └─────┬─────┬─────┬─────┬──── Tiempo (años)
     0    10    20    30    40    50

Transformación: 60-70% neoplásticas en t=20 años


ESCENARIO 5: CELL_DIVISION_HEALTHY (μ=0.0, div=10%)
───────────────────────────────────────────────────

Población
    │
150 │                        ╱╱╱ VIVOS (blue)
    │                       ╱╱
100 │              ╱╱╱╱╱╱╱╱
    │             ╱╱
 50 │ ▓▓▓▓▓▓▓▓▓▓╱╱
    │ ▓▓▓▓▓▓▓▓
  0 └─────┬─────┬─────┬─────┬──── Tiempo (años)
    0    1    2    3    4    5

Exponencial: ~2.7x crecimiento en 5 años


ESCENARIO 6: REALISTIC_DIVISION (μ_BRCA1=0.01, μ_TP53=0.01, div=1%)
────────────────────────────────────────────────────────────────────

Población
    │
 80 │              ╱╱╱╱╱╱ VIVOS (blue)
    │             ╱╱
 60 │            ╱╱    ████ NEOPLÁSTICAS (red)
    │           ╱╱     ████╱
 40 │ ▓▓▓▓▓▓▓▓╱╱       ████╱
    │ ▓▓▓▓▓▓  ╱   ▒▒▒▒████╱ MUERTAS (gray)
 20 │ ▓▓▓▓  ╱     ▒▒▒▒╱
    │ ▓▓▓▓╱       ▒▒▒▒╱
  0 └─────┬─────┬─────┬─────┬──── Tiempo (años)
     0    10    20    30    40    50

Realista: crecimiento controlado + mutaciones progresivas
```

---

## 4️⃣ MATRIZ DE TRANSICIÓN (Probabilidades)

```
┌─────────────────────────────────────────────────────────────────┐
│         MATRIZ DE TRANSICIÓN - 2x3 GENES (BRCA1 + TP53)          │
└─────────────────────────────────────────────────────────────────┘

Notación: (BRCA1 state, TP53 state)

                      POR CICLO (Fase 3)
              ┌─────────────────────────────────────────────────┐
              │  Probabilidades de transición (threshold × coef) │
              └─────────────────────────────────────────────────┘

De estado →     A estado          Probabilidad       Evento
───────────────────────────────────────────────────────────────


(+/-,+/+)   →   (+/-,+/-)         μ_TP53             BRCA1 estable, TP53 mutación
            →   (-/-,+/+)         μ_BRCA1            BRCA1 mutación → MUERTE (Fase 1)
            →   (-/-,+/-)         μ_BRCA1 × μ_TP53   Ambas → MUERTE
            →   (+/-,+/+)         1 - Σμᵢ            Sin mutación (estado estable)

(+/-,+/-)   →   (-/-,+/-)         μ_BRCA1            BRCA1 mutación → MUERTE
            →   (+/-,-/-)         μ_TP53             TP53 mutación (vulnerable)
            →   (-/-,-/-)         μ_BRCA1 × μ_TP53   Ambas → MUERTE
            →   (+/-,+/-)         1 - Σμᵢ            Sin mutación (estado estable)

(+/-,-/-)   →   (-/-,-/-)         μ_BRCA1            BRCA1 mutación → MUERTE
            →   (+/-,-/-)         1 - μ_BRCA1        Sin mutación (estado estable)

```

#### ESCENARIO DEFAULT (threshold_BRCA1=0.01, threshold_TP53=0.01, genomic_instability=1.0):

| De Estado | A Estado | P(transición) | % | Descripción |
|---|---|---|---|---|
| **(+/-,+/+)** | (+/-,+/+) | 0.9801 | 98.01% | No mutation |
| | (+/-,+/-) | 0.0100 | 1.00% | TP53 mut |
| | (-/-,+/+) | 0.0099 | 0.99% | BRCA1 mut → MUERTE |
| | (-/-,+/-) | 0.0001 | 0.01% | Both mut → MUERTE |
| **(+/-,+/-)** | (+/-,+/-) | 0.9801 | 98.01% | No mutation |
| | (+/-,-/-) | 0.0100 | 1.00% | TP53 mut |
| | (-/-,+/-) | 0.0099 | 0.99% | BRCA1 mut → MUERTE |
| | (-/-,-/-) | 0.0001 | 0.01% | Both mut → MUERTE |
| **(+/-,-/-)** | (+/-,-/-) | 0.9900 | 99.00% | No BRCA1 mut |
| | (-/-,-/-) | 0.0100 | 1.00% | BRCA1 mut → MUERTE |

#### ESCENARIO HIGH_BRCA_APOPTOSIS (threshold_BRCA1=0.5, threshold_TP53=0.01, genomic_instability=1.0):

| De Estado | A Estado | P(transición) | % | Descripción |
|---|---|---|---|---|
| **(+/-,+/+)** | (+/-,+/+) | 0.4850 | 48.50% | No mutation |
| | (+/-,+/-) | 0.0100 | 1.00% | TP53 mut |
| | (-/-,+/+) | 0.5000 | 50.00% | BRCA1 mut → MUERTE |
| | (-/-,+/-) | 0.0050 | 0.50% | Both mut → MUERTE |

**RESULTADO:** 50% de muertes CADA CICLO

---

## 📈 Leyenda General

### Símbolos:
- `+/+` = Homocigoto dominante (ambos alelos funcionales)
- `+/-` = Heterocigoto (un alelo funcional, uno deficiente)
- `-/-` = Homocigoto recesivo (ambos alelos deficientes)
- `μᵢ` = Probabilidad de mutación del gen i
- `threshold` = Umbral de probabilidad base de mutación
- `genomic_instability` = Factor multiplicador de inestabilidad (1.0 en células normales, >1.0 en neoplásticas)

### Colores en diagramas (conceptual):
- 🔵 **AZUL**: Células vivas normales
- 🔴 **ROJO**: Células neoplásticas (transformadas)
- ⚪ **GRIS**: Células muertas

### Protección contra Neoplasia (CRÍTICO):
- **TP53 +/+** → ✅ PROTEGIDO, inestabilidad base
- **TP53 +/-** → ✅ PROTEGIDO (sigue siendo funcional), inestabilidad += low_delta
- **TP53 -/-** → ❌ NO PROTEGIDO (vulnerable a transformación), inestabilidad += high_delta

**Nota:** La "degradación progresiva" de TP53 +/- es por inestabilidad elevada, NO por pérdida de protección.
La transformación neoplástica solo ocurre cuando TP53 = -/- (completamente deficiente).

### Probabilidades:
- Todas las probabilidades de mutación se aplican **POR CICLO** (Fase 3)
- Pueden acumularse si la inestabilidad aumenta
- Son **independientes** entre genes (eventos aleatorios)

---

**Próximo paso:** Convertir estos diagramas ASCII a **Mermaid.js** para visualización en GitHub.

