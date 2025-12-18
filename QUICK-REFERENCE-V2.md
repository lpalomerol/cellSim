# ⚡ QUICK REFERENCE - cellSim V2 (2025-12-18)

## 🎯 Estado Actual
- ✅ **Refactorización D1+D2 completada**: 7 pasos + Paso 8 de limpieza
- ✅ **75/75 tests pasando**: cero warnings
- ✅ **4/4 ejecutables funcionales**
- ✅ **Tissue V1 eliminado completamente**: 100% V2

---

## 📁 Estructura Clave

```
cellSim/
├── src/domain/cell/
│   ├── AgenticCell_v2.h/cpp         ← Célula con D1 + D2
│   ├── CellLifeStage.h              ← 6 estados (DEAD, BASELINE, UNSTABLE, UNPROTECTED, PRIMER, TUMORAL)
│   ├── InstabilityDeltas.h          ← Matriz BRCA1×TP53 deltas
│   └── CellFactory_v2.h/cpp         ← Factory para crear células V2
│
├── src/domain/tissue/
│   └── TissueV2.h/cpp               ← Population orchestrator
│
├── app/
│   ├── main.cpp                     ← Simulador principal
│   ├── run_all_scenarios.cpp        ← 12 escenarios (migrado a V2)
│   ├── single_cell_evolution.cpp    ← Célula individual (migrado a V2)
│   └── main_interactive.cpp         ← Modo interactivo (V2)
│
└── tests/
    ├── CellLifeStageTransitionTest.cpp       ← Tests estados
    ├── SimulationTissueV2IntegrationTest.cpp ← Tests integración
    └── ... (69 tests más)
```

---

## 🔑 Conceptos Clave

### D1 vs D2
```cpp
double d1_dna_damage_;          // Mutaciones (aumenta con cada ciclo)
double d2_immunosuppression_;   // Evasión inmune (aumenta con cada ciclo)

// Fase 4: Ambos se actualizan según BRCA1×TP53
d1 = min(d1² + delta_d1, 999)
d2 = min(d2² + delta_d2, 999)

// Fase 2: Apoptosis basada en D2
if (d2 > 5.0) survive();  // D2 > 5.0 → resiste
else die();               // D2 ≤ 5.0 → muere
```

### Matriz de Deltas
```
BRCA1 +/-  & TP53 +/+ → delta = 0.001 (LOW)
BRCA1 +/-  & TP53 +/- → delta = 0.002 (MEDIUM)
BRCA1 +/-  & TP53 -/- → delta = 0.003 (HIGH)
BRCA1 -/-  & TP53 -/- → delta = 0.004 (VERY_HIGH)
```

### CellLifeStage (derivado on-the-fly)
```
DEAD        ← !alive()
BASELINE    ← TP53 +/+ & BRCA1 +/-
UNSTABLE    ← TP53 +/- & BRCA1 +/-
UNPROTECTED ← TP53 -/-
PRIMER      ← TP53 -/- & D1 > 2.0 (pretumoral)
TUMORAL     ← is_neoplastic_ == true
```

---

## 🚀 Comandos Útiles

### Compilar
```bash
cd cellSim
cmake --build build
```

### Tests
```bash
ctest -V                           # Todos los tests (verboso)
ctest -R CellLifeStageTransition   # Tests específicos
ctest --output-on-failure          # Solo fallidos
```

### Ejecutables
```bash
./build/cellSim                    # Simulador principal (100 escenarios, 1000 células, 80 años)
./build/run_all_scenarios          # 12 escenarios predefinidos
./build/single_cell_evolution      # Célula individual (100 años)
./build/interactive                # Modo interactivo
```

### Ver commits recientes
```bash
git log --oneline -10              # Últimos 10 commits
git show ddf90a1                   # Ver PASO 8 (limpieza V1)
```

---

## 📊 Cambios Respecto a V1

