# CellSim: Resumen técnico del proceso de calibración
*Para discusión interna — borrador*

---

## (i) El modelo

CellSim es un simulador estocástico basado en agentes escrito en C++20. Cada
*tejido* se representa como una población de 500 células autónomas. Cada célula
lleva un genoma con dos genes clave:

- **BRCA1**: se inicializa en estado heterocigoto (+/-), como corresponde a una
  portadora germinal. Puede mutar hacia la pérdida de heterocigosidad (-/-).
- **TP53**: se inicializa como salvaje (+/+). Controla la apoptosis intrínseca.

En cada *tick* (= 1 año), las células acumulan daño genómico (acumuladores D1 y
D2), pueden mutar sus genes, y transitan por seis estadios de vida:

```
BASELINE → UNSTABLE → UNPROTECTED → PRIMER → TUMORAL → DEAD
```

La progresión es emergente: no está programada directamente sino que surge de las
reglas locales de cada célula. El tejido "desarrolla tumor" cuando la fracción de
células neoplásicas supera el **5%** (≈ 10⁹ células reales, límite de detección
mamográfica). El modelo se ejecuta durante 80 ticks (años 0–80).

---

## (ii) Parámetros del modelo

### Parámetros primarios (los que calibramos)

| Parámetro | Descripción | Rango explorado |
|-----------|-------------|-----------------|
| `brca1_rate` | Probabilidad anual de que BRCA1 +/- mute a -/- (segundo golpe) | 0.010 – 0.100 |
| `low_delta` | Incremento de daño D1/D2 por tick en célula BRCA1 +/- | 0.020 – 0.150 |
| `high_delta` | Incremento de daño en célula BRCA1 -/- (= 2 × low_delta) | derivado |

### Parámetros secundarios (fijos durante la calibración)

| Parámetro | Descripción | Valor |
|-----------|-------------|-------|
| `tp53_rate` | Probabilidad anual de mutación TP53 | 0.003 |
| `d1_threshold` | Umbral de D1 para entrar en estado PRIMER | 2.0 |
| `d2_threshold` | Umbral de D2 para evasión inmune (TUMORAL) | 5.0 |
| `tumor_threshold` | Fracción neoplásica para declarar onset | 5% |
| `n_cells` | Células por tejido simulado | 500 |
| `max_t` | Duración de la simulación | 80 ticks |

---

## (iii) Valores por defecto y criterio de selección

Los valores por defecto fueron establecidos como **punto de partida razonable**,
no como valores validados. El criterio fue: "por algo hay que empezar, y estos
valores producen un comportamiento cualitativamente interesante sin colapsar el
modelo." Por ejemplo:

- `brca1_rate = 0.008` → ~1 mutación cada 125 años: plausible pero lento
- `low_delta = 0.15`, `high_delta = 0.40` → proporciones arbitrarias que generan
  progresión en el rango de 40–80 años

Con estos valores por defecto, el modelo produce SSE = 5295 frente a Kuchenbaecker:
la curva llega muy tarde (cero tumores antes de los 50) y se pasa en el plateau
(84% a los 80 años en lugar de 70%). Es un modelo que "funciona" pero no reproduce
la clínica.

*(Ver **Figura 1**: parámetros iniciales vs. calibrados — `fig1_initial_vs_calibrated.png`)*

---

## (iv) Fine tuning inicial: el big bang es necesario

Realizamos una búsqueda en rejilla con 36 combinaciones de `brca1_rate` × `low_delta`
(N=200 runs por combo). El mejor resultado sin big bang fue SSE=2078.

**Problema estructural descubierto:** existe una incompatibilidad fundamental:
- Subir `brca1_rate` → onset más temprano, pero el plateau final es demasiado bajo
- Bajar `brca1_rate` → plateau correcto (~70%), pero onset muy tardío

Ninguna combinación lograba simultáneamente r40 ≈ 26% Y r80 ≈ 70%.

**Causa raíz:** sin big bang, la tasa de división de células neoplásicas es
idéntica a la normal (0.1% por tick). Una única célula tumoral tarda décadas en
expandirse al 5% del tejido, retrasando artificialmente el onset declarado. El
modelo no puede separar "cuándo aparece la primera célula tumoral" de "cuánto tarda
el clon en crecer".

*(Ver **Figura 2**: efecto del Big Bang — `fig2_bigbang_effect.png`)*

