# 📊 Diagramas del Simulador cellSim

## 1️⃣ FSM DE GENES (Finite State Machine)

### 🧬 Gen BRCA1: Modelo de Viabilidad

```
┌─────────────────────────────────────────────────────────────────┐
│                     FSM BRCA1 - VIABILIDAD                      │
└─────────────────────────────────────────────────────────────────┘

                    P(mutación) = threshold × genomic_instability
                                        ↓
        ┌──────────────┐           ┌──────────────┐
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
    ┌──────────────┐           ┌──────────────┐           ┌──────────────┐
    │    🟢 +/+    │ ────────→ │    🟡 +/-    │ ────────→ │    🔴 -/-    │
    │✅ PROTEGIDO  │   μ₁      │✅ PROTEGIDO  │   μ₂      │❌ VULNERABLE │
    │ inest=base   │ ────────→ │ inest=base   │ ────────→ │ inest=base   │
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
  • TP53 🟢 +/+ → inestabilidad = base (1.0)
  • TP53 🟡 +/- → inestabilidad += low_delta_instability (default: 0.5)
  • TP53 🔴 -/- → inestabilidad += high_delta_instability (default: 1.0)
  Nota: Estos valores pueden variar según el escenario (ver tabla de validación)

Estados de Protección contra Neoplasia:
  • 🟢 +/+ : ✅ PROTEGIDO (TP53 totalmente funcional)
  • 🟡 +/- : ✅ PROTEGIDO (TP53 parcialmente funcional, pero sigue siendo funcional)
  • 🔴 -/- : ❌ NO PROTEGIDO (TP53 deficiente, vulnerable a transformación neoplástica)

Consecuencias:
  • TP53 🟢 +/+ → Rechaza transformación neoplástica, inestabilidad baja
  • TP53 🟡 +/- → Rechaza transformación neoplástica, pero INESTABILIDAD AUMENTA (aumento moderado: +0.5)
  • TP53 🔴 -/- → PERMITE transformación neoplástica, inestabilidad aumenta más (aumento severo: +1.0)
```

---

### 🔗 FSM Combinado: BRCA1 + TP53 (Protección vs Inestabilidad)

```
┌─────────────────────────────────────────────────────────────────┐
│         FSM COMBINADO - 2D STATE SPACE (PROTECCIÓN)             │
└─────────────────────────────────────────────────────────────────┘
```

#### TP53 STATE - BRCA1 STATE MATRIX:

| BRCA1 STATE \ TP53 STATE | 🟢 +/+ | 🟡 +/- | 🔴 -/- |
|---|---|---|---|
| **🟢 +/-** | ✅ VIVO<br>Seguro<br>inest=1.0 | ✅ VIVO<br>Protegido<br>inest=1.0+0.5 | ❌ VIVO<br>Vulnerable<br>inest=1.0+1.0 |
| **🔴 -/-** | ✗ MUERTE<br>(Fase1) | ✗ MUERTE<br>(Fase1) | ✗ MUERTE<br>(Fase1) |

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

###  RESUMEN DEL FLUJO COMPLETO

```
ENTRADA: Célula viva en ciclo

┌─ FASE 0-1: ¿BRCA1 viable?
│  └─ NO → MUERTE (excepción)
│  └─ SÍ → continúa
│
├─ FASE 2: ¿Apoptosis aceptada?
│  └─ SÍ (instability ≤ 10) → MUERTE
│  └─ NO (instability > 10) → continúa
│
├─ FASE 3: Mutaciones (independiente)
│  └─ BRCA1 y TP53 pueden mutar
│
├─ FASE 4: Remodeling (Decisiones secuenciales)
│  ├─ PRIMERO: ¿Transformación? (TP53 check)
│  │  ├─ TP53 = -/- → TRANSFORMA (neoplástica) → Fase 5
│  │  └─ TP53 ≠ -/- → PROTEGIDO (normal) → SEGUNDO: División
│  │
│  └─ SEGUNDO: ¿División? (solo si normal)
│     ├─ P(div)% → Clona hija (edad=0)
│     └─ Sin división → Continúa normal
│
├─ FASE 5: Exocytosis
│  ├─ Células normales → Emite señales normales
│  └─ Células neoplásticas → Emite NeoplasmSignal
│
└─ SALIDA: age++, siguiente ciclo

```



