# CellSim — Nota técnica para validación estadística
*Versión 2.0 — mayo 2026 — Pendiente de revisión externa*

---

> **Propósito de este documento.** Se solicita validación estadística y metodológica de dos aspectos del experimento de calibración: (1) la idoneidad del ajuste del modelo a los datos clínicos, y (2) la interpretación de una estructura correlacional en el posterior. Se señalan explícitamente los puntos de incertidumbre donde se desea confirmación o rechazo por parte del revisor.

---

## 1. Objetivo del estudio

**CellSim** es un simulador estocástico de tejido mamario portador de mutación germinal BRCA1. El objetivo de la calibración es **identificar los parámetros biológicos del modelo** que mejor reproducen el riesgo acumulado de cáncer de mama observado en la cohorte prospectiva de Kuchenbaecker et al. (2017, JAMA), la referencia epidemiológica de mayor tamaño en portadoras BRCA1 (n ≈ 6.000).

Pregunta científica: ¿Existe un conjunto de parámetros del modelo que reproduzca simultáneamente la curva de incidencia clínica, y qué información sobre la biología tumoral podemos extraer de ese conjunto?

---

## 2. Diseño experimental

### 2.1 El simulador

Agente individual estocástico. Cada tejido = 500 células. Cada tick = 1 año. Tick máximo: 80 (edad del paciente 0–80 años). Variable de salida: **onset** = tick en que ≥5% de células alcanzan estado TUMORAL.

**Cuatro parámetros libres** calibrados:

| Símbolo | Nombre | Prior |
|---------|--------|-------|
| β | `brca1_rate` — tasa de acumulación de daño BRCA1 | U[0.025, 0.080] |
| δ_low | `low_delta` — señalización de daño en BRCA1 +/- | U[0.060, 0.220] |
| δ_high | `high_delta` — señalización de daño en BRCA1 -/- | U[0.120, 0.600] |
| ν | `neoplastic_div_rate` — tasa división neoplásica | U[0.100, 0.240] |

**Tres parámetros fijos** (justificados empíricamente o por sensibilidad nula):

- `tp53_rate = 0.003`: valor óptimo identificado en barrido previo (doc 3); actúa como interruptor, no como dial continuo
- `d1_threshold = 2.0`, `d2_threshold = 5.0`: umbrales adimensionales de activación; insensibles localmente (se asume en esta versión del análisis)

Restricción dura: δ_high > δ_low (el estado homocigoto daña más que el heterocigoto).

> **Nota sobre la interpretación de δ_low:** δ_low y δ_high son **escalares de velocidad de acumulación de daño compartidos entre TP53 y BRCA1**, aplicados según el estado cigótico (+/- → δ_low; -/- → δ_high para TP53; +/- → δ_low, -/- → 2·δ_high para BRCA1). No son específicos de ninguna vía biológica. En la práctica, como todas las células parten de BRCA1 +/-, δ_low determina la velocidad base a la que D2 se acumula en todo el tejido. La interpretación de "tejido con mayor o menor presión de vigilancia" es una propiedad emergente de esta velocidad de acumulación, no un parámetro de señalización directo.

### 2.2 Los datos observados

**Kuchenbaecker et al. (2017)** — Tabla 1, mutaciones BRCA1: CDF de onset a las edades {30, 40, 50, 60, 70, 80} años con intervalos de confianza al 95%.

| Edad | CDF obs. | IC 95% inf. | IC 95% sup. |
|------|----------|-------------|-------------|
| 30 | 0.04 | 0.02 | 0.07 |
| 40 | 0.26 | 0.22 | 0.30 |
| 50 | 0.46 | 0.41 | 0.52 |
| 60 | 0.58 | 0.52 | 0.65 |
| 70 | 0.65 | 0.56 | 0.73 |
| 80 | 0.70 | 0.60 | 0.80 |

---

## 3. Metodología: ABC-SMC

### 3.1 Fundamento

Se usa **Approximate Bayesian Computation — Sequential Monte Carlo** (Toni et al., 2009, J. Roy. Soc. Interface) porque la verosimilitud del simulador es analíticamente intratable.

El algoritmo construye el posterior aproximado:

```
p_ε(θ | D_obs) ∝ π(θ) · P[ ρ(D_sim(θ), D_obs) ≤ ε ]
```

donde π(θ) son los priors uniformes de §2.1, D_sim(θ) son datos simulados y ρ es la métrica definida en §3.2.

### 3.2 Métrica de distancia

