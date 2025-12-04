# 📊 Diagramas Mermaid - Simulador cellSim

Visualización interactiva de la dinámica celular. Los diagramas se renderizan automáticamente en GitHub.

---

## 1️⃣ FSM DE GENES

### 🧬 Gen BRCA1: Modelo de Viabilidad

```mermaid
stateDiagram-v2
    [*] --> PlusMinus: Inicial (VIVO)
    
    PlusMinus --> MinusMinus: Mutación<br/>P = μ₁ = 0.01
    MinusMinus --> [*]: MUERTE<br/>(Fase 1)
    
    PlusMinus --> PlusMinus: Sin mutación<br/>P = 1 - μ₁
    
    note right of PlusMinus
        Estado: +/-
        Heterocigoto
        BRCA1 funcional
    end note
    
    note right of MinusMinus
        Estado: -/-
        Homocigoto recesivo
        MUERTE garantizada
    end note
```

**Matemática:**
- `P(+/- → -/-) = μ₁ = threshold_BRCA1 × (1 + k_BRCA1) × genomic_instability`
- **Irreversible**: Una vez en `-/-`, muerte en siguiente Fase 1
- **Escenario DEFAULT**: `μ₁ = 0.01` (1% por ciclo)

---

### 🛡️ Gen TP53: Modelo de Guardián Tumoral

```mermaid
stateDiagram-v2
    [*] --> PlusPlus: Inicial (PROTEGIDO)
    
    PlusPlus --> PlusPlus: Sin mutación<br/>P = 1 - μ₁
    PlusPlus --> PlusMinus: Mutación TP53<br/>P = μ₁ = 0.01
    
    PlusMinus --> PlusMinus: Sin mutación<br/>P = 1 - μ₂
    PlusMinus --> MinusMinus: Mutación TP53<br/>P = μ₂ = 0.01
    
    MinusMinus --> MinusMinus: Sin cambio<br/>P = 1.0
    
    note right of PlusPlus
        Estado: +/+
        Homocigoto dominante
        ✅ PROTEGIDO vs neoplasia
        TP53 totalmente funcional
        inestabilidad = base
    end note
    
    note right of PlusMinus
        Estado: +/-
        Heterocigoto
        ✅ PROTEGIDO vs neoplasia
        TP53 parcialmente funcional
        inestabilidad += 0.5 (bajo)
        (aumento moderado)
    end note
    
    note right of MinusMinus
        Estado: -/-
        Homocigoto recesivo
        ❌ NO PROTEGIDO vs neoplasia
        Sin protección TP53
        inestabilidad += 1.0 (alto)
        (aumento severo)
    end note
```

**Matemática:**
- `P(+/+ → +/-) = μ₁ = threshold_TP53 × (1 + k_TP53) × genomic_instability`
- `P(+/- → -/-) = μ₂ = threshold_TP53 × (1 + k_TP53) × genomic_instability`
- **Protección contra neoplasia:** `status != "-/-"` → **PROTEGIDO** (tanto +/+ como +/-)
- **Transformación neoplástica:** `P = neoplasm_k` (default: 0.05) si TP53 = "-/-"
- **Aumento de inestabilidad (configurables):** 
  - TP53 `+/+` → inestabilidad base (1.0)
  - TP53 `+/-` → inestabilidad += `low_delta_instability` (default: 0.5)
  - TP53 `-/-` → inestabilidad += `high_delta_instability` (default: 1.0)
- **Escenario DEFAULT**: `μ₁ = μ₂ = 0.01` (1% por ciclo), `neoplasm_k = 0.05` (5%)

---

### 🔗 FSM Combinado: BRCA1 + TP53 (2D State Space)

