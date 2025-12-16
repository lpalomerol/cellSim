# Plan: Refactorizar AgenticCell - D1 + D2 + CellLifeStage

## Resumen Ejecutivo

Construir `AgenticCell_v2` limpia con D1+D2+CellLifeStage en paralelo, validar con tests nuevos, luego reemplazar y limpiar versión vieja. Estrategia de vaciado incremental en 7 pasos atómicos, garantizando cero deuda técnica.

**Objetivo:** Separar `genomic_instability_` (D1, DNA damage) e introducir `immunosuppression_` (D2, immune evasion). Ambos crecen cada tick en fase 4 con deltas según matriz genética BRCA1×TP53. Fase 2 (Endocytosis) usa D2 para decidir apoptosis (D2 alto → resiste); Fase 4 ajusta ambos contadores y calcula transición a PRIMER. `getCurrentCellLifeStage()` es getter derivado (BASELINE/INTERMEDIO1/INTERMEDIO2/PRIMER/TUMORAL). Umbrales: D1>2.0 para PRIMER, D2>5.0 para resistencia apoptosis.

---

## Contexto Biológico

### Atributos Genéticos Internos

- **BRCA1** (DNA repair capacity): +/+ → +/- → -/- (unidireccional)
- **TP53** (Apoptosis protector): +/+ → +/- → -/- (unidireccional)
- **D1** (Genomic instability - DNA damage): Contador que crece cada tick
- **D2** (Immunosuppression - immune evasion): Contador que crece cada tick

### Matriz de Instabilidad: BRCA1 × TP53

Cada combinación genética define dos deltas (delta_d1, delta_d2):

| BRCA1 | TP53 | delta_d1 | delta_d2 | Nombre |
|-------|------|----------|----------|---------|
| +/- | +/+ | bajo | bajo | LOW |
| +/- | +/- | medio | medio | MEDIUM |
| +/- | -/- | alto | alto | HIGH |
| -/- | -/- | muy_alto | muy_alto | VERY_HIGH |

### Estados Celulares (CellLifeStage)

Derivados on-the-fly según (TP53 status, BRCA1 status, D1, D2):

1. **BASELINE**: Estado por defecto (célula normal)
2. **INTERMEDIO1**: TP53 +/- & BRCA1 +/- (primera inestabilidad detectada)
3. **INTERMEDIO2**: TP53 -/- & BRCA1 -/- (inestabilidad severa)
4. **PRIMER**: TP53 -/- & D1 > 2.0 (pretumoral, visible para tejido)
5. **TUMORAL**: `is_neoplastic_` = true (transformación completada)

### Transiciones Clave

- **Fase 2 (Endocytosis):** Apoptosis decide según D2
  - Si D2 > 5.0 → resiste apoptosis
  - Si D2 ≤ 5.0 → acepta apoptosis (muere)

- **Fase 4 (Cytoplasmic Remodeling):** Incrementa D1 y D2
  - Calcula deltas según matriz BRCA1×TP53
  - D1 ← min(D1² + delta_d1, 999)
  - D2 ← min(D2² + delta_d2, 999)
  - Detecta transición a PRIMER (TP53 -/- & D1 > 2.0)
  - Luego decide neoplasm según protección TP53

---

## Cambios Implementativos

### Inicialización

- D1 y D2 inician en 1.0 (como actual `genomic_instability_`)
- Ambos heredables en `clone()` (la célula hija hereda daño parental)

### Logs Separados

- Reemplazar `"genomic_instability=X"` por `"d1=X d2=Y"`
- En `details()`: mostrar ambos contadores
- En transiciones: loguear cambio de estado

### Crecimiento

- Cada tick en fase 4 crece D1 y D2
- Crecimiento cuadrático: `x_new = x_old²`
- Suma de deltas según matriz genética
- Límite superior: 999

---

## Roadmap (7 Pasos)

### PASO 1: Crear structs/enums base

**Archivos a crear:**
- `src/domain/cell/CellLifeStage.h` (enum + helper `toString()`)
- `src/domain/cell/InstabilityDeltas.h` (struct con matriz BRCA1×TP53)

