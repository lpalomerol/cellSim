# Plan de Trabajo: Refactorización AgenticCell D1+D2

**Proyecto**: cellSim - Simulación de Evolución Celular  
**Objetivo**: Refactorizar `AgenticCell` para separar `D1 (DNA damage)` de `D2 (immunosuppression)` con estados celulares explícitos  
**Inicio**: 2025-12-16  
**Duración estimada**: 2.5 - 3 horas  
**Estrategia**: Construcción v2 limpia en paralelo + reemplazo atómico + vaciado final

---

## 📋 Resumen Ejecutivo

### Problema Actual
La clase `AgenticCell` original usa un único contador `genomic_instability_` que mezcla dos aspectos biológicos independientes:
- **D1**: Daño al DNA (mutaciones acumuladas)
- **D2**: Evasión inmunológica (capacidad de resistir apoptosis)

Esto hace difícil rastrear y validar fenotipos neoplásticos.

### Solución Propuesta
Crear `AgenticCell_v2` con:
1. **Dos contadores separados**: `d1_dna_damage_` y `d2_immunosuppression_`
2. **Estados celulares explícitos**: Enum `CellLifeStage` con 5 estados derivados on-the-fly
3. **Matriz de instabilidad**: BRCA1 × TP53 determina deltas de crecimiento
4. **Lógica de apoptosis mejorada**: D2 decide resistencia (>5.0 resiste, ≤5.0 muere)
5. **Tests exhaustivos**: Cobertura de transiciones de estado
6. **Backward compatibility**: Coexistencia segura durante migración

### Beneficios
- ✅ Separación de concerns biológicos
- ✅ Validación más rigurosa de fenotipos
- ✅ Tests más claros y mantenibles
- ✅ Trazabilidad mejorada en logs
- ✅ Cero deuda técnica post-migración

---

## 🎯 Matriz de Decisión Biológica

### Estados Celulares (CellLifeStage)

| Estado | Condiciones | Significado Biológico |
|--------|-----------|----------------------|
| **BASELINE** | TP53 +/+ & BRCA1 +/- (default) | Célula normal, protegida |
| **INTERMEDIO1** | TP53 +/- & BRCA1 +/- | Reparación parcial, inestabilidad moderada |
| **INTERMEDIO2** | TP53 -/- & BRCA1 -/- | Sin protección, alta inestabilidad |
| **PRIMER** | TP53 -/- & D1 > 2.0 | Pre-tumoral, acumulación de daño |
| **TUMORAL** | is_neoplastic_ == true | Neoplasia establecida |
| **DEAD** | BRCA1 -/- solo | Muerte automática (apoptosis intrínseca) |

### Matriz BRCA1 × TP53 → Deltas de Instabilidad

```
BRCA1 +/-  & TP53 +/+ → δd1=LOW (0.001),     δd2=LOW (0.001)      [BASELINE]
BRCA1 +/-  & TP53 +/- → δd1=MEDIUM (0.002), δd2=MEDIUM (0.002)   [INTERMEDIO1]
BRCA1 +/-  & TP53 -/- → δd1=HIGH (0.004),   δd2=HIGH (0.004)     [INTERMEDIO2]
BRCA1 -/-  & TP53 -/- → δd1=V_HIGH (0.010), δd2=V_HIGH (0.010)   [MUERE]
```

### Lógica de Apoptosis Mejorada

**Phase 2 (Endocytosis)**:
- Apoptosis extrínseca usa **D2** (immunosuppression), no `genomic_instability_`
- Si `D2 > 5.0` → célula **resiste** apoptosis
- Si `D2 ≤ 5.0` → célula **acepta** apoptosis y muere
- Apoptosis intrínseca: Si BRCA1 -/- → **muerte automática**

---

## 🔧 Plan de Ejecución: 7 Pasos Atómicos

### Fase 1: Construcción Limpia (Pasos 1-4)

#### PASO 1: Enums y Structs Base (15 min)
**Archivos a crear**:
- `src/domain/cell/CellLifeStage.h` (headers-only)
  - `enum class CellLifeStage { BASELINE, INTERMEDIO1, INTERMEDIO2, PRIMER, TUMORAL }`
  - `std::string toString(CellLifeStage stage)`
  - Documentación de cada estado

