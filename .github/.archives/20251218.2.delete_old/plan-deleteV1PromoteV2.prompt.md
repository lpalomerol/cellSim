# Plan: Eliminar V1 y Promover V2 a Base

**Objetivo**: Convertir AgenticCell_v2 y CellFactory_v2 en las clases base, eliminando V1 completamente  
**Fecha**: 2025-12-18  
**Duración estimada**: 1.5 - 2 horas  
**Estrategia**: Renombrar V2 a base, eliminar V1, actualizar referencias

---

## 📋 Resumen Ejecutivo

### Estado Actual
- ✅ AgenticCell_v2 completamente implementada y funcional
- ✅ CellFactory_v2 completamente implementada y funcional
- ✅ Tissue.h/cpp ya usan **solo** AgenticCell_v2
- ✅ **Cero referencias a V1** en código fuente (grep verificó)
- ⚠️ Archivos V1 (AgenticCell.h/cpp, CellFactory.h/cpp) aún existen

### Decisión
Eliminar completamente V1 y renombrar V2 a versión base:
- `AgenticCell_v2.h` → `AgenticCell.h`
- `AgenticCell_v2.cpp` → `AgenticCell.cpp`
- `CellFactory_v2.h` → `CellFactory.h`
- `CellFactory_v2.cpp` → `CellFactory.cpp`

### Beneficios
- ✅ Claridad: No hay ambigüedad de versiones
- ✅ Simplificación: Nombres más limpios
- ✅ Mantenibilidad: Menos archivos que entender
- ✅ Tests: Todos apuntan a versión correcta
- ✅ Zero risk: V1 no se usa en ningún lado

---

## 🔧 Plan de Ejecución: 4 Fases

### Fase 1: Análisis Previo (5 min)

#### Paso 1.1: Verificar Referencias V1
**Acción**: Confirmar que NO hay referencias a V1 en código
```bash
grep -r "AgenticCell\.h" src/ app/ tests/
grep -r "CellFactory\.h" src/ app/ tests/
# Esperado: 0 resultados
```

**Status**: ✅ Ya verificado (grep encontró cero resultados)

#### Paso 1.2: Listar Archivos a Eliminar
**Archivos V1 (para eliminar)**:
- `src/domain/cell/AgenticCell.h`
- `src/domain/cell/AgenticCell.cpp`
- `src/domain/cell/CellFactory.h`
- `src/domain/cell/CellFactory.cpp`

**Archivos V2 (para renombrar)**:
- `src/domain/cell/AgenticCell_v2.h` → `AgenticCell.h`
- `src/domain/cell/AgenticCell_v2.cpp` → `AgenticCell.cpp`
- `src/domain/cell/CellFactory_v2.h` → `CellFactory.h`
- `src/domain/cell/CellFactory_v2.cpp` → `CellFactory.cpp`

---

### Fase 2: Actualizar Código (30 min)

#### Paso 2.1: Actualizar includes en Tissue.h
**Archivo**: `src/domain/tissue/Tissue.h`

**Cambios**:
- Actualizar comentario: `TissueV2` → `Tissue`
- Include: `#include "../cell/AgenticCell_v2.h"` → `#include "../cell/AgenticCell.h"` (después de renombrar)

**Validación**:
- ✅ Compila sin errores
- ✅ No hay referencias a V1

---

#### Paso 2.2: Actualizar includes en Tissue.cpp
**Archivo**: `src/domain/tissue/Tissue.cpp`

**Cambios**:
- Include: `#include "../cell/AgenticCell_v2.h"` → `#include "../cell/AgenticCell.h"` (después de renombrar)
- Cambiar cast: `dynamic_cast<AgenticCell_v2*>` → `dynamic_cast<AgenticCell*>` (en método getCellsByStage aproximadamente línea ~57)

**Validación**:
- ✅ Compila sin errores
- ✅ Live y getCellsByStage() funcionan

---

#### Paso 2.3: Actualizar includes en CellFactory_v2.h
**Archivo**: `src/domain/cell/CellFactory_v2.h`

**Cambios**:
- Verificar que include `AgenticCell_v2.h`
- Cambiar a: `AgenticCell.h` (después de renombrar)

**Validación**:
- ✅ Compila sin errores

---

#### Paso 2.4: Actualizar includes en CellFactory_v2.cpp
**Archivo**: `src/domain/cell/CellFactory_v2.cpp`

**Cambios**:
- Incluye: `#include "AgenticCell_v2.h"`
- Cambiar a: `#include "AgenticCell.h"` (después de renombrar)
- Return type: `new AgenticCell_v2(...)` → `new AgenticCell(...)` (después de renombrar)

