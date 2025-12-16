# Checklist Ejecutable: Refactorización AgenticCell D1+D2

## Estado del Proyecto

**Inicio:** 2025-12-16
**Estrategia:** Construcción v2 limpia en paralelo + reemplazo atómico + vaciado final
**Objetivo:** D1 (DNA damage) + D2 (immunosuppression) + CellLifeStage, cero deuda técnica

---

## FASE 1: CONSTRUCCIÓN LIMPIA (Pasos 1-4)

### Paso 1: Enums y Structs Base

**Status:** ⏳ TODO

**Archivos a crear:**
- [ ] `src/domain/cell/CellLifeStage.h`
  - [ ] `enum class CellLifeStage { BASELINE, INTERMEDIO1, INTERMEDIO2, PRIMER, TUMORAL }`
  - [ ] `std::string toString(CellLifeStage stage)`
  - [ ] Documentación de cada estado

- [ ] `src/domain/cell/InstabilityDeltas.h`
  - [ ] Struct/class para matriz BRCA1×TP53
  - [ ] Método `getDeltas(tp53_status, brca1_status) → pair<double, double>`
  - [ ] Valores configurables (bajo, medio, alto, muy_alto)

**Cambios a archivos existentes:**
- [ ] `tests/CMakeLists.txt` - añadir headers-only

**Validación:**
- [ ] Compilación exitosa
- [ ] No hay warnings
- [ ] Headers incluibles sin dependencias

---

### Paso 2: Interface AgenticCell_v2.h

**Status:** ⏳ TODO

**Archivo a crear:**
- [ ] `src/domain/cell/AgenticCell_v2.h`
  - [ ] Heredar `ICell` + `ILoggeable`
  - [ ] Atributos privados: `d1_dna_damage_`, `d2_immunosuppression_` (double, 1.0 default)
  - [ ] Getters públicos: `getD1()`, `getD2()`, `getCurrentCellLifeStage()`
  - [ ] Constructor (igual a AgenticCell actual)
  - [ ] Métodos públicos (live, alive, isNeoplastic, etc.)
  - [ ] Fases privadas (phase0-5, pero phase2 y phase4 ajustadas)

**Validación:**
- [ ] Header compila
- [ ] Sintaxis correcta
- [ ] Documentación de atributos nuevos

---

### Paso 3: Implementación AgenticCell_v2.cpp

**Status:** ⏳ TODO

**Archivo a crear:**
- [ ] `src/domain/cell/AgenticCell_v2.cpp`
  - [ ] Constructor: inicia D1=1.0, D2=1.0
  - [ ] `live()`: 6 fases, lógica igual a original (con ajustes en 2 y 4)
  - [ ] Phase 0: Baseline (igual)
  - [ ] Phase 1: G1 Checkpoint (igual)
  - [ ] **Phase 2: Endocytosis (AJUSTADO)**
    - [ ] Apoptosis usa D2 (no `genomic_instability_`)
    - [ ] Si D2 > 5.0 → resiste apoptosis
    - [ ] Si D2 ≤ 5.0 → acepta apoptosis
    - [ ] Logs: `"d1=X d2=Y | apoptosis decision based on D2"`
  - [ ] Phase 3: Nuclear Dynamics (igual)
  - [ ] **Phase 4: Cytoplasmic Remodeling (REFACTORIZADO)**
    - [ ] Calcula deltas: `auto [delta_d1, delta_d2] = InstabilityDeltas::getDeltas(tp53, brca1)`
    - [ ] Actualiza D1: `d1 = min(d1² + delta_d1, 999)`
    - [ ] Actualiza D2: `d2 = min(d2² + delta_d2, 999)`
    - [ ] Detecta transición a PRIMER: TP53==-// && D1>2.0
    - [ ] Logs separados: `d1_update`, `d2_update`, `cell_stage`
  - [ ] Phase 5: Exocytosis (igual)
  - [ ] `getCurrentCellLifeStage()`: getter on-the-fly
    - [ ] BASELINE: default
    - [ ] INTERMEDIO1: TP53 +/- & BRCA1 +/-
    - [ ] INTERMEDIO2: TP53 -/- & BRCA1 -/-
    - [ ] PRIMER: TP53 -/- & D1 > 2.0
    - [ ] TUMORAL: is_neoplastic_ == true
  - [ ] `clone()`: hereda D1 + D2
  - [ ] Logs actualizados: reemplazar `genomic_instability_` por `d1=X d2=Y`

