# cellSim

[![CI](https://github.com/lpalomerol/cellSim/actions/workflows/ci.yml/badge.svg)](https://github.com/lpalomerol/cellSim/actions)

Simulador de evolución celular y neoplasia. Permite simular múltiples células a lo largo de los años, aplicando reglas de mutación genética y aparición de neoplasias.

## Estructura del proyecto

```
src/
├── application/
│   ├── config/SimulationConfig.h    # Configuración centralizada
│   └── simulation/                  # Orquestación (Simulation, Simulations, Interactive)
├── domain/
│   ├── cell/                        # Células (AgenticCell, CellFactory)
│   ├── gene/                        # Genes y genomas (Gene, Genome, GenomeFactory)
│   ├── signal/                      # Sistema de señales (BaseSignal, Neoplasm, Apoptosis, CellDivision)
│   ├── tissue/                      # Tejidos (Tissue, GeneticTrackingData, GeneticTrackingService)
│   ├── ports/                       # Interfaces (ICell, INoiseSource, ILogger)
│   └── adapters/                    # Adaptadores (RandomNoise, FixedNoise, Logger)
app/                                 # Ejecutables (main.cpp, interactive, single_cell_evolution)
tests/                               # 53 tests unitarios (GoogleTest)
docs/                                # Documentación del proyecto
```

## Ejecutables

| Ejecutable | Descripción |
|-----------|------------|
| **cellSim** | Simulación batch de 100 escenarios |
| **interactive** | Modo interactivo con control manual de mutaciones |
| **single_cell_evolution** | Análisis detallado de evolución de célula única |

## Refactor de Limpieza Reciente

### Consolidaciones principales:

1. **GeneticTrackingData**: Migración a arrays con enum
   - De 10 miembros a 3 arrays
   - Reducción de 70% en código duplicado
   - Acceso unificado via enum `Category`

2. **SimulationConfig**: Centralización de configuración
   - Una fuente de verdad para parámetros
   - Métodos factory para cargar escenarios
   - Eliminación de duplicación en `main*.cpp`

3. **Signal System**: Refactor a jerarquía limpia
   - `BaseSignal`: clase base con lógica común
   - `NeoplasmSignal`, `ApoptosisSignal`, `CellDivisionSignal` heredan
   - Reducción de 60% en código duplicado

4. **Ports/Adapters**: Estandarización de namespaces
   - `domain::adapters` uniforme
   - `ICell` consolidado (absorbe `IGeneticProfile`)
   - Eliminación de interfaces redundantes

5. **Simulation**: Extracción de helpers comunes
   - `Simulation::executeCellCycle()` - lógica compartida
   - `InteractiveSimulation` hereda de `Simulation`
   - Eliminación de duplicación

### Eliminaciones:

**Archivos innecesarios:**
- ❌ SimpleCell, OncoMatrix
- ❌ ExperimentalTracking (redundante con GeneticTrackingData)
- ❌ main_random_cells.cpp (duplicaba main.cpp)

**Tests redundantes (66 → 53):**
- ❌ MockLoggerDemoTest (demo, no test real)
- ❌ GenomeTest (cubierto por GenomeFactory/GenomeMutate)
- ❌ NeoplasmSignalTest (cubierto por integration tests)
- ❌ AgenticCellSignalTest (redundante)
- ❌ TissueIntegrationSignalTest (redundante)
- ❌ AgenticCellReceiveMessageTest (redundante)

**Documentación obsoleta:**
- ❌ CAMBIOS_NEOPLASIAS_ACTIVAS.md
- ❌ GMOCK_SETUP.md
- ❌ Archivos CSV/JSON de datos de ejemplo

### Resultados de limpieza:

| Métrica | Impacto |
|---------|---------|
| Líneas de código duplicado eliminadas | ~500+ |
| Archivos redundantes eliminados | 9 |
| Tests simplificados | 20% |
| Compilación | ✅ Exitosa |
| Tests pasando | ✅ 53/53 |

## Configuración y Uso

### Batch Mode
```bash
./cellSim
```
Ejecuta 100 simulaciones con parámetros por defecto configurables en `SimulationConfig::loadDefault()`.

### Modo Interactivo
```bash
./interactive
```
- Controla mutaciones: `1` (BRCA1), `2` (TP53), `q` (salir)
- Simulación de 1 célula durante 30 años
- Logs detallados por año

### Análisis Detallado
```bash
./single_cell_evolution
```
- Simula 1 célula durante 100 años
- Output: `single_cell_evolution_log.txt`
- Rastreo de inestabilidad genómica
- Detección de transiciones a neoplasia

## Modelo Biológico

### Genes
- **BRCA1**: Protección contra mutaciones (threshold configurable)
- **TP53**: "Guardián del genoma" - protege contra neoplasias

### Mutación
- Umbral base + componente de inestabilidad
- La inestabilidad se aplica solo si TP53 está comprometido

### Neoplasia
- Probabilidad base `neoplasm_k` (configurable)
- TP53 intacto ("+/+") = protección
- TP53 heterocigoto ("+/-") o homocigoto (("-/-") = vulnerable

### Apoptosis
- Células neoplásticas que no evaden mueren
- Umbral de inestabilidad genómica

### División Celular
- Herencia de genoma a daughter cell
- Herencia de estado neoplástico/inmortalidad

## Documentación Adicional

Ver `/docs/` para:
- `QUICK_INDEX.md` - Índice rápido
- `EXECUTIVE_SUMMARY.md` - Resumen ejecutivo
- `SCENARIOS.md` - Escenarios de simulación
- `biological-concepts/` - Conceptos biológicos

## Compilación

### CMake (recomendado)
```bash
mkdir cmake-build-debug
cd cmake-build-debug
cmake -DBUILD_TESTS=ON ..
make -j4
```

### Tests
```bash
cd cmake-build-debug
ctest
# o directamente:
./tests/unit_tests
```

## Arquitectura Limpia

El proyecto sigue principios de arquitectura limpia:

- **Domain**: Lógica de negocio pura (sin dependencias)
- **Application**: Orquestación de casos de uso
- **Adapters**: Implementaciones concretas (ruido, logging)
- **Ports**: Interfaces que definen contratos

Esto permite:
- ✅ Fácil testing
- ✅ Bajo acoplamiento
- ✅ Alta cohesión
- ✅ Facilidad de cambios