**Validación**:
- ✅ Compila sin errores
- ✅ Factory crea correctamente

---

#### Paso 2.5: Actualizar includes en Simulation.h/cpp
**Archivo**: `src/application/simulation/Simulation.h`

**Cambios**:
- Include: `#include "path/to/CellFactory_v2.h"` → `#include "path/to/CellFactory.h"`

**Archivo**: `src/application/simulation/Simulation.cpp`

**Cambios**:
- Include: `#include "../domain/cell/CellFactory_v2.h"` → `#include "../domain/cell/CellFactory.h"`
- Factory calls: `CellFactory_v2::create...` → `CellFactory::create...`

**Validación**:
- ✅ Compila sin errores
- ✅ Simulation funciona

---

#### Paso 2.6: Actualizar includes en TissueV2Adapter.h
**Archivo**: `src/application/simulation/TissueV2Adapter.h`

**Cambios**:
- Verificar includes, cambiar referencias de V2 a base si existen

**Validación**:
- ✅ Compila sin errores

---

#### Paso 2.7: Actualizar apps
**Archivos**: `app/*.cpp`

**Cambios en run_all_scenarios.cpp**:
- Include: `#include "CellFactory_v2.h"` → `#include "CellFactory.h"`
- Factory calls: `CellFactory_v2::create...` → `CellFactory::create...`

**Cambios en main.cpp, main_interactive.cpp, single_cell_evolution.cpp**:
- Si usan V1: cambiar a nuevas referencias base
- Verificar compilación

**Validación**:
- ✅ Todas las apps compilan
- ✅ Sin warnings

---

#### Paso 2.8: Actualizar tests
**Archivos**: `tests/*.cpp`

**Cambios en AgenticCellTest.cpp** (si existe):
- Include: `#include "AgenticCell_v2.h"` → `#include "AgenticCell.h"`
- Factory/crear: `CellFactory_v2::create...` → `CellFactory::create...`

**Cambios en otros tests**:
- Si importan Cell o Factory: actualizar includes
- Cambiar referencias _v2 a base

**Validación**:
- ✅ Todos los tests compilan
- ✅ Todos los tests pasan

---

#### Paso 2.9: Actualizar CMakeLists.txt (raíz)
**Archivo**: `CMakeLists.txt`

**Cambios**:
- Buscar referencias a `AgenticCell_v2.cpp/h` → reemplazar por `AgenticCell.cpp/h`
- Buscar referencias a `CellFactory_v2.cpp/h` → reemplazar por `CellFactory.cpp/h`

**Validación**:
- ✅ Compila todos los targets
- ✅ Linking sin errores

---

#### Paso 2.10: Actualizar tests/CMakeLists.txt
**Archivo**: `tests/CMakeLists.txt`

**Cambios**:
- Buscar referencias a `AgenticCell_v2` → reemplazar por `AgenticCell`
- Buscar referencias a `CellFactory_v2` → reemplazar por `CellFactory`

**Validación**:
- ✅ Tests compilan
- ✅ Sin warnings

---

### Fase 3: Renombrar Archivos (30 min)

#### Paso 3.1: Renombrar AgenticCell_v2.h → AgenticCell.h
```bash
cd src/domain/cell
mv AgenticCell_v2.h AgenticCell.h
```

**Cambio en archivo**:
- `#pragma once` (mantener)
- Clase sigue siendo igual

**Validación**: ✅ No hay cambios de contenido, solo nombre

---

#### Paso 3.2: Renombrar AgenticCell_v2.cpp → AgenticCell.cpp
```bash
cd src/domain/cell
mv AgenticCell_v2.cpp AgenticCell.cpp
```

**Cambio en archivo**:
- Include: `#include "AgenticCell_v2.h"` → `#include "AgenticCell.h"`

**Validación**:
- ✅ Compila sin errores
- ✅ Lógica sin cambios

---

#### Paso 3.3: Renombrar CellFactory_v2.h → CellFactory.h
```bash
cd src/domain/cell
mv CellFactory_v2.h CellFactory.h
```

**Cambio en archivo**:
- Include: `#include "AgenticCell_v2.h"` → `#include "AgenticCell.h"`

**Validación**: ✅ Compila sin errores

---

#### Paso 3.4: Renombrar CellFactory_v2.cpp → CellFactory.cpp
```bash
cd src/domain/cell
mv CellFactory_v2.cpp CellFactory.cpp
```

