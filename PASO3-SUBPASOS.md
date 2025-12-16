# PASO 3: Implementar AgenticCell_v2.cpp (Dividido en Sub-pasos)

## Objetivo General
Implementar `AgenticCell_v2.cpp` (~400 líneas) de forma **modular y legible**, evitando complejidad innecesaria.

---

## Estrategia: 5 Sub-pasos + Tests

### PASO 3.1: Constructor + Helpers Básicos (~80 líneas)
**¿Qué hace?**
- Constructor: inicializa D1=1.0, D2=1.0
- Métodos simples: getters (getD1, getD2), setId, id, getSeed, getAge, etc.
- Métodos simples: alive(), isNeoplastic(), isNeoplasticProtected()
- Métodos simples: getTP53(), getBRCA1()

**¿Por qué sub-paso aparte?**
- Son métodos **triviales**, sin lógica
- Necesito verificar compilación antes de añadir lógica compleja
- Líneas: ~80

**Archivos a crear:**
- Borrador de `AgenticCell_v2.cpp` (solo constructor y getters)

**Validación:**
- Compilar ✓
- Sin errores ✓

---

### PASO 3.2: Método `getCurrentCellLifeStage()` (~40 líneas)
**¿Qué hace?**
- Getter on-the-fly que retorna `CellLifeStage`
- Calcula estado según: (TP53 status, BRCA1 status, D1, D2)

**Lógica (clara y simple):**
```
Si !alive() → BASELINE
Si TP53==+/- && BRCA1==+/- → INTERMEDIO1
Si TP53==-/- && BRCA1==-/- → INTERMEDIO2
Si TP53==-/- && D1>2.0 → PRIMER
Si is_neoplastic_ → TUMORAL
Default → BASELINE
```

**¿Por qué sub-paso aparte?**
- Es **crítico** y necesita tests propios
- Permite validar matriz de transiciones sin depender de otras fases

**Validación:**
- Compilar ✓
- Test simple: verificar transiciones (manual, inline)

---

### PASO 3.3: Phase 2 (Endocytosis) - Simplificado (~40 líneas)
**¿Qué hace?**
- Procesa signals (apoptosis)
- **Decisión simple: usa D2 (no genomic_instability)**
  - Si D2 > 5.0 → resiste apoptosis (sobrevive)
  - Si D2 ≤ 5.0 → acepta apoptosis (muere)

**¿Por qué simplificado?**
- Elimina lógica vieja de `genomic_instability_`
- Una sola línea de decisión clara

**Pseudo-código:**
```cpp
while (!incoming_messages_.empty()) {
  // Procesar message
  if (es ApoptosisSignal) {
    if (d2_immunosuppression_ > 5.0) {
      // Resiste
    } else {
      // Muere
    }
  }
}
```

**Validación:**
- Compilar ✓
- Test: apoptosis signal + D2 alto/bajo

---

### PASO 3.4: Phase 4 (Cytoplasmic Remodeling) - Core Logic (~80 líneas)
**¿Qué hace?**
- Actualiza D1 y D2 con matriz BRCA1×TP53
- Detecta transición a PRIMER
- Decide neoplasm

**Lógica:**
```cpp
1. Calcula deltas: InstabilityDeltas::getDeltas(tp53, brca1)
2. D1 ← min(D1² + delta_d1, 999)
3. D2 ← min(D2² + delta_d2, 999)
4. Si TP53==-/- && D1>2.0 → entra PRIMER (log)
5. Si !isNeoplasticProtected() → desarrolla neoplasm
```

**¿Por qué es el "corazón"?**
- Aquí ocurren las **transiciones de estado**
- Donde D1 y D2 crecen

**Validación:**
- Compilar ✓
- Test: verificar D1 y D2 crecen correctamente
- Test: verificar transición a PRIMER

---

### PASO 3.5: Phases 0, 1, 3, 5 + Resto (~150 líneas)
**¿Qué hace?**
- Phase 0: Baseline assessment (logging)
- Phase 1: G1 checkpoint (validar viva)
- Phase 3: Nuclear dynamics (mutaciones, genes)
- Phase 5: Exocytosis (emitir signals)
- Métodos helpers: `increaseAge()`, `attemptDivision()`, `clone()`, etc.

**¿Por qué al final?**
- Son **copia casi literal** del original
- Cambio mínimo: reemplazar `genomic_instability_` por `d1_dna_damage_` en calls
- No tienen lógica nueva

**Validación:**
- Compilar ✓
- Tests viejos deben pasar (AgenticCellTest.cpp replicado)

---

