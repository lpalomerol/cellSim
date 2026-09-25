## ADDED Requirements

### Requirement: Resumen ejecutivo
El documento SHALL comenzar con un resumen de 5 líneas que describa el modelo, la calibración, y el hallazgo principal (dos fenotipos de crecimiento tumoral).

#### Scenario: El lector obtiene una visión general inmediata
- **WHEN** un miembro del grupo abre el documento por primera vez
- **THEN** el resumen ejecutivo le permite entender el propósito y el hallazgo principal en <30 segundos

### Requirement: Descripción del modelo
El documento SHALL describir el modelo CellSim incluyendo: genes BRCA1/TP53, acumuladores D1/D2, seis estadios de vida, y los cuatro parámetros libres (β, δ_low, δ_high, ν) con tabla de rangos y justificación.

#### Scenario: Un biólogo entiende la biología del modelo
- **WHEN** un biólogo lee la sección del modelo
- **THEN** comprende qué representan D1, D2, los genes, y cómo se relacionan con la progresión tumoral sin necesidad de leer código

#### Scenario: Un informático identifica los parámetros configurables
- **WHEN** un informático lee la sección del modelo
- **THEN** encuentra una tabla con todos los parámetros, sus símbolos, rangos y valores por defecto

### Requirement: Calibración bootstrap (grid search)
El documento SHALL describir la calibración inicial por grid search, incluyendo: el descubrimiento de que Big Bang mode es estructuralmente necesario (SSE 5295 → 2078 → 48.5), y los parámetros óptimos resultantes.

#### Scenario: Un matemático evalúa la necesidad del Big Bang
- **WHEN** un matemático lee la sección de calibración
- **THEN** encuentra el argumento cuantitativo de por qué sin Big Bang el modelo no puede reproducir simultáneamente onset temprano y plateau correcto

### Requirement: Refinamiento ABC-SMC
El documento SHALL describir el refinamiento bayesiano con ABC-SMC, incluyendo: tabla de convergencia Gen 0–4, distribuciones posteriores de los cuatro parámetros, e identificación de δ_high como parámetro "sloppy".

#### Scenario: Un matemático evalúa la calidad del posterior
- **WHEN** un matemático revisa la sección ABC-SMC
- **THEN** encuentra la tabla de convergencia con ESS/N, los valores del posterior (media, std, IC95%), y la interpretación de qué parámetros están bien identificados

### Requirement: Correlación y dos fenotipos
El documento SHALL presentar el hallazgo central: correlación r(δ_low, ν) = −0.609, argumento de los cuatro cuadrantes demostrando que es estructural (no artefactual), identificación de dos fenotipos (A/B) por k-means, y tabla comparativa.

#### Scenario: Un biólogo entiende los dos fenotipos
- **WHEN** un biólogo lee la sección de fenotipos
- **THEN** encuentra una interpretación clara de qué significa cada fenotipo: A (tejido naive, expansión explosiva) vs B (tejido entrenado, crecimiento lento), con la analogía de la costa

#### Scenario: Un matemático valida la correlación
- **WHEN** un matemático revisa la correlación
- **THEN** encuentra el argumento de los cuatro cuadrantes que demuestra que la anticorrelación es impuesta por la forma de la curva Kuchenbaecker, no por el prior ni el kernel SMC

### Requirement: sat50 como discriminador
El documento SHALL reportar que onset_age no difiere entre fenotipos (43.7 vs 43.5, p=0.25) mientras que sat50 sí (56.5 vs 59.3, p<0.001), e interpretar por qué δ_low controla la velocidad de expansión, no el inicio.

#### Scenario: Un clínico entiende la relevancia de sat50
- **WHEN** un clínico o biólogo lee los resultados
- **THEN** comprende que los dos fenotipos tienen el mismo riesgo de iniciar un tumor a una edad dada, pero diferente velocidad de progresión una vez iniciado

### Requirement: Interpretación biológica
El documento SHALL incluir una sección de interpretación biológica que describa los fenotipos A (tejido naive, expansión explosiva) y B (tejido entrenado, crecimiento lento), y mencione brevemente el marco evolutivo (antagonismo pleiotrópico).

#### Scenario: Un lector obtiene una imagen cualitativa de los mecanismos
- **WHEN** un leedor de cualquier perfil llega a la interpretación
- **THEN** encuentra una explicación cualitativa que conecta los parámetros numéricos con comportamientos biológicos distintos

### Requirement: Limitaciones
El documento SHALL enumerar las limitaciones principales: δ_low/δ_high compartidos entre genes, ratio D2/D1 fijo (2.5), multiplicador LOH-BRCA1 fijo (2×), y ausencia de heterogeneidad clonal.

#### Scenario: Un científico evalúa la aplicabilidad del modelo
- **WHEN** un científico lee las limitaciones
- **THEN** entiende qué simplificaciones se asumen y cómo afectan la interpretación de los resultados

### Requirement: Formato y estilo
El documento SHALL estar escrito en español, en markdown, con un máximo de ~5 páginas, usando tablas para datos numéricos, y referenciando figuras existentes en `docs/paper6/`.

#### Scenario: El documento es consultable en repositorio
- **WHEN** un miembro del grupo abre el archivo .md en GitHub o en un editor
- **THEN** el documento se renderiza correctamente con tablas, listas, y referencias a figuras

#### Scenario: El documento es breve
- **WHEN** un lector hojea el documento
- **THEN** encuentra ~5 páginas de contenido sustancial, sin secciones relleno ni contenido fuera de alcance
