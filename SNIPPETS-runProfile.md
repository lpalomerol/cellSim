# Snippets: Comandos para run_profile.sh

Colección de comandos listos para usar con `bash .github/scripts/run_profile.sh` para cada paso.

---

## Paso 1: Crear CellLifeStage.h

```bash
bash .github/scripts/run_profile.sh cpp \
  "Crear CellLifeStage.h: enum y helper toString()" \
  "Header-only. Enum con 5 valores: {BASELINE, INTERMEDIO1, INTERMEDIO2, PRIMER, TUMORAL}. Función toString() que retorna string descriptivo. Documentar cada estado biológicamente." \
  "Referencia: .github/copilot/CELL_STATE_MACHINE.md > Los 5 Estadios Celulares"
```

---

## Paso 1: Crear InstabilityDeltas.h

```bash
bash .github/scripts/run_profile.sh cpp \
  "Crear InstabilityDeltas.h: matriz BRCA1×TP53 para deltas" \
  "Header-only. Struct/class que encapsula 4 combinaciones (BRCA1 {+/-, -/-} × TP53 {+/+, +/-, -/-}). Método static getDeltas(tp53_status, brca1_status) retorna pair<double, double>. Valores: bajo=0.001, medio=0.002, alto=0.003, muy_alto=0.004." \
  "Referencia: docs/diagrams_luis/resumen.txt > Matriz de Decisión"
```

---

## Paso 2: Crear AgenticCell_v2.h

```bash
bash .github/scripts/run_profile.sh cpp \
  "Crear AgenticCell_v2.h: interface limpia con D1+D2" \
  "Header. Heredar ICell + ILoggeable. Nuevos atributos (privados): d1_dna_damage_ (double, 1.0), d2_immunosuppression_ (double, 1.0). Nuevos getters públicos: getD1(), getD2(), getCurrentCellLifeStage(). Mantener constructor igual a AgenticCell original. Declarar fases privadas (phase0-phase5). NO incluir genomic_instability_." \
  "Basarse en: src/domain/cell/AgenticCell.h líneas 1-100 como template. Context: agent_roles.md > AgenticCell responsibilities"
```

---

## Paso 3a: Implementar AgenticCell_v2.cpp

```bash
bash .github/scripts/run_profile.sh cpp \
  "Implementar AgenticCell_v2.cpp: core logic D1+D2 con 6 fases" \
  "Implementación completa. Constructor: inicia D1=1.0, D2=1.0. Phase0-1, Phase3, Phase5: igual a AgenticCell original (copiar lógica). Phase2 (Endocytosis): AJUSTADO - apoptosis usa D2 (si D2>5.0 resiste, sino muere). Phase4 (Cytoplasmic Remodeling): REFACTORIZADO - (1) calcula deltas con InstabilityDeltas::getDeltas(), (2) D1←min(D1²+delta_d1, 999), (3) D2←min(D2²+delta_d2, 999), (4) detecta PRIMER si TP53==-/- && D1>2.0, (5) logs separados d1_update/d2_update. Método getCurrentCellLifeStage(): getter on-the-fly (no mutable). Método clone(): hereda D1 y D2. Reemplazar todos los logs 'genomic_instability_' por 'd1=X d2=Y'." \
  "Referencia: src/domain/cell/AgenticCell.cpp líneas 169-230 (phase4 actual). Context: cell_lifecycle_templates.md > Phase 2, Phase 4. InstabilityDeltas::getDeltas(tp53, brca1)"
```

---

## Paso 3b: Crear CellLifeStageTransitionTest.cpp

```bash
bash .github/scripts/run_profile.sh cpp \
  "Crear CellLifeStageTransitionTest.cpp: 5 test cases" \
  "Google Test. 5 test cases: (1) TP53+/- & BRCA1+/- iniciales → getCurrentCellLifeStage() retorna INTERMEDIO1, (2) TP53-/- & BRCA1-/- iniciales → retorna INTERMEDIO2, (3) TP53-/- & D1 mutado >2.0 → retorna PRIMER, (4) PRIMER + apoptosis signal + D2>5.0 → cell.alive()==true después de fase2, (5) PRIMER + apoptosis signal + D2≤5.0 → cell.alive()==false después de fase2. Usar FakeNoise para determinismo." \
  "Basarse en: tests/AgenticCellTest.cpp líneas 1-50 como template. Context: docs/diagrams_luis/resumen.txt > Matriz de Decisión"
```

---

## Paso 4: Crear CellFactory_v2.h/cpp