### 2️⃣.1️⃣ SUBDIAGRAMA 1: CHECKPOINT INICIAL (Fases 0-1)

```
┌─────────────────────────────────────────────────────────────────┐
│              FASE 0 → FASE 1: INTEGRIDAD CELULAR                │
└─────────────────────────────────────────────────────────────────┘

    ┌──────────────┐         ┌──────────────┐     ┌─ BRCA1 ≠ -/-
    │  FASE 0:     │         │  FASE 1:     │     │  (VIVO) → CONTINÚA
    │  Baseline    │    →    │  G1 Integrity│  →  │
    │  Assessment  │         │  Checkpoint  │     └─ BRCA1 = -/-
    └──────────────┘         └──────────────┘        (MUERTE) → CellDeathException
```

✅ **Checkpoint BRCA1:** Verifica integridad del gen BRCA1
- **Si BRCA1 ≠ -/-:** Célula viable, continúa al siguiente checkpoint
- **Si BRCA1 = -/-:** Muerte garantizada → lanza excepción y sale del ciclo

---

### 2️⃣.2️⃣ SUBDIAGRAMA 2: DECISIÓN CRÍTICA (Fases 2-3)

```
┌─────────────────────────────────────────────────────────────────┐
│         FASE 2 → FASE 3: APOPTOSIS vs MUTACIÓN                  │
└─────────────────────────────────────────────────────────────────┘

                       ┌──────────────┐
                       │  FASE 2:     │
                       │  Endocytosis │
                       │  (Mensajes)  │
                       └──────┬───────┘
                              │
                   ┌──────────┴──────────┐
                   │                     │
                   ↓                     ↓
          [ApoptosisSignal]    [Otros signals]
                   │                     │
          ┌────────┴────────┐           │
          │                 │           │
          ↓                 ↓           ↓
      Instability      Instability    [sin decision]
       ≤ 10.0?          > 10.0?            │
          │                 │              │
      ✅ ACEPTA        ❌ RECHAZA          │
      APOPTOSIS       APOPTOSIS            │
          │                 │              │
          ↓                 ↓              ↓
        MUERTE           VIVE         CONTINÚA
      (Apoptosis)    (Evasión)     ┌────────────────┐
                                   │  FASE 3:       │
                                   │  Nuclear       │
                                   │  Dynamics      │
                                   │  (Mutaciones)  │
                                   └────────────────┘
```

⚠️ **Checkpoint Apoptosis (Fase 2):**
- Recibe señal de apoptosis → Evalúa instability genómica
- **Si instability ≤ 10.0:** Acepta apoptosis → MUERTE
- **Si instability > 10.0:** Rechaza apoptosis → EVASIÓN (sobrevive)

📊 **Mutaciones (Fase 3):** 
- Cada gen muta según: `P(mutación) = threshold × (1 + coef) × genomic_instability`
- Independiente de apoptosis

---

### 2️⃣.3️⃣ SUBDIAGRAMA 3: SALIDA Y DESTINO (Fases 4-5)