**Cambios:**
- `tests/CMakeLists.txt` → añadir 2 headers (headers-only, sin .cpp)

**Tests:** Ninguno (son tipos)
**Vaciado:** NINGUNO

---

### PASO 2: Crear `AgenticCell_v2.h` (interface limpia)

**Archivo a crear:** `src/domain/cell/AgenticCell_v2.h`

**Contiene:**
- Heredar de `ICell` + `ILoggeable` (igual que original)
- Constructor igual al actual
- **Nuevos atributos:** `d1_dna_damage_`, `d2_immunosuppression_` (ambos 1.0 default)
- **Nuevos getters:** `getD1()`, `getD2()`, `getCurrentCellLifeStage()`
- **Private methods:** phases 0–5 (igual que original, pero fase 2+4 ajustadas)
- **NO:** `genomic_instability_`, métodos viejos

**Tests:** Ninguno (es interface)
**Vaciado:** NINGUNO

---

### PASO 3: Implementar `AgenticCell_v2.cpp` (core logic)

**Archivo a crear:** `src/domain/cell/AgenticCell_v2.cpp`

**Contiene:**
- Constructor: inicia D1=1.0, D2=1.0
- `phase2_Endocytosis()`: apoptosis basada en D2 (>5.0 = resiste)
- `phase4_CytoplasmicRemodeling()`:
  - Calcula deltas con `InstabilityDeltas::getDeltas(tp53, brca1)`
  - D1 ← min(D1² + delta_d1, 999)
  - D2 ← min(D2² + delta_d2, 999)
  - Detecta transición a PRIMER (TP53 -/- & D1 > 2.0)
  - Logs separados: `d1=X d2=Y`
- `getCurrentCellLifeStage()`: getter on-the-fly
- `clone()`: hereda D1 + D2

**Tests:** Crear `tests/CellLifeStageTransitionTest.cpp` (5 casos):
- TP53 +/- & BRCA1 +/- → INTERMEDIO1
- TP53 -/- & BRCA1 -/- → INTERMEDIO2
- TP53 -/- & D1 >2.0 → PRIMER
- Apoptosis + D2 >5.0 → sobrevive
- Apoptosis + D2 ≤5.0 → muere

**Vaciado:** NINGUNO (aún)

---

### PASO 4: Crear `CellFactory_v2.cpp` (builder limpio)

**Archivos a crear:**
- `src/domain/cell/CellFactory_v2.h`
- `src/domain/cell/CellFactory_v2.cpp`

**Contiene:** Equivalente a `CellFactory` actual pero instancia `AgenticCell_v2`

**Tests:** Actualizar `tests/CMakeLists.txt` para compilar
**Vaciado:** NINGUNO

---

### PASO 5: Integración fase 1 (Tissue + Tests)

**Cambios:**
- `src/domain/tissue/Tissue.cpp`: swapear `AgenticCell` → `AgenticCell_v2` + `CellFactory_v2`
- `tests/CMakeLists.txt`: añadir `AgenticCell_v2.cpp`, `CellFactory_v2.cpp`, `CellLifeStageTransitionTest.cpp`
- Compilar y validar todos los tests (viejos + nuevos)

**Tests:** Todos los tests viejos deben pasar + nuevos de transición
**Vaciado:** NINGUNO (versión vieja aún en CMake pero no usada)

---

### PASO 6: Reemplazar en todas partes

**Cambios:**
- `tests/CMakeLists.txt`:
  - Remover `AgenticCell.cpp`, `AgenticCell.h`
  - Renombrar en CMake `AgenticCell_v2` → `AgenticCell`
- `src/domain/cell/CellFactory.cpp` → remover, usar `CellFactory_v2` como `CellFactory`

**Verificar:** Compilar, todos los tests deben pasar
**Vaciado:** Remover de CMake (no del disco aún)

---

### PASO 7: Vaciado final

