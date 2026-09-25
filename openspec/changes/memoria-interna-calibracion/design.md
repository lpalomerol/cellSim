## Context

Disponemos de dos documentos de trabajo con información complementaria sobre la calibración de CellSim:

- **Paper 3** (`docs/paper3/`): Calibración bootstrap con grid search. Establece que Big Bang mode es estructuralmente necesario, encuentra parámetros óptimos β=0.045, δ_low=0.120, δ_high=0.240 con SSE=48.5. Incluye outline de paper IMRaD.
- **Paper 6** (`docs/paper6/`): Refinamiento ABC-SMC. Añade ν como parámetro libre, obtiene distribuciones posteriores, descubre correlación r(δ_low, ν)=−0.609 y dos fenotipos de crecimiento (A/B). Incluye documento de validación para revisión externa.

Ambos describen el mismo modelo base pero con metodologías, profundidad y formato distintos. El grupo necesita un documento único que presente la narrativa completa (modelo → grid → ABC-SMC → fenotipos) en un formato accesible para los tres perfiles.

**Restricciones:**
- Documento interno (no paper de revista)
- Lengua: español
- Público: biólogos, matemáticos, informáticos del grupo
- Extensión: ~4-5 páginas (máxima brevedad)
- Formato: markdown (control de versiones en repo)

## Goals / Non-Goals

**Goals:**
- Sintetizar Paper 3 + Paper 6 en un documento único y coherente
- Presentar el hallazgo central (dos fenotipos, correlación estructural) como núcleo narrativo
- Ser accesible a los tres perfiles sin segregar contenido en apéndices
- Incluir tablas de parámetros, resultados de calibración, y figuras referenciadas
- Mantener trazabilidad con los papers originales

**Non-Goals:**
- No incluir estado del arte ni comparación con otros simuladores
- No incluir descripción detallada de la arquitectura software (Clean Architecture, tests)
- No incluir algoritmo ABC-SMC completo (solo descripción conceptual)
- No incluir hipótesis de inmunoterapia traslacional
- No incluir discusión extensa ni marco evolutivo detallado

## Decisions

| Decisión | Opción elegida | Alternativas consideradas | Razón |
|----------|---------------|--------------------------|-------|
| Formato | Markdown (.md) | LaTeX, Quarto, DOCX | Control de versiones nativo, renderizado automático en GitHub, edición colaborativa, sin barrera de entrada |
| Estructura | 8 secciones planas | IMRaD clásico, artículo de revista | Es una memoria interna, no un paper; estructura plana es más natural para consulta rápida |
| ν (tasa neoplásica) | Libre en ABC-SMC | Fijo como en Paper 3 | Paper 3 fijó ν por simplicidad; Paper 6 demostró que ν libre revela los fenotipos. La progresión metodológica (3 parámetros → 4 parámetros) se documenta como avance |
| Ecuaciones | Solo las esenciales (δ_D1, δ_D2) | Conjunto completo (SMC, SSE, etc.) | Suficiente para informáticos y matemáticos; biólogos necesitan entender qué controla cada parámetro |
| Figuras | Referencia a archivos existentes en docs/paper6/ | Regenerar todas | Evita duplicación; los archivos PNG/PDF ya existen y están versionados |
| Profundidad del algoritmo SMC | Conceptos clave + tabla de convergencia | Algoritmo completo con kernel y pesos | Matemáticos del grupo conocen ABC-SMC; biólogos necesitan entender qué hace, no cómo se implementa |

## Risks / Trade-offs

| Riesgo | Mitigación |
|--------|-----------|
| El documento queda demasiado denso para biólogos | Usar tablas para datos numéricos, texto narrativo para conceptos. Analogía de la costa para los fenotipos |
| La correlación r(δ_low, ν) se malinterpreta como artefacto | Incluir el argumento de los cuatro cuadrantes (sección 5) que demuestra que es estructural |
| Se pierde información del Paper 3 o Paper 6 | Los documentos originales se conservan; la memoria es una síntesis, no un reemplazo |
| El grupo quiere añadir secciones no contempladas | El documento es vivo; el diseño actual es el mínimo viable para discusión |
