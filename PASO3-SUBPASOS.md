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
- Calcula estado según: (alive, is_neoplastic, TP53 status, BRCA1 status, D1)

**Lógica (clara y simple - orden de evaluación):**
```
1. Si !alive() → DEAD
2. Si is_neoplastic_ → TUMORAL
3. Si TP53==-/- → UNPROTECTED (cualquier BRCA1: +/- o -/-)
   └─ Si además D1>2.0 → PRIMER (dentro de UNPROTECTED)
4. Si TP53==+/- && BRCA1==+/- → UNSTABLE
5. Si TP53==+/+ && BRCA1==+/- → BASELINE
6. Default → DEAD (catch-all for safety)
```

**Notas importantes:**
- UNPROTECTED es el estado base cuando TP53 está -/-
  - Si UNPROTECTED + D1>2.0 → es PRIMER (pre-tumoral)
  - Si UNPROTECTED + D1≤2.0 → es UNPROTECTED (esperando que D1 suba)
- BASELINE requiere AMBOS: TP53 +/+ Y BRCA1 +/-
- UNSTABLE es heterozygous en ambos

**¿Por qué sub-paso aparte?**
- Es **crítico** y necesita tests propios
- Permite validar matriz de transiciones sin depender de otras fases

**Validación:**
- Compilar ✓
- Test simple: verificar transiciones (manual, inline)

---

### PASO 3.3: Phase 2 (Endocytosis) - Apoptosis EXTRÍNSECA (~50 líneas)
**¿Qué hace?**
- Procesa signals (ApoptosisSignal) del tejido
- **Apoptosis EXTRÍNSECA:** Tejido intenta matar célula PRIMER basada en D2
  - Si D2 > 5.0 → resiste apoptosis (evasión inmune exitosa)
  - Si D2 ≤ 5.0 → acepta apoptosis (muere, immune clearance)

**Contraste: APOPTOSIS INTRÍNSECA vs EXTRÍNSECA**

- **INTRÍNSECA** (Fase 1, Sub-paso 3.5):
  - Trigger: BRCA1 -/-
  - Quién mata: La célula misma (sin reparación de DNA)
  - Escapatoria: NINGUNA (garantizada muerte)
  - D2 Factor: NO (no interviene)
  
- **EXTRÍNSECA** (Fase 2, Sub-paso 3.3):
  - Trigger: PRIMER (TP53 -/-, D1 > 2.0) + ApoptosisSignal
  - Quién mata: El tejido (respuesta inmunológica)
  - Escapatoria: SÍ (si D2 > 5.0)
  - D2 Factor: SÍ (crítico para decidir)

**Contexto: Flujo PRIMER**

PRIMER es estado transitorio donde:
1. Célula: TP53 -/- & D1 > 2.0 (pretumoral)
2. Tejido detecta y envía ApoptosisSignal (intento de matar)
3. **En Fase 2, decisión basada en D2:**
   - D2 alto (>5.0): Sobrevive → continúa a Fase 3-4 → transforma a TUMORAL
   - D2 bajo (≤5.0): Muere → lanza CellDeathException (extrinsic apoptosis accepted)

**Pseudo-código:**
```cpp
// EXTRINSIC APOPTOSIS CHECK
while (!incoming_messages_.empty()) {
  auto signal = pop_message();
  
  if (signal is ApoptosisSignal) {
    // Extrinsic apoptosis: tissue sends signal, D2 decides
    if (d2_immunosuppression_ > 5.0) {
      // Resists: immune evasion successful
      logger_->logCell("Apoptosis blocked: D2=" + d2 + " > 5.0");
    } else {
      // Dies: immune clearance successful
      logger_->logCell("Apoptosis accepted: D2=" + d2 + " <= 5.0");
      alive_ = false;
      throw CellDeathException("extrinsic_apoptosis@phase2");
    }
  }
}
```

**¿Por qué está aquí?**
- Es la **puerta de entrada a TUMORAL**
- D2 es el **factor clave** de evasión inmune
- Determina si célula en PRIMER vive o muere
- Solo ocurre en PRIMER (TP53 -/-, D1 > 2.0)

**Validación:**
- Compilar ✓
- Test: PRIMER + apoptosis + D2 > 5.0 → sobrevive
- Test: PRIMER + apoptosis + D2 ≤ 5.0 → muere
- Log diferencia: "Apoptosis blocked" vs "Apoptosis accepted"

---

