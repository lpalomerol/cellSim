# cellSim - Simulador de Dinámica Celular

## 📋 Descripción

**cellSim** es un simulador de evolución de poblaciones celulares que modela mutaciones genéticas, división celular, apoptosis y transformación neoplástica. Permite estudiar cómo diferentes tasas de mutación afectan la composición y viabilidad del tejido.

---

## 🧬 Estructura Celular

Una célula contiene:

| Componente | Descripción |
|-----------|-------------|
| **BRCA1** | Gene de viabilidad (BRCA1 -/- = muerte) |
| **TP53** | Gene guardián (TP53 -/- = sin protección contra neoplasias) |
| **Edad** | Ciclos de vida completados |
| **Inestabilidad Genómica** | Degradación progresiva del genoma |

### Estados Genéticos

**BRCA1:** `+/-` (inicial) → `-/-` (mutación, irreversible) → Muerte

**TP53:** `+/+` (inicial) → `+/-` → `-/-` (vulnerable a neoplasias)

---

## 🔄 Ciclo de Vida (6 Fases)

```
Fase 0: Baseline Assessment
Fase 1: G1 Integrity Checkpoint → ¿BRCA1 -/-? MUERE
Fase 2: Endocytosis → Recibe apoptosis, puede rechazar
Fase 3: Nuclear Dynamics → Mutaciones genéticas
Fase 4: Cytoplasmic Remodeling → División, transformación neoplástica
Fase 5: Exocytosis → Placeholder
```

### Decisiones Críticas

| Fase | Evento | Condición | Resultado |
|------|--------|-----------|-----------|
| 1 | BRCA1 Checkpoint | BRCA1 = `-/-` | MUERTE (irreversible) |
| 2 | Rechazo de apoptosis | inestabilidad > umbral | Permanece, se vuelve neoplástica |
| 3 | Mutación genética | Threshold aleatorio | BRCA1: `+/-` → `-/-`; TP53: avanza |
| 4 | División | Probabilidad configurable | Crea célula hija (clonada) |
| 4 | Transformación neoplástica | Umbral supera k | Genera señal neoplástica |

---

## 📊 9 Escenarios de Validación

Cada escenario configura parámetros de mutación, inestabilidad y división:

| ID | Escenario | BRCA1 | TP53 | low_δ | high_δ | DIV | Años | Vivas Final | Neo (%) | Inmortales |
|----|-----------|-------|------|-------|--------|-----|------|-----------|---------|-----------|
| 01 | ctrl_baseline_no_mutations_no_division | 0.0 | 0.0 | 0.5 | 1.0 | 0% | 10 | 1000 | 0% | 0% |
| 02 | ctrl_baseline_no_mutations_high_division | 0.0 | 0.0 | 0.5 | 1.0 | 15% | 10 | 4955 | 0% | 0% |
| 03 | ctrl_brca_mutations_high | 0.2 | 0.0 | 0.5 | 1.0 | 0% | 50 | 0 | 0% | 0% |
| 04 | ctrl_tp53_mutations_high | 0.0 | 0.1 | 0.5 | 1.0 | 0% | 50 | 462 | 98% | 100% |
| 05 | ctrl_tp53_mutations_high_unstable | 0.0 | 0.1 | 0.1 | 0.5 | 0% | 50 | 573 | 98% | 99.5% |
| 06 | realistic_baseline | 0.05 | 0.01 | 0.5 | 1.0 | 0% | 50 | 59 | 20% | 100% |
| 07 | realistic_low_tp53_instability | 0.05 | 0.005 | 0.5 | 1.0 | 5% | 80 | 484 | 2.5% | 100% |
| 08 | realistic_high_tp53_instability | 0.05 | 0.02 | 1.0 | 1.5 | 5% | 80 | 153 | 46% | 100% |
| 09 | realistic_balanced | 0.05 | 0.01 | 0.5 | 1.0 | 5% | 80 | 362 | 4.7% | 94% |

**Leyenda:** low_δ = low_delta_instability, high_δ = high_delta_instability, DIV = division_rate, Neo = neoplásticas, Inmortales = resistentes a apoptosis

---

## 🎮 Uso

```bash
# Compilar
cd /home/luis/CLionProjects/cellSim
cmake --build cmake-build-debug --target run_all_scenarios -j4

# Ejecutar todos los 9 escenarios
./cmake-build-debug/run_all_scenarios

# Las trazas se generarán en:
cmake-build-debug/traces/
├── 01_ctrl_baseline_no_mutations_no_division/
├── 02_ctrl_baseline_no_mutations_high_division/
├── ... (9 escenarios totales)
└── 09_realistic_balanced/
```