- `src/domain/cell/InstabilityDeltas.h` (headers-only)
  - Matriz BRCA1×TP53 con deltas configurables
  - Método `getDeltas(tp53_status, brca1_status) → pair<double, double>`
  - Valores por defecto: LOW, MEDIUM, HIGH, V_HIGH

**Validación**:
- ✅ Compilación exitosa (headers-only)
- ✅ Sin warnings
- ✅ Inclusibles sin dependencias

---

#### PASO 2: Interface AgenticCell_v2.h (15 min)
**Archivo a crear**:
- `src/domain/cell/AgenticCell_v2.h`

**Estructura**:
```cpp
class AgenticCell_v2 : public ICell, public ILoggeable {
    // Nuevos atributos
    double d1_dna_damage_ = 1.0;
    double d2_immunosuppression_ = 1.0;
    double d1_primer_threshold_ = 2.0;  // parametrizable
    double d2_apoptosis_threshold_ = 5.0;  // parametrizable
    
    // Getters nuevos
    double getD1() const;
    double getD2() const;
    CellLifeStage getCurrentCellLifeStage() const;
    
    // Métodos heredados de ICell
    void live() override;
    bool alive() const override;
    bool isNeoplastic() const override;
    std::unique_ptr<ICell> clone() const override;
    
    // Fases privadas (0-5)
    // phase2_Endocytosis() usa D2, no genomic_instability_
    // phase4_Cytoplasmic() actualiza D1 y D2
};
```

**Validación**:
- ✅ Header compila
- ✅ Sintaxis correcta
- ✅ Interfaces implementadas

---

#### PASO 3: Implementación AgenticCell_v2.cpp (60 min)

**3.1: Constructor y getters** (~80 líneas)
- Inicializa D1=1.0, D2=1.0, thresholds parametrizables
- Acepta TP53 y BRCA1 en constructor
- Getters para D1, D2, getCurrentCellLifeStage

**3.2: getCurrentCellLifeStage()** (~55 líneas)
- Derivar estado on-the-fly según genotipos y valores D1/D2
- Prioridad: DEAD > TUMORAL > PRIMER > INTERMEDIO2 > INTERMEDIO1 > BASELINE

**3.3: phase2_Endocytosis()** (~40 líneas)
- Reemplazar lógica de `genomic_instability_` con D2
- Si D2 > threshold → resiste apoptosis
- Si D2 ≤ threshold → acepta apoptosis
- Logs: `"d1=X d2=Y | apoptosis_decision=..."`

**3.4: phase4_Cytoplasmic()** (~45 líneas)
- Calcular deltas: `auto [delta_d1, delta_d2] = InstabilityDeltas::getDeltas(tp53, brca1)`
- Actualizar D1: `d1 = min(d1² + delta_d1, 999)`
- Actualizar D2: `d2 = min(d2² + delta_d2, 999)`
- Detectar transición a PRIMER: TP53 -/- && D1 > threshold
- Logs separados: `d1_update`, `d2_update`, `cell_stage`

**3.5: Fases 0, 1, 3, 5 + helpers** (~150 líneas)
- Copiar lógica del AgenticCell original
- Sin cambios excepto logs (reemplazar `genomic_instability_` por `d1` y `d2`)
- `clone()`: hereda D1, D2, thresholds, neoplastic status

**3.6: Tests CellLifeStageTransitionTest.cpp** (~250 líneas)
```cpp
TEST(CellLifeStageTransition, TP53HetBRCA1Het_IsINTERMEDIO1)
TEST(CellLifeStageTransition, TP53NullBRCA1Null_IsINTERMEDIO2)
TEST(CellLifeStageTransition, TP53Null_D1GreaterThan2_IsPRIMER)
TEST(CellLifeStageTransition, ApoptosisSignal_D2GreaterThan5_Survives)
TEST(CellLifeStageTransition, ApoptosisSignal_D2LessEqualThan5_Dies)
TEST(CellLifeStageTransition, BRCA1Null_AutomaticDeath)
TEST(CellLifeStageTransition, Clone_InheritsD1D2)
TEST(CellLifeStageTransition, D1Update_FollowsMatrix)
TEST(CellLifeStageTransition, D2Update_FollowsMatrix)
TEST(CellLifeStageTransition, ParametrizedThresholds_Respected)
```

**3.7: Parametrizar Thresholds** (~30 líneas)
- `d1_primer_threshold_` (default 2.0)
- `d2_apoptosis_threshold_` (default 5.0)
- Usados en `getCurrentCellLifeStage()` y `phase2_Endocytosis()`
- Heredados en `clone()`