### PASO 3.4: Phase 4 (Cytoplasmic Remodeling) - Transformación a TUMORAL (~90 líneas)
**¿Qué hace?**
- Actualiza D1 y D2 con matriz BRCA1×TP53
- Detecta transición a PRIMER (si D1 > 2.0 y TP53 -/-)
- **Transforma a TUMORAL si sobrevivió apoptosis en PRIMER**
- Decide si desarrollar neoplasm

**Lógica (orden de ejecución):**
```cpp
1. Calcula deltas: InstabilityDeltas::getDeltas(tp53, brca1)
2. D1 ← min(D1² + delta_d1, 999)
3. D2 ← min(D2² + delta_d2, 999)
4. Log: "d1_update: X → Y", "d2_update: X → Y"

5. Detecta estado actual:
   - Si TP53==-/- && D1>2.0 → Estado es PRIMER (log)

6. Si TP53 -/- && !is_neoplastic_:
   └─ Desarrolla neoplasm:
      develop_neoplasm();
      // is_neoplastic_ = true
      // Estado cambia a TUMORAL
```

**Flujo específico PRIMER → TUMORAL:**

```
Ciclo t+1 (si sobrevivió apoptosis en Fase 2):
  Fase 4 comienza:
    - Actualiza D1, D2
    - Detecta PRIMER (TP53 -/-, D1 > 2.0)
    - develop_neoplasm() ejecuta
    - is_neoplastic_ = true
    
  Estado: TUMORAL ✓
  Ciclo t+2: Comportamiento neoplástico (fase5 emite NeoplasmSignal)
```

**¿Por qué es el "corazón"?**
- Aquí ocurren las **transiciones de estado** críticas
- Donde D1 y D2 crecen
- **Donde PRIMER → TUMORAL se completa**

**Pseudo-código:**
```cpp
void phase4_CytoplasmicRemodeling() {
    // 1. Actualizar D1 y D2
    auto [delta_d1, delta_d2] = InstabilityDeltas::getDeltas(tp53, brca1);
    d1_dna_damage_ = min(d1² + delta_d1, 999);
    d2_immunosuppression_ = min(d2² + delta_d2, 999);
    
    // 2. Log de actualización
    logger_->logCell("d1_update: " + prev_d1 + " → " + d1);
    logger_->logCell("d2_update: " + prev_d2 + " → " + d2);
    
    // 3. Detectar PRIMER
    if (tp53 == "-/-" && d1 > 2.0) {
        logger_->logCell("Cell entered PRIMER state");
    }
    
    // 4. Si pasó PRIMER (sobrevivió apoptosis), transformar
    if (!isNeoplasticProtected()) {  // TP53 -/-
        develop_neoplasm();
        // is_neoplastic_ = true → Estado = TUMORAL
    }
}
```

**Validación:**
- Compilar ✓
- Test: verificar D1 y D2 crecen correctamente
- Test: verificar transición a PRIMER (TP53 -/-, D1 > 2.0)
- Test: verificar PRIMER → TUMORAL (sobrevivió apoptosis)

---

### PASO 3.5: Phases 0, 1, 3, 5 + Resto (~150 líneas)
**¿Qué hace?**
- Phase 0: Baseline assessment (logging, cell details)
- **Phase 1: G1 checkpoint (APOPTOSIS INTRÍNSECA)**
- Phase 3: Nuclear dynamics (mutaciones, genes)
- Phase 5: Exocytosis (emitir signals)
- Métodos helpers: `increaseAge()`, `attemptDivision()`, `clone()`, etc.

**Detalle Phase 1: APOPTOSIS INTRÍNSECA**

```
APOPTOSIS INTRÍNSECA: Ocurre en Phase 1
Trigger: BRCA1 -/- (cualquier momento, cualquier TP53 status)
Quién mata: La célula misma (sin reparación de DNA)
Escapatoria: NINGUNA (muerte garantizada)
D2 Factor: NO (no interviene D2)

Código en phase1_G1IntegrityCheckpoint():
  if (!alive()) {
    // alive() retorna false si BRCA1 -/-
    throw CellDeathException("intrinsic_apoptosis@phase1");
  }

Escenarios:
  - BRCA1 -/- & TP53 +/+ → DEAD (intrínseca)
  - BRCA1 -/- & TP53 +/- → DEAD (intrínseca)
  - BRCA1 -/- & TP53 -/- → DEAD (intrínseca)

NOTA: Si BRCA1 -/-, NUNCA llega a PRIMER
      Muere en Fase 1 (mucho antes que el tejido intente matar)
```

