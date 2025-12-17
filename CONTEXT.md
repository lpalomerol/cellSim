# Context Engineering - cellSim Project

## 🎯 Propósito del Proyecto

Simulación de evolución celular con capacidades agentic, mutaciones genéticas y neoplasias.

## 📋 Decisiones Arquitectónicas Importantes

**⚠️ SIEMPRE consultar `/docs/adr/` antes de hacer cambios**

### ADR-0001: Arquitectura de Capas ✅
- **Application Layer**: Use cases, controllers, orquestación
- **Domain Layer**: Lógica de negocio pura (Cell, Tissue, Genome)
- **Shared Layer**: Utilidades, excepciones, interfaces

### ADR-0002: Agentic Cell Pattern ✅
- Las células son agentes autónomos con D1 (DNA damage) + D2 (Immunosuppression)
- 5 fases: Check → Intrinsic AP → Extrinsic AP → Neoplasm → Remodeling
- 6 CellLifeStages: DEAD, BASELINE, UNSTABLE, UNPROTECTED, PRIMER, TUMORAL
- Ver: `AgenticCell_v2.h`, `CellLifeStage.h`

### ADR-0003: Manejo de Mutaciones ✅
- Genes con 3 estados: `+/+`, `+/-`, `-/-`
- Mutaciones probabilísticas (dependen de ruido)
- **TP53**: Guardián del genoma, default `+/+` (wild-type)
- **BRCA1**: Reparación de ADN, default `+/-` (⚠️ NATIVO NUNCA ES `+/+`, es supresor tumoral)
- Ver: `Gene.h`, `Genome.h`, `GenomeFactory.h`

### ADR-0004: Sistema de Apoptosis ✅
- **Phase 1 (Intrínseca)**: TP53-based, rara en TP53 +/+, común en TP53 -/-
- **Phase 2 (Extrínseca)**: D2-based, visible si D2 < 5.0
- Ambas lanzan `CellDeathException` capturada por Tissue
- Ver: `AgenticCell_v2.h` (Phases 1-2)

### ADR-0005: Big Bang Mode ✅
- División acelerada de neoplasias
- `enable_big_bang_mode`: Flag global (true/false)
- `neoplastic_division_rate`: Tasa de división (10x normal)
- Ver: `AgenticCell_v2.h` (Phase 4)

### ADR-0006: Tissue Manager ✅
- Contenedor de células: `vector<unique_ptr<ICell>>`
- Ciclo: Ejecuta `live()` en cada célula, captura excepciones/signals
- Asigna IDs únicos, maneja divisiones
- Ver: `TissueV2.h`

## 📁 Estructura de Carpetas

```
docs/
├── adr/                 # Architecture Decision Records
├── diagrams.md
└── README.md

src/
├── application/
│   ├── scenarios/       # Casos de uso
│   └── simulation/
├── domain/
│   ├── cell/           # Lógica de célula
│   ├── genome/         # Genes y mutaciones
│   ├── tissue/         # Contenedor de células
│   ├── exception/      # Excepciones de dominio
│   └── types/          # Tipos enumerados
└── shared/
    ├── utils/
    └── logger/

tests/
└── [Test files por componente]

traces/
└── [Archivos de ejecución]
```

## 🔧 Tecnología

- **Lenguaje**: C++17
- **Build**: CMake
- **Testing**: Google Test
- **Logging**: Sistema de logs propio

## ⚡ Reglas de Código

1. Usar `unique_ptr` para gestión de memoria
2. Evitar raw pointers excepto en getters
3. Usar excepciones para flujo anómalo (ej: cell death)
4. Mantener lógica de negocio en Domain, no en Application
5. Usar enums para estados bien definidos

## 📚 Documentación Relacionada

- `GUIDE-agenticCellRefactor.md`: Guía del refactor
- `TLDR-agenticCellRefactor.md`: Resumen rápido
- `RESUMEN-PASO*.md`: Notas de progreso
- `docs/diagrams.md`: Diagramas de arquitectura

## ⚙️ Próximas Tareas

Ver: `CHECKLIST-agenticCellRefactor.md`

---

**Última actualización**: 2025-12-17
**Maintainer**: Equipo de cellSim