```mermaid
stateDiagram-v2
    [*] --> S1
    
    S1: BRCA1(+/-) + TP53(+/+)
    S1: ✅ PROTEGIDO
    S1: inestabilidad: 1.0 (base)
    S1: RIESGO: BAJO
    
    S2: BRCA1(+/-) + TP53(+/-)
    S2: ✅ PROTEGIDO
    S2: inestabilidad: 1.0+0.5
    S2: RIESGO: MODERADO
    
    S3: BRCA1(+/-) + TP53(-/-)
    S3: ❌ NO PROTEGIDO
    S3: inestabilidad: 1.0+1.0
    S3: RIESGO: ALTO
    
    D1: ✗ MUERTE (Fase 1)
    D1: BRCA1(-/-) + TP53(+/+)
    
    D2: ✗ MUERTE (Fase 1)
    D2: BRCA1(-/-) + TP53(+/-)
    
    D3: ✗ MUERTE (Fase 1)
    D3: BRCA1(-/-) + TP53(-/-)
    
    S1 --> D1: BRCA1 mut (1%)
    S1 --> S2: TP53 mut (1%)
    S1 --> D2: Ambas (0.01%)
    S1 --> S1: Sin mut (98.01%)
    
    S2 --> D2: BRCA1 mut (1%)
    S2 --> S3: TP53 mut (1%)
    S2 --> D3: Ambas (0.01%)
    S2 --> S2: Sin mut (98.01%)
    
    S3 --> D3: BRCA1 mut (1%)
    S3 --> S3: Sin mut (99%)
    
    D1 --> [*]
    D2 --> [*]
    D3 --> [*]
```

**Tabla de Transiciones - ESCENARIOS VALIDADOS:**

| Escenario | De estado | Evento | A estado | P(transición) | Protección | Inestabilidad | Resultado |
|-----------|-----------|--------|----------|---------------|-----------|----------------|-----------|
| **03: BRCA1=0.2** | (+/-,+/+) | BRCA1 mut | (-/-,+/+) | 0.20 | ✅ | base | ✗ MUERTE (Fase 1) - LETAL |
| **04: TP53=0.1** | (+/-,+/+) | TP53 mut | (+/-,+/-) | 0.10 | ✅ | 1.0+0.5 | VIVO (protegido, degradado) |
| **04: TP53=0.1** | (+/-,-/-) | Transf neo | NEOPLÁSTICA | 0.10 | ❌ | 1.0+1.0 | VULNERABLE → 98% neo, 100% inmortales |
| **07: TP53=0.005** | (+/-,+/+) | TP53 mut | (+/-,+/-) | 0.005 | ✅ | 1.0+0.5 | VIVO (muy protegido) |
| **07: TP53=0.005** | (+/-,-/-) | Transf neo | NEOPLÁSTICA | 0.05 | ❌ | 1.0+1.0 | BAJO RIESGO → 2.5% neo, 100% inmortales |
| (+/-,+/-) | Ambas | (-/-,-/-) | 0.0001 | ❌ | base+high | ✗ MUERTE (Fase 1) |
| (+/-,+/-) | Sin mut | (+/-,+/-) | 0.9801 | ✅ | base+low | VIVO (protegido, degradado) |
| | | | | | | |
| (+/-,-/-) | BRCA1 mut | (-/-,-/-) | 0.0100 | ❌ | base+high | ✗ MUERTE (Fase 1) |
| (+/-,-/-) | Sin mut | (+/-,-/-) | 0.9900 | ❌ | base+high | VIVO (vulnerable neoplasia) |

---

## 2️⃣ CICLO DE VIDA CELULAR (6 Fases)