**Test file:**
- [ ] `tests/CellLifeStageTransitionTest.cpp`
  - [ ] Test 1: TP53 +/- & BRCA1 +/- → INTERMEDIO1
  - [ ] Test 2: TP53 -/- & BRCA1 -/- → INTERMEDIO2
  - [ ] Test 3: TP53 -/- & D1 >2.0 → PRIMER
  - [ ] Test 4: PRIMER + apoptosis signal + D2 >5.0 → sobrevive
  - [ ] Test 5: PRIMER + apoptosis signal + D2 ≤5.0 → muere

**Cambios a archivos existentes:**
- [ ] `tests/CMakeLists.txt` - añadir CellLifeStageTransitionTest.cpp

**Validación:**
- [ ] Compilación exitosa
- [ ] 5 tests nuevos pasan
- [ ] Logs contienen `d1=` y `d2=` (no `genomic_instability_`)

---

### Paso 4: CellFactory_v2

**Status:** ⏳ TODO

**Archivos a crear:**
- [ ] `src/domain/cell/CellFactory_v2.h`
  - [ ] Clase factory para `AgenticCell_v2`
  - [ ] Métodos públicos: `makeDefaultGenome()`, etc.

- [ ] `src/domain/cell/CellFactory_v2.cpp`
  - [ ] Implementación factory
  - [ ] Instancia `AgenticCell_v2` correctamente

**Cambios a archivos existentes:**
- [ ] `tests/CMakeLists.txt` - añadir CellFactory_v2.cpp/h

**Validación:**
- [ ] Compilación exitosa
- [ ] Factory instancia AgenticCell_v2 correctamente

---

## FASE 2: INTEGRACIÓN (Paso 5)

### Paso 5: Integración Tissue

**Status:** ⏳ TODO

**Cambios a archivos existentes:**
- [ ] `src/domain/tissue/Tissue.cpp`
  - [ ] Reemplazar `AgenticCell` → `AgenticCell_v2` (includes)
  - [ ] Reemplazar `CellFactory` → `CellFactory_v2` (uso)
  - [ ] Mantener lógica de Tissue igual

- [ ] `tests/CMakeLists.txt` - actualizar referencias

**Validación:**
- [ ] Compilación exitosa
- [ ] Todos los tests viejos pasan (no usar AgenticCell_v2 directamente, sino Tissue)
- [ ] Todos los tests nuevos pasan
- [ ] Sin warnings

---

## FASE 3: LIMPIEZA (Pasos 6-7)

### Paso 6: CMakeLists.txt Refactor

**Status:** ⏳ TODO

**Cambios a archivos existentes:**
- [ ] `tests/CMakeLists.txt`
  - [ ] Remover líneas: `../src/domain/cell/AgenticCell.cpp`, `../src/domain/cell/AgenticCell.h`
  - [ ] Renombrar en CMake: `AgenticCell_v2` → `AgenticCell`
  - [ ] Renombrar en CMake: `CellFactory_v2` → `CellFactory`
  - [ ] Verificar no hay duplicados

**Validación:**
- [ ] Compilación exitosa
- [ ] Todos los tests pasan
- [ ] CMakeLists.txt limpio

---

### Paso 7: Vaciado Final (Eliminar archivos viejos)

**Status:** ⏳ TODO

**Archivos a eliminar:**
- [ ] `src/domain/cell/AgenticCell.h` (viejo)
- [ ] `src/domain/cell/AgenticCell.cpp` (viejo)
- [ ] `src/domain/cell/CellFactory.h` (viejo, si existe)
- [ ] `src/domain/cell/CellFactory.cpp` (viejo)
- [ ] Confirmación: `OncoState.h` ya fue eliminado

