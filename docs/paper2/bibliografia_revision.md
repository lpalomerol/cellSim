# Revisión Bibliográfica — Modelos de Simulación en Cáncer y Cribado Poblacional

> Generado: 2026-04-29  
> Contexto: cellSim / paper2  
> PDFs fuente: `docs/paper2/refs/`

---

## Resumen General

Los seis artículos revisados abordan, desde perspectivas complementarias, el uso de modelos matemáticos, computacionales y de simulación para estudiar la iniciación, progresión y detección temprana del cáncer. Se identifican tres ejes temáticos principales: **(1) modelos matemáticos de evolución tumoral**, **(2) revisiones metodológicas de modelos de simulación en cribado**, y **(3) aplicaciones clínicas y epidemiológicas**.

---

## Artículo 1 — Paterson et al. (2020), PNAS

**Título:** *Mathematical model of colorectal cancer initiation*  
**DOI:** 10.1073/pnas.2003771117  
**Archivo:** `paterson-et-al-mathematical-model-of-colorectal-cancer-initiation-1.pdf`

**Tipo:** Modelado matemático / Biología computacional

**Resumen:**
Se desarrolla un modelo estocástico de iniciación del cáncer colorrectal (CRC) que incorpora la inactivación de los supresores tumorales *APC* y *TP53*, y la activación del oncogén *KRAS*. A diferencia de enfoques previos, el modelo permite que las mutaciones se adquieran en cualquier orden, generando una red compleja de genotipos premalignosoobsolete en el camino hacia la malignidad. El modelo se parametriza con tasas de mutación medidas experimentalmente *in vivo* en tejido colónico y con ventajas selectivas reportadas para cada alteración driver.

**Hallazgos clave:**
- El orden de adquisición de eventos driver está determinado principalmente por los efectos de *fitness*, no por sus tasas de mutación.
- La inactivación de *APC* es el evento driver más frecuente como primer paso (67,9% de los casos).
- *TP53* nunca fue observado como primer evento en el camino a la malignidad.
- El riesgo de CRC a lo largo de la vida puede recuperarse sin asumir tasas de mutación elevadas ni grandes ventajas proliferativas en lesiones premalignas.
- Los resultados sugieren que el sistema inmunológico podría no ejercer presión significativa sobre las lesiones premalignas durante la evolución tumoral.

**Relevancia para cellSim:** ⭐⭐⭐ Alta  
Valida el enfoque de mutaciones unidireccionales en *TP53* y *BRCA1*, y respalda el modelado de redes complejas de genotipos en lugar de trayectorias mutacionales lineales fijas.

---

## Artículo 2 — Sridharan & Ghosh (2025), *Scientific Reports*

**Título:** *Gene expression and agent-based modeling improve precision prognosis in breast cancer*  
**DOI:** 10.1038/s41598-025-01275-w  
**Archivo:** `s41598-025-01275-w.pdf`

**Tipo:** Modelado basado en agentes (ABM) / Aprendizaje automático

**Resumen:**
Se propone un marco unificado que combina el perfilado de expresión génica (GEP, *Gene Expression Profiling*) con modelado basado en agentes (ABM, *Agent-Based Modeling*) para mejorar la predicción de supervivencia en cáncer de mama. El modelo identifica genes diferencialmente expresados relevantes para la progresión tumoral, los incorpora como propiedades de los agentes en el ABM y simula el comportamiento del tumor y la respuesta al tratamiento a nivel individual.

**Hallazgos clave:**
- Validado en tres cohortes independientes (METABRIC, TCGA-BRCA, GSE96058): C-index 0,82 y AUC-ROC 0,81 para supervivencia a 5 años en TCGA-BRCA.
- Las características más predictivas (recuento de mutaciones, afectación de nódulos linfáticos, grado histológico) fueron consistentes entre cohortes.
- La integración de SVD (*Singular Value Decomposition*) con ABM mejora la robustez de las predicciones frente a análisis GEP clásicos.

**Relevancia para cellSim:** ⭐⭐⭐ Alta  
Proporciona evidencia metodológica de que el ABM a nivel celular individual, parametrizado con datos de expresión génica, es capaz de generar predicciones clínicamente válidas.

---

## Artículo 3 — Koleva-Kolarova et al. (2015), *The Breast*