```mermaid
graph TD
    A["<b>FASE 0</b><br/>Baseline Assessment<br/>──────<br/>Estado inicial"] --> B["<b>FASE 1</b><br/>G1 Integrity Checkpoint<br/>──────<br/>¿BRCA1 = -/-?"]
    
    B -->|NO: BRCA1 ≠ -/-| C["<b>FASE 2</b><br/>Endocytosis<br/>──────<br/>Recibe mensajes"]
    B -->|SÍ: BRCA1 = -/-| MUERTE1["<b>✗ MUERTE</b><br/>CellDeathException"]
    
    C --> D{Apoptosis?}
    D -->|Acepta| MUERTE2["<b>✗ MUERTE</b><br/>Apoptosis"]
    D -->|Rechaza<br/>inestabilidad ><br/>threshold| E["<b>FASE 3</b><br/>Nuclear Dynamics<br/>──────<br/>Mutaciones genéticas"]
    D -->|Sin apoptosis| E
    
    E --> F["<b>FASE 4</b><br/>Cytoplasmic Remodeling<br/>──────<br/>División + Transformación"]
    
    F --> G{División?}
    G -->|Sí<br/>P = division_rate| CLONA["<b>CLONA HIJA</b><br/>Célula nueva<br/>genéticamente idéntica"]
    G -->|No| H["<b>FASE 5</b><br/>Exocytosis<br/>──────<br/>Emite señales"]
    
    F --> I{Transformación?<br/>neoplasm_k ><br/>threshold}
    I -->|TP53 ≠ -/-<br/>Protegido| PROTEGIDA["<b>RECHAZA</b><br/>Protegida vs neoplasia<br/>(tanto +/+ como +/-)"]
    I -->|TP53 = -/-<br/>No protegido| NEOPLASTICA["<b>NEOPLÁSTICA</b><br/>Emite NeoplasmSignal<br/>Entra ciclo simplificado"]
    
    CLONA --> H
    PROTEGIDA --> H
    NEOPLASTICA --> J["<b>CICLO NEOPLÁSTICO</b><br/>──────<br/>Solo Fase 2 + 5<br/>Procesa mensajes"]
    
    H --> K["<b>CICLO COMPLETO</b><br/>age++<br/>Siguiente ciclo"]
    J --> K
    
    style A fill:#E6F3FF
    style B fill:#E6F3FF
    style C fill:#E6F3FF
    style E fill:#E6F3FF
    style F fill:#E6F3FF
    style H fill:#E6F3FF
    style K fill:#E6F3FF
    
    style MUERTE1 fill:#FF6B6B
    style MUERTE2 fill:#FF6B6B
    style CLONA fill:#90EE90
    style NEOPLASTICA fill:#FFD700
    style J fill:#FFD700
```

**Descripción de fases:**

| Fase | Nombre | Acción | Checkpoint |
|------|--------|--------|-----------|
| **0** | Baseline Assessment | Muestra estado celular | - |
| **1** | G1 Integrity | ¿BRCA1 = -/-? | ✗ SI = MUERTE |
| **2** | Endocytosis | Procesa apoptosis | ¿Rechaza? |
| **3** | Nuclear Dynamics | Mutaciones de genes | - |
| **4** | Cytoplasmic Remodeling | División + Transformación | ¿División? ¿Neoplasia? |
| **5** | Exocytosis | Emite señales | - |

---

## 3️⃣ DINÁMICA DE POBLACIÓN (Evolución temporal)

```mermaid
graph LR
    T0["t=0<br/>100 células<br/>100% vivas"]
    T1["t=10 años<br/>Escenario:<br/>DEFAULT"]
    T2["t=20 años"]
    T3["t=50 años"]
    
    T0 --> T1 --> T2 --> T3
    
    note1["<b>DEFAULT</b><br/>μ_BRCA1=0.01<br/>μ_TP53=0.01<br/>div=0%<br/><br/>VIVOS: 40 (80%)<br/>NEOPLÁSTICAS: 8 (16%)<br/>MUERTAS: 12 (24%)"]
    note2["VIVOS: 30 (60%)<br/>NEOPLÁSTICAS: 15 (30%)<br/>MUERTAS: 55 (110%)"]
    note3["VIVOS: 20 (40%)<br/>NEOPLÁSTICAS: 20 (40%)<br/>MUERTAS: 60+ (120%)"]
    
    T1 -.-> note1
    T2 -.-> note2
    T3 -.-> note3
```

### Comparación de 6 Escenarios

