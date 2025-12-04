# Implementación del Modelo Big Bang Tumoral

## Descripción

Se implementó un nuevo **workflow para Big Bang tumoral** que permite que las células neoplásticas (TP53 -/-) se dividan de forma descontrolada con una tasa configurable, mientras que las células normales pueden tener una tasa de división diferente o nula.

## Cambios Realizados

### 1. **SimulationConfig.h** - Nuevos Parámetros
```cpp
struct SimulationConfig {
    double neoplastic_division_rate = 0.001;  // Tasa de división neoplástica (Big Bang)
    bool enable_big_bang_mode = false;        // Activar Big Bang
    // ... otros parámetros existentes ...
};
```

### 2. **AgenticCell.h/cpp** - Lógica de Big Bang
- Agregados campos: `neoplastic_division_rate_`, `enable_big_bang_mode_`
- Modificado constructor para recibir nuevos parámetros
- **Lógica clave en `attemptDivision()`:**
  ```cpp
  if (enable_big_bang_mode_ && is_neoplastic_) {
      // Usar tasa acelerada para neoplásticas
      effective_division_rate = neoplastic_division_rate_;
  }
  ```

### 3. **CellFactory.h/cpp** - Propagación de Parámetros
- Actualizados ambos overloads para pasar `neoplastic_division_rate` y `enable_big_bang_mode`

### 4. **Archivos que Usan CellFactory** - Actualizados
- `main_interactive.cpp`
- `single_cell_evolution.cpp`
- `Simulations.cpp`
- Todos los tests en `tests/`

### 5. **run_all_scenarios.cpp** - Tres Nuevos Escenarios Big Bang

#### **Escenario 10: Big Bang Puro**
```cpp
{"10_big_bang_tumoral",
 "Big Bang Tumoral: TP53 -/- con división acelerada (20%)",
 0.05, 0.02, 0.20,      // BRCA1=5%, TP53=2%, neoplasm_k=20%
 0.5, 1.0,              // low_delta=0.5, high_delta=1.0
 0.0,                   // division_rate=0% (células normales NO se dividen)
 0.20,                  // neoplastic_division_rate=20% (células TP53 -/- dividen)
 true,                  // enable_big_bang_mode=true
 30}                    // 30 años
```

**Característica:** Divisiones de células neoplásticas sin interferencia de células normales.

#### **Escenario 11: Big Bang + Reproducción Normal**
```cpp
{"11_big_bang_tumoral_reproduccion",
 "Big Bang con Reproducción Normal: TP53 -/- (10%) + Normal (5%)",
 0.05, 0.02, 0.20,      // BRCA1=5%, TP53=2%, neoplasm_k=20%
 0.5, 1.0,              // low_delta=0.5, high_delta=1.0
 0.05,                  // division_rate=5% (células normales dividen)
 0.10,                  // neoplastic_division_rate=10% (neoplásticas dividen menos)
 true,                  // enable_big_bang_mode=true
 30}                    // 30 años
```

**Característica:** Competencia entre células normales (5% div) y neoplásticas (10% div).

#### **Escenario 12: Big Bang con Bajo Threshold TP53**
```cpp
{"12_big_bang_tumoral_low_threshold",
 "Big Bang con Bajo Threshold TP53: TP53 -/- (10%) + Normal (5%)",
 0.05, 0.001, 0.20,     // BRCA1=5%, TP53=0.1%, neoplasm_k=20%
 0.25, 0.5,             // low_delta=0.25, high_delta=0.5 (menor inestabilidad)
 0.05,                  // division_rate=5% (células normales dividen)
 0.10,                  // neoplastic_division_rate=10% (neoplásticas dividen)
 true,                  // enable_big_bang_mode=true
 30}                    // 30 años
```

**Característica:** Threshold TP53 muy bajo (0.1%) con inestabilidad genómica controlada.

## Justificación Biológica

✅ **Big Bang vs. Gradual:**
- **Fase gradual (años):** Acumulación de mutaciones tempranas (Escenarios 1-9)
- **Fase Big Bang (meses):** Explosión exponencial tras TP53 loss (Escenario 10)

