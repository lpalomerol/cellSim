# ADR-0001: Arquitectura de Capas en cellSim

**Status**: Aceptada  
**Date**: 2025-12-17  
**Decision Makers**: Equipo de desarrollo

## Context

El proyecto cellSim necesita una estructura clara que separe responsabilidades y facilite el testing, mantenimiento y evolución del código.

La simulación de células requiere:
- Lógica de dominio compleja y bien testeable
- Casos de uso variados (scenarios)
- Gestión eficiente de recursos
- Trazabilidad de decisiones

## Decision

Se adopta una **Arquitectura de Capas de 3 niveles**:

```
┌──────────────────────────────┐
│   Application Layer          │ Use cases, scenarios, controllers
├──────────────────────────────┤
│   Domain Layer               │ Lógica de negocio (Cell, Tissue, Genome)
├──────────────────────────────┤
│   Shared Layer               │ Utilidades, logger, excepciones
└──────────────────────────────┘
```

### Responsabilidades

**Application Layer**:
- Orquestación de casos de uso
- Controllers de simulación
- Scenarios (baseline, brca, tp53, etc.)

**Domain Layer**:
- Lógica de células (AgenticCell_v2)
- Genoma y mutaciones
- Tissue (contenedor de células)
- Estados y ciclo de vida
- Excepciones de dominio

**Shared Layer**:
- Logging
- Utilidades generales
- Constantes globales

## Consequences

✅ **Ventajas**:
- Separación clara de responsabilidades
- Fácil de testear unitariamente
- Decisiones arquitectónicas documentadas
- Facilita onboarding de nuevos desarrolladores

⚠️ **Desventajas**:
- Puede haber duplicación de tipos entre capas
- Requiere disciplina en el diseño

## Alternatives Considered

1. **Hexagonal Architecture**: Más compleja para este caso de uso
2. **Monolithic**: Sin separación clara, difícil de testear
3. **Microservicios**: Overkill para una simulación local

## Related ADRs

- ADR-0002: Agentic Cell Pattern
- ADR-0003: Manejo de Mutaciones