---

## (v) Añadimos Big Bang y hacemos nuevo fine tuning

**Big Bang:** cuando una célula entra en estado PRIMER, su tasa de división pasa
de 0.1% a **10% por tick** (×100). Esto modela la expansión clonal acelerada
característica de los tumores tempranos (dinámica gompertziana). **No afecta las
tasas de mutación** — solo la velocidad de crecimiento del clon una vez iniciado.

Con big bang activo, repetimos la búsqueda en rejilla en dos fases:

**Fase 3 (rejilla gruesa, N=200):** SSE bajó de 2078 a **243** (mejora ×8.5).
El mejor combo identificado: brca1=0.040, low_d=0.100.

**Fase 4 (rejilla fina, N=500):** 42 combinaciones alrededor del óptimo.
Mejor resultado: brca1=0.045, low_d=0.120, high_d=0.240 → **SSE=48.5**

Confirmado con N=1000 runs:

| Edad | CellSim | Kuchenbaecker | IC 95% |
|------|---------|---------------|--------|
| 30   | 1.7%    | 4.0%          | [2–7%] ← ligero gap |
| 40   | 22.7%   | 26.0%         | [22–30%] ✅ |
| 50   | 49.1%   | 46.0%         | [41–52%] ✅ |
| 60   | 61.6%   | 58.0%         | [52–65%] ✅ |
| 70   | 68.1%   | 65.0%         | [56–73%] ✅ |
| 80   | 70.3%   | 70.0%         | [60–80%] ✅ |

El 30% de tejidos que nunca supera el umbral (no penetrancia) emerge
naturalmente de la protección de TP53, sin programarlo explícitamente.

*(Ver **Figura 3**: calibración final vs. Kuchenbaecker — `fig3_calibration_kuchenbaecker.png`)*

---

## (vi) Exploración de tp53_rate: comportamiento binario

Para intentar cerrar el gap residual en r30 (1.7% vs 4%), exploramos si
aumentar ligeramente `tp53_rate` podría incrementar el onset temprano.

Realizamos dos barridos: uno grueso (0.003–0.020) y uno muy fino (0.0025–0.0050
en pasos de 0.0005). Resultado: el parámetro se comporta como un **interruptor**,
no como un dial continuo:

| tp53_rate | r30  | r40   | r80   | SSE     |
|-----------|------|-------|-------|---------|
| 0.0025    | 0.6% | 16.2% | 58.0% | 486 ❌  |
| **0.0030**| **1.8%** | **22.4%** | **70.0%** | **53 ✅** |
| 0.0035    | 2.4% | 31.6% | 80.4% | 670 ❌  |
| 0.0040    | 4.4% | 40.6% | 88.8% | 2281 ❌ |
| 0.0050    | 8.2% | 58.0% | 96.0% | 5171 ❌ |

El paso de 0.0030 a 0.0035 (+0.0005, apenas un +17%) ya dispara el SSE de 53
a 670 y eleva el plateau a 80%. Irónicamente, tp53=0.0040 *sí* clava el r30
(4.4% ≈ 4%), pero destroza completamente r40–r80.

Esto confirma que **0.003 es el óptimo global**: el parámetro controla la
no-penetrancia del modelo, y cualquier incremento elimina el 30% de tejidos
que nunca desarrollan tumor, que es precisamente lo que reproduce el dato clínico.

**Conclusión:** el gap en r30 (1.7% vs 4%) no es un artefacto de calibración
insuficiente — es una **limitación estructural del modelo**. Con los parámetros
actuales, el modelo está en su óptimo global. Dado que r40 ya está dentro del
IC de Kuchenbaecker [22–30%], el modelo es válido para publicación. El gap en
r30 se reporta honestamente como limitación.

---

## Resumen del proceso de calibración

| Fase | Configuración | Mejor SSE |
|------|--------------|-----------|
| Baseline (defaults) | sin BB | 5295 |
| Rejilla gruesa | sin BB | 2078 |
| Rejilla gruesa | **con BB** | 243 |
| Rejilla fina | **con BB** | **48.5** ✅ |
| Exploración TP53 | con BB | 48.5 (sin mejora) |

**Parámetros calibrados finales:**
- `brca1_rate = 0.045`
- `low_delta = 0.120`
- `high_delta = 0.240`
- `big_bang = true` (neoplastic_division_rate = 0.1)

