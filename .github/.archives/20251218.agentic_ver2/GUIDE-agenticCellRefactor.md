# Guía de Uso: Plan de Refactorización AgenticCell (D1+D2)

## Resumen

Plan de construcción incremental de `AgenticCell_v2` con D1 (DNA damage) + D2 (immunosuppression) + CellLifeStage.

**Archivo principal:** `plan-agenticCellRefactorD1D2.prompt.md`

---

## Uso Rápido con run_profile.sh

Puedes generar prompts específicos para cada paso usando:

```bash
bash .github/scripts/run_profile.sh cpp "TAREA" "REQUISITOS" "INPUTS"
```

### Ejemplos por Paso

#### Paso 1: Crear enums y structs base

```bash
bash .github/scripts/run_profile.sh cpp \
  "Crear CellLifeStage.h y InstabilityDeltas.h" \
  "Headers-only, sin .cpp. CellLifeStage: enum {BASELINE, INTERMEDIO1, INTERMEDIO2, PRIMER, TUMORAL} + toString(). InstabilityDeltas: matriz BRCA1×TP53 → (delta_d1, delta_d2)" \
  "Basarse en: agent_roles.md > AgenticCell role, CELL_STATE_MACHINE.md > 5 estados"
```

#### Paso 2: Interface AgenticCell_v2.h

```bash
bash .github/scripts/run_profile.sh cpp \
  "Crear AgenticCell_v2.h (interface limpia)" \
  "Header-only. Heredar ICell + ILoggeable. Nuevos: d1_dna_damage_, d2_immunosuppression_ (double, 1.0 default), getters getD1(), getD2(), getCurrentCellLifeStage(). Fases 0-5 privadas." \
  "Ver AgenticCell.h líneas 1-100 como referencia. Context: cell_lifecycle_templates.md > phase2, phase4"
```

#### Paso 3: Implementación AgenticCell_v2.cpp

```bash
bash .github/scripts/run_profile.sh cpp \
  "Implementar AgenticCell_v2.cpp (core logic D1+D2)" \
  "Constructor inicia D1=1.0, D2=1.0. Phase2: apoptosis basada en D2 (>5.0 resiste). Phase4: calcula deltas, D1←min(D1²+delta_d1,999), D2←min(D2²+delta_d2,999), detecta PRIMER. Clone hereda D1+D2. Logs separados: d1=X d2=Y" \
  "InstabilityDeltas::getDeltas(tp53, brca1). Ver AgenticCell.cpp líneas 169-230 (phase4 actual)"
```

#### Paso 3b: Tests CellLifeStageTransitionTest.cpp

```bash
bash .github/scripts/run_profile.sh cpp \
  "Crear CellLifeStageTransitionTest.cpp (5 test cases)" \
  "Usar Google Test. Casos: (1) TP53+/- & BRCA1+/- → INTERMEDIO1, (2) TP53-/- & BRCA1-/- → INTERMEDIO2, (3) TP53-/- & D1>2.0 → PRIMER, (4) Apoptosis + D2>5.0 → sobrevive, (5) Apoptosis + D2≤5.0 → muere" \
  "Basarse en tests/AgenticCellTest.cpp como template. Context: resumen.txt > matriz de decisión"
```

#### Paso 4: CellFactory_v2

```bash
bash .github/scripts/run_profile.sh cpp \
  "Crear CellFactory_v2.h/cpp" \
  "Equivalente a CellFactory actual pero instancia AgenticCell_v2. Misma interfaz pública." \
  "Ver CellFactory.h/cpp como referencia"
```

#### Paso 5: Integración Tissue

```bash
bash .github/scripts/run_profile.sh cpp \
  "Integrar AgenticCell_v2 en Tissue.cpp" \
  "Reemplazar AgenticCell → AgenticCell_v2, CellFactory → CellFactory_v2. Mantener lógica de Tissue igual." \
  "Ver Tissue.cpp líneas 1-50"
```

#### Paso 6: CMakeLists.txt (refactor)

```bash
bash .github/scripts/run_profile.sh cpp \
  "Actualizar tests/CMakeLists.txt: remover AgenticCell.cpp/h, renombrar v2" \
  "Remover referencias a AgenticCell.cpp old, AgenticCell.h old. Renombrar AgenticCell_v2 → AgenticCell, CellFactory_v2 → CellFactory en CMake." \
  "Ver tests/CMakeLists.txt líneas 1-60"
```

