# TL;DR: Refactorización AgenticCell D1+D2

## El Plan en 30 Segundos

Construir `AgenticCell_v2` limpia en paralelo (D1 DNA damage + D2 immunosuppression), validar con tests, luego reemplazar versión vieja y limpiar. **7 pasos atómicos, cero deuda técnica.**

---

## Cambios Principales

### Antes (genomic_instability único)
```cpp
double genomic_instability_ = 1.0;  // Todo en uno
// Fase 4: genomic_instability_ = genomic_instability_ * genomic_instability_ + delta
// Fase 2: apoptosis basada en genomic_instability_
```

### Después (D1 + D2 separados)
```cpp
double d1_dna_damage_ = 1.0;          // DNA damage (mutaciones)
double d2_immunosuppression_ = 1.0;   // Immune evasion

// Fase 4: actualiza ambos con deltas según BRCA1×TP53
d1 = min(d1² + delta_d1, 999);
d2 = min(d2² + delta_d2, 999);

// Fase 2: apoptosis basada en D2 (>5.0 resiste)
if (d2 > 5.0) ignore_apoptosis();
else die();
```

---

## 7 Pasos

| # | Acción | Archivos | Duración |
|---|--------|----------|----------|
| 1 | Crear enums + structs | CellLifeStage.h, InstabilityDeltas.h | 15 min |
| 2 | Header AgenticCell_v2 | AgenticCell_v2.h | 15 min |
| 3 | Implementación + tests | AgenticCell_v2.cpp, CellLifeStageTransitionTest.cpp | 60 min |
| 4 | Factory v2 | CellFactory_v2.h/cpp | 15 min |
| 5 | Integración Tissue | Tissue.cpp, CMakeLists.txt | 30 min |
| 6 | CMakeLists cleanup | tests/CMakeLists.txt | 10 min |
| 7 | Eliminar viejos | rm AgenticCell.cpp/h, etc. | 5 min |

**Total:** ~2.5-3 horas

---

## Matriz de Instabilidad (BRCA1 × TP53)

```
BRCA1 +/-  & TP53 +/+ → δd1=LOW,    δd2=LOW      (BASELINE)
BRCA1 +/-  & TP53 +/- → δd1=MEDIUM, δd2=MEDIUM  (INTERMEDIO1)
BRCA1 +/-  & TP53 -/- → δd1=HIGH,   δd2=HIGH    (INTERMEDIO2)
BRCA1 -/-  & TP53 -/- → δd1=V_HIGH, δd2=V_HIGH  (INTERMEDIO2)
```

---

## Estados Celulares (Derivados On-the-Fly)

```
BASELINE      → Célula normal
INTERMEDIO1   → TP53 +/- & BRCA1 +/-
INTERMEDIO2   → TP53 -/- & BRCA1 -/-
PRIMER        → TP53 -/- & D1 > 2.0 (pretumoral)
TUMORAL       → is_neoplastic_ == true
```

---

## Decisiones Clave

### Fase 2 (Endocytosis): Apoptosis
```
D2 > 5.0  → RESISTE (célula sobrevive)
D2 ≤ 5.0  → ACEPTA (célula muere)
```

### Fase 4 (Cytoplasmic Remodeling)
```
1. Calcula deltas según BRCA1×TP53
2. D1 ← min(D1² + delta_d1, 999)
3. D2 ← min(D2² + delta_d2, 999)
4. Si TP53 -/- & D1 > 2.0 → entra PRIMER
5. Detecta neoplasm (desarrollar tumoral)
```

### Clone (Herencia)
```
Hija hereda D1 y D2 del padre
Representa "inestabilidad parental"
```

---

## Logs

### Antes
```
[Trace] genomic_instability: prev=1.0 -> next=2.5
```

### Después
```
[Trace] d1_update: prev=1.0 -> next=2.5 | delta_d1=0.001
[Trace] d2_update: prev=1.0 -> next=3.2 | delta_d2=0.002
[Trace] cell_stage: PRIMER (D1=2.5 > threshold=2.0)
```

---

## Archivos Finales

```
src/domain/cell/
├── CellLifeStage.h ✨
├── InstabilityDeltas.h ✨
├── AgenticCell.h/cpp (fue v2)
└── CellFactory.h/cpp (fue v2)

tests/
├── CellLifeStageTransitionTest.cpp ✨
└── ... (resto sin cambios)
```

---

## Rollback Fácil

Hasta **Paso 5**, si algo falla:
```bash
git checkout tests/CMakeLists.txt
# Y listo, vuelve a versión vieja
```

Después de **Paso 6**, código limpio + versión vieja eliminada = no hay rollback.

---

## Checklist Rápido

**Paso 1:** Compilar headers
```bash
cmake --build build && echo "✓ OK"
```

**Paso 3:** Tests nuevos pasan
```bash
ctest -R CellLifeStageTransition --output-on-failure
```

**Paso 5:** Todos los tests pasan
```bash
ctest --output-on-failure
```

**Paso 6:** CMakeLists limpio
```bash
grep -n "AgenticCell.cpp\|AgenticCell.h\|_v2" tests/CMakeLists.txt | wc -l
# Debe retornar 0 (no hay referencias viejas)
```

---

## Decisiones Pendientes

1. **Valores de deltas:** ¿Cuánto valen LOW, MEDIUM, HIGH, V_HIGH?
   - Sugerencia: 0.001, 0.002, 0.003, 0.004

2. **¿InstabilityDeltas como singleton o inyectada?**
   - Sugerencia: Inyectada en constructor (más flexible)

3. **¿Compatibilidad hacia atrás?**
   - NO (no hay ABI), eliminar versión vieja completamente

---

## Links Rápidos

- Plan detallado: `plan-agenticCellRefactorD1D2.prompt.md`
- Guía paso a paso: `GUIDE-agenticCellRefactor.md`
- Checklist completo: `CHECKLIST-agenticCellRefactor.md`
- Especificación: `.github/copilot/CELL_STATE_MACHINE.md`
- Diagrama de Luis: `docs/diagrams_luis/resumen.txt`

---

## Próximos Pasos

1. Revisar este documento con el equipo
2. Ejecutar Paso 1 (enums + structs)
3. Compilar y validar
4. Continuar Pasos 2-4 (construcción)
5. Integración (Paso 5)
6. Limpieza (Pasos 6-7)

**¿Comenzamos?**