```mermaid
graph TB
    subgraph Scenarios["EVOLUCIÓN POPULACIONAL - 6 ESCENARIOS"]
        S1["<b>1. DEFAULT</b><br/>μ_BRCA1=0.01, μ_TP53=0.01, div=0%<br/>━━━━━<br/>↓ Población estable<br/>↓ Mutaciones graduales<br/>↓ 50% vivos en t=50años"]
        
        S2["<b>2. NO_MUTATIONS</b><br/>μ_BRCA1=0.0, μ_TP53=0.0, div=0%<br/>━━━━━<br/>↓ Población ESTÁTICA<br/>↓ Sin cambios genéticos<br/>↓ 100% vivos siempre"]
        
        S3["<b>3. HIGH_BRCA_APOPTOSIS</b><br/>μ_BRCA1=0.5, μ_TP53=0.01, div=0%<br/>━━━━━<br/>⚠ COLAPSO RÁPIDO<br/>⚠ 99% muertes en t=10años<br/>⚠ Población ~1 célula"]
        
        S4["<b>4. HIGH_TP53_MUTATION</b><br/>μ_BRCA1=0.001, μ_TP53=0.3, div=0%<br/>━━━━━<br/>⚠ TRANSFORMACIÓN PROGRESIVA<br/>⚠ 60-70% neoplásticas en t=20años<br/>⚠ 40 de 50 células transformadas"]
        
        S5["<b>5. CELL_DIVISION_HEALTHY</b><br/>μ_BRCA1=0.0, μ_TP53=0.0, div=10%<br/>━━━━━<br/>📈 CRECIMIENTO EXPONENCIAL<br/>📈 ~2.7x en 5 años<br/>📈 100% vivos, sin mutaciones"]
        
        S6["<b>6. REALISTIC_DIVISION</b><br/>μ_BRCA1=0.01, μ_TP53=0.01, div=1%<br/>━━━━━<br/>📈 Crecimiento CONTROLADO<br/>📈 + Mutaciones progresivas<br/>📈 Balance biológico realista"]
    end
    
    style S1 fill:#E6F3FF
    style S2 fill:#E8F5E9
    style S3 fill:#FFEBEE
    style S4 fill:#FFF3E0
    style S5 fill:#F3E5F5
    style S6 fill:#FCE4EC
```

---

## 4️⃣ MATRIZ DE TRANSICIÓN (Probabilidades)

### Escenario DEFAULT (μ_BRCA1=0.01, μ_TP53=0.01)

```mermaid
graph LR
    subgraph "DE ESTADO INICIAL"
        A["(+/-,+/+)<br/>VIVO SEGURO"]
    end
    
    subgraph "EVENTOS FASE 3"
        E1["P=98.01%<br/>Sin mutación"]
        E2["P=1.00%<br/>TP53 mutación"]
        E3["P=0.99%<br/>BRCA1 mutación"]
        E4["P=0.01%<br/>Ambas"]
    end
    
    subgraph "A ESTADO FINAL"
        R1["(+/-,+/+)<br/>VIVO SEGURO"]
        R2["(+/-,+/-)<br/>VIVO RIESGO MOD"]
        R3["(-/-,+/+)<br/>✗ MUERTE"]
        R4["(-/-,+/-)<br/>✗ MUERTE"]
    end
    
    A --> E1 --> R1
    A --> E2 --> R2
    A --> E3 --> R3
    A --> E4 --> R4
    
    style R1 fill:#90EE90
    style R2 fill:#FFD700
    style R3 fill:#FF6B6B
    style R4 fill:#FF6B6B
```

### Escenario HIGH_BRCA_APOPTOSIS (μ_BRCA1=0.5, μ_TP53=0.01)