**Cambio en archivo**:
- Include: `#include "CellFactory_v2.h"` → `#include "CellFactory.h"`
- Include: `#include "AgenticCell_v2.h"` → `#include "AgenticCell.h"`

**Validación**:
- ✅ Compila sin errores
- ✅ Factory funciona

---

### Fase 4: Limpiar y Validar (30 min)

#### Paso 4.1: Eliminar archivos V1
```bash
cd src/domain/cell
rm -f AgenticCell.h.bak AgenticCell.cpp.bak    # si existen backups
rm -f CellFactory.h.bak CellFactory.cpp.bak    # si existen backups
```

**Status**: Los archivos V1 originales ya no existen (fueron sobrescritos)

**Validación**:
- ✅ Directorio limpio: solo AgenticCell.h/cpp, CellFactory.h/cpp (nuevos)

---

#### Paso 4.2: Compilación Completa
```bash
cd /home/luis/CLionProjects/cellSim
cmake --build cmake-build-debug --target all -j4
```

**Esperado**:
- ✅ Cero errores de compilación
- ✅ Cero warnings relevantes
- ✅ Todos los targets linkan

**Targets a verificar**:
- cellSim ✅
- interactive ✅
- run_all_scenarios ✅
- single_cell_evolution ✅
- unit_tests ✅

---

#### Paso 4.3: Ejecutar Tests
```bash
cd cmake-build-debug
ctest --output-on-failure
```

**Esperado**:
- ✅ 86+ tests pasan
- ✅ Cero fallos
- ✅ Cero regresiones

**Tests críticos**:
- CellLifeStageTransitionTest: 10/10 ✅
- SimulationTissueV2IntegrationTest: 10/10 ✅
- TissueV2Test: 12/12 ✅
- Otros tests legacy: sin regresiones ✅

---

#### Paso 4.4: Ejecutar una App
```bash
./build/run_all_scenarios
```

**Esperado**:
- ✅ Ejecuta sin errores
- ✅ Genera trazas (escenarios 01-09)
- ✅ Salida de datos consistente

---

#### Paso 4.5: Limpieza de Código
**Revisar**:
- [ ] Comentarios que mencionan "V2" → actualizar a "base"
- [ ] Documentación interna → actualizar versión
- [ ] Logs que dicen "[TissueV2]" → cambiar a "[Tissue]" o mantener si es específico
- [ ] Deprecated markers en Tissue.h → **REMOVER** (ya no es V2 alternativa)

**Cambios específicos en Tissue.h**:
```cpp
// ANTES:
/// TissueV2: Collection of ICell instances using AgenticCell_v2 (with D1/D2)
class Tissue : public ports::ILoggeable {

// DESPUÉS:
/// Tissue: Collection of ICell instances with cell lifecycle management (D1/D2)
class Tissue : public ports::ILoggeable {
```

---

#### Paso 4.6: Verificación Final de Referencias V1
```bash
# Buscar cualquier referencia residual a V1
grep -r "AgenticCell_v2" src/ app/ tests/ CMakeLists.txt
grep -r "CellFactory_v2" src/ app/ tests/ CMakeLists.txt
# Esperado: 0 resultados
```

**Validación**:
- ✅ Cero referencias a _v2
- ✅ Todo apunta a versión base

---

## 📊 Cambios Resumidos

| Componente | Antes | Después | Acción |
|------------|-------|---------|--------|
| **AgenticCell_v2.h** | Existe como V2 | AgenticCell.h | Renombrar |
| **AgenticCell_v2.cpp** | Existe como V2 | AgenticCell.cpp | Renombrar |
| **CellFactory_v2.h** | Existe como V2 | CellFactory.h | Renombrar |
| **CellFactory_v2.cpp** | Existe como V2 | CellFactory.cpp | Renombrar |
| **AgenticCell.h (V1)** | Existe (deprecated) | ELIMINADO | Borrar |
| **AgenticCell.cpp (V1)** | Existe (deprecated) | ELIMINADO | Borrar |
| **CellFactory.h (V1)** | Existe (deprecated) | ELIMINADO | Borrar |
| **CellFactory.cpp (V1)** | Existe (deprecated) | ELIMINADO | Borrar |
| **Tissue.h includes** | `#include "AgenticCell_v2.h"` | `#include "AgenticCell.h"` | Actualizar |
| **Tissue.cpp includes** | `#include "AgenticCell_v2.h"` | `#include "AgenticCell.h"` | Actualizar |
| **Tissue.cpp cast** | `dynamic_cast<AgenticCell_v2*>` | `dynamic_cast<AgenticCell*>` | Actualizar |
| **CMakeLists.txt** | `AgenticCell_v2.cpp/h` | `AgenticCell.cpp/h` | Actualizar |
| **Tests includes** | `AgenticCell_v2.h`, `CellFactory_v2.h` | `AgenticCell.h`, `CellFactory.h` | Actualizar |
| **Apps includes** | `CellFactory_v2.h` | `CellFactory.h` | Actualizar |
| **Apps factory** | `CellFactory_v2::create` | `CellFactory::create` | Actualizar |