**¿Por qué?**
- Son **copia casi literal** del original (para phases 0, 3, 5)
- Cambio mínimo: reemplazar `genomic_instability_` por `d1_dna_damage_` en calls
- No tienen lógica nueva EXCEPTO Phase 1 (ahora documenta intrínseca)

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
| 3.1 | Constructor + getters | ~80 | 15 min | ✅ HECHO |
| 3.2 | getCurrentCellLifeStage() | ~40 | 15 min | ✅ HECHO |
| 3.3 | Phase 2 (Endocytosis) | ~50 | 15 min | ⏳ TODO |
| 3.4 | Phase 4 (Cytoplasmic Remodeling) | ~90 | 20 min | ⏳ TODO |
| 3.5 | Phases 0,1,3,5 + helpers | ~150 | 30 min | ⏳ TODO |
| 3.6 | Tests (7 cases) | ~250 | 25 min | ⏳ TODO |
| **TOTAL** | **AgenticCell_v2.cpp** | **~440** | **~120 min** | ⏳ TODO |

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
- [x] Compila sin errores ✅
- [x] Retorna **DEAD** si !alive() ✅
- [x] Retorna TUMORAL si is_neoplastic_ ✅
- [x] Retorna UNPROTECTED si TP53 -/- (cualquier BRCA1) ✅
- [x] Retorna PRIMER si TP53 -/- && D1 > 2.0 ✅
- [x] Retorna UNSTABLE si TP53 +/- && BRCA1 +/- ✅
- [x] Retorna BASELINE si TP53 +/+ && BRCA1 +/- ✅
- [x] Orden de evaluación es correcto (DEAD → TUMORAL → UNPROTECTED → UNSTABLE → BASELINE) ✅

### 3.3: Phase 2 (Endocytosis)
- [ ] Compila sin errores
- [ ] Procesa ApoptosisSignal correctamente
- [ ] Si D2 > 5.0 → cell.alive() == true (después de phase2)
- [ ] Si D2 ≤ 5.0 → cell.alive() == false (lanza CellDeathException)
- [ ] Log muestra decisión: "Apoptosis blocked: D2=X > 5.0" o "Apoptosis accepted: D2=X <= 5.0"

### 3.4: Phase 4 (Cytoplasmic Remodeling)
- [ ] Compila sin errores
- [ ] D1 crece: D1_new = min(D1_old² + delta_d1, 999)
- [ ] D2 crece: D2_new = min(D2_old² + delta_d2, 999)
- [ ] Deltas varían según BRCA1 × TP53 (matriz)
- [ ] Detecta PRIMER cuando TP53 -/- && D1 > 2.0
- [ ] **Transforma a TUMORAL si TP53 -/- (develop_neoplasm ejecuta)**
- [ ] Logs muestran: "d1_update", "d2_update", "Cell entered PRIMER"

### 3.5: Phases 0,1,3,5 + Helpers
- [ ] Compila sin errores
- [ ] live() ejecuta 6 fases en orden
- [ ] Neoplastic cells ejecutan: phase2 → phase4 → phase5 (skip 0,1,3)
- [ ] clone() hereda D1 y D2
- [ ] Tests viejos de AgenticCellTest pasan (adaptados para v2)

### 3.6: Tests CellLifeStageTransitionTest.cpp
- [ ] Compila sin errores
- [ ] **Test 1:** TP53 +/+ & BRCA1 +/- → BASELINE ✓
- [ ] **Test 2:** TP53 +/- & BRCA1 +/- → UNSTABLE ✓
- [ ] **Test 3:** TP53 -/- (any BRCA1) → UNPROTECTED ✓
- [ ] **Test 4:** TP53 -/- & D1 >2.0 → PRIMER ✓
- [ ] **Test 5:** PRIMER + apoptosis + D2 >5.0 → sobrevive (alive==true) ✓
- [ ] **Test 6:** PRIMER + apoptosis + D2 ≤5.0 → muere (alive==false) ✓
- [ ] **Test 7:** PRIMER + sobrevivió apoptosis → transforma a TUMORAL ✓
- [ ] Logs muestran: "d1=X d2=Y" (no genomic_instability)

---

## Recomendación

**Comienza con Sub-paso 3.1 ahora.** Es el más simple y abre el camino para el resto.

Cada sub-paso toma ~15–30 minutos. Puedes hacer 2–3 en una sesión.

**¿Comenzamos con 3.1?**