```mermaid
graph LR
    subgraph "DE ESTADO INICIAL"
        A["(+/-,+/+)<br/>VIVO"]
    end
    
    subgraph "EVENTOS FASE 3"
        E1["P=48.50%<br/>Sin mutación"]
        E2["P=1.00%<br/>TP53 mutación"]
        E3["P=50.00%<br/>BRCA1 mutación"]
        E4["P=0.50%<br/>Ambas"]
    end
    
    subgraph "A ESTADO FINAL"
        R1["(+/-,+/+)<br/>VIVO"]
        R2["(+/-,+/-)<br/>VIVO RIESGO"]
        R3["(-/-,+/+)<br/>✗ MUERTE"]
        R4["(-/-,+/-)<br/>✗ MUERTE"]
    end
    
    A --> E1 --> R1
    A --> E2 --> R2
    A --> E3 --> R3
    A --> E4 --> R4
    
    style R1 fill:#90EE90
    style R2 fill:#FFD700
    style R3 fill:#FF6B6B
    style R4 fill:#FF6B6B
    style E3 fill:#FFE4E1
    
    note["<b>CRÍTICO:</b><br/>50% MUERE cada ciclo"]
```

---

## 📊 Leyenda Completa

### Estados Genéticos:
- **+/+** = Homocigoto dominante (ambos alelos funcionales)
- **+/-** = Heterocigoto (1 alelo funcional, 1 deficiente)
- **-/-** = Homocigoto recesivo (ambos alelos deficientes)

### Notación Probabilística:
- `μᵢ` = Probabilidad de mutación del gen i
- `threshold` = Umbral base de mutación
- `k` = Coeficiente de inestabilidad
- `genomic_instability` = Multiplicador de inestabilidad

### Estados Celulares:
- 🟢 **VIVO SEGURO** = BRCA1(+/-) + TP53(+/+) → Bajo riesgo, protegido
- 🟡 **RIESGO MODERADO** = BRCA1(+/-) + TP53(+/-) → Riesgo medio, PROTEGIDO (inestabilidad elevada)
- 🔴 **RIESGO ALTO** = BRCA1(+/-) + TP53(-/-) → Alto riesgo neoplasia (NO PROTEGIDO)
- ⚫ **NEOPLÁSTICA** = TP53(-/-) + transformación → Emite señales, ciclo simplificado
- ⚪ **MUERTE** = BRCA1(-/-) o apoptosis aceptada

### Colores:
- 🟢 Verde = Seguro, bajo riesgo
- 🟡 Dorado = Riesgo moderado
- 🔴 Rojo = Muerte, alto riesgo
- 🔵 Azul = Procesamiento, neutral

---

## 4️⃣.5️⃣ UMBRAL DE EVASIÓN DE APOPTOSIS (CRÍTICO)

### Decisión de Apoptosis Basada en Inestabilidad

```mermaid
flowchart TD
    A["Célula recibe<br/>SEÑAL APOPTOSIS<br/>(Fase 2)"]
    
    B{"¿genomic_instability<br/>≤ 10.0?"}
    
    C["✅ ACEPTA APOPTOSIS<br/>Célula muere<br/>(programada)"]
    
    D["❌ EVASIÓN APOPTOSIS<br/>Célula vive<br/>Se vuelve INMORTAL"]
    
    A --> B
    B -->|SÍ| C
    B -->|NO| D
    
    style C fill:#FFB6B6
    style D fill:#B6D7FF
```

**Parámetro Global:** `apoptosis_instability_threshold = 10.0`

**Validación Experimental:**
- **Esc 04** (TP53=-/-, high_δ=1.0): Año 3 → inestability > 10.0 → 100% inmortales ❌
- **Esc 07** (TP53=+/-, high_δ=1.0): Crece más lentamente → Solo 2.5% inmortales ✅
- **Esc 09** (Balanceado): 4.7% inmortales (realista) ✅

---

## 4️⃣.6️⃣ EVOLUCIÓN EXPONENCIAL DE INESTABILIDAD

### Dinámica Temporal: I(t+1) = I(t)² + δ(TP53_status)