---

## ⚠️ Riesgos y Mitigaciones

| Riesgo | Probabilidad | Mitigación |
|--------|-------------|-----------|
| Referencias residuales a _v2 | Muy baja | grep verifica antes de declarar éxito |
| CMakeLists.txt inconsistente | Muy baja | Compilar después de cada cambio |
| Tests fallan por includes | Muy baja | Ejecutar ctest al final |
| Archivos V1 no se eliminan | N/A | Verificar con ls |
| Reversibilidad | BUENA | git permite revert si hay problema |

---

## 🛡️ Reversibilidad (Git)

Todos los cambios pueden revertirse fácilmente:

```bash
# Si algo falla:
git status                    # Ver cambios
git diff src/domain/cell/    # Revisar específico
git checkout src/            # Revertir src/
git reset HEAD CMakeLists.txt # Revertir CMakeLists
```

---

## ✅ Criterios de Aceptación Final

- [ ] Cero referencias a `AgenticCell_v2` en código
- [ ] Cero referencias a `CellFactory_v2` en código
- [ ] Todos los targets compilan sin errores
- [ ] Cero warnings relevantes
- [ ] 86+ tests pasan
- [ ] Cero regresiones en tests
- [ ] Apps ejecutan correctamente
- [ ] grep verifica cero referencias a _v2
- [ ] Directorio cell/ limpio (solo AgenticCell, CellFactory base)
- [ ] Documentación actualizada

---

## 🎯 Órdenes de Ejecución

### Orden 1: Cambios de Código (antes de renombrar)
```
2.1 → 2.2 → 2.3 → 2.4 → 2.5 → 2.6 → 2.7 → 2.8 → 2.9 → 2.10
(Todos en paralelo o secuencial, **SIN COMPILAR**)
```

### Orden 2: Renombrar Archivos (después de cambios)
```
3.1 → 3.2 → 3.3 → 3.4
(Secuencial, **OBLIGATORIO** este orden)
```

### Orden 3: Limpiar y Validar (después de renombrar)
```
4.1 → 4.2 → 4.3 → 4.4 → 4.5 → 4.6
(Secuencial, compilar en 4.2)
```

---

## 📈 Estimación de Tiempo

| Fase | Duración | Parallelizable |
|------|----------|----------------|
| Análisis previo | 5 min | - |
| Actualizar código (10 pasos) | 20 min | Sí (todos excepto 4.1) |
| Renombrar archivos (4 pasos) | 5 min | No (secuencial) |
| Compilar + validar | 10 min | No |
| Tests + apps | 10 min | No |
| Limpieza final | 5 min | No |
| **TOTAL** | **55 min** | Posible ~40 min en paralelo |

---

## 📝 Notas Finales

### Ventajas de Este Plan
1. ✅ **Bajo riesgo**: Código V1 no se usa, solo se renombra
2. ✅ **Reversible**: Git permite revert en cualquier momento
3. ✅ **Validable**: Tests verifican éxito en cada paso
4. ✅ **Limpio**: Elimina ambigüedad de versiones
5. ✅ **Documentado**: Este plan es el roadmap

### Después de Completar
- ✅ AgenticCell y CellFactory son versión **base** (sin "_v2")
- ✅ V1 completamente eliminado
- ✅ Codebase más limpio y mantenible
- ✅ Nombres intuitivos (sin confusión V1/V2)
- ✅ 86+ tests validando funcionamiento

---

## 🎓 Contexto Biológico

AgenticCell base (ex-v2) mantiene:
- **D1 (DNA damage)**: Acumulación de mutaciones
- **D2 (Immunosuppression)**: Evasión inmunológica
- **6 Estados**: DEAD, BASELINE, INTERMEDIO1, INTERMEDIO2, PRIMER, TUMORAL
- **Matriz BRCA1×TP53**: Determina deltas de crecimiento
- **Apoptosis mejorada**: D2 decide resistencia

No cambia nada biológicamente, solo nombres y referencias.

---

**Estado**: ✅ PLAN LISTO PARA EJECUCIÓN
**Próximo**: Ejecutar plan paso por paso, comenzando por Fase 1