**SSE ponderado por incertidumbre clínica:**

```
ρ(θ) = Σ_{a ∈ {30,40,50,60,70,80}}  [CDF_sim(a; θ) − CDF_obs(a)]² / σ²_a
```

donde σ_a = IC_95(a) / (2 × 1.96) es la desviación estándar implícita del IC clínico, y CDF_sim(a; θ) se estima sobre n_sim = 150 réplicas independientes por evaluación.

> **⚠ Punto de validación [V1]:** El uso de n_sim = 150 réplicas introduce ruido de estimación en CDF_sim. La varianza de la estimación es O(1/√150). ¿Es suficiente esta resolución para discriminar entre partículas cercanas en eps finales (~5.0)? Se solicitó análisis de sensibilidad a n_sim en versiones futuras, pero no se ha realizado.

### 3.3 Protocolo SMC

- N = 200 partículas por generación
- ε_0 = 20.0, ε_final = 5.0, α = 0.6 (cuantil adaptativo)
- Kernel de perturbación: Gaussiano con σ_d = 2 · √(Var_d del posterior anterior)
- Máximo 100.000 intentos por generación (límite de cómputo)

| Gen | ε | dist̄ | dist_min | ESS/N |
|-----|---|------|---------|-------|
| 0 | 20.00 | 13.50 | 1.94 | 1.000 |
| 1 | 15.43 | 9.98 | 1.17 | 0.965 |
| 2 | 11.61 | 7.31 | 1.08 | 0.942 |
| 3 | 8.19 | 5.54 | 0.57 | 0.960 |
| 4 | 6.20 | 4.45 | 0.92 | 0.917 |

La **Gen 5** (ε = 4.92) falló al no alcanzar 200 partículas en 100.000 intentos (0.1% tasa de aceptación). Gen 4 se considera el **posterior final**.

> **⚠ Punto de validación [V2]:** ESS/N = 0.917 en Gen 4 es aceptable (> 0.5 habitualmente), pero el salto ε 8.19 → 6.20 es relativamente pequeño. ¿Considera el revisor que la convergencia es suficiente dado el ESS?

---

## 4. Resultados principales

### 4.1 Ajuste a los datos de Kuchenbaecker

El posterior gen4 reproduce los datos clínicos. Evaluando la partícula de mejor ajuste (SSE = 0.92) con n = 500 réplicas:

| Edad | Modelo | Kuchenbaecker | IC 95% | En IC |
|------|--------|---------------|--------|-------|
| 30 | 2.6% | 4.0% | [2–7%] | ✅ |
| 40 | 25.8% | 26.0% | [22–30%] | ✅ |
| 50 | 45.8% | 46.0% | [41–52%] | ✅ |
| 60 | 55.8% | 58.0% | [52–65%] | ✅ |
| 70 | 61.4% | 65.0% | [56–73%] | ✅ |
| 80 | 63.6% | 70.0% | [60–80%] | ✅ |

Los 6 puntos caen dentro del IC 95% clínico. La meseta en ~63% (vs. 70% observado) refleja la fracción de tejidos no penetrantes por TP53, biológicamente esperada.

### 4.2 Posterior de los parámetros

| Parámetro | Media | Std | IC 95% | Prior cubierto (%) |
|-----------|-------|-----|--------|-------------------|
| β_BRCA1 | 0.0445 | 0.0041 | [0.0372, 0.0526] | 28% |
| δ_low | 0.1093 | 0.0189 | [0.0836, 0.1468] | 39% |
| δ_high | 0.3504 | 0.1258 | [0.1498, 0.5914] | ~100% |
| ν | 0.1564 | 0.0410 | [0.1015, 0.2300] | 68% |

β_BRCA1 y δ_low están bien identificados (posterior ≈ 5× más estrecho que prior). δ_high es **"sloppy"**: el IC 95% cubre prácticamente todo el prior, indicando insensibilidad del modelo a este parámetro una vez fijados los demás.

### 4.3 Estructura correlacional: los cuatro cuadrantes

El posterior muestra una anticorrelación significativa:

```
r(δ_low, ν) = −0.609   (p < 10⁻²¹,  n = 200)
```

Para entender por qué el ABC selecciona esta estructura, considérense los cuatro cuadrantes posibles en el espacio (δ_low × ν):