### PASO 3.6: Tests `CellLifeStageTransitionTest.cpp` (~200 líneas)
**¿Qué hace?**
- 5 test cases críticos:
  1. TP53 +/- & BRCA1 +/- → INTERMEDIO1
  2. TP53 -/- & BRCA1 -/- → INTERMEDIO2
  3. TP53 -/- & D1 >2.0 → PRIMER
  4. PRIMER + apoptosis signal + D2 >5.0 → sobrevive
  5. PRIMER + apoptosis signal + D2 ≤5.0 → muere

**¿Por qué?**
- Valida que el modelo funciona
- Tests específicos de v2 (no son los del original)

**Validación:**
- Compilar ✓
- 5 tests pasan ✓

---

## Tabla de Sub-pasos

| Sub-paso | Contenido | Líneas | Duración | Status |
|----------|-----------|--------|----------|--------|
| 3.1 | Constructor + getters | ~80 | 15 min | ⏳ TODO |
| 3.2 | getCurrentCellLifeStage() | ~40 | 15 min | ⏳ TODO |
| 3.3 | Phase 2 (Endocytosis) | ~40 | 15 min | ⏳ TODO |
| 3.4 | Phase 4 (Cytoplasmic Remodeling) | ~80 | 20 min | ⏳ TODO |
| 3.5 | Phases 0,1,3,5 + helpers | ~150 | 30 min | ⏳ TODO |
| 3.6 | Tests (5 cases) | ~200 | 20 min | ⏳ TODO |
| **TOTAL** | **AgenticCell_v2.cpp** | **~400** | **~115 min** | ⏳ TODO |

---

## Orden de Ejecución

**IMPORTANTE:** Ir en orden. Cada sub-paso depende del anterior.

1. ✅ Sub-paso 3.1: Constructor + getters
   - Crea esqueleto de AgenticCell_v2.cpp
   - Compilar, verificar

2. ✅ Sub-paso 3.2: getCurrentCellLifeStage()
   - Añade lógica de transiciones
   - Verificar con logs simples

3. ✅ Sub-paso 3.3: Phase 2
   - Lógica de apoptosis con D2
   - Compilar, verificar

4. ✅ Sub-paso 3.4: Phase 4
   - Core logic (D1 + D2 updates)
   - **Más importante**

5. ✅ Sub-paso 3.5: Phases restantes
   - Copia del original con ajustes mínimos
   - Compilar

6. ✅ Sub-paso 3.6: Tests
   - Crear CellLifeStageTransitionTest.cpp
   - 5 tests pasan

---

## Criterios de Aceptación por Sub-paso

### 3.1: Constructor + Getters
- [ ] Compila sin errores
- [ ] No hay referencias a métodos no implementados (phase0, live, etc.)
- [ ] getD1() y getD2() retornan 1.0 inicialmente

### 3.2: getCurrentCellLifeStage()
- [ ] Compila sin errores
- [ ] Retorna BASELINE por defecto
- [ ] Retorna INTERMEDIO1 si TP53 +/- && BRCA1 +/-
- [ ] Retorna INTERMEDIO2 si TP53 -/- && BRCA1 -/-
- [ ] Retorna PRIMER si TP53 -/- && D1 > 2.0

### 3.3: Phase 2 (Endocytosis)
- [ ] Compila sin errores
- [ ] Procesa ApoptosisSignal
- [ ] Si D2 > 5.0 → cell.alive() == true (después de phase2)
- [ ] Si D2 ≤ 5.0 → cell.alive() == false (lanza excepción)

### 3.4: Phase 4 (Cytoplasmic Remodeling)
- [ ] Compila sin errores
- [ ] D1 crece: D1_new = D1_old² + delta_d1
- [ ] D2 crece: D2_new = D2_old² + delta_d2
- [ ] Deltas varían según BRCA1 × TP53 (matriz)
- [ ] Detecta PRIMER cuando TP53 -/- && D1 > 2.0

### 3.5: Phases 0,1,3,5 + Helpers
- [ ] Compila sin errores
- [ ] live() ejecuta 6 fases en orden (neoplastic cells skip algunos)
- [ ] clone() hereda D1 y D2
- [ ] Tests viejos de AgenticCellTest pasan (adaptados para v2)

### 3.6: Tests
- [ ] CellLifeStageTransitionTest.cpp compila
- [ ] 5 test cases pasan
- [ ] Logs muestran d1=X d2=Y (no genomic_instability)

---

## Recomendación

**Comienza con Sub-paso 3.1 ahora.** Es el más simple y abre el camino para el resto.

Cada sub-paso toma ~15–30 minutos. Puedes hacer 2–3 en una sesión.

**¿Comenzamos con 3.1?**