| Aspecto | V1 | V2 |
|---------|----|----|
| **Instabilidad** | 1 contador (`genomic_instability_`) | 2 contadores (`d1`, `d2`) |
| **Apoptosis** | Basada en `genomic_instability_` | Basada en `d2` (D2 > 5.0 resiste) |
| **Deltas** | Uniform | Matriz BRCA1×TP53 (4 valores) |
| **Estados** | Implícitos | CellLifeStage explícito (6 valores) |
| **Tissue** | V1 (deprecated) | TissueV2 (actual) |
| **Tests** | 87 tests (incluye legacy) | 75 tests (100% V2) |
| **Arquitectura** | Tissue en Application | TissueV2 en Domain + Adapter |

---

## 🔍 Debugging Rápido

### Ver estado de una célula en logs
```
[Phase0] Cycle start: stage=BASELINE, age=0, d1=1.000000, d2=1.000000
[Phase3] Genome status: TP53=+/+, BRCA1=+/-
[Phase4] d1_update: 1.000000 → 1.001000 (delta=0.001000)
[Phase4] d2_update: 1.000000 → 1.001000 (delta=0.001000)
```

### Verificar matriz de deltas
Ver: `src/domain/cell/InstabilityDeltas.h` (getDeltas method)

### Revisar transiciones de estados
Ver: `src/domain/cell/AgenticCell_v2.cpp` (getCurrentCellLifeStage method)

### Verificar apoptosis
Ver: `src/domain/cell/AgenticCell_v2.cpp` (phase2_Endocytosis method)

---

## 📝 Próximos Pasos Recomendados

1. **Validación clínica** de deltas (¿son realistas?)
   → Ver `PROXIMOS-PASOS-POST-V2.md` INICIATIVA 1

2. **Regenerar traces** con V2 para comparar vs V1
   → Ver `PROXIMOS-PASOS-POST-V2.md` INICIATIVA 2

3. **Documentar comportamiento** en QUICKSTART.md
   → Ver `PROXIMOS-PASOS-POST-V2.md` INICIATIVA 4

---

## 📦 Files de Referencia

- `RESUMEN-EJECUCION-REFACTORIZATION-V2.md` ← Documento completo de lo que se hizo
- `PROXIMOS-PASOS-POST-V2.md` ← Plan de continuación
- `.github/.archives/20251218.agentic_ver2/` ← Documentación detallada de cada paso

---

## 🧪 Tests Importantes

```bash
# Validar CellLifeStage
./build/tests/unit_tests --gtest_filter=CellLifeStageTransitionTest

# Validar TissueV2
./build/tests/unit_tests --gtest_filter=TissueV2Test

# Validar integración
./build/tests/unit_tests --gtest_filter=SimulationTissueV2IntegrationTest

# Todos los tests
ctest
```

---

## 🎯 Commits Importantes (git)

```
ddf90a1 - PASO 8: Remove legacy Tissue V1 completely
8c5ba8c - CellFactory_v2 + TissueV2 integration
fdff498 - Configurable thresholds for D1 and D2
9863494 - Enhance AgenticCell_v2 with apoptosis checks
8106999 - Implement AgenticCell_v2 with D1 and D2
1fa3706 - Add CellLifeStage enum and InstabilityDeltas
```

---

## 💡 Quick Tips

1. **Para agregar parámetro nuevo a AgenticCell_v2**
   - Agregar en header constructor
   - Agregar miembro con inicialización
   - Actualizar clone()
   - Actualizar logs en constructor

2. **Para cambiar delta values**
   - Modificar `InstabilityDeltas.h` constants
   - Recompilar
   - Regenerar traces

3. **Para cambiar threshold D1 → PRIMER**
   - Parámetro en constructor: `d1_primer_threshold`
   - Default: 2.0 (configurable)

4. **Para debuggear CellLifeStage transitions**
   - Ver método `getCurrentCellLifeStage()` en AgenticCell_v2.cpp
   - Agregar logs adicionales si needed

---

## ✅ Checklist Pre-Commit

- [ ] Tests pasan: `ctest` ✅
- [ ] Compila: `cmake --build build` ✅
- [ ] Sin warnings: `cmake --build build 2>&1 | grep warning` (vacío)
- [ ] Logs sensatos: revisar stdout/stderr
- [ ] Commits pequeños y descriptivos

---

**Última actualización**: 2025-12-18 09:45 UTC  
**Status**: ✅ Production Ready  
**Next Review**: Cuando ejecutes INICIATIVA 1 (Validación Clínica)

---

