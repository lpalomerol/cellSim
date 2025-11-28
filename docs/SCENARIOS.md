```

**Esperado:**
- 100 células iniciales
- Año 1: ~50% muertas (BRCA1 -> -/- causa apoptosis)
- Año 10: Casi todas muertas (~99%)
- Gráfica de supervivencia: pendiente negativa pronunciada

# Escenarios de Simulación en cellSim

## Descripción General

Los escenarios permiten ejecutar simulaciones con diferentes configuraciones de probabilidades de mutación y otros parámetros genómicos. Actualmente, el ejecutable `random_cells` soporta múltiples escenarios.

---

## Escenarios Disponibles

### 1. `default` (Mutaciones Normales)

**Descripción:**
Escenario estándar con tasas de mutación realistas.

**Configuración:**
```
BRCA1: threshold=0.01, k=0.01
TP53:  threshold=0.01, k=0.01
Neoplasm k: 0.02
```

**Uso:**
```bash
./random_cells 1000 100 42 --scenario default
# O simplemente (default es el valor por defecto):
./random_cells 1000 100 42
```

**Características:**
- ✅ Mutaciones activas en BRCA1 y TP53
- ✅ Cascadas de inestabilidad cuando TP53 se daña
- ✅ Posible desarrollo de neoplasias
- ✅ Apoptosis cuando BRCA1 -> -/-

**Caso de uso:**
- Simulaciones realistas de tumorigénesis
- Estudios de evolución clonal
- Análisis del rol de genes supresores de tumores

---

### 2. `no_mutations` (Sin Mutaciones)

**Descripción:**
Escenario de control donde NO hay mutaciones genéticas. Todas las probabilidades de mutación están en 0.

**Configuración:**
```
BRCA1: threshold=0.0, k=0.0
TP53:  threshold=0.0, k=0.0
Neoplasm k: 0.02
```

**Uso:**
```bash
./random_cells 1000 100 42 --scenario no_mutations
```

**Características:**
- ❌ Sin mutaciones en genes
- ❌ Sin inestabilidad genómica
- ❌ Sin neoplasias por daño genético
- ✅ Sistema inmunológico simulado funciona (si neoplasm_k > 0)

**Caso de uso:**
- **Control negativo**: validar que sin mutaciones no hay cambios genómicos
- Verificar comportamiento basal del tejido
- Estudiar el rol de mutaciones en tumorigénesis
- Debugging de lógica de células

**Resultado esperado:**
- Todas las células mantienen su estado inicial (BRCA1 +/-, TP53 +/+)
- Resumen genético: todas las células en la categoría [BRCA+/- TP53+/+]
- Sin cambios a lo largo de la simulación

---

### 3. `high_brca_apoptosis` (Alta Mutación BRCA1 - Apoptosis Masiva)

**Descripción:**
Escenario extremo donde BRCA1 tiene una probabilidad de mutación muy alta (50% por año). Esto causa apoptosis masiva de células.

**Configuración:**
```
BRCA1: threshold=0.5, k=0.1
TP53:  threshold=0.01, k=0.01
Neoplasm k: 0.02
```

**Uso:**
```bash
./random_cells 1000 100 42 --scenario high_brca_apoptosis
```

**Características:**
- ⚠️ BRCA1 muta FRECUENTEMENTE (50% de probabilidad por año)
- ✅ TP53 normal (baseline)
- 💀 Apoptosis masiva (BRCA1 -/- es letal)
- ❌ Muerte casi total en ~7-10 años

**Caso de uso:**
- Estudiar el rol crítico de BRCA1 en viabilidad
- Validar que BRCA1 -/- causa apoptosis
- Entender cascadas de muerte celular
- Testing de lógica de viabilidad

**Resultado esperado:**
```
Año 0:  100 células [BRCA+/- TP53+/+]
Año 1:  ~50 células vivas, ~50 muertas
Año 5:  ~5 células vivas, ~95 muertas
Año 10: Casi todas muertas (apoptosis completa)
```

**Gráfica de supervivencia esperada:**
```
Vivas |
100   |●●
      |  ●●
 80   |    ●
      |     ●
 60   |      ●
      |       ●●
 40   |         ●
      |          ●
 20   |           ●
      |            ●●●
  0   |________________●●●●●●
      0 1 2 3 4 5 6 7 8 9 10 (años)
