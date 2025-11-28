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

---

### 5. `cell_division_healthy` (División Celular - Crecimiento Saludable)

**Descripción:**
Escenario donde NO hay mutaciones genéticas pero SÍ hay división celular activa. Las células se dividen con una tasa del 10% por año, permitiendo crecimiento exponencial de la población.

**Configuración:**
```
BRCA1: threshold=0.0, k=0.0
TP53:  threshold=0.0, k=0.0
Division rate: 0.1 (10%)
Neoplasm k: 0.02
```

**Uso:**
```bash
./random_cells 20 5 42 --scenario cell_division_healthy
```

**Características:**
- ✅ Sin mutaciones genéticas (todas las células permanecen normales)
- 📈 División celular activa a tasa del 10% por año
- 🧬 Genoma estable (no hay neoplasias)
- 📊 Crecimiento exponencial de la población
- ⚠️ TASA DE DIVISIÓN MUY ALTA para visualización clara

**Caso de uso:**
- Validar mecánica de división celular
- Estudiar crecimiento exponencial en tejidos sin mutaciones
- Testing del sistema de señales de división
- Visualizar dinámicas de proliferación

**Resultado esperado (10 células iniciales, 5 años):**
```
Año 0: 10 células [BRCA+/- TP53+/+]
Año 1: 11 células (1 división)
Año 2: 14 células (3 divisiones)
Año 3: 17 células (3 divisiones adicionales)
Año 4: 21 células (4 divisiones adicionales)
Año 5: 27 células (6 divisiones adicionales)
```

**Gráfica de crecimiento esperada:**
```
Células|
   30  |              ●
   28  |            ●
   26  |          ●
   24  |        ●
   22  |      ●
   20  |    ●
   18  |  ●
   16  |●
   14  |
   12  |●
   10  |●──────────────────
       0 1 2 3 4 5 (años)
```

**Mensajes del tissue esperados:**
```
[Tissue] Cell division signal detected from cell id=0; adding daughter cell
[Tissue] Cell division signal detected from cell id=1; adding daughter cell
[Tissue] Cell division signal detected from cell id=7; adding daughter cell
```

---

### 6. `realistic_division` (Realista con División Celular - 50 años)

**Descripción:**
Escenario realista que combina mutaciones (BRCA1 con threshold=0.01) y división celular (rate=0.01, 1% por año). Simula un tejido normal que crece mientras algunas células eventualmente mueren por apoptosis. Pensado para correr 50 años y observar dinámicas complejas a largo plazo.

**Configuración:**
```
BRCA1: threshold=0.01, k=0.01
TP53:  threshold=0.01, k=0.01
Division rate: 0.01 (1%)
Neoplasm k: 0.02
```

**Uso:**
```bash
./random_cells 100 50 42 --scenario realistic_division
```

**Características:**
- ✅ Mutaciones realistas en BRCA1 y TP53 (1% por año)
- 📈 División celular realista (1% por año, similar a BRCA1)
- 💀 Apoptosis ocasional cuando BRCA1 -> -/-
- 🧬 Posible desarrollo de neoplasias a largo plazo
- ⏱️ Simulación larga (50 años) para ver dinámicas complejas
- ⚖️ Balance entre crecimiento, muerte y transformación

**Caso de uso:**
- Simulaciones realistas de tejidos normales en crecimiento
- Estudiar interacción entre división, mutación y apoptosis a largo plazo
- Entender dinámica a largo plazo (50 años) de poblaciones celulares
- Análisis de competencia clonal en presencia de división celular
- Estudios de envejecimiento tisular y transformación neoplástica

**Resultado esperado (100 células iniciales, 50 años):**
```
Año 0:  100 células [BRCA+/- TP53+/+]
Año 10: 110-120 células (crecimiento lento, algunas muertas)
Año 20: 120-150 células (balance entre división y muerte)
Año 30: 140-180 células (crecimiento gradual se mantiene)
Año 40: 160-200 células (población se estabiliza con divisiones y muertes)
Año 50: 180-240 células (crecimiento neto moderado, neoplasias visibles)
```