**Cambios a archivos existentes**:
- `tests/CMakeLists.txt`: Agregar CellLifeStageTransitionTest.cpp

**Validación**:
- ✅ Compila sin errores
- ✅ 10 tests pasan
- ✅ Coverage de casos críticos

---

#### PASO 4: Factory v2 (15 min)
**Archivos a crear**:
- `src/domain/cell/CellFactory_v2.h/cpp`

**Interfaz**:
```cpp
class CellFactory_v2 {
    static std::unique_ptr<ICell> createAgenticCell_v2(
        int tp53_status, 
        int brca1_status,
        double d1_primer_threshold = 2.0,
        double d2_apoptosis_threshold = 5.0
    );
    
    static std::unique_ptr<ICell> createCustomCell_v2(
        const Genome& genome,
        int age,
        double d1_primer_threshold = 2.0,
        double d2_apoptosis_threshold = 5.0
    );
};
```

**Cambios a archivos existentes**:
- `tests/CMakeLists.txt`: Agregar CellFactory_v2.cpp/h

**Validación**:
- ✅ Factory compila
- ✅ Métodos crean AgenticCell_v2 correctamente

---

### Fase 2: Integración (Pasos 5-6)

#### PASO 5: Integración Tissue (30 min)

**5.1: Crear TissueV2** (~150 líneas)
**Archivos a crear**:
- `src/domain/tissue/TissueV2.h/cpp`

**Interfaz**:
```cpp
class TissueV2 : public ILoggeable {
    std::vector<std::unique_ptr<ICell>> cells_;
    std::map<CellLifeStage, int> stage_counts_;
    
    void addCell(std::unique_ptr<ICell> cell);
    void live();
    
    // Métodos nuevos
    std::vector<ICell*> getCellsByStage(CellLifeStage stage);
    int getCountByStage(CellLifeStage stage) const;
    
    // Métodos heredados
    std::size_t size() const;
    ICell* getCell(std::size_t idx);
    std::vector<ICell*> getAllCells();
};
```

**5.2: Integración con Simulation** (~30 líneas)

**Archivo a modificar**: `src/application/simulation/Simulation.h/cpp`

**Cambios**:
- Agregar miembro: `std::unique_ptr<TissueV2> tissue_v2_`
- Agregar miembro: `bool use_tissue_v2_ = false`
- Agregar método: `void enableTissueV2(bool enable)`
- Refactorizar `executeCellCycle()` con lógica condicional

**Archivo a crear**: `src/application/simulation/TissueV2Adapter.h`
- Wrapper que expone interfaz compatible, delegando a TissueV2

**5.3: Crear Tests de Integración** (~192 líneas)

**Archivo a crear**: `tests/SimulationTissueV2IntegrationTest.cpp`

```cpp
TEST(SimulationTissueV2Integration, EnableTissueV2Flag)
TEST(SimulationTissueV2Integration, CellTransferOnEnable)
TEST(SimulationTissueV2Integration, ExecuteCellCycle_WithV2)
TEST(SimulationTissueV2Integration, CellLifeStageTracking)
TEST(SimulationTissueV2Integration, ApoptosisLogic_V2)
TEST(SimulationTissueV2Integration, CloneInheritance_V2)
TEST(SimulationTissueV2Integration, NeoplasticDetection_V2)
TEST(SimulationTissueV2Integration, RuntimeSwitch_Reversible)
TEST(SimulationTissueV2Integration, StageCountAccuracy)
TEST(SimulationTissueV2Integration, ParallelExecution_Comparable)
```

**Cambios a archivos existentes**:
- `CMakeLists.txt`: Agregar TissueV2.cpp/h
- `tests/CMakeLists.txt`: Agregar SimulationTissueV2IntegrationTest.cpp, TissueV2Adapter.h

**Validación**:
- ✅ Compilación exitosa (Simulation + TissueV2 + tests)
- ✅ Tests nuevos: 10/10 pasan
- ✅ Tests antiguos: Sin regresiones
- ✅ Flag `use_tissue_v2_` reversible en runtime

---

#### PASO 6: Migración Application Layer (20 min)

