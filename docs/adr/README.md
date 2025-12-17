# Architecture Decision Records (ADRs)

Este directorio contiene las decisiones arquitectónicas documentadas del proyecto cellSim.

## Formato

Cada ADR sigue el formato de [MADR](https://adr.github.io/madr/) con las siguientes secciones:

- **Title**: Título breve de la decisión
- **Status**: Propuesta, Aceptada, Deprecada, Reemplazada
- **Context**: ¿Por qué se necesita esta decisión?
- **Decision**: ¿Qué decidimos?
- **Consequences**: ¿Cuáles son las implicaciones?
- **Alternatives**: Alternativas consideradas

## ADRs Actuales

### Status: Aceptadas ✅

- [ADR-0001: Arquitectura de Capas](./0001-arquitectura-capas.md) - Separación en 3 capas (Application, Domain, Shared)
- [ADR-0002: Patrón Agentic Cell](./0002-agentic-cell-pattern.md) - Células como agentes autónomos con D1+D2
- [ADR-0003: Manejo de Mutaciones](./0003-manejo-mutaciones.md) - Sistema de genes (TP53, BRCA1) y mutaciones probabilísticas
- [ADR-0004: Sistema de Apoptosis](./0004-sistema-apoptosis.md) - Apoptosis intrínseca (TP53) y extrínseca (D2)
- [ADR-0005: Big Bang Mode](./0005-big-bang-mode.md) - División acelerada para células neoplásticas
- [ADR-0006: Tissue Manager](./0006-tissue-manager.md) - Contenedor de células con ciclo de vida y gestión de excepciones

### Status: Pendientes 📝

- ADR-0007: Escenarios de Simulación (Baseline, BRCA1, TP53, etc)
- ADR-0008: Logging y Tracing (Sistema de logs distribuido)
- ADR-0009: Paralelización (Thread-safety para múltiples tissues)

## Cómo usar las ADRs

1. **Antes de cambios arquitectónicos**: Revisa las ADRs relevantes
2. **Proponer cambios**: Crea una nueva ADR siguiendo el template
3. **Documentar decisiones**: Especifica Context, Decision, Consequences
4. **Referencias cruzadas**: Vincula con otras ADRs relacionadas

