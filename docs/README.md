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

## 📊 6 Escenarios Configurables

Cada escenario configura la **probabilidad de mutación** de cada gen (umbral):

| Escenario | P(BRCA1→-/-) | P(TP53 mutación) | División | Duración | Resultado |
|-----------|-------|------|----------|----------|-----------|
| default | 0.01 | 0.01 | 0% | 50 años | Balance |
| no_mutations | 0.0 | 0.0 | 0% | 50 años | Población estable |
| high_brca_apoptosis | 0.5 | 0.01 | 0% | 10 años | Colapso ~99% (muertes masivas) |
| high_tp53_mutation | 0.001 | 0.3 | 0% | 50 años | 60-70% neoplásticas en 20 años |
| cell_division_healthy | 0.0 | 0.0 | 10% | 5 años | 2.7x crecimiento exponencial |
| realistic_division | 0.01 | 0.01 | 1% | 50 años | Crecimiento controlado con mutaciones |

---

## 🎮 Uso

```bash
# Compilar
make -j4

# Ejecutar con escenario
./build/random_cells [n_cells] [max_t] [seed] --scenario [name]

# Ejemplos
./build/random_cells 100 50 42 --scenario realistic_division
./build/random_cells 100 10 42 --scenario high_brca_apoptosis
./build/random_cells 20 5 42 --scenario cell_division_healthy
```

---

## 🔬 Insights Clave

1. **BRCA1 es viabilidad** - BRCA1 -/- = muerte garantizada
2. **TP53 es guardián** - Sin TP53, 60-70% neoplasias en 20 años
3. **Inestabilidad crece exponencialmente** - `instability = instability²` cada ciclo
4. **Resistencia a apoptosis** - Mayor inestabilidad = rechaza más muertes
5. **División-Mutación balance** - Poblaciones pueden crecer con riesgo bajo

---

## ✅ Funcionalidades Implementadas

- ✅ Mutaciones genéticas (BRCA1, TP53)
- ✅ Apoptosis por BRCA1 -/-
- ✅ Transformación neoplástica
- ✅ Rechazo de apoptosis (basado en inestabilidad)
- ✅ División celular con clonación
- ✅ Inestabilidad genómica progresiva
- ✅ 6 escenarios implementados
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