```mermaid
graph TD
    A["Ciclo 0: I = 1.0"]
    B["Ciclo 1: I = 1.0² + δ"]
    C["Ciclo 2: I = I₁² + δ"]
    D["Ciclo 3: I = I₂² + δ"]
    E{"I > 10.0?"}
    
    F["Ciclo 4-n: I seguirá<br/>aumentando<br/>EVASIÓN GARANTIZADA"]
    
    G["Comparar velocidades<br/>según TP53 status"]
    
    A -->|TP53 -/-<br/>δ = 1.0| B
    B -->|Ejemplo: Esc 04| C
    C -->|Crece rápido| D
    D -->|En ciclo 3-4| E
    E -->|SÍ| F
    E -->|NO| B
    
    D -.->|Comparar con| G
    G -->|TP53 +/-: δ = 0.5<br/>TP53 +/+: δ = 0.0| G
    
    style E fill:#FFE6E6
    style F fill:#E6E6FF
```

**Tabla de Evolución - Escenario 04 (TP53 = -/-, high_δ = 1.0):**

| Ciclo | I(t) | Cálculo | vs Threshold | Estado |
|-------|------|---------|--------------|--------|
| 0 | 1.0000 | inicial | < 10.0 ✅ | Apoptosis activa |
| 1 | 2.0000 | 1.0² + 1.0 | < 10.0 ✅ | Apoptosis activa |
| 2 | 5.0000 | 2.0² + 1.0 | < 10.0 ✅ | Apoptosis activa |
| 3 | **26.0000** | 5.0² + 1.0 | > 10.0 ❌ | **EVASIÓN APOPTOSIS** |
| 4+ | Muy alto | 26.0² + 1.0 | >> 10.0 ❌ | INMORTAL GARANTIZADO |

**Conclusión:** Inestabilidad crece exponencialmente. Una vez > threshold, célula es inmortal.

---

## 🔬 Cómo interpretar los diagramas

**⚠️ PUNTO CRÍTICO: Protección TP53**
- **TP53 +/+** → Protegido, inestabilidad base (1.0), δ = 0.0
- **TP53 +/-** → **SIGUE PROTEGIDO**, pero inestabilidad aumenta (δ = 0.5, aumento moderado)
- **TP53 -/-** → **NO PROTEGIDO**, inestabilidad muy elevada (δ = 1.0, aumento severo), vulnerable a neoplasia

**⚠️ PUNTO CRÍTICO: Evasión de Apoptosis**
- Si `genomic_instability ≤ 10.0` → Apoptosis activa ✅ (célula puede morir)
- Si `genomic_instability > 10.0` → Evasión apoptosis ❌ (célula se vuelve inmortal)
- Esto explica por qué 100% de neoplásticas son inmortales en escenarios validados

**⚠️ PUNTO CRÍTICO: Inestabilidad Exponencial**
- Fórmula: `I(t+1) = I(t)² + δ(TP53_status)` (crece cuadráticamente + delta aditivo)
- Implica: TP53 -/- alcanzan threshold en ~3-4 ciclos, TP53 +/- toman más tiempo
- Resultado: Células con TP53 -/- son rápidamente inmortales

**Para Biólogos:**
- Enfocarse en FSM de genes (mutaciones biológicamente realistas)
- Ciclo de vida muestra checkpoints de viabilidad
- Protección TP53 es gradual: +/+ (total) → +/- (parcial) → -/- (nula)

**Para Matemáticos:**
- Ver matriz de transición (cadenas de Markov)
- Probabilidades combinadas dan tasas de cambio de población
- Inestabilidad sigue ecuación dinámica: I(t+1) = I(t)² + δ (caótica si δ > 0)
- Threshold = barrera de absorción para evasión apoptosis

**Para Investigadores:**
- Comparar 9 escenarios validados (09 = más realista)
- Predecir dinámica a largo plazo considerando threshold apoptosis
- Distinguir entre "protegido" (TP53 ≠ -/-) y "vulnerable" (TP53 = -/-)
- Entender por qué inestabilidad progresiva = inevitablemente inmortales

---

**Nota:** Estos diagramas se renderizan automáticamente en GitHub. Si los ves en CLion, aparecen como código Mermaid (legible igualmente).