**Cambios**:
- `app/run_all_scenarios.cpp`: 
  - Reemplazar `Tissue` → `TissueV2`
  - Reemplazar `CellFactory` → `CellFactory_v2`
  - Reemplazar `AgenticCell*` → `AgenticCell_v2*`
  - Acceder a `getD1()`, `getD2()` para tracking

- `CMakeLists.txt` (targets app/):
  - Agregar `CellFactory_v2.cpp/h`, `TissueV2.cpp/h`, `AgenticCell_v2.cpp/h` a targets

**Validación**:
- ✅ run_all_scenarios compila
- ✅ Escenarios ejecutan con V2
- ✅ Outputs generan datos D1/D2

---

### Fase 3: Limpieza (Paso 7)

#### PASO 7: Deprecación y Cleanup (20 min)

**7.1: Marcar Tissue.h como Deprecated**
```cpp
class [[deprecated("Use TissueV2 instead")]] Tissue : public ILoggeable {
    // ...
};
```

**7.2: Mantener Tests Selectivamente**
- ✅ `CellDivisionIntegrationTest`: Usa V1 (si no hay breaking changes)
- ✅ `TissueV2Test`: Usa V2 (nativa)
- ✅ `SimulationTissueV2IntegrationTest`: Usa V2 (integration)
- ✅ Tests legacy: Sin cambios (warning [[deprecated]] es esperado)

**7.3: Validación Final**
```bash
cmake --build build
ctest --output-on-failure
# Esperado: 87+ tests pasan, warnings de deprecated OK
```

**Resultado final**:
- ✅ Tissue V1 marcado como deprecated
- ✅ Application layer migrado a V2
- ✅ Todos los targets compilan
- ✅ 87+ tests pasan
- ✅ Cero deuda técnica

---

## 📊 Cronograma Estimado

| Paso | Tarea | Duración | Acumulado |
|------|-------|----------|----------|
| 1 | Enums + structs | 15 min | 15 min |
| 2 | Header AgenticCell_v2 | 15 min | 30 min |
| 3 | Implementación + tests | 60 min | 90 min |
| 4 | Factory v2 | 15 min | 105 min |
| 5 | Integración Tissue + Simulation | 50 min | 155 min |
| 6 | Migración Application | 20 min | 175 min |
| 7 | Deprecación + cleanup | 20 min | **195 min (~3.25 h)** |

**Total**: 2.5 - 3.5 horas (según experiencia)

---

## 🛡️ Riesgos y Mitigaciones

| Riesgo | Probabilidad | Mitigación |
|--------|-------------|-----------|
| Tests fallan en Paso 3 | Media | Caso de prueba: ejecutar pruebas incrementalmente |
| Linking en app/ targets | Baja | Agregar archivos a CMakeLists.txt correctamente |
| Regression en tests legacy | Baja | Mantener V1 durante migración, coexistencia segura |
| D1/D2 no convergen | Muy baja | Usar matriz BRCA1×TP53 validada biológicamente |

---

## 📋 Rollback Plan

**Hasta Paso 5**: Reversible sin problema
```bash
git checkout tests/CMakeLists.txt
# Y listo, versión vieja restaurada
```

**Después de Paso 6**: Código V2 limpio + V1 deprecated
- No hay rollback, pero V1 sigue disponible para debugging

---

## ✅ Criterios de Aceptación Final

- [x] AgenticCell_v2 compila sin errores
- [x] CellLifeStageTransitionTest: 10/10 tests pasan
- [x] TissueV2 integrada con Simulation
- [x] SimulationTissueV2IntegrationTest: 10/10 tests pasan
- [x] run_all_scenarios usa V2 correctamente
- [x] Tests legacy sin regresiones
- [x] Tissue.h marked as deprecated
- [x] Total: 87+ tests pasan

---

## 📝 Notas Biológicas Importantes

✅ **BRCA1**: Solo puede ser +/- o -/- (nunca +/+)  
✅ **TP53**: Puede ser +/+, +/-, o -/-  
✅ **Apoptosis Intrínseca**: BRCA1 -/- → muerte automática  
✅ **Apoptosis Extrínseca**: D2 decide resistencia (>5.0 resiste)  
✅ **Transición a TUMORAL**: Acumulación D1 + escape inmunológico D2  
✅ **Herencia**: Clones heredan D1, D2, thresholds y estado neoplástico  

---

**Próximo**: Ver `RESULTADOS-REFACTORIZACION-D1D2.md` para estado final de implementación

