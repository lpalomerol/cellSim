## 1. Cabecera y resumen ejecutivo

- [ ] 1.1 Escribir resumen ejecutivo de 5 líneas (modelo, calibración, hallazgo)
- [ ] 1.2 Añadir metadatos: fecha, autores, referencia a Paper 3 y Paper 6 como fuentes

## 2. Descripción del modelo

- [ ] 2.1 Describir arquitectura CellSim: 500 células, 6 estadios de vida, ciclo anual
- [ ] 2.2 Describir genes BRCA1 (inicial +/-) y TP53 (inicial +/+), mutaciones unidireccionales
- [ ] 2.3 Describir D1 (daño genómico) y D2 (inmunosupresión): qué son, de qué dependen, umbrales
- [ ] 2.4 Describir mecanismo Big Bang: división neoplásica acelerada (ν)
- [ ] 2.5 Incluir tabla de parámetros con símbolos, rangos, fijo/libre, justificación breve

## 3. Calibración bootstrap (grid search)

- [ ] 3.1 Describir datos clínicos de Kuchenbaecker (tabla con edades, CDF, IC 95%)
- [ ] 3.2 Explicar grid search: 3 parámetros (β, δ_low, δ_high), ν fijo, SSE como métrica
- [ ] 3.3 Presentar hallazgo: sin Big Bang el modelo no puede ajustar (trade-off onset vs plateau)
- [ ] 3.4 Mostrar resultados con Big Bang: SSE=48.5, parámetros óptimos (β=0.045, δ_low=0.120)

## 4. Refinamiento ABC-SMC

- [ ] 4.1 Explicar por qué ABC es necesario (verosimilitud intratable) en 2-3 frases
- [ ] 4.2 Describir el protocolo: 4 parámetros libres (β, δ_low, δ_high, ν), N=200, ε adaptativo
- [ ] 4.3 Incluir tabla de convergencia Gen 0–4 (ε, dist media, ESS/N)
- [ ] 4.4 Reportar posteriores: media, std, IC95% para β, δ_low, δ_high, ν
- [ ] 4.5 Señalar que δ_high es "sloppy" (no identificable)

## 5. Correlación y fenotipos

- [ ] 5.1 Reportar r(δ_low, ν) = −0.609 con p-valor
- [ ] 5.2 Incluir argumento de los cuatro cuadrantes (Exceso/Defecto/Fenotipo A/B)
- [ ] 5.3 Explicar por qué la correlación es estructural (no artefactual)
- [ ] 5.4 Identificar dos fenotipos por k-means: tabla A vs B (δ_low, ν, n)
- [ ] 5.5 Reportar sat50 como discriminador (56.5 vs 59.3, p<0.001) vs onset (p=0.25)
- [ ] 5.6 Referenciar figuras de docs/paper6/ en las secciones correspondientes

## 6. Interpretación biológica y limitaciones

- [ ] 6.1 Describir Fenotipo A (tejido naive, expansión explosiva)
- [ ] 6.2 Describir Fenotipo B (tejido entrenado, crecimiento lento)
- [ ] 6.3 Incluir analogía de la costa (gestión activa vs acumulación silenciosa)
- [ ] 6.4 Mencionar marco evolutivo: antagonismo pleiotrópico (1-2 frases)
- [ ] 6.5 Enumerar limitaciones: δ compartidos, ratio D2/D1 fijo, multiplicador 2×, sin heterogeneidad clonal
- [ ] 6.6 Referencias: Kuchenbaecker 2017, Toni et al 2009, Williams 1957

## 7. Revisión final

- [ ] 7.1 Verificar que todos los requisitos del spec están cubiertos
- [ ] 7.2 Verificar coherencia entre secciones (parámetros, definiciones)
- [ ] 7.3 Verificar que las figuras referenciadas existen en docs/paper6/
- [ ] 7.4 Revisión por al menos un miembro de cada perfil (biólogo, matemático, informático)