```

---

### 4. `high_tp53_mutation` (Alta Mutación TP53 - Neoplasias Masivas)

**Descripción:**
Escenario donde TP53 tiene una probabilidad de mutación muy alta (30% por año), mientras que BRCA1 permanece baja (0.1% por año). Esto favorece el desarrollo de muchas neoplasias sin apoptosis masiva.

**Configuración:**
```
BRCA1: threshold=0.001, k=0.001
TP53:  threshold=0.3, k=0.2
Neoplasm k: 0.02
```

**Uso:**
```bash
./random_cells 1000 100 42 --scenario high_tp53_mutation
```

**Características:**
- 💀 TP53 muta FRECUENTEMENTE (30% de probabilidad por año)
- ✅ BRCA1 muy bajo (evita apoptosis masiva)
- 🧬 Cascada de inestabilidad genómica cuando TP53 se daña
- 🎯 Muchas neoplasias desarrolladas (~60-70% en 20 años)
- ⚠️ Población celular se mantiene pero transformada

**Caso de uso:**
- Estudiar el rol de TP53 en prevención de tumores
- Validar que daño de TP53 favorece transformación neoplástica
- Entender cascadas de tumorigénesis
- Estudios de heterogeneidad clonal en tumores

**Resultado esperado:**
```
Año 0:   100 células normales [BRCA+/- TP53+/+]
Año 5:   80-90 células, 10-20 neoplasias detectadas
Año 10:  70-80 células, 30-40 neoplasias detectadas
Año 20:  60-70 células, 60-70 neoplasias detectadas
```

**Gráfica de transformación esperada:**
```
Neoplasias|
100%      |
   80%    |              ────●
   60%    |          ●────
   40%    |    ●────
   20%    | ●──
    0%    |●─────────────────
          0 5 10 15 20 (años)
```

## Parámetros Comunes

Todos los escenarios aceptan los mismos parámetros posicionales:

```bash
./random_cells [n_cells] [max_t] [seed] [--scenario SCENARIO]
```

| Parámetro | Descripción | Defecto |
|-----------|-------------|---------|
| `n_cells` | Número de células iniciales | 1000 |
| `max_t` | Años máximos de simulación | 50 |
| `seed` | Semilla RNG (-1 = aleatorio) | -1 |
| `--scenario` | Escenario a ejecutar | "default" |

---

## Comparación de Escenarios

| Aspecto | Default | No Mutations | High BRCA Apoptosis | High TP53 Mutation |
|--------|---------|--------------|---------------------|---------------------|
| Mutaciones BRCA1 | ✅ 0.01 | ❌ 0.0 | ⚠️ 0.5 (ALTO) | ✅ 0.001 (BAJO) |
| Mutaciones TP53 | ✅ 0.01 | ❌ 0.0 | ✅ 0.01 | ⚠️ 0.3 (ALTO) |
| Inestabilidad | ✅ Posible | ❌ No | ✅ Normal | 🔴 MUY Alta |
| Cambios genómicos | ✅ Frecuentes | ❌ Ninguno | ⚠️ Solo BRCA1 | 🔴 Muy frecuentes |
| Neoplasias | ✅ Posibles | ❌ Solo azar | ✅ Posibles | 🎯 MUCHAS |
| Apoptosis | ✅ Ocasional | ❌ Nunca | 💀 Masiva | ✅ Rara |
| Supervivencia celular | Intermedia | Máxima | Mínima | Intermedia-Alta |
| % Neoplasias en 20 años | 10-20% | ~0% | N/A (mueren) | 60-70% |
| Casos de uso | Realista | Control | Testing BRCA1 | Testing TP53 |

---

## Ejemplos de Uso

### Ejemplo 1: Simulación control (sin mutaciones)

```bash
cd /home/luis/CLionProjects/cellSim/cmake-build-debug
./random_cells 100 20 42 --scenario no_mutations
```

**Esperado:**
- 100 células, todas con BRCA1 +/-, TP53 +/+
- 20 años sin mutaciones
- Resumen genético: siempre [BRCA+/- TP53+/+] = 100

### Ejemplo 2: Simulación con mutaciones

```bash
./random_cells 100 20 42 --scenario default
```

**Esperado:**
- 100 células iniciales
- Algunas células mueren (BRCA1 -> -/-)
- Algunas células desarrollan neoplasias
- Resumen genético: distribuido entre categorías

### Ejemplo 3: Escenario por defecto (sin especificar)

```bash
./random_cells 500 50 123
# Equivalente a: --scenario default
```

### Ejemplo 4: Simulación con apoptosis masiva (BRCA1 alta)

```bash
./random_cells 100 10 42 --scenario high_brca_apoptosis
```

**Esperado:**
- 100 células iniciales
- Año 1: ~50% muertas (BRCA1 -> -/- causa apoptosis)
- Año 10: Casi todas muertas (~99%)
- Gráfica de supervivencia: pendiente negativa pronunciada

### Ejemplo 5: Simulación con muchas neoplasias (TP53 alta)

```bash
./random_cells 100 20 42 --scenario high_tp53_mutation
```

**Esperado:**
- 100 células iniciales
- Año 5: ~10-20 neoplasias detectadas
- Año 10: ~35 neoplasias detectadas
- Año 20: ~60-70 neoplasias detectadas
- Gráfica de transformación: curva sigmoidea creciente

## Archivos de Configuración (Próximos Pasos)

En futuras versiones, se podrá:
- Crear escenarios adicionales (ej. "high_mutation", "tp53_knockout")
- Cargar escenarios desde archivos JSON o YAML
- Modificar parámetros de neoplasm_k, low_delta_instability, etc.

---

## Validación de Escenarios

Para verificar que un escenario está correctamente configurado:

### `no_mutations`: Todos los genes deben ser estables

```
[Tissue Description] id=0  cells=N  identified_neoplasms=0
  Resumen genético:     N(0)      0(0)      0(0)         0 
    [BRCA+/- TP53+/+] [BRCA+/- TP53+/-] [BRCA+/- TP53-/-] [BRCA-/-]