✅ **TP53 -/- como desencadenante:**
- Pérdida de p53 permite evasión de apoptosis ✓
- Inmortalización ✓
- **Proliferación sin control (NUEVO)**

✅ **División acelerada es realista:**
- Células normales: ~24h ciclo, baja división (5%)
- Células neoplásticas: ~12-18h ciclo, alta división (50%)

## Comportamiento Esperado

| Métrica | Esc 09 (Baseline) | Esc 10 (BB Puro) | Esc 11 (BB Normal) | Esc 12 (BB LowThreshold) |
|---------|------------------|------------------|-------------------|------------------------|
| Población año 30 | ~300 células | 500-1000+ | 600-1200+ | 400-800+ |
| % Neoplásticas | 5-10% | 50-80% | 30-60% | 40-70% |
| Crecimiento | Lineal lento | Exponencial | Exponencial mixto | Exponencial controlada |
| Año de explosión | Gradual | Año 5-10 | Año 3-8 | Año 5-10 |
| División Normal | 5% | 0% | 5% | 5% |
| División Neoplástica | 0% | 20% | 10% | 10% |

## Cómo Ejecutar

### Compilar:
```bash
cd /home/luis/CLionProjects/cellSim/build
cmake .. -DCMAKE_BUILD_TYPE=Release
make run_all_scenarios
```

### Ejecutar todos los escenarios (12 total):
```bash
./run_all_scenarios
```

### Estructura de resultados:
```
traces/
  ├── 01_ctrl_baseline_no_mutations_no_division/
  ├── 02_ctrl_baseline_no_mutations_high_division/
  ├── 03_ctrl_brca_mutations_high/
  ├── ...
  ├── 09_realistic_balanced/
  ├── 10_big_bang_tumoral/                    ← Big Bang puro
  ├── 11_big_bang_tumoral_reproduccion/       ← Big Bang + reproducción
  └── 12_big_bang_tumoral_low_threshold/      ← Big Bang bajo threshold
```

### Cada escenario genera:
```
traces/[escenario]/
  ├── [escenario]_config.txt                 (Parámetros)
  ├── [escenario]_yearly_stats.csv          (Estadísticas por año)
  ├── [escenario]_neoplastic_details.csv    (Detalles neoplásticas)
  └── [escenario]_gene_distribution.csv     (Distribución génica)
```

## Características Heredables

- **Genomic Instability:** Heredada de padre a hija durante división
- **Apoptosis Evasion:** Células inmortales producen células inmortales
- **Big Bang Parameters:** Todas las células neoplásticas usan la misma `neoplastic_division_rate`
- **Mutaciones acumuladas:** Se heredan en el genoma clonado

## Backward Compatibility

✅ **Totalmente compatible:**
- Parámetro `enable_big_bang_mode = false` por defecto
- Escenarios 1-9 funcionan igual (sin cambios)
- `neoplastic_division_rate` = `division_rate` si Big Bang está desactivado

## Archivos Modificados

1. `src/application/config/SimulationConfig.h` ✓
2. `src/domain/cell/AgenticCell.h` ✓
3. `src/domain/cell/AgenticCell.cpp` ✓
4. `src/domain/cell/CellFactory.h` ✓
5. `src/domain/cell/CellFactory.cpp` ✓
6. `app/run_all_scenarios.cpp` ✓
7. `app/main_interactive.cpp` ✓
8. `app/single_cell_evolution.cpp` ✓
9. `src/application/simulation/Simulations.cpp` ✓
10. `tests/CellDivisionInstabilityPropagationTest.cpp` ✓
11. `tests/NeoplasticImmortalityTest.cpp` ✓
12. `tests/ApoptosisNeoplasticIntegrationTest.cpp` ✓

## Validación

✅ Compilación exitosa sin errores
✅ Todos los tests pasan
✅ Backward compatible con escenarios existentes (1-9)
✅ Parámetros sensatos:
  - Escenario 10: Big Bang puro (div neoplástica 20%, normal 0%)
  - Escenario 11: Big Bang mixto (div neoplástica 10%, normal 5%)
  - Escenario 12: Big Bang bajo threshold (div neoplástica 10%, normal 5%, threshold TP53 0.1%)
✅ Comportamiento emergente: Explosión de células TP53 -/- con división acelerada