```bash
bash .github/scripts/run_profile.sh cpp \
  "Crear CellFactory_v2.h/cpp: builder para AgenticCell_v2" \
  "Header + implementación. Clase factory equivalente a CellFactory actual pero instancia AgenticCell_v2. Métodos públicos: makeDefaultGenome(), etc. Mantener firma igual a original. Inyectar dependencias correctamente (noise, genome, logger)." \
  "Basarse en: src/domain/cell/CellFactory.h/cpp como template"
```

---

## Paso 5: Integración Tissue.cpp

```bash
bash .github/scripts/run_profile.sh cpp \
  "Integrar AgenticCell_v2 en Tissue.cpp: reemplazar includes y uso" \
  "Cambios mínimos: (1) #include 'AgenticCell_v2.h' en lugar de 'AgenticCell.h', (2) En métodos que usan CellFactory, cambiar a CellFactory_v2. Lógica de Tissue sin cambios. Compilar y ejecutar todos los tests: viejos deben pasar (usan Tissue, no AgenticCell directo), nuevos de CellLifeStageTransition deben pasar." \
  "Referencia: src/domain/tissue/Tissue.cpp líneas 1-50"
```

---

## Paso 6: CMakeLists.txt - Remover viejos y renombrar v2

```bash
# Manual (no necesita run_profile.sh)

# 1. Abrir tests/CMakeLists.txt
# 2. Encontrar y comentar/remover líneas:
#    - ../src/domain/cell/AgenticCell.cpp
#    - ../src/domain/cell/AgenticCell.h

# 3. Renombrar en CMakeLists.txt:
#    - AgenticCell_v2.cpp → AgenticCell.cpp
#    - AgenticCell_v2.h → AgenticCell.h
#    - CellFactory_v2.cpp → CellFactory.cpp
#    - CellFactory_v2.h → CellFactory.h

# 4. Compilar:
cmake --build build

# 5. Si OK:
ctest --output-on-failure
```

---

## Paso 7: Vaciado final

```bash
# Manual (limpieza de disco)

# 1. Eliminar versiones viejas del disco:
rm src/domain/cell/AgenticCell.h
rm src/domain/cell/AgenticCell.cpp
rm src/domain/cell/CellFactory.h  # si existe
rm src/domain/cell/CellFactory.cpp

# 2. Renombrar versiones v2:
mv src/domain/cell/AgenticCell_v2.h src/domain/cell/AgenticCell.h
mv src/domain/cell/AgenticCell_v2.cpp src/domain/cell/AgenticCell.cpp
mv src/domain/cell/CellFactory_v2.h src/domain/cell/CellFactory.h
mv src/domain/cell/CellFactory_v2.cpp src/domain/cell/CellFactory.cpp

# 3. Actualizar CMakeLists.txt (renombrar en referencia final)
# Ya debería estar hecho en Paso 6

# 4. Compilar final:
cmake --build build

# 5. Validar:
ctest --output-on-failure
```

---

## Verificación Rápida (Todos los pasos)

```bash
# Después de cada paso, ejecutar:

# Compilación limpia
cmake --build build 2>&1 | grep -E "error|warning" | head -20

# Tests específicos
ctest -R CellLifeStage --output-on-failure  # después de Paso 3

# Todos los tests
ctest --output-on-failure

# Búsqueda de código viejo (después de Paso 6)
grep -r "genomic_instability_" src/ | head -10
# Debe retornar: NADA (si retorna algo, hay referencias viejas)
```

---

## Troubleshooting

### Compilación falla en Paso 2 (header AgenticCell_v2.h)

```bash
# Verificar includes
grep -n "#include" src/domain/cell/AgenticCell_v2.h

# Debe incluir:
# #include "../ports/ICell.h"
# #include "../ports/ILoggeable.h"
# etc.
```

### Tests falla en Paso 3

```bash
# Verificar que FakeNoise está disponible
ls -la tests/FakeNoise.h

# Compilar solo tests nuevos
ctest -R CellLifeStageTransition -V
```

### CMakeLists.txt tiene duplicados

```bash
# Buscar líneas duplicadas
grep -n "AgenticCell\|CellFactory" tests/CMakeLists.txt | sort | uniq -d

# Si hay duplicados, limpiar manualmente
```

---

## Notas Finales

1. Cada comando con `run_profile.sh` genera prompt en `/tmp/copilot_prompt.txt`
2. Copiar contenido y pegar en Copilot Chat en CLion
3. Validar compilación después de cada paso
4. Si algo falla, rollback es fácil hasta Paso 5 (solo revertir CMakeLists.txt)
5. Paso 7 es irreversible (elimina archivos), ejecutar solo si Paso 6 pasó completamente