**Eliminar del disco:**
- `src/domain/cell/AgenticCell.h`
- `src/domain/cell/AgenticCell.cpp`
- `src/domain/cell/CellFactory.h` (si existe)
- Viejo `src/domain/cell/CellFactory.cpp`
- Viejo `src/domain/cell/OncoState.h` (ya eliminado)

**Renombrar:**
- `AgenticCell_v2.h/cpp` → `AgenticCell.h/cpp`
- `CellFactory_v2.h/cpp` → `CellFactory.h/cpp`

---

## Tabla de Dependencias

| Paso | Archivos Creados | Archivos Modificados | Código Viejo Tocado |
|------|------------------|----------------------|---------------------|
| 1    | CellLifeStage.h, InstabilityDeltas.h | tests/CMakeLists.txt | ❌ NO |
| 2    | AgenticCell_v2.h | - | ❌ NO |
| 3    | AgenticCell_v2.cpp, CellLifeStageTransitionTest.cpp | tests/CMakeLists.txt | ❌ NO |
| 4    | CellFactory_v2.h/cpp | tests/CMakeLists.txt | ❌ NO |
| 5    | - | Tissue.cpp, tests/CMakeLists.txt | ❌ NO |
| 6    | - | tests/CMakeLists.txt | ⚠️ REMOVER DE CMake |
| 7    | - | - | 🗑️ ELIMINAR ARCHIVOS |

---

## Beneficios

- ✅ **Cero basura en desarrollo:** Código viejo no se toca hasta paso 6
- ✅ **Rollback fácil:** Cualquier paso falsa, revertir CMake y listo
- ✅ **Tests paralelos:** Nuevos tests no interfieren con viejos
- ✅ **Vaciado limpio:** Paso 7 es limpieza pura, sin lógica
- ✅ **Compilaciones atómicas:** Cada paso deja código compilable

---

## Criterios de Aceptación

### Por Paso

1. **Paso 1:** Código compila, headers-only sin errores
2. **Paso 2:** Header compila, no implementación aún
3. **Paso 3:** Implementación completa, 5 tests nuevos pasan
4. **Paso 4:** Factory compila, instancia `AgenticCell_v2` correctamente
5. **Paso 5:** Tissue usa `AgenticCell_v2`, todos los tests (viejos+nuevos) pasan
6. **Paso 6:** CMake limpio (no duplicados), compilación exitosa
7. **Paso 7:** Archivos viejos eliminados, renombrados como versión final, compilación exitosa

### Global

- ✅ Todos los tests pasan (viejos replicados + nuevos)
- ✅ Sin archivos huérfanos ni código muerto
- ✅ Logs separados para D1 y D2 en todas partes
- ✅ `getCurrentCellLifeStage()` retorna estado correcto on-the-fly
- ✅ Apoptosis usa D2 (no D1) para decisión
- ✅ Fase 4 actualiza D1 y D2 según matriz genética

---

## Notas Importantes

1. **No sabríamos qué hacer en fase 4 → ajuste antes de neoplasm check:** Se recomienda orden: `updateInstability()` → detectar PREMIER → luego decidir si `develop_neoplasm()`.

2. **PRIMER debe ser tumoral visible:** Una vez en PRIMER (D1 > 2.0 & TP53 -/-), el tejido puede verlo y enviará apoptosis en fase 2 siguiente. La célula decidirá según D2.

3. **Inicialización:** D1 y D2 inician en 1.0, igual que `genomic_instability_` original.

4. **Heredabilidad:** Ambos D1 y D2 se heredan en `clone()` (hija nace con inestabilidad parental).

5. **Deltas configurable:** `InstabilityDeltas` encapsula matriz pero no es singleton; permite inyectar en constructor si fuera necesario.

---

## Referencias

- `.github/copilot/CELL_STATE_MACHINE.md` - Especificación de 5 estados
- `.github/copilot/agent_roles.md` - Rol de AgenticCell
- `.github/copilot/cell_lifecycle_templates.md` - Fases 2 y 4 detalladas
- `docs/diagrams_luis/resumen.txt` - Modelo de Luis (4 atributos internos, transiciones)