```
┌─────────────────────────────────────────────────────────────────┐
│     FASE 4 → FASE 5: REMODELING → EXOCYTOSIS → DESTINO         │
└─────────────────────────────────────────────────────────────────┘

                    ┌──────────────────────┐
                    │  FASE 4:             │
                    │  Cytoplasmic         │
                    │  Remodeling          │
                    └──────────┬───────────┘
                               │
                    ┌──────────┴──────────┐
                    │                     │
                    ↓                     ↓
        ╔════════════════════╗  ╔════════════════════╗
        ║  TP53 = -/-        ║  ║  TP53 ≠ -/-        ║
        ║  (NO PROTEGIDO)    ║  ║  (PROTEGIDO)       ║
        ╚════════╤═══════════╝  ╚════════╤═══════════╝
                 │                       │
                 ↓                       ↓
        ┌─────────────────┐    ┌──────────────────┐
        │ TRANSFORMA      │    │ ¿División?       │
        │ (Neoplástica)   │    │ P(div)=x%        │
        │                 │    └────────┬─────────┘
        │ INMORTALIZA     │             │
        │ EMITE           │    ┌────────┴────────┐
        │ NeoplasmSignal  │    │                 │
        │                 │    ↓                 ↓
        │                 │  🟢[SÍ]          🟢[NO]
        │                 │   │                 │
        │                 │   ↓                 ↓
        │                 │ 🔵 CLONA HIJA  🔵 CONTINÚA
        │                 │  (edad=0)      (sin div)
        └─────────┬───────┘    │                 │
                  │            └────────┬────────┘
                  │                    │
                  └────────────┬───────┘
                               │
                               ↓
                    ┌──────────────────────┐
                    │  FASE 5:             │
                    │  Exocytosis          │
                    │  (Emite señales)     │
                    └──────────┬───────────┘
                               │
                    ┌──────────┴──────────┐
                    │                     │
                    ↓                     ↓
        🔴[NeoplasmSignal]    🔵[Señales Normales]
        (Célula neoplástica) (Células normales)
                    │                     │
                    └──────────┬──────────┘
                               │
                               ↓
                    ┌──────────────────────┐
                    │  CICLO COMPLETO      │
                    │  age++               │
                    │  → Siguiente ciclo   │
                    └──────────────────────┘
```

🔄 **Salida (Fases 4-5) - DECISIONES SECUENCIALES:**

**RAMA 1: Si TP53 = -/- (NO PROTEGIDO)**
- ✗ **TRANSFORMA** en célula neoplástica
- **NO se divide** (es singular, inmortal)
- Emite **NeoplasmSignal** en Fase 5

**RAMA 2: Si TP53 ≠ -/- (PROTEGIDO)**
- **¿División?** (P=div%)
  - Sí → Crea **CLONA HIJA** (edad=0)
  - No → Continúa sin dividirse
- Emite **Señales Normales** en Fase 5

**Fase 5 - Exocytosis:** Convergen ambas ramas
- Emiten sus respectivas señales al tejido
- Completan ciclo: age++ → siguiente iteración

---


---

## 2️⃣.5️⃣ UMBRAL DE EVASIÓN DE APOPTOSIS (CRÍTICO)