**Título:** *Simulation models in population breast cancer screening: A systematic review*  
**DOI:** 10.1016/j.breast.2015.03.013  
**Archivo:** `j.breast.2015.03.013.pdf`

**Tipo:** Revisión sistemática de modelos de simulación

**Resumen:**
Revisión crítica de los modelos de simulación publicados para el cribado poblacional de cáncer de mama. Se desarrolló un marco de evaluación cualitativa que incluye: tipo de modelo, parámetros de entrada, enfoque de modelado, transparencia, análisis de sensibilidad, sesgo y validación. Se identificaron siete modelos originales de uso múltiple (MISCAN, SPECTRUM, entre otros).

**Hallazgos clave:**
- Todos los modelos pertenecen a la categoría de *individual sampling models*, con transiciones Markovianas o no Markovianas.
- Los modelos tendieron a sobreestimar la reducción de mortalidad (11–24%) frente al 10% estimado en RCTs (IC 95%: −2–21%).
- Ninguno de los siete modelos realizó validación externa; solo validación interna y cruzada.
- Los perjuicios del cribado (falsos positivos, sobrediagnóstico) solo comenzaron a reportarse en publicaciones recientes a la fecha del artículo.

**Relevancia para cellSim:** ⭐⭐ Media  
Justifica la necesidad de validación externa en modelos de simulación y evidencia el riesgo de sesgo cuando los parámetros de entrada no están sistemáticamente sustentados.

---

## Artículo 4 — Bespalov et al. (2021), *BMC Medical Informatics and Decision Making*

**Título:** *Cancer screening simulation models: a state of the art review*  
**DOI:** 10.1186/s12911-021-01713-5  
**Archivo:** `s12911-021-01713-5.pdf`

**Tipo:** Revisión de estado del arte / Análisis metodológico

**Resumen:**
Revisión sistemática de 263 artículos (período 1999–2018) sobre enfoques de simulación para programas de cribado en cáncer de mama, pulmón, colorrectal, próstata y cérvix. Se empleó un checklist propio para evaluar la calidad metodológica.

**Hallazgos clave:**
- Enfoques más comunes: modelos de Markov a nivel individual (34%) y a nivel de cohorte (41%).
- Tipos de cáncer más modelados: mama (25%) y colorrectal (24%).
- La mayoría de estudios se realizaron en poblaciones de América del Norte (42%) y Europa (39%).
- La calidad de los estudios aumentó progresivamente a lo largo del período analizado.
- Tendencias futuras: modelos individuales de Markov complementados con datos de ensayos de cribado, mayor validación y apertura de datos.

**Relevancia para cellSim:** ⭐⭐ Media  
Sitúa cellSim en el contexto del estado del arte metodológico. El modelo individual de Markov —análogo al enfoque celular de cellSim— es la tendencia dominante y más prometedora.

---

## Artículo 5 — Baeyens-Fernández et al. (2018), *BMC Cancer*

**Título:** *Trends in incidence, mortality and survival in women with breast cancer from 1985 to 2012 in Granada, Spain*  
**DOI:** 10.1186/s12885-018-4682-1  
**Archivo:** `s12885-018-4682-1.pdf`

**Tipo:** Estudio epidemiológico poblacional

**Resumen:**
Estudio de tendencias basado en datos del Registro de Cáncer de Granada (España) para 8.502 nuevos casos de cáncer de mama entre 1985 y 2012. Se analizaron incidencia, mortalidad y supervivencia mediante regresión Joinpoint y estimación de supervivencia neta.

**Hallazgos clave:**
- Incidencia ajustada por edad: 48,0 → 83,4 casos/100.000 mujeres (APC = 2,5%; IC 95%: 2,1–2,9).
- Mayor incremento en mujeres < 40 años (APC = 3,5%).
- Tras el cribado, solo aumentó la incidencia de tumores en estadio I, sin reducción de estadios avanzados.
- Mortalidad ajustada por edad disminuyó (APC = −1,0%), especialmente en mujeres de 50–69 años.
- Supervivencia neta a 5 años: 67,5% (1985–1989) → 83,7% (2010–2012).
- Supervivencia por estadio: I = 96,6%, II = 88,2%, III = 62,5%, IV = 23,3%.