---

## Checklist por Paso

### ✅ Paso 1: Enums y Structs Base
- [ ] `CellLifeStage.h` creado con enum + `toString()`
- [ ] `InstabilityDeltas.h` creado con matriz BRCA1×TP53
- [ ] Compilación exitosa (headers-only)

### ✅ Paso 2: Interface AgenticCell_v2.h
- [ ] `AgenticCell_v2.h` creado
- [ ] Heredar `ICell` + `ILoggeable`
- [ ] Atributos D1, D2, getters
- [ ] Fases privadas
- [ ] Compilación exitosa

### ✅ Paso 3: Implementación AgenticCell_v2.cpp
- [ ] `AgenticCell_v2.cpp` implementado
- [ ] Constructor inicializa D1=1.0, D2=1.0
- [ ] Phase2: apoptosis con D2
- [ ] Phase4: actualiza D1 y D2 según matriz
- [ ] `getCurrentCellLifeStage()` retorna correcto
- [ ] `clone()` hereda D1+D2
- [ ] `CellLifeStageTransitionTest.cpp` creado con 5 casos
- [ ] Todos los tests nuevos pasan

### ✅ Paso 4: CellFactory_v2
- [ ] `CellFactory_v2.h/cpp` creados
- [ ] Instancia `AgenticCell_v2`
- [ ] Compilación exitosa

### ✅ Paso 5: Integración Tissue
- [ ] `Tissue.cpp` usa `AgenticCell_v2` + `CellFactory_v2`
- [ ] Todos los tests viejos pasan
- [ ] Todos los tests nuevos pasan
- [ ] Compilación exitosa

### ✅ Paso 6: CMakeLists.txt Refactor
- [ ] `tests/CMakeLists.txt` limpio (sin duplicados)
- [ ] Remover referencias a versión vieja
- [ ] Renombrar en CMake: v2 → final
- [ ] Compilación exitosa

### ✅ Paso 7: Vaciado Final
- [ ] Eliminar archivos viejos del disco
- [ ] Renombrar archivos finales
- [ ] Compilación exitosa
- [ ] Sin archivos huérfanos

---

## Estructura de Archivos Finales

```
src/domain/cell/
├── CellLifeStage.h ✨ NEW
├── InstabilityDeltas.h ✨ NEW
├── AgenticCell.h/cpp (renombrado de v2)
├── CellFactory.h/cpp (renombrado de v2)
├── CellState.h (conservado, stub)
└── [ELIMINADOS: AgenticCell_v2.h/cpp, CellFactory_v2.h/cpp, OncoState.h]

tests/
├── CellLifeStageTransitionTest.cpp ✨ NEW
├── AgenticCellTest.cpp (replicado del viejo, adaptado)
└── ... (resto de tests)
```

---

## Logs: Antes vs Después

### Antes (genomic_instability_ único)
```
[Trace] genomic_instability: prev=1.0 -> next=2.5 | low_delta=0.0001 | high_delta=0.0002 | TP53=+/-
```

### Después (D1 + D2 separados)
```
[Trace] d1_update: prev=1.0 -> next=2.5 | delta_d1=0.001 | TP53=+/-
[Trace] d2_update: prev=1.0 -> next=3.2 | delta_d2=0.002 | BRCA1=-/-
[Trace] cell_stage: PRIMER (TP53=-/-, D1=2.5 > threshold=2.0)
```

---

## Referencias Documentales

- **Especificación de estados:** `.github/copilot/CELL_STATE_MACHINE.md`
- **Roles de agentes:** `.github/copilot/agent_roles.md`
- **Fases del ciclo:** `.github/copilot/cell_lifecycle_templates.md`
- **Diagrama de Luis:** `docs/diagrams_luis/resumen.txt`
- **Plan completo:** `plan-agenticCellRefactorD1D2.prompt.md`

---

## Notas Finales

1. **Orden crítico:** Siempre pasos 1→4 (construcción limpia) antes de 5 (integración).
2. **Compilación atómica:** Cada paso debe compilar exitosamente antes de seguir.
3. **Tests paralelos:** Viejos y nuevos deben coexistir y pasar juntos.
4. **Rollback:** Si algo falla, solo revertir CMakeLists.txt para rollback fácil.
5. **Vaciado final:** Paso 7 es limpieza pura, no lógica; ejecútalo una vez validado todo.

