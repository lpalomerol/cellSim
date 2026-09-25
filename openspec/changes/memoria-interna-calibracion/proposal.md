## Why

Necesitamos una memoria interna de trabajo que integre los hallazgos de la calibración bootstrap (Paper 3) y el refinamiento ABC-SMC (Paper 6) en un único documento coherente, accesible para biólogos, matemáticos e informáticos del grupo. Actualmente la información está fragmentada en dos documentos con distinto formato, nivel de detalle y audiencia, lo que dificulta la discusión interdisciplinar.

## What Changes

- Creación de un documento unificado en markdown (~5 páginas) que sintetiza Paper 3 + Paper 6
- Eliminación de contenido redundante (ambos papers describen el mismo modelo)
- Priorización del hallazgo central: la correlación r(δ_low, ν) = −0.609 revela dos fenotipos de crecimiento tumoral indistinguibles a nivel poblacional
- Exclusión de secciones propias de un paper formal (estado del arte, comparación con otros simuladores, hipótesis de inmunoterapia, algoritmo SMC detallado)
- Control de versiones en el repositorio para iteración del grupo

## Capabilities

### New Capabilities

- `calibracion-unificada`: Documento interno consolidado que describe el modelo CellSim, la calibración bootstrap, el refinamiento ABC-SMC, y el descubrimiento de dos fenotipos de progresión tumoral con interpretación biológica y evolutiva.

### Modified Capabilities

*(Ninguna — es un documento nuevo, no un cambio sobre capacidades existentes)*

## Impact

- Creación de `docs/memoria-interna-calibracion.md` como documento vivo del grupo
- Los papers originales (`docs/paper3/`, `docs/paper6/`) se conservan como material de referencia y trazabilidad
- Ningún código fuente se modifica