**Relevancia para cellSim:** ⭐⭐ Media  
Proporciona datos epidemiológicos reales de una población española útiles para calibrar parámetros de incidencia y progresión tumoral en cellSim. Los datos por estadio son especialmente valiosos para mapear los estadios del modelo celular con tasas clínicas observadas.

---

## Artículo 6 — Chen et al. (2024), *The Lancet Regional Health – Western Pacific*

**Título:** *Cost-effectiveness of population-based screening for chronic obstructive pulmonary disease in China: a simulation modeling study*  
**DOI:** 10.1016/j.lanwpc.2024.101065  
**Archivo:** `1-s2.0-S2666606524000592-main.pdf`

**Tipo:** Microsimulación / Análisis de coste-efectividad

**Resumen:**
Modelo de microsimulación que simula la incidencia, historia natural y manejo clínico de la EPOC (*Enfermedad Pulmonar Obstructiva Crónica*) a lo largo del horizonte temporal de vida en la población china de 35–80 años. Se evaluaron políticas de cribado con diferentes métodos y frecuencias, calculando la razón de coste-efectividad incremental (ICER).

**Hallazgos clave:**
- Todas las políticas de cribado evaluadas fueron coste-efectivas (ICER: 8.034–13.209 USD/QALY; umbral: 38.441 USD/QALY).
- El cribado bianual en dos pasos (cuestionario + espirómetro portátil) fue la estrategia más coste-efectiva.
- El cribado podría evitar entre el 0,39% y el 8,10% de las muertes relacionadas con la EPOC.
- Mejorar la vinculación cribado → diagnóstico/tratamiento aumenta significativamente la coste-efectividad.

**Relevancia para cellSim:** ⭐ Metodológica  
Aunque el dominio de aplicación es EPOC, es metodológicamente relevante como referencia de microsimulación de historia natural con múltiples estados y análisis de sensibilidad aplicables al diseño de estudios de coste-efectividad basados en cellSim.

---

## Síntesis Comparativa

| Artículo | Enfoque | Tipo de modelo | Patología | Relevancia cellSim |
|---|---|---|---|---|
| Paterson et al. (2020) | Modelado estocástico mutaciones driver | Red de genotipos | CRC | ⭐⭐⭐ Alta |
| Sridharan & Ghosh (2025) | ABM + expresión génica | Agentes individuales | Mama | ⭐⭐⭐ Alta |
| Koleva-Kolarova et al. (2015) | Revisión sistemática simulación | Individual Markov / no-Markov | Mama | ⭐⭐ Media |
| Bespalov et al. (2021) | Estado del arte cribado | Markov individual y cohorte | Múltiple | ⭐⭐ Media |
| Baeyens-Fernández et al. (2018) | Epidemiología poblacional | Registro de cáncer | Mama | ⭐⭐ Media |
| Chen et al. (2024) | Microsimulación / coste-efectividad | Historia natural | EPOC | ⭐ Metodológica |

---

## Conclusiones de la Revisión

La literatura revisada converge en señalar que los modelos de simulación individual —en particular los modelos basados en agentes y los modelos de Markov a nivel individual— representan el estado del arte para la simulación de la progresión tumoral y la evaluación de programas de cribado. El artículo de Paterson et al. es especialmente relevante por validar el enfoque de modelado de redes de genotipos con mutaciones en *TP53*, *APC* y *KRAS*, directamente comparables con los genes *TP53*, *BRCA1*, *D1* y *D2* de cellSim. La integración de ABM con datos de expresión génica (Sridharan & Ghosh) y la disponibilidad de datos epidemiológicos reales de poblaciones españolas (Baeyens-Fernández et al.) ofrecen tanto fundamento metodológico como datos de calibración para el desarrollo de cellSim como herramienta de simulación validable.

---

## Pendiente / Próximos pasos

- [ ] Ampliar con más referencias si se añaden PDFs a `docs/paper2/refs/`
- [ ] Redactar sección de Introducción / Estado del Arte para el paper2 usando esta revisión como base
- [ ] Mapear datos de supervivencia por estadio (Baeyens-Fernández) a los 5 estadios de cellSim
- [ ] Evaluar si el modelo de Chen et al. (EPOC) puede usarse como plantilla metodológica para análisis de coste-efectividad