```
                     ν bajo                  ν alto
              ┌────────────────────┬──────────────────────┐
  δ_low alto  │  FENOTIPO B ✅     │  EXCESO ❌            │
              │  CDF ajusta        │  Onset demasiado      │
              │  Señal fuerte +    │  temprano Y rápido    │
              │  expansión lenta   │  → CDF sobre          │
              │                   │  Kuchenbaecker        │
              ├────────────────────┼──────────────────────┤
  δ_low bajo  │  DEFECTO ❌        │  FENOTIPO A ✅        │
              │  Onset tardío Y    │  CDF ajusta           │
              │  expansión lenta   │  Señal débil +        │
              │  → CDF bajo        │  expansión explosiva  │
              │  Kuchenbaecker     │                       │
              └────────────────────┴──────────────────────┘
```

Los cuadrantes fuera de la diagonal son excluidos por la **forma específica de la curva Kuchenbaecker**, no por el prior (que es rectangular y no induce correlación):

- **Exceso (δ_low alto + ν alto):** el daño es muy visible Y el clon se expande muy rápido → el tejido desarrolla tumor demasiado temprano y demasiado rápido → la CDF simulada queda por encima de Kuchenbaecker en todas las edades → SSE alto → rechazado.
- **Defecto (δ_low bajo + ν bajo):** el daño es silencioso Y el clon crece despacio → la CDF simulada queda por debajo de Kuchenbaecker → apenas se desarrollan tumores en el horizonte de 80 años → SSE alto → rechazado.

La diagonal superviviente representa **dos estrategias de compensación** que producen la misma incidencia poblacional mediante mecanismos opuestos:

| | Fenotipo A | Fenotipo B |
|--|-----------|-----------|
| δ_low | 0.096 — señal débil | 0.117 — señal fuerte |
| ν | 0.191 — expansión rápida | 0.129 — expansión lenta |
| sat50 | **56.5 años** | **59.3 años** |
| Cuadrante | bajo-alto | alto-bajo |

> **⚠ Punto de validación [V3 — crítico]:** La anticorrelación r(δ_low, ν) = −0.609 se atribuye aquí a la forma de la curva Kuchenbaecker (la diagonal es la única región de bajo SSE en el espacio 2D), no al prior ni al kernel SMC. La implicación es que la correlación **no es un artefacto** sino una propiedad estructural del modelo. **Se solicita al revisor:** ¿Es esta interpretación correcta, o podría el kernel SMC gaussiano inducir correlaciones artificiales incluso partiendo de priors independientes?

> **⚠ Punto de validación [V4]:** El clustering K-means k=2 es operacional — operacionaliza los extremos de un gradiente continuo. ¿Tiene sentido estadístico identificar dos clusters discretos en este contexto, o sería más honesto describir únicamente el eje de correlación?

### 4.4 Implicación en agresividad: métrica sat50

| Métrica | Fenotipo A (δ_low bajo) | Fenotipo B (δ_low alto) | p-valor (t-test) |
|---------|------------------------|------------------------|------------------|
| onset_age | 43.7 ± 1.5 años | 43.5 ± 1.4 años | 0.25 ❌ |
| sat50 | **56.5 ± 1.9 años** | **59.3 ± 2.3 años** | <0.001 ✅ |

ANOVA sobre tres grupos (terciles): onset_age F=1.26, p=0.285 ❌; sat50 F=18.91, p<10⁻⁶ ✅. Tukey HSD sat50: todos los pares de terciles significativos.

Interpretación: los dos fenotipos inician el tumor a la **misma edad** (~43 años) pero difieren en la **velocidad de colonización tisular post-onset**. δ_low no controla cuándo empieza el tumor sino la dinámica de expansión.

> **⚠ Punto de validación [V5]:** sat50 podría ser una consecuencia tautológica de ν (mayor ν → sat50 menor por construcción). ¿Cómo interpretaría el revisor la significación estadística de sat50 dado que ν es un parámetro directamente calibrado?

---

## 5. Discusión: interpretación evolutiva de los fenotipos

### 5.1 El tejido como sistema de vigilancia entrenado o naive

Dado que todas las células del modelo parten de BRCA1 +/-, δ_low determina la **velocidad base de acumulación de D2 en todo el tejido**. Un δ_low alto no significa que el tejido "detecta mejor" el daño — significa que el daño se acumula más rápido, lo que a su vez produce más eliminación activa de células que cruzan los umbrales. El resultado comportamental es equivalente a un tejido con mayor presión de vigilancia, pero el mecanismo es cinético, no de señalización.