**Gráfica de dinámica esperada:**
```
Células|
  250  |              ●───●───●
  200  |          ●───          
  150  |      ●───                
  100  |  ●───                    
   50  |●                         
    0  |____________________________
      0  10  20  30  40  50 (años)
```

**Indicadores de validación:**
- ✅ Población nunca decrece a 0 (división > muerte en promedio)
- ✅ Crecimiento es lento pero sostenido (balance entre procesos)
- ✅ Algunas neoplasias pueden detectarse (~5-15% en 50 años)
- ✅ Resumen genético muestra distribución en varias categorías
- ✅ Algunas células en estado BRCA-/- (muertas pero contadas)

**Dinámicas esperadas a diferentes puntos de tiempo:**

*Año 10:*
```
[Tissue Description] id=0  cells=112  identified_neoplasms=0-2
  Resumen genético:     90(0)      15(0)      5(0-2)         2
```

*Año 30:*
```
[Tissue Description] id=0  cells=155  identified_neoplasms=5-8
  Resumen genético:    120(0)      20(0)      12(5-8)         3
```

*Año 50:*
```
[Tissue Description] id=0  cells=210  identified_neoplasms=12-20
  Resumen genético:    170(0)      25(0)      10(12-20)       5
```

---

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

| Aspecto | Default | No Mutations | High BRCA Apoptosis | High TP53 Mutation | Cell Division | Realistic Division |
|--------|---------|--------------|---------------------|---------------------|----------------|---------------------|
| Mutaciones BRCA1 | ✅ 0.01 | ❌ 0.0 | ⚠️ 0.5 (ALTO) | ✅ 0.001 (BAJO) | ❌ 0.0 | ✅ 0.01 |
| Mutaciones TP53 | ✅ 0.01 | ❌ 0.0 | ✅ 0.01 | ⚠️ 0.3 (ALTO) | ❌ 0.0 | ✅ 0.01 |
| División celular | ❌ 0% | ❌ 0% | ❌ 0% | ❌ 0% | ✅ 10% (ALTA) | ✅ 1% |
| Inestabilidad | ✅ Posible | ❌ No | ✅ Normal | 🔴 MUY Alta | ❌ No | ✅ Posible |
| Cambios genómicos | ✅ Frecuentes | ❌ Ninguno | ⚠️ Solo BRCA1 | 🔴 Muy frecuentes | ❌ Ninguno | ✅ Graduales |
| Neoplasias | ✅ Posibles | ❌ Solo azar | ✅ Posibles | 🎯 MUCHAS | ❌ No (sin mutaciones) | ✅ Gradualmente |
| Apoptosis | ✅ Ocasional | ❌ Nunca | 💀 Masiva | ✅ Rara | ❌ Nunca | ✅ Ocasional |
| Crecimiento poblacional | Estable | Estable | Decreciente | Estable | 📈 Exponencial | 📈 Lento pero sostenido |
| Supervivencia celular | Intermedia | Máxima | Mínima | Intermedia-Alta | Máxima | Alta |
| % Neoplasias en 20 años | 10-20% | ~0% | N/A (mueren) | 60-70% | ~0% | 5-10% |
| Duración típica | 50 años | 50 años | 10 años | 50 años | 5-10 años | 50 años (larga) |
| Casos de uso | Realista base | Control | Testing BRCA1 | Testing TP53 | Testing división | Realista completo |

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

### Ejemplo 6: Simulación con división celular (sin mutaciones)

```bash
./random_cells 20 5 42 --scenario cell_division_healthy
```

**Esperado:**
- 20 células iniciales, todas normales
- División celular a tasa del 10% por año
- Año 1: 21 células
- Año 2: 24 células
- Año 3: 27 células
- Año 4: 31 células
- Año 5: 35+ células
- Mensajes: `[Tissue] Cell division signal detected from cell id=X; adding daughter cell`
- Resumen genético: todas las células en [BRCA+/- TP53+/+]

