# cellSim - Simulador de Dinámica Celular

## 🎯 ¿Qué es cellSim?

**cellSim** es un simulador estocástico basado en agentes que modela la evolución de poblaciones celulares epiteliales bajo presión de mutaciones genéticas, apoptosis y transformación neoplástica. 

**Caso de uso:** Entender cómo diferentes tasas de mutación en genes críticos (BRCA1, TP53) afectan la composición poblacional, la resistencia a apoptosis y el riesgo de neoplasias.

**Audiencia:** Biólogos moleculares, bioinformáticos, y investigadores en oncología.

> 📖 **Nueva documentación disponible**: Para una descripción completa del modelo Agentic Cell, incluyendo estructura de la célula, estados, evolución de D1/D2, y modelo matemático, consulta **[AGENTIC_CELL_DOCUMENTATION.md](./AGENTIC_CELL_DOCUMENTATION.md)**.

---

## 🧬 Modelo Biológico

### Gen BRCA1 (Breast Cancer susceptibility protein 1)
- **Función:** Reparación del ADN, control de ciclo celular
- **Estados genéticos:** 
  - `+/-` (heterocigoto, funcional, VIVO)
  - `-/-` (homocigoto recesivo, deficiente, MUERE en Fase 1)
- **Modelado:** Mutación **unidireccional e irreversible** (`+/-` → `-/-`)
- **Biología:** BRCA1 -/- es incompatible con vida en la mayoría de tejidos

### Gen TP53 (Tumor Protein 53)
- **Función:** "Guardián del genoma" - detiene ciclo celular ante daño, induce apoptosis
- **Estados genéticos:**
  - `+/+` (homocigoto dominante, máxima protección)
  - `+/-` (heterocigoto, protección parcial pero funcional)
  - `-/-` (homocigoto recesivo, sin protección, VULNERABLE a neoplasias)
- **Modelado:** Mutación **bidireccional y progresiva** (`+/+` → `+/-` → `-/-`)
- **Biología:** TP53 -/- permite evasión de apoptosis e inmortalización

### Inestabilidad Genómica (Genomic Instability)
- **Definición:** Acumulación de mutaciones a causa del aumento de defectos de reparación
- **Fórmula:** `I(t+1) = I(t)² + δ(TP53_status)`
- **Interpretación:** 
  - Crecimiento **cuadrático** (retroalimentación positiva)
  - Delta (`δ`) depende del estado de TP53:
    - TP53 `+/+` → δ = 0.0 (reparación eficiente)
    - TP53 `+/-` → δ = 0.5 (reparación moderada)
    - TP53 `-/-` → δ = 1.0 (reparación deficiente)

---

## 🔄 Ciclo de Vida Celular (6 Fases)

Cada célula completar **6 fases secuenciales** en cada iteración:

| Fase | Nombre | Proceso | Checkpoint |
|------|--------|---------|-----------|
| **0** | Baseline Assessment | Evaluación de viabilidad inicial | - |
| **1** | G1 Integrity | Verificación de integridad BRCA1 | ❌ BRCA1=-/- → MUERTE |
| **2** | Endocytosis | Recepción de señales apoptóticas | 🔄 Acepta/rechaza apoptosis |
| **3** | Nuclear Dynamics | Mutaciones genéticas (BRCA1, TP53) | 🧬 Cambios aleatorios |
| **4** | Cytoplasmic Remodeling | Transformación neoplástica + división | 🔴 TP53=-/- → neoplástica O 🔵 División |
| **5** | Exocytosis | Emisión de señales al tejido | Señales normales O NeoplasmSignal |

### Checkpoints Críticos

**Fase 1 - BRCA1 Check:**
- Si BRCA1 = `-/-` → Lanza excepción, célula muere inmediatamente
- Biología: Deficiencia de BRCA1 es incompatible con vida

**Fase 2 - Apoptosis Checkpoint:**
- Célula recibe señal de apoptosis
- Evalúa su `inestabilidad genómica`
- Si `inestabilidad ≤ 10.0` → ✅ **Acepta apoptosis** → MUERTE
- Si `inestabilidad > 10.0` → ❌ **Rechaza apoptosis** → EVASIÓN (inmortalización)
- Biología: Inestabilidad genómica amplificada permite evasión

**Fase 3 - Mutaciones:**
- BRCA1: Muta `+/-` → `-/-` con probabilidad `P = threshold × genomic_instability`
- TP53: Muta escalonadamente con misma probabilidad
- Biología: Tasas de mutación realistas (~1% en DEFAULT)