```

Indicadores:
- ✅ Primera columna = N (todas las células)
- ✅ Otras columnas = 0 (sin cambios)
- ✅ Sin neoplasias identificadas (expected_neoplasms=0)

### `default`: Cambios esperados

```
[Tissue Description] id=0  cells=M  identified_neoplasms=X
  Resumen genético:     A(Y)      B(Z)      C(W)         D 
    [BRCA+/- TP53+/+] [BRCA+/- TP53+/-] [BRCA+/- TP53-/-] [BRCA-/-]
```

Indicadores:
- M < N (algunas células murieron)
- A + B + C + D = M (total concuerda)
- B, C, W > 0 (cambios genómicos presentes)
- X > 0 (neoplasias posibles)

### `high_brca_apoptosis`: Apoptosis masiva esperada

```
Año 0: [Tissue Description] id=0  cells=100  identified_neoplasms=0
         Resumen genético:    100(0)      0(0)      0(0)         0 

Año 1: [Tissue Description] id=0  cells=100  identified_neoplasms=0
         Resumen genético:     50(0)      0(0)      0(0)        50 

Año 5: [Tissue Description] id=0  cells=100  identified_neoplasms=0
         Resumen genético:      5(0)      0(0)      0(0)        95 

Año 10: [Tissue Description] id=0  cells=100  identified_neoplasms=0
          Resumen genético:      0(0)      0(0)      0(0)       100 
```

Indicadores:
- ✅ Primera columna decae rápidamente
- ✅ Última columna (BRCA-/-) aumenta rápidamente
- ✅ Células totales = 100 (pero muchas muertas)
- ✅ A los 10 años: casi todas las células están en estado BRCA-/- (muertas)

### `high_tp53_mutation`: Muchas neoplasias esperadas

```
Año 0: [Tissue Description] id=0  cells=100  identified_neoplasms=0
         Resumen genético:    100(0)      0(0)      0(0)         0 

Año 5: [Tissue Description] id=0  cells=100  identified_neoplasms=12
         Resumen genético:     80(0)      10(0)    8(12)         2 

Año 10: [Tissue Description] id=0  cells=100  identified_neoplasms=35
          Resumen genético:     70(0)      10(0)    15(35)        5 

Año 20: [Tissue Description] id=0  cells=100  identified_neoplasms=68
          Resumen genético:     60(0)      5(0)     28(68)        7 
```

Indicadores:
- ✅ Neoplasias detectadas aumentan rápidamente
- ✅ Tercera columna (BRCA+/- TP53-/-) tiene neoplasias detectadas en paréntesis
- ✅ Población celular se mantiene (~100 células)
- ✅ A los 20 años: ~60-70% de células son neoplásticas

---

## Troubleshooting

### "Todas las células mueren rápidamente"
- Verificar que `--scenario` sea correcto
- Si usas `no_mutations`, BRCA1 nunca muta a -/-, así que esto no debería pasar
- Si usas `default`, es posible que la población inicial tenga mala suerte

### "No hay cambios con default"
- Normal: mutaciones son estocásticas
- Aumentar `n_cells` y `max_t` para ver más eventos
- Verificar que `thresholds` sean > 0

### "Programa corre muy lento"
- Reducir `n_cells` o `max_t`
- Desactivar `verbose` en el código si está activado
- Usar `no_mutations` para debugging (menos computo)

---

## Siguiente Paso

Próximamente se añadirán más escenarios:
- `high_mutation`: Tasas altas de mutación (threshold=0.1)
- `tp53_knockout`: TP53 siempre -/- desde inicio
- `brca1_protected`: BRCA1 protegido contra mutaciones
- Escenarios con parámetros configurables desde JSON/YAML