### Ejemplo 7: Simulación realista con división (50 años)

```bash
./random_cells 100 50 42 --scenario realistic_division
```

**Esperado:**
- 100 células iniciales
- División celular a tasa del 1% (similar a mutación de BRCA1)
- Año 10: ~110-120 células
- Año 20: ~120-150 células
- Año 30: ~140-180 células
- Año 40: ~160-200 células
- Año 50: ~180-240 células
- Neoplasias detectadas: 5-20 en 50 años
- Gráfica de crecimiento: lenta pero sostenida
- Balance entre mutaciones, divisiones y muertes

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

### `cell_division_healthy`: Crecimiento exponencial esperado

```
Año 0: [Tissue Description] id=0  cells=10  identified_neoplasms=0
         Resumen genético:     10(0)      0(0)      0(0)         0 

Año 1: [Tissue Description] id=0  cells=11  identified_neoplasms=0
         Resumen genético:     11(0)      0(0)      0(0)         0 
         [Tissue] Cell division signal detected from cell id=2; adding daughter cell

Año 2: [Tissue Description] id=0  cells=14  identified_neoplasms=0
         Resumen genético:     14(0)      0(0)      0(0)         0 
         [Tissue] Cell division signal detected from cell id=0; adding daughter cell
         [Tissue] Cell division signal detected from cell id=5; adding daughter cell

Año 5: [Tissue Description] id=0  cells=27  identified_neoplasms=0
         Resumen genético:     27(0)      0(0)      0(0)         0 
```

Indicadores:
- ✅ Mensajes de división detectables en cada año
- ✅ Todas las células en [BRCA+/- TP53+/+] (sin cambios genómicos)
- ✅ Crecimiento exponencial (10 → 27 en 5 años)
- ✅ Sin neoplasias detectadas (mutation_rate = 0)
- ✅ Crecimiento acelerado con el tiempo (no lineal)

### `realistic_division`: Balance entre procesos

```
Año 0: [Tissue Description] id=0  cells=100  identified_neoplasms=0
         Resumen genético:    100(0)      0(0)      0(0)         0 

Año 10: [Tissue Description] id=0  cells=112  identified_neoplasms=1-2
          Resumen genético:     85(0)      15(0)      8(1-2)      4 

Año 30: [Tissue Description] id=0  cells=155  identified_neoplasms=5-8
          Resumen genético:    115(0)      20(0)     15(5-8)      5 

Año 50: [Tissue Description] id=0  cells=210  identified_neoplasms=12-20
          Resumen genético:    170(0)      25(0)     10(12-20)     5 
```

Indicadores:
- ✅ Crecimiento lento pero sostenido (100 → 210 en 50 años)
- ✅ Neoplasias aumentan gradualmente (~25% en 50 años)
- ✅ Algunas células muertas (BRCA-/- positivas)
- ✅ Distribución genómica en múltiples categorías
- ✅ Balance entre división (↑) y muerte (↑)
- ✅ Crecimiento neto positivo (división > muerte)

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

Escenarios completados: 6
- ✅ `default` - Mutaciones normales
- ✅ `no_mutations` - Control sin mutaciones
- ✅ `high_brca_apoptosis` - Apoptosis masiva
- ✅ `high_tp53_mutation` - Neoplasias masivas
- ✅ `cell_division_healthy` - División celular pura (10%)
- ✅ `realistic_division` - División realista con mutaciones (1%)

Próximos escenarios a implementar:
- `high_mutation`: Tasas altas de mutación en ambos genes (threshold=0.1)
- `tp53_knockout`: TP53 siempre -/- desde inicio
- `brca1_protected`: BRCA1 protegido contra mutaciones
- `cell_division_aggressive`: División celular muy alta (20%) con mutaciones
- Escenarios con parámetros configurables desde JSON/YAML


