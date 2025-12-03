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
    │  (protegido) │ ────────→ │ (reducido)   │ ────────→ │ (vulnerable) │
    │              │           │              │           │              │
    └──────────────┘           └──────────────┘           └──────────────┘
     (INICIAL)                                         (NEOPLASIAS ↑)

Matemática:
──────────
P(+/+ → +/-) = μ₁ = threshold_TP53 × (1 + k_TP53) × genomic_instability
P(+/- → -/-) = μ₂ = (threshold_TP53 + Δk) × (1 + k_TP53) × genomic_instability

Nota: Δk depende del escenario (normalmente pequeño)

Estados:
  • +/+ : Protegido (TP53 totalmente funcional)
  • +/- : Reducido (TP53 parcialmente funcional, riesgo moderado)
  • -/- : Vulnerable (TP53 deficiente, alto riesgo de neoplasia)

Consecuencias:
  • TP53 +/+ → Rechaza transformación neoplástica
  • TP53 +/- → Riesgo moderado de neoplasia (threshold elevado)
  • TP53 -/- → Vulnerable a neoplasia (threshold bajo)
```

---

### 🔗 FSM Combinado: BRCA1 + TP53

```
┌─────────────────────────────────────────────────────────────────┐
│              FSM COMBINADO - 2D STATE SPACE                      │
└─────────────────────────────────────────────────────────────────┘

                              TP53 STATE
                         +/+  │  +/-  │  -/-
                    ─────┼────┼────┼────┼─────
                         │    │    │    │
BRCA1    +/-    │ VIVO   │ VIVO  │ VIVO
STATE           │ Seg.   │ Riesgo│ Riesgo
                │        │ Mod.  │ Alto
                ├────────┼────────┼────────
                │        │        │
          -/-   │ MUERTE │ MUERTE │ MUERTE
                │ (Fase1)│ (Fase1)│ (Fase1)
                │        │        │


Tabla de Transiciones (por ciclo):
───────────────────────────────────

Estado Inicial          Evento Fase 3               Estado Final    Destino
─────────────────────────────────────────────────────────────────────────────
BRCA1(+/-) + TP53(+/+)  • BRCA1 mutación (μ₁)     → BRCA1(-/-)    → MUERTE
                        • TP53 mutación (μ₁)      → TP53(+/-)     → VIVO*
                        • Ambas (μ₁×μ₂)           → BRCA1(-/-) TP53(+/-) → MUERTE

BRCA1(+/-) + TP53(+/-)  • BRCA1 mutación (μ₁)     → BRCA1(-/-)    → MUERTE
                        • TP53 mutación (μ₂)      → TP53(-/-)     → VIVO (Riesgo!)
                        • Ambas (μ₁×μ₂)           → BRCA1(-/-) TP53(-/-) → MUERTE

BRCA1(+/-) + TP53(-/-)  • BRCA1 mutación (μ₁)     → BRCA1(-/-)    → MUERTE
                        • TP53 sin cambio         → Permanece     → VIVO (Vulnerable)
                        • Nada                    → Permanece     → VIVO (Vulnerable)


* VIVO: Célula sobrevive Fase 1 (BRCA1 aún +/-)
```

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
   ┌────┴──────────┐                   ┌──────┴──────┐
   │               │                   │             │
   ↓               ↓                   ↓             ↓
  SÍ              NO              [TP53=+/+]   [TP53=-/-]
   │               │                   │             │
   ↓               ↓                   ↓             ↓
 CLONA         CONTINÚA         RECHAZA      TRANSFORMA
 HIJA           (sin div)        (muere)     (neoplástica)
   │               │                   │             │
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
```

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


ESCENARIO DEFAULT (threshold_BRCA1=0.01, threshold_TP53=0.01, genomic_instability=1.0):
────────────────────────────────────────────────────────────────────────────────────

                    De estado           A estado           P(transición)    %
                    ──────────────────────────────────────────────────────────
                    (+/-,+/+)     →     (+/-,+/+)          0.9801          98.01%  [No mutation]
                                  →     (+/-,+/-)          0.0100          1.00%   [TP53 mut]
                                  →     (-/-,+/+)          0.0099          0.99%   [BRCA1 mut → MUERTE]
                                  →     (-/-,+/-)          0.0001          0.01%   [Both mut → MUERTE]

                    (+/-,+/-)     →     (+/-,+/-)          0.9801          98.01%  [No mutation]
                                  →     (+/-,-/-)          0.0100          1.00%   [TP53 mut]
                                  →     (-/-,+/-)          0.0099          0.99%   [BRCA1 mut → MUERTE]
                                  →     (-/-,-/-)          0.0001          0.01%   [Both mut → MUERTE]

                    (+/-,-/-)     →     (+/-,-/-)          0.9900          99.00%  [No BRCA1 mut]
                                  →     (-/-,-/-)          0.0100          1.00%   [BRCA1 mut → MUERTE]


ESCENARIO HIGH_BRCA_APOPTOSIS (threshold_BRCA1=0.5, threshold_TP53=0.01, genomic_instability=1.0):
──────────────────────────────────────────────────────────────────────────────────────────────────

                    De estado           A estado           P(transición)    %
                    ──────────────────────────────────────────────────────────

                    (+/-,+/+)     →     (+/-,+/+)          0.4850          48.50%  [No mutation]
                                  →     (+/-,+/-)          0.0100          1.00%   [TP53 mut]
                                  →     (-/-,+/+)          0.5000          50.00%  [BRCA1 mut → MUERTE]
                                  →     (-/-,+/-)          0.0050          0.50%   [Both mut → MUERTE]
                    
                    → RESULTADO: 50% de muertes CADA CICLO
```

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

### Probabilidades:
- Todas las probabilidades de mutación se aplican **POR CICLO** (Fase 3)
- Pueden acumularse si la inestabilidad aumenta
- Son **independientes** entre genes (eventos aleatorios)

---

**Próximo paso:** Convertir estos diagramas ASCII a **Mermaid.js** para visualización en GitHub.