**Fase 4 - Decisión Terminal:**
- **PRIMERO:** ¿TP53 = `-/-`? → SÍ: Transforma en neoplástica
- **SEGUNDO (si NO transformada):** ¿División? → P(div)% → Clona hija

---

## 📊 14 Escenarios de Validación

Cada escenario prueba un aspecto diferente del modelo:

### Controles Positivos & Negativos (01-03)

| Escenario | Descripción | Parámetros | Resultado Esperado | Resultado Real |
|-----------|-------------|-----------|-------------------|----------------|
| **01. Baseline Estable** | Sin mutaciones, sin división | μ=0%, div=0% | Población = 1000 (100% vivas) | ✅ 1000 vivas, 0% neo |
| **02. Crecimiento Exponencial** | Sin mutaciones, alta división | μ=0%, div=15% | Población ~5000 (crecimiento 5×) | ✅ 4955 vivas, 5× |
| **03. Colapso BRCA1** | BRCA1 muy mutágeno | μ_BRCA=0.2, div=0% | Extinción total | ✅ 0 vivas en 50 años |

### Fenotipos Neoplásticos (04-05)

| Escenario | Descripción | Parámetros | Resultado Esperado | Resultado Real |
|-----------|-------------|-----------|-------------------|----------------|
| **04. Invasión TP53** | TP53 muy mutágeno | μ_TP53=0.1, div=0% | 90%+ neoplásticas, inmortales | ✅ 462 vivas, 98% neo, 100% inmortales |
| **05. Efecto de Inestabilidad** | Mayor inestabilidad TP53 | low_δ=0.1, high_δ=0.5 | Más neoplásticas | ✅ 573 vivas vs 462 |

### Escenarios Realistas (06-09)

| Escenario | Descripción | Parámetros | Resultado Esperado | Resultado Real |
|-----------|-------------|-----------|-------------------|----------------|
| **06. Realista Basal** | Tasas reales bajas | μ_BRCA=0.05, μ_TP53=0.01, div=0% | ~20% neoplásticas | ✅ 59 vivas, 20% neo |
| **07. Protección TP53 Baja** | TP53 muy baja, división controlada | μ_TP53=0.005, div=5% | <5% neoplásticas | ✅ 484 vivas, 2.5% neo |
| **08. Inestabilidad Alta** | Inestabilidad severa | high_δ=1.5, div=5% | 40-50% neoplásticas | ✅ 153 vivas, 46% neo |
| **09. Balanced (RECOMENDADO)** | Parámetros biológicamente realistas | μ_BRCA=0.05, μ_TP53=0.01, div=5% | 4-5% neoplásticas | ✅ 362 vivas, 4.7% neo |

### Modo Big Bang: Transformación Masiva (10-14)

| Escenario | Descripción | Parámetros | Resultado Esperado | Resultado Real |
|-----------|-------------|-----------|-------------------|----------------|
| **10. Big Bang Bajo Umbral** | Transformación masiva, baja inestabilidad inicial | μ_BRCA=0.05, μ_TP53=0.01, k=0.02, div=0.05 | 90%+ neoplásticas al año 80 | ✅ 9,580 células, 92% neo |
| **11. Big Bang Moderado** | Transformación más acelerada | μ_BRCA=0.06, μ_TP53=0.015, k=0.03, div=0.06 | 85%+ neoplásticas | 📊 En validación |
| **12. Big Bang Agresivo** | Transformación muy rápida | μ_BRCA=0.08, μ_TP53=0.02, k=0.05, div=0.08 | 95%+ neoplásticas | 📊 En validación |
| **13. Big Bang Inestabilidad Extrema** | Ultra-alta inestabilidad | high_δ=2.0, μ_TP53=0.02, k=0.05 | 99%+ neoplásticas, >99% inmortales | 📊 En validación |
| **14. Big Bang Control** | Parámetros optimizados para validación | μ_BRCA=0.05, μ_TP53=0.01, k=0.02, div=0.05 | Reproducible, documentado | ✅ Validado |

**Población inicial:** 1000 células/escenario | **Duración:** 80 años simulados | **📖 Documentación:** Ver [BIG_BANG_MODE.md](./BIG_BANG_MODE.md)

---

## 💥 Modo Big Bang - Transformación Masiva Neoplástica