**Archivos a renombrar:**
- [ ] `AgenticCell_v2.h` → `AgenticCell.h`
- [ ] `AgenticCell_v2.cpp` → `AgenticCell.cpp`
- [ ] `CellFactory_v2.h` → `CellFactory.h`
- [ ] `CellFactory_v2.cpp` → `CellFactory.cpp`

**Cambios a archivos existentes:**
- [ ] `tests/CMakeLists.txt` - actualizar nombres finales (quitar _v2)
- [ ] Cualquier otro archivo que incluya CellFactory (si aplica)

**Validación:**
- [ ] Compilación exitosa
- [ ] Todos los tests pasan
- [ ] No hay archivos viejos en disco
- [ ] Estructura final limpia

---

## Criterios Globales de Aceptación

### Funcionalidad

- [ ] D1 y D2 crecen cada tick en fase 4
- [ ] D1 usa delta según TP53 status
- [ ] D2 usa delta según BRCA1 status
- [ ] Apoptosis en fase 2 usa D2 (>5.0 resiste)
- [ ] `getCurrentCellLifeStage()` retorna estado correcto
- [ ] Transición a PRIMER: TP53 -/- & D1 > 2.0
- [ ] Clone hereda D1 y D2
- [ ] Logs separados para D1 y D2

### Calidad

- [ ] Compilación sin warnings
- [ ] Todos los tests viejos pasan (replicados)
- [ ] Todos los tests nuevos pasan (5 casos mínimo)
- [ ] Sin archivos huérfanos
- [ ] Sin código muerto
- [ ] Comentarios documentan funciones públicas

### Arquitectura

- [ ] Cero deuda técnica (versión vieja totalmente eliminada)
- [ ] Separación limpia: v2 creada en paralelo
- [ ] Refactor atómico: cada paso deja código compilable
- [ ] Rollback fácil (hasta paso 5)

---

## Notas y Decisions Pendientes

| Item | Status | Notas |
|------|--------|-------|
| Delta values (bajo, medio, alto, muy_alto) | ⏳ TODO | Definir valores numéricos exactos |
| Matriz BRCA1×TP53 completa | ✅ DEFINED | Ver plan principal |
| Umbrales (D1=2.0, D2=5.0) | ✅ DEFINED | Confirmados en plan |
| Heredabilidad D1+D2 | ✅ DEFINED | Sí, en clone() |
| Logs formato | ✅ DEFINED | d1=X d2=Y |
| CellLifeStage on-the-fly vs mutable | ✅ DEFINED | On-the-fly (getter) |

---

## Timeline Estimado

| Fase | Pasos | Duración Estimada |
|------|-------|-------------------|
| Construcción | 1-4 | 2-3 horas |
| Integración | 5 | 30-45 minutos |
| Limpieza | 6-7 | 15-30 minutos |
| **TOTAL** | **7** | **~3-4 horas** |

---

## Comandos Rápidos

```bash
# Verificar compilación (hacer después de cada paso)
cmake --build /home/luis/CLionProjects/cellSim/cmake-build-debug

# Ejecutar tests
ctest --output-on-failure

# Ver errores de compilación
cmake --build /home/luis/CLionProjects/cellSim/cmake-build-debug 2>&1 | grep error

# Generar prompt con run_profile.sh
bash .github/scripts/run_profile.sh cpp "TAREA" "REQUISITOS" "INPUTS"
```

---

## Referencias Documentales

- **Plan completo:** `plan-agenticCellRefactorD1D2.prompt.md`
- **Guía de uso:** `GUIDE-agenticCellRefactor.md`
- **Especificación de estados:** `.github/copilot/CELL_STATE_MACHINE.md`
- **Roles de agentes:** `.github/copilot/agent_roles.md`
- **Fases del ciclo:** `.github/copilot/cell_lifecycle_templates.md`
- **Diagrama de Luis:** `docs/diagrams_luis/resumen.txt`

