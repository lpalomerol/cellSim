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
        inestabilidad += low_delta
        (degradación leve)
    end note
    
    note right of MinusMinus
        Estado: -/-
        Homocigoto recesivo
        ❌ NO PROTEGIDO vs neoplasia
        Sin protección TP53
        inestabilidad += high_delta
        (degradación severa)
    end note
```

**Matemática:**
- `P(+/+ → +/-) = μ₁ = threshold_TP53 × (1 + k_TP53) × genomic_instability`
- `P(+/- → -/-) = μ₂ = threshold_TP53 × (1 + k_TP53) × genomic_instability`
- **Protección contra neoplasia:** `status != "-/-"` → **PROTEGIDO** (tanto +/+ como +/-)
- **Aumento de inestabilidad:** 
  - TP53 `+/+` → inestabilidad base
  - TP53 `+/-` → inestabilidad += `low_delta_instability`
  - TP53 `-/-` → inestabilidad += `high_delta_instability`
- **Escenario DEFAULT**: `μ₁ = μ₂ = 0.01` (1% por ciclo)

---

### 🔗 FSM Combinado: BRCA1 + TP53 (2D State Space)

```mermaid
stateDiagram-v2
    [*] --> S1
    
    S1: BRCA1(+/-) + TP53(+/+)
    S1: ✅ PROTEGIDO
    S1: inestabilidad: base
    S1: RIESGO: BAJO
    
    S2: BRCA1(+/-) + TP53(+/-)
    S2: ✅ PROTEGIDO
    S2: inestabilidad: base+low
    S2: RIESGO: MODERADO
    
    S3: BRCA1(+/-) + TP53(-/-)
    S3: ❌ NO PROTEGIDO
    S3: inestabilidad: base+high
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

**Tabla de Transiciones (DEFAULT scenario):**

| De estado | Evento | A estado | P(transición) | Protección | Inestabilidad | Resultado |
|-----------|--------|----------|---------------|-----------|----------------|-----------|
| (+/-,+/+) | BRCA1 mut | (-/-,+/+) | 0.0099 | ✅ | base | ✗ MUERTE (Fase 1) |
| (+/-,+/+) | TP53 mut | (+/-,+/-) | 0.0100 | ✅ | base+low | VIVO (protegido, degradado) |
| (+/-,+/+) | Ambas | (-/-,+/-) | 0.0001 | ✅ | base+low | ✗ MUERTE (Fase 1) |
| (+/-,+/+) | Sin mut | (+/-,+/+) | 0.9801 | ✅ | base | VIVO (seguro) |
| | | | | | | |
| (+/-,+/-) | BRCA1 mut | (-/-,+/-) | 0.0099 | ✅ | base+low | ✗ MUERTE (Fase 1) |
| (+/-,+/-) | TP53 mut | (+/-,-/-) | 0.0100 | ❌ | base+high | VIVO (vulnerable neoplasia) |
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

## 🔬 Cómo interpretar los diagramas

**⚠️ PUNTO CRÍTICO: Protección TP53**
- **TP53 +/+** → Protegido, inestabilidad base
- **TP53 +/-** → **SIGUE PROTEGIDO**, pero inestabilidad aumenta (low_delta)
- **TP53 -/-** → **NO PROTEGIDO**, inestabilidad muy elevada (high_delta), vulnerable a neoplasia

**Para Biólogos:**
- Enfocarse en FSM de genes (mutaciones biológicamente realistas)
- Ciclo de vida muestra checkpoints de viabilidad
- TP53 +/- es una "degradación progresiva" de protección, no una pérdida total

**Para Matemáticos:**
- Ver matriz de transición (cadenas de Markov)
- Probabilidades combinadas dan tasas de cambio de población
- Inestabilidad crece cuadráticamente + deltas aditivos por TP53

**Para Investigadores:**
- Comparar 6 escenarios (impacto de mutaciones en población)
- Predecir dinámica a largo plazo
- Distinguir entre "protegido" (TP53 ≠ -/-) y "vulnerable" (TP53 = -/-)

---

**Nota:** Estos diagramas se renderizan automáticamente en GitHub. Si los ves en CLion, aparecen como código Mermaid (legible igualmente).