Esta distinción importa para la interpretación: los dos fenotipos no difieren en "cómo de visible es el daño" sino en **a qué velocidad se acumula el daño por tick en el tejido +/-**. Las consecuencias sobre el microambiente son:

**Tejido entrenado (δ_low alto — Fenotipo B):** el microambiente reconoce activamente las células +/- dañadas y las elimina de forma continua. Existe presión selectiva constante sobre todo el tejido. El tumor que eventualmente emerge ha "sobrevivido" a esta vigilancia continua durante décadas: es el resultado de una presión evolutiva acumulada, no de un evento puntual. El onset es inevitable pero el crecimiento es lento porque el tumor se desarrolla en un tejido activamente vigilante.

**Tejido naive (δ_low bajo — Fenotipo A):** el microambiente no reconoce eficientemente las células +/- dañadas. Las células con daño acumulan silenciosamente. No hay presión selectiva que entrene la respuesta tisular. Cuando un clon finalmente cruza el umbral neoplásico (por acumulación estocástica), lo hace en un microambiente sin resistencia entrenada. El evento es menos probable estadísticamente, pero cuando ocurre produce una expansión explosiva: el tejido no tiene "memoria" de la amenaza.

La consecuencia clave: **ambos fenotipos tienen el mismo onset medio** (~43 años) porque la probabilidad de que un clon cruce el umbral es similar en ambos casos — lo que varía es la velocidad de colonización una vez cruzado. δ_low controla la resistencia del microambiente, no la probabilidad de iniciación.

### 5.2 Conexión con el concepto de inmunoedición tumoral

Este gradiente es coherente con el modelo de inmunoedición tumoral (Schreiber et al., 2011, Science), que describe tres fases: eliminación, equilibrio y escape. En el Fenotipo B, el tejido opera permanentemente en fase de *equilibrio* (elimina activamente, el clon persiste pero controlado); en el Fenotipo A, el tejido pasa directamente de *ignorancia inmunológica* a *escape* sin fase de equilibrio detectable.

### 5.3 Implicación evolutiva: el Fenotipo B como mecanismo adaptativo

Un argumento adicional a favor de la plausibilidad biológica del gradiente es su coherencia con la teoría evolutiva del envejecimiento.

La selección natural opera exclusivamente dentro de la ventana reproductiva, que en la historia evolutiva humana se situaba aproximadamente entre los 15 y 40 años. Un tejido BRCA1 +/- con δ_low **alto** (Fenotipo B) es funcionalmente óptimo dentro de esa ventana: la vigilancia activa mantiene el tejido limpio durante las décadas de mayor valor reproductivo. El tumor que eventualmente emerge lo hace a los 59+ años — una edad históricamente post-reproductiva y, en la mayoría de los contextos evolutivos, post-vital. La selección natural no tuvo mecanismo para "ver" este coste, y por tanto tampoco para eliminarlo.

Desde esta perspectiva, el Fenotipo B no es solo clínicamente "menos malo" que el A — es **evolutivamente deseable**: realiza un barrido de fondo continuo del tejido durante las décadas que importan biológicamente, y externaliza el coste oncológico hacia edades que la naturaleza no estaba diseñada para mantener. Solo la extensión de la esperanza de vida humana más allá del rango evolutivo convierte este mecanismo en un problema clínico.

El Fenotipo A es el más problemático en el contexto moderno precisamente porque su sat50 de 56.5 años cae dentro de la ventana de vida activa ampliada que la medicina ha conseguido alcanzar. No es un fallo del sistema biológico — es un sistema que nunca fue seleccionado para ese escenario.

Esta interpretación conecta con dos marcos teóricos establecidos:

- **Antagonismo pleiotrópico** (Williams, 1957): un mismo mecanismo genético puede ser ventajoso en edades tempranas y perjudicial en edades tardías. δ_low bajo puede conferir ventajas de tolerancia tisular en la juventud (menos eliminación de células con daño leve → mayor plasticidad celular) a expensas de mayor riesgo oncológico tardío.
- **Soma desechable** (Kirkwood, 1977): la inversión en mantenimiento y reparación del soma es suficiente para maximizar la aptitud reproductiva, no para la longevidad indefinida. El Fenotipo B invierte más en mantenimiento tisular (δ_low alto → vigilancia activa) — una estrategia adaptativa dentro del ciclo de vida evolutivo, pero cuyo coste diferido solo se hace visible porque vivimos décadas más de lo que la selección natural "planificó".