```
┌─────────────────────────────────────────────────────────────────┐
│        EVASIÓN DE APOPTOSIS - UMBRAL DE INESTABILIDAD           │
└─────────────────────────────────────────────────────────────────┘

PARÁMETRO GLOBAL: apoptosis_instability_threshold = 10.0

┌─────────────────────────────────────────┐
│  Fase 2: Endocitosis (Recibe Apoptosis) │
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
        🔵 ✗ MUERTE             🔴 VIVE → INMORTAL
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
│     DINÁMICA: inestability = inestability² + delta_progressivo  │
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

### 📊 Comparación de 6 Escenarios

#### ESCENARIO 1: DEFAULT (μ_BRCA1=0.01, μ_TP53=0.01, div=0%)

| Años | Vivos | Neoplásticas | Muertas | % Neoplásticas |
|---|---|---|---|---|
| 0 | 50 | 0 | 0 | 0% |
| 10 | 40 | 5 | 5 | 11% |
| 20 | 35 | 12 | 3 | 25% |
| 30 | 32 | 15 | 3 | 32% |
| 40 | 30 | 18 | 2 | 37% |
| 50 | 28 | 20 | 2 | 42% |

**Descripción:** Balance biológico realista. Población se mantiene pero con aumento gradual de células neoplásticas por mutaciones lentas. Las muertas se remueven.

---

#### ESCENARIO 2: NO_MUTATIONS (μ=0.0, div=0%)

| Años | Vivos | Neoplásticas | Muertas | % Neoplásticas |
|---|---|---|---|---|
| 0 | 50 | 0 | 0 | 0% |
| 10 | 50 | 0 | 0 | 0% |
| 20 | 50 | 0 | 0 | 0% |
| 30 | 50 | 0 | 0 | 0% |
| 40 | 50 | 0 | 0 | 0% |
| 50 | 50 | 0 | 0 | 0% |

**Descripción:** Control perfecto. Sin mutaciones (μ=0), la población permanece completamente estable. Células viven indefinidamente sin transformación.

---

#### ESCENARIO 3: HIGH_BRCA_APOPTOSIS (μ_BRCA1=0.5, μ_TP53=0.01, div=0%)

| Años | Vivos | Neoplásticas | Muertas | % Neoplásticas |
|---|---|---|---|---|
| 0 | 50 | 0 | 0 | 0% |
| 2 | 10 | 1 | 39 | 9% |
| 4 | 2 | 0 | 48 | 0% |
| 6 | 0 | 0 | 50 | 0% |
| 10 | 0 | 0 | 50 | 0% |

**Descripción:** COLAPSO. Mutación BRCA1 muy agresiva (50% por ciclo). La mayoría muere en Fase 1 (BRCA1=-/-). Sistema biológico se colapsa en ~6 años. Pocas neoplásticas porque no hay tiempo.

---

#### ESCENARIO 4: HIGH_TP53_MUTATION (μ_BRCA1=0.001, μ_TP53=0.3, div=0%)

| Años | Vivos | Neoplásticas | Muertas | % Neoplásticas |
|---|---|---|---|---|
| 0 | 50 | 0 | 0 | 0% |
| 5 | 35 | 8 | 7 | 19% |
| 10 | 20 | 25 | 5 | 56% |
| 15 | 12 | 35 | 3 | 74% |
| 20 | 8 | 40 | 2 | 83% |
| 50 | 5 | 45 | 0 | 90% |

**Descripción:** INVASIÓN NEOPLÁSTICA. Mutación TP53 muy frecuente (30% por ciclo). Muchas células se transforman en neoplásticas. Población normal colapsa pero neoplásticas dominan.

---

#### ESCENARIO 5: CELL_DIVISION_HEALTHY (μ=0.0, div=10%)

| Años | Vivos | Neoplásticas | Muertas | Población Total |
|---|---|---|---|---|
| 0 | 50 | 0 | 0 | 50 |
| 1 | 55 | 0 | 0 | 55 |
| 2 | 67 | 0 | 0 | 67 |
| 3 | 82 | 0 | 0 | 82 |
| 4 | 101 | 0 | 0 | 101 |
| 5 | 123 | 0 | 0 | 123 |

**Descripción:** EXPLOSIÓN. Sin mutaciones pero con división (10%), la población crece exponencialmente ~2.7x cada año. Ideal para tejidos normales, catastrófico si fuera tumor.

---

#### ESCENARIO 6: REALISTIC_DIVISION (μ_BRCA1=0.01, μ_TP53=0.01, div=1%)

| Años | Vivos | Neoplásticas | Muertas | Población Total | % Neoplásticas |
|---|---|---|---|---|---|
| 0 | 50 | 0 | 0 | 50 | 0% |
| 10 | 55 | 3 | 2 | 58 | 5% |
| 20 | 62 | 8 | 1 | 70 | 11% |
| 30 | 68 | 14 | 1 | 82 | 17% |
| 40 | 74 | 20 | 1 | 94 | 21% |
| 50 | 80 | 27 | 1 | 107 | 25% |

**Descripción:** BALANCE REALISTA. Crecimiento lento (div=1%) con mutaciones (1% BRCA1+TP53). Población aumenta pero con presencia de neoplásticas. Simula tejido sano con transformación progresiva.

---

### 📈 Resumen Comparativo

| Escenario | Parámetros | Dinámica | Biología |
|---|---|---|---|
| **1. Default** | μ=0.01, div=0% | Estable con neoplásticas | Mutaciones lentas |
| **2. No Mut** | μ=0, div=0% | Completamente estable | Sistema perfecto |
| **3. High BRCA** | μ_B=0.5, div=0% | COLAPSO (~6 años) | Apoptosis masiva |
| **4. High TP53** | μ_TP=0.3, div=0% | Invasión neoplástica | Transformación rápida |
| **5. High Div** | μ=0, div=10% | Crecimiento exponencial | Proliferación normal |
| **6. Realistic** | μ=0.01, div=1% | Crecimiento + neoplásticas | Sistema biológico real |

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

### 🎨 Colores y Convenciones Visuales

**Para visualizar estos diagramas con colores, usa:**
- GitHub/GitLab renderiza automáticamente con colores en tablas
- CLion: Abre en navegador para ver colores HTML
- Terminal: Instala `mdcat` para colores ANSI

#### Estados Celulares:
- 🔵 **AZUL** - Células vivas normales (TP53 ≠ -/-, BRCA1 ≠ -/-)
- 🔴 **ROJO** - Células neoplásticas (TP53 = -/-, transformadas)
- ⚪ **GRIS** - Células muertas (removidas del tejido)
- 🟡 **AMARILLO** - Células con mutaciones parciales (TP53 +/-, BRCA1 ≠ -/-)

#### Genes y Estados:
- ✅ **VERDE** - Protegido/Funcional (BRCA1 ≠ -/-, TP53 ≠ -/-)
- ❌ **ROJO** - Vulnerable/Deficiente (BRCA1 = -/-, TP53 = -/-)
- ⚠️ **NARANJA** - Degradado/Parcial (heterocigoto +/-)

#### Procesos Biológicos:
- 🔄 **CICLO** - Celular (6 fases)
- ↓ **FLUJO** - Progresión lógica
- ├─ **BIFURCACIÓN** - Decisión/Checkpoint
- ✗ **MUERTE** - Apoptosis o excepción
- ✓ **ACEPTACIÓN** - Proceso continúa

#### Umbrales Críticos:
- `instability ≤ 10.0` → ✅ Acepta apoptosis (MUERTE)
- `instability > 10.0` → ❌ Rechaza apoptosis (EVASIÓN → INMORTAL)

---

### 📊 Convención en Tablas Poblacionales

| Color | Tipo Celular | Símbolo | Ejemplo |
|---|---|---|---|
| 🔵 Azul | Vivas normales | ████ | Escenario 1: 40 vivas |
| 🔴 Rojo | Neoplásticas | ████ | Escenario 4: 40 neoplásticas |
| ⚪ Gris | Muertas | ▓▓▓▓ | Escenario 3: 39 muertas |
| 🟡 Amarillo | Transición (+/-) | ░░░░ | Escenario 6: 10 en transición |

---

### 🧬 Código de Genes

| Gen | Estado | Protección | Inestabilidad | Símbolo |
|---|---|---|---|---|
| **BRCA1** | +/- | ✅ Funcional | Base | 🟢 |
| **BRCA1** | -/- | ❌ Deficiente | Base | 🔴 |
| **TP53** | +/+ | ✅ Total | Base (1.0) | 🟢 |
| **TP53** | +/- | ✅ Parcial | +0.5 (bajo) | 🟡 |
| **TP53** | -/- | ❌ Nulo | +1.0 (alto) | 🔴 |

---

### 🎯 Puntos Clave de Interpretación

1. **Protección contra Neoplasia:**
   - TP53 +/+ o +/- → ✅ PROTEGIDO (rechaza transformación)
   - TP53 = -/- → ❌ VULNERABLE (permite transformación)

2. **Inestabilidad Genómica:**
   - Aumenta cuadráticamente: `I(t+1) = I(t)² + δ`
   - Umbral crítico: 10.0 (evasión de apoptosis)

3. **Mutaciones:**
   - ~1% por ciclo en DEFAULT
   - Pueden acumularse con inestabilidad

4. **División Celular:**
   - Solo células normales (TP53 ≠ -/-)
   - Neoplásticas NO se dividen (inmortales pero singulares)

5. **Ciclo de Vida:**
   - 6 fases secuenciales
   - Checkpoints críticos en Fases 1, 2, 4

---