El **Modo Big Bang** modela la transformación rápida y masiva de una población celular normal a neoplástica, un fenómeno crítico en oncogénesis. Ver [documentación completa del Big Bang](./BIG_BANG_MODE.md).

**Características clave:**
- 🔴 Fase de latencia (0-30 años): Acumulación lenta de mutaciones TP53
- ⚡ Fase crítica (30-60 años): Explosión exponencial de inestabilidad genómica
- 💥 Punto de ruptura (50-70 años): Transformación masiva neoplástica
- 📈 Dominio tumoral (70-80 años): 90-99% neoplásticas, casi 100% inmortales

**Biomarkers observados:**
- TP53 mutado (-/-): >90% de población
- Inestabilidad genómica extrema: >50-100 (vs normal <10)
- Resistencia masiva a apoptosis: >98% de neoplásticas
- Crecimiento exponencial descontrolado

---

## 🎮 Cómo Usar cellSim

### Compilación y Ejecución

```bash
# 1. Compilar todo el proyecto
cd /home/luis/CLionProjects/cellSim
cmake --build cmake-build-debug --target run_all_scenarios -j4

# 2. Ejecutar todos los 14 escenarios (toma ~3-7 minutos)
./cmake-build-debug/run_all_scenarios

# 3. Ver trazas generadas
ls -la cmake-build-debug/traces/

# 4. Ver escenarios Big Bang específicamente
ls -la cmake-build-debug/traces/ | grep big_bang
```

### Archivos de Salida

Cada escenario genera dos archivos:

```
traces/[escenario_id]_[nombre]/
├── [escenario]_run1_POPULATION.md       ← Tabla Markdown (anual)
├── [escenario]_run1_POPULATION.csv      ← CSV para análisis externo
└── [otros archivos de log]
```

**Contenido de POPULATION.md:**
- Tabla anual con: Año, Vivas, Neoplásticas, Muertas, TP53 +/+ / +/- / -/-, etc.
- Fácil de analizar en Excel, R, Python

### Parámetros Configurables

| Parámetro | Rango | Default | Interpretación |
|-----------|-------|---------|----------------|
| `BRCA1_threshold` | 0.0-0.2 | 0.05 | Tasa de mutación BRCA1 por ciclo (%) |
| `TP53_threshold` | 0.0-0.1 | 0.01 | Tasa de mutación TP53 por ciclo (%) |
| `neoplasm_k` | 0.0-0.1 | 0.05 | Prob. base de transformación neoplástica |
| `low_delta_instability` | 0.0-1.0 | 0.5 | Incremento de inestabilidad (TP53 +/-) |
| `high_delta_instability` | 0.0-1.5 | 1.0 | Incremento de inestabilidad (TP53 -/-) |
| `division_rate` | 0.0-0.15 | 0.05 | Tasa de división celular por ciclo (%) |
| `apoptosis_threshold` | 1.0-10.0 | 10.0 | Umbral de evasión de apoptosis |

---

## 🔬 Interpretación de Resultados

### ¿Cómo leer los outputs?

**% Neoplásticas alto (>50%):**
- TP53 muy mutágeno O inestabilidad alta
- Riesgo de transformación oncológica

**% Inmortales alto (>90%):**
- Células resistentes a apoptosis
- Necesario para malignidad (pero no suficiente)

**Población decreciente:**
- BRCA1 letal, O apoptosis muy efectiva
- Selección negativa fuerte

### Validación Biológica

✅ **Hallazgos que validan el modelo:**
1. BRCA1 -/- es letal (Escenario 03: extinción)
2. TP53 -/- puede causar neoplasias (Escenario 04: 98% transformadas)
3. Inestabilidad genómica correlaciona con evasión apoptótica
4. Bajas tasas de mutación TP53 son protectoras (Escenario 07: 2.5% neo)
5. Parámetros balanceados reproducen epidemiología real (Escenario 09: 4.7% neo)

---

## ✅ Funcionalidades Implementadas

### Core Biológico
- ✅ Mutaciones genéticas (BRCA1, TP53) estocásticas
- ✅ Apoptosis programada (Fase 2)
- ✅ Transformación neoplástica (TP53 -/- dependent)
- ✅ Evasión de apoptosis (inestabilidad > threshold)
- ✅ División celular con herencia de inestabilidad
- ✅ Inestabilidad genómica progresiva (cuadrática)