Una consecuencia clínica relevante: el Fenotipo B, por tener un microambiente entrenado con memoria inmune activa, debería responder mejor a terapias de checkpoint inmune (anti-PD1, anti-CTLA4) que buscan "desbloquear" una respuesta ya existente. El Fenotipo A, sin esa memoria, requeriría estrategias de sensibilización inmune previas. Esta hipótesis es verificable y podría orientar protocolos de tratamiento diferenciado para portadoras BRCA1.

### 5.4 Limitaciones del diseño paramétrico actual

El análisis anterior asume que δ_low y δ_high son escalares globales válidos, pero el código revela tres suposiciones fuertes no validadas:

1. **Escalares compartidos entre genes:** el mismo δ_low controla la acumulación de daño para TP53 +/- Y BRCA1 +/- simultáneamente. Biológicamente, TP53 y BRCA1 tienen funciones distintas (reparación de doble cadena vs. regulación del ciclo celular) y podrían acumular daño a ritmos diferentes. Una parametrización más honesta requeriría `delta_tp53 ≠ delta_brca1`.

2. **Ratio D2/D1 fijo (2.5):** los umbrales D1=2.0 y D2=5.0 son suposiciones a priori. El ratio 2.5 codifica que alcanzar evasión inmune requiere 2.5× más daño que inestabilidad inicial — sin respaldo cuantitativo. Calibrar este ratio sería un experimento futuro.

3. **Multiplicador LOH-BRCA1 fijo (2×):** BRCA1 -/- contribuye 2·δ_high a D2, mientras que TP53 -/- contribuye solo δ_high. Este factor amplificador es una suposición de diseño, no un parámetro estimado.

Estas limitaciones no invalidan los resultados actuales — el modelo con estos supuestos reproduce Kuchenbaecker correctamente — pero sí acotan la interpretabilidad biológica de los parámetros estimados.

> **⚠ Punto de validación [V6]:** ¿Considera el revisor que las suposiciones (1)–(3) introducen sesgos que afectan materialmente a la interpretación de los fenotipos A/B, o son simplificaciones aceptables para un modelo de primer orden?

Para contextualizar la dinámica a audiencias no especializadas en biología molecular, proponemos la siguiente analogía:

Imaginemos dos tipos de costa frente a fenómenos de erosión. La **primera costa** tiene un sistema de gestión activo: vigilancia continua, pequeñas intervenciones de mantenimiento frecuentes, el peligro es reconocido y contenido regularmente. Cuando finalmente ocurre un evento mayor, la infraestructura de respuesta ya existe y la erosión progresa de forma gradual y predecible.

La **segunda costa** no tiene gestión activa: el desgaste se acumula silenciosamente durante años, sin que ningún mecanismo lo detecte ni lo contenga. La costa parece estable durante mucho tiempo. Pero cuando la acumulación de daño supera un umbral crítico, el colapso es rápido y difícil de frenar — precisamente porque no existe infraestructura de respuesta.

La curva de incidencia Kuchenbaecker es la suma de ambas dinámicas sobre la población de portadoras BRCA1: algunos tejidos responden activamente (Fenotipo B, progresión gradual) y otros han acumulado daño silenciosamente hasta un punto de no retorno (Fenotipo A, progresión explosiva). La media poblacional produce la curva sigmoide observada, pero los mecanismos subyacentes son cualitativamente distintos.

---

## 6. Solicitud de validación: resumen de preguntas abiertas

| Ref. | Pregunta | Naturaleza |
|------|----------|------------|
| **V1** | ¿Es n_sim = 150 suficiente para discriminar partículas a eps = 6? | Ruido de estimación |
| **V2** | ¿Es Gen 4 (ESS/N = 0.917) un posterior convergido? | Convergencia SMC |
| **V3** | ¿Es r(δ_low, ν) = −0.609 estructura real o podría el kernel SMC inducirla artificialmente? | **Validez del resultado central** |
| **V4** | ¿Tiene sentido k-means sobre un gradiente continuo? | Interpretación clustering |
| **V5** | ¿Es la diferencia sat50 entre fenotipos una predicción no trivial o consecuencia tautológica de ν? | **Validez interpretación biológica** |

---

*Figuras disponibles bajo petición: `gen4_cdf_fit.png`, `gen4_pairwise.png`, `gen4_phenotypes.png`, `gen4_aggressiveness_3groups.png`. Datos: `results/abc_corrected_v1/gen_04.csv` (200 filas, 10 columnas).*
