# cellSim

[![CI](https://github.com/lpalomerol/cellSim/actions/workflows/ci.yml/badge.svg)](https://github.com/lpalomerol/cellSim/actions)

Simulador de evolución celular y neoplasia. Permite simular múltiples células a lo largo de los años, aplicando reglas de mutación genética y aparición de neoplasias.

## 📦 Instalación

### Requisitos previos

- **CMake** >= 3.14
- **g++** o **clang++** con soporte C++17
- **make**

```bash
# Ubuntu/Debian
sudo apt install cmake g++ make

# Fedora/RHEL
sudo dnf install cmake gcc-c++ make

# macOS
brew install cmake
```

### Instalación rápida (script automatizado)

```bash
# Clonar el repositorio
git clone https://github.com/lpalomerol/cellSim.git
cd cellSim

# Instalación global (requiere sudo)
chmod +x install.sh
./install.sh

# O instalación local (sin sudo)
PREFIX=$HOME/.local ./install.sh
```

Los binarios se instalarán en `/usr/local/bin` (o en `$PREFIX/bin` si se especifica).

### Instalación manual con Makefile

```bash
# Compilar el proyecto
make rebuild

# Instalar en /usr/local/bin (requiere sudo)
sudo make install

# O instalar en directorio personalizado
make install PREFIX=$HOME/.local

# Verificar instalación
which cellSim_cli
cellSim_cli --help
```

### Desinstalación

```bash
# Con script
./uninstall.sh

# O con Makefile
sudo make uninstall

# Si instalaste en directorio personalizado
make uninstall PREFIX=$HOME/.local
```

### Añadir al PATH (instalación local)

Si instalaste en `$HOME/.local`, añade esto a tu `~/.bashrc` o `~/.zshrc`:

```bash
export PATH=$HOME/.local/bin:$PATH
```

Luego recarga el shell:

```bash
source ~/.bashrc  # o source ~/.zshrc
```

### Compilación sin instalación

Si solo quieres compilar y ejecutar sin instalar:

```bash
# Con Makefile
make rebuild

# Ejecutar desde build/
./build/cellSim_cli --config configs/default.json

# O con CMake directamente
mkdir build && cd build
cmake ..
make -j4
```

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

| Ejecutable | Descripción | Trazas |
|-----------|------------|--------|
| **cellSim** | Simulación batch de 100 escenarios | En memoria |
| **interactive** | Modo interactivo con control manual | Verbose logging |
| **single_cell_evolution** | Análisis detallado de célula única | single_cell_evolution_log.txt |
| **run_all_scenarios** | ✨ **VALIDACIÓN**: 14 escenarios (controles + realistas + **Big Bang**) | Markdown + CSV |
| **cellSim_cli** | 🔧 Simulación configurable con JSON | Configurable por CLI |

## 🔧 Ejecutable CLI Configurable (NUEVO)

**cellSim_cli** permite ejecutar simulaciones con configuración JSON personalizada estructurada en tres grupos lógicos.

### Uso básico

```bash
./build/cellSim_cli --config configs/default.json
```

### Opciones disponibles

- `--config <path>`: Archivo JSON de configuración (obligatorio)
- `--max-t <años>`: Años de simulación (override)
- `--cells <número>`: Número de células (override)
- `--seed <valor>`: Semilla aleatoria, -1=aleatorio (override)
- `--neoplasm-k <probabilidad>`: Prob. base neoplasia 0.0-1.0 (override)
- `--verbose`: Activar logging detallado (override)
- `--division-rate <tasa>`: Tasa de división celular (override)
- `--help`: Mostrar ayuda completa

### Estructura JSON requerida

El JSON se organiza en tres secciones:

**1. config** (configuración básica):
- `description`: Texto descriptivo (opcional)
- `seed`: Semilla aleatoria (-1 = aleatorio)
- `verbose`: Logging detallado (true/false)
- `use_random_noise`: Ruido aleatorio vs fijo (true/false)

**2. simulation_context** (contexto de simulación):
- `max_t`: Años simulados
- `n_cells`: Número de células

**3. tissue_parameters** (parámetros del tejido):
- `neoplasm_k`: Probabilidad base de neoplasia
- `division_rate`: Tasa de división celular normal
- `neoplastic_division_rate`: Tasa de división neoplástica
- `enable_big_bang_mode`: Modo Big Bang (true/false)
- `genes`: Objeto con TP53 y BRCA1, cada uno con:
  - `mutation_rate`: Tasa de mutación
  - `instability_rate`: Tasa de inestabilidad

### Plantillas disponibles

- `configs/default.json`: Configuración estándar
- `configs/high_tp53.json`: Alta mutación TP53
- `configs/no_mutations.json`: Sin mutaciones (baseline)

### Ejemplos

```bash
# Ejecutar con seed fija
./build/cellSim_cli --config configs/default.json --seed 42

# Escenario TP53 con 50 células
./build/cellSim_cli --config configs/high_tp53.json --cells 50 --verbose

# Baseline sin mutaciones, 100 años
./build/cellSim_cli --config configs/no_mutations.json --max-t 100
```

### Validaciones automáticas

- `seed >= -1`
- `max_t > 0`, `n_cells > 0`
- `0.0 <= neoplasm_k <= 1.0`
- Tasas de mutación/división >= 0.0
- Presencia obligatoria de genes TP53 y BRCA1 con ambas tasas

## 💥 Modo Big Bang (NUEVO)

El simulador incluye 5 nuevos escenarios que modelan la **transformación masiva neoplástica**:

- **Escenario 10-14:** Modo Big Bang con variantes (bajo, moderado, agresivo, extremo, control)
- **Características:** Fase latencia → Aceleración → Explosión crítica → Dominio tumoral
- **Resultado:** 90-99% neoplásticas con >99% inmortalidad
- **Documentación:** Ver `docs/BIG_BANG_MODE.md`

## Documentación

| Archivo | Descripción |
|---------|-----------|
| `docs/README.md` | Modelo biológico y parámetros (🎯 COMIENZA AQUÍ) |
| `docs/diagrams.md` | FSM de genes, ciclo celular, diagramas comparativos |
| `docs/BIG_BANG_MODE.md` | 💥 Transformación masiva neoplástica (NUEVO) |
| `BIG_BANG_SUMMARY.md` | 📊 Resumen ejecutivo del Big Bang (NUEVO) |
| `INDEX.md` | Índice de validación con 14 escenarios |
| `USAGE_GUIDE.md` | Cómo interpretar resultados |
| `EXECUTIVE_DASHBOARD.md` | Gráficos para stakeholders |

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

### ✨ Validación Experimental (9 Escenarios)
```bash
./cmake-build-debug/run_all_scenarios
```
Ejecuta y valida 9 escenarios científicos:
- **2 controles basales** - Validación del modelo (sin mutaciones ±división)
- **3 controles paramétricos** - Análisis de impacto (BRCA1 alto, TP53 alto, inestabilidad)
- **4 escenarios realistas** - Validación biológica (parámetros moderados con división)

**Resultados:** Trazas en `cmake-build-debug/traces/` (Markdown + CSV)
- Tiempo: ~3.4 segundos (9 escenarios × 1000 células)
- Reportes: `EXECUTIVE_DASHBOARD.md`, `VALIDATION_RESULTS_ANALYSIS.md`

---

## ✅ Validación 2025

| Escenario | Vivas | Neoplásticas | Inmortales | Años | Hallazgo |
|-----------|-------|--------------|-----------|------|----------|
| 01 - Sin nada | 1000 | 0% | 0% | 10 | ✅ Estable (modelo OK) |
| 02 - División 15% | 4955 | 0% | 0% | 10 | ✅ Crec 5× |
| 03 - BRCA1=0.2 | 0 | 0% | 0% | 50 | ⚠️ Letal |
| 04 - TP53=0.1 | 462 | 98% | 100% | 50 | ✅ Neoplasia |
| 05 - TP53 + inest | 573 | 98% | 99.5% | 50 | ✅ Mayor resistencia |
| 06 - Realista | 59 | 20% | 100% | 50 | ✅ Apoptosis activa |
| 07 - TP53 baja | 484 | 2.5% | 100% | 80 | ✅ Protector |
| 08 - TP53 + inest | 153 | 46% | 100% | 80 | ⚠️ Arriesgado |
| 09 - Balanceado | 362 | 4.7% | 94% | 80 | ⭐ RECOMENDADO |

**Conclusión:** Parámetros balanceados (escenario 09) son biológicamente realistas y reproducen fenotipos esperados.

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

