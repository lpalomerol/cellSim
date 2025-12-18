# Flujo PRIMER → TUMORAL/DEAD (Sub-paso 3.2 + 3.3)

## Estado PRIMER: La transición crítica

### ¿Cuándo entra la célula en PRIMER?

**Precondiciones:**
- TP53 -/- (sin protección P53)
- D1 > 2.0 (DNA damage suficiente)
- !is_neoplastic_ (aún no transformada)

**Estado durante PRIMER:**
- La célula es **pretumoral** (visible para el tejido)
- El tejido la detecta y envía apoptosis
- La célula está en **1 ciclo de decisión crítica**

---

## Ciclo en PRIMER: Fase 2 (Endocytosis) - DECISIÓN

### Lo que pasa en FASE 2 cuando está en PRIMER

**Entrada a Fase 2:**
```
Cell state: PRIMER (TP53 -/-, D1 > 2.0)
Tissue envia: ApoptosisSignal (intento de matar)
```

**Lógica de decisión (en phase2_Endocytosis):**

```cpp
// PRIMER + ApoptosisSignal → Decisión basada en D2

if (apoptosis_signal_received) {
    if (D2 > 5.0) {
        // D2 ALTO: Célula resiste apoptosis
        // → Evasión inmunológica exitosa
        // → Continúa viva para pasar a Fase 4
    } else {
        // D2 BAJO: Célula NO puede resistir
        // → Apoptosis extrínseca tiene éxito
        // → Célula muere (throw CellDeathException)
    }
}
```

---

## Ciclo en PRIMER: Fase 4 (Cytoplasmic Remodeling) - TRANSFORMACIÓN

### Lo que pasa en FASE 4 si sobrevivió apoptosis

**Precondición para llegar a Fase 4:**
- Sobrevivió Fase 2 (D2 > 5.0)
- Aún no es neoplastic

**Lógica en phase4_CytoplasmicRemodeling:**

```cpp
// PRIMER + sobrevivió apoptosis → Transforma a TUMORAL

if (current_stage == PRIMER && survived_apoptosis) {
    // Actualiza D1 y D2 (igual que siempre)
    updateInstability();
    
    // Ahora checamos: ¿debería transformarse?
    if (!isNeoplasticProtected()) {  // TP53 -/-
        // Sí, desarrolla neoplasm
        develop_neoplasm();
        // → is_neoplastic_ = true
        // → Estado cambia a TUMORAL
    }
}
```

---

## Matriz de Decisión: PRIMER → Outcomes

```
┌─────────────────────────────────────────────────────────────┐
│                    CÉLULA EN PRIMER                         │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  Fase 2: ¿Recibe ApoptosisSignal del tejido?               │
│                                                              │
│    ├─ SÍ, D2 ≤ 5.0  →  MUERE (apoptosis extrínseca)       │
│    │                   State: DEAD                          │
│    │                   Exception: CellDeathException        │
│    │                                                        │
│    └─ SÍ, D2 > 5.0  →  SOBREVIVE apoptosis                │
│                        Continúa a Fase 4                   │
│                                                              │
│  Fase 4: (Si sobrevivió)                                   │
│                                                              │
│    └─ develop_neoplasm()  →  Transforma a TUMORAL          │
│                              State: TUMORAL                 │
│                              is_neoplastic_ = true          │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## Timelines por Escenario

### Escenario A: D2 BAJO (<= 5.0)

```
Ciclo t:     Estado = PRIMER
Ciclo t+1:   Fase 2 recibe ApoptosisSignal
             D2 = 3.0 (bajo)
             → Apoptosis tiene éxito
             → Cell.alive() = false
             → CellDeathException
             → DEAD
```

### Escenario B: D2 ALTO (> 5.0)

```
Ciclo t:     Estado = PRIMER
Ciclo t+1:   Fase 2 recibe ApoptosisSignal
             D2 = 7.0 (alto)
             → Apoptosis FALLA
             → Cell continúa viva
             → Fase 3: Mutaciones (normales)
             → Fase 4: updateInstability() + develop_neoplasm()
             → is_neoplastic_ = true
             
Ciclo t+2:   Estado = TUMORAL
             Neoplastic behavior activa
```

---

## Umbrales Críticos

| Concepto | Valor | Significado |
|----------|-------|-------------|
| D1_PRIMER_THRESHOLD | 2.0 | Mínimo D1 para transitar a PRIMER |
| D2_APOPTOSIS_RESISTANCE | 5.0 | Mínimo D2 para resistir apoptosis |
| BASELINE_D1 | 1.0 | D1 inicial |
| BASELINE_D2 | 1.0 | D2 inicial |

---

## Notas Importantes

1. **PRIMER es estado transitorio**: La célula NO puede permanecer en PRIMER indefinidamente
   - Si D2 ≤ 5.0: Muere en el mismo ciclo (Fase 2)
   - Si D2 > 5.0: Transforma a TUMORAL en el siguiente ciclo (Fase 4)

2. **Apoptosis es extrínseca en PRIMER**: El tejido es quien envía la señal
   - Diferente a apoptosis "interna" (BRCA KO antes que TP53)
   - La célula solo responde según D2

3. **D1 y D2 crecen INDEPENDIENTEMENTE** durante todos los ciclos
   - Incluso mientras está en PRIMER
   - Lo que importa es el valor en el momento de recibir ApoptosisSignal

4. **Transición a TUMORAL es AUTOMÁTICA** en Fase 4
   - Si sobrevivió apoptosis
   - Y TP53 -/-
   - → develop_neoplasm() se ejecuta
   - → is_neoplastic_ = true

---

## ¿Está documentado en el código?

**ACTUALMENTE NO.** Necesitamos añadir documentación en:

1. **Sección de 3.3 (Phase 2 Endocytosis)**
   - Decisión D2 > 5.0 para apoptosis
   
2. **Sección de 3.4 (Phase 4 Cytoplasmic Remodeling)**
   - Lógica de develop_neoplasm() cuando TP53 -/-

3. **Header de AgenticCell_v2.h**
   - Docstring sobre transición PRIMER

4. **Tests de CellLifeStageTransitionTest.cpp**
   - Tests específicos: PRIMER + D2 bajo → DEAD
   - Tests específicos: PRIMER + D2 alto → TUMORAL

---

## Próximo paso

Cuando implementemos **3.3 (Phase 2)** y **3.4 (Phase 4)**, esta documentación debe estar en el código como comentarios claros.

¿Quieres que lo integre en PASO3-SUBPASOS.md?