### Parámetros Globales
- **Población inicial:** 1000 células por escenario
- **Apoptosis threshold:** 10.0 (instabilidad genómica máxima permitida para que apoptosis sea efectiva)
- **Seed:** Aleatorio (-1) para variabilidad

---

## ⚙️ Configuración de Parámetros

| Parámetro | Significado | Rango | Default |
|-----------|------------|-------|---------|
| **BRCA1** | Tasa de mutación BRCA1 | 0.0 - 0.2 | 0.05 |
| **TP53** | Tasa de mutación TP53 | 0.0 - 0.1 | 0.01 |
| **neoplasm_k** | Probabilidad base de neoplasia | 0.0 - 0.1 | 0.05 |
| **low_delta** | Inestabilidad con TP53 +/- | 0.0 - 1.0 | 0.5 |
| **high_delta** | Inestabilidad con TP53 -/- | 0.0 - 1.5 | 1.0 |
| **division_rate** | Tasa de división celular | 0.0 - 0.15 | 0.05 |
| **apoptosis_threshold** | Umbral de evasión de apoptosis | 1.0 - 10.0 | 10.0 |

---

## 🔬 Insights Clave (Validación Experimental)

### Hallazgos de Control Basal
1. **Sin mutaciones → población estable** (Escenario 01: 1000 vivas, 0% neoplásticas)
2. **Sin mutaciones + división → crecimiento exponencial** (Escenario 02: 4955 vivas, 5× en 10 años)

### Hallazgos Paramétricos
3. **BRCA1=0.2 es letal** - Extinción total en 50 años (Escenario 03: 0 vivas)
4. **TP53=0.1 + neoplasm_k=0.1 → 98% neoplásticas** (Escenario 04: 462 vivas, 98% neo)
5. **Mayor inestabilidad amplifica neoplasias** (Escenario 05 vs 04: +111 células con high_delta más alto)

### Hallazgos Realistas
6. **TP53 baja (0.5%) es muy protectora** - 2.5% neoplásticas (Escenario 07: 484 vivas)
7. **TP53 moderada (2%) + inestabilidad alta → 46% neoplásticas** (Escenario 08: 153 vivas)
8. **Parámetros balanceados son realistas** - 4.7% neoplásticas (Escenario 09: RECOMENDADO)

### Mecanismos Validados
- ✅ Inestabilidad genómica correlaciona con resistencia a apoptosis
- ✅ Umbral de apoptosis (10.0) es permisivo: células muy inestables evaden muerte
- ✅ División celular mantiene población incluso bajo presión selectiva
- ✅ TP53 es crítico para protección contra neoplasias

---

## ✅ Funcionalidades Implementadas

- ✅ Mutaciones genéticas (BRCA1, TP53) con tasa configurable
- ✅ Apoptosis por BRCA1 -/- (muerte garantizada)
- ✅ Transformación neoplástica condicionada a TP53 -/-
- ✅ Evasión de apoptosis basada en inestabilidad genómica
- ✅ División celular con clonación y herencia de inestabilidad
- ✅ Inestabilidad genómica progresiva (actualizacion de deltas por TP53 status)
- ✅ **9 escenarios de validación** (controles + realistas)
- ✅ **Generación automática de trazas** (Markdown + CSV)
  - Tablas anuales con evolución de población
  - Análisis de neoplásticas susceptibles vs resistentes
  - Estadísticas de TP53 status por año

### Archivos de Salida
```
traces/[escenario]/
├── [escenario]_run1_POPULATION.md      (Tabla Markdown con evolución anual)
└── [escenario]_run1_POPULATION.csv     (Datos en CSV para análisis externo)
```
- ✅ 55+ tests unitarios PASSING

---

## 📂 Estructura del Proyecto

```
src/
├── application/
├── domain/
│   ├── cell/
│   ├── gene/
│   ├── tissue/
│   └── ...
└── shared/

tests/
├── unit_tests
└── integration tests

build/
└── ejecutables (cellSim, random_cells, interactive, etc)
```

---

## 🧬 Compilación y Tests

```bash
# Compilar todo
make -j4

# Ejecutar tests
./cmake-build-debug/tests/unit_tests

# Build limpio
rm -rf build cmake-build-debug && make -j4
```

---

**Autor:** Simulador de Dinámica Celular
**Licencia:** Abierto para investigación educativa
**Última actualización:** 2025