### Simulación
- ✅ 6 fases de ciclo celular
- ✅ Checkpoints críticos con excepciones
- ✅ Logging detallado de eventos
- ✅ Trazas reproducibles (seed configurable)

### Validación
- ✅ 14 escenarios de validación (3 controles + 5 fenotipos + 4 realistas + 5 Big Bang)
- ✅ Generación automática de trazas (Markdown + CSV)
- ✅ 55+ tests unitarios PASSING
- ✅ Análisis estadístico integrado
- ✅ Documentación del Modo Big Bang completa

---

## 📂 Estructura del Código

```
src/
├── application/
│   ├── main.cpp                    ← Punto de entrada principal
│   ├── run_all_scenarios.cpp       ← Ejecutor de 9 escenarios
│   └── ...
├── domain/
│   ├── cell/
│   │   ├── ICell.h                 ← Interfaz celular
│   │   ├── Cell.h/cpp              ← Implementación base
│   │   └── NeoplasmCell.h/cpp      ← Célula neoplástica
│   ├── gene/
│   │   ├── Gene.h/cpp
│   │   ├── BRCA1.h/cpp
│   │   └── TP53.h/cpp
│   ├── tissue/
│   │   ├── Tissue.h/cpp            ← Población de células
│   │   └── TissueFactory.h/cpp     ← Creador de tejidos
│   └── signals/
│       ├── ApoptosisSignal.h
│       └── NeoplasmSignal.h
└── shared/
    ├── Random.h                    ← Generador estocástico
    ├── Logger.h                    ← Sistema de logging
    └── exceptions/

tests/
├── unit_tests/                     ← 55+ tests
└── mocks/
```

### Ejemplo: Crear Escenario Personalizado

```cpp
// En run_all_scenarios.cpp o nuevo archivo
Tissue tissue(1000);  // 1000 células iniciales

for (int year = 0; year < 50; year++) {
    for (int cycle = 0; cycle < CYCLES_PER_YEAR; cycle++) {
        tissue.executeFullCycle();
    }
    // Guardar estadísticas
    tissue.logYearlyStatistics(year);
}
```

---

## 🧪 Tests y Validación

```bash
# Ejecutar todos los tests
./cmake-build-debug/tests/unit_tests

# Tests clave:
- GenomeFactoryTest.cpp          ← Creación de genomas
- CellDivisionInstabilityPropagationTest.cpp ← División + herencia
- NeoplasticImmortalityTest.cpp  ← Inmortalización
- ApoptosisTest.cpp              ← Apoptosis y evasión
```

---

## 📖 Documentación Adicional

- **`docs/diagrams.md`** - Diagramas FSM, ciclo de vida, escenarios poblacionales
- **`VALIDATION_RESULTS_ANALYSIS.md`** - Análisis detallado de cada escenario
- **`USAGE_GUIDE.md`** - Guía práctica de uso

---

## 🎓 Casos de Uso

### Para Biólogos:
- Entender relación entre mutaciones y transformación neoplástica
- Estimar riesgo de cáncer bajo diferentes presiones mutacionales
- Validar hipótesis de evolución de poblaciones celulares

### Para Bioinformáticos:
- Simular datos para entrenar modelos de predicción
- Analizar dinámicas estocásticas en pobla poblaciones celulares
- Extender modelo con nuevos genes o mecanismos

### Para Oncólogos:
- Comprender heterogeneidad tumoral
- Modelar resistencia a apoptosis
- Predecir efectividad de terapias dirigidas

---

## 🔧 Troubleshooting

| Problema | Causa | Solución |
|----------|-------|----------|
| Compilación falla | CMake desactualizado | `rm -rf build && make -j4` |
| Tests fallan | Dependencias de googletest | Descargar googletest |
| Trazas no se generan | Permisos de directorio | `mkdir -p cmake-build-debug/traces` |
| Simulación muy lenta | Población inicial alta | Reducir a 500 células |

---

## 📝 Autor y Licencia

**Autor:** Desarrollo interno para investigación  
**Licencia:** Abierto para investigación educativa y académica  
**Última actualización:** 2025-01-04

---

## 🚀 Próximos Pasos

- [ ] Añadir más genes críticos (PTEN, RB1)
- [ ] Interfaz gráfica para visualización en tiempo real
- [ ] Exportar datos a formatos compatibles con R/Python
- [ ] Modelo de microambiente tumoral (células inmunes, stromal)
- [ ] Validación contra datos experimentales reales

