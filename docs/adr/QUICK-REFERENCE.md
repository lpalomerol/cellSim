# 📋 ADR Quick Reference - cellSim

## Para Copilot: Decisiones Críticas a Considerar

### ✅ ADR-0001: Arquitectura de 3 Capas
**Aplicar siempre que**: Crees archivos nuevos o refactorices

```
src/
├── application/    ← Use cases, orquestación
├── domain/         ← Lógica de negocio (Cell, Tissue, Genome)
└── shared/         ← Utilidades, excepciones, logger
```

**Regla de oro**: Lógica de negocio NUNCA en Application. Application orquesta Domain.

---

### ✅ ADR-0002: Agentic Cell Pattern
**Aplicar siempre que**: Modifiques `AgenticCell_v2` o crees nuevas células

#### Estado multidimensional
```cpp
// D1: DNA damage (acumula con mutaciones)
double d1_dna_damage_;

// D2: Immunosuppression (resiste apoptosis)
double d2_immunosuppression_;
```

#### Ciclo de 5 fases en `live()`
```
Phase 0: Check alive?
Phase 1: Intrinsic apoptosis (TP53-based)
Phase 2: Extrinsic apoptosis (D2-based)
Phase 3: Neoplasm transformation
Phase 4: Cytoplasm remodeling + division
```

#### 6 Etapas de vida
```
DEAD       → !alive()
BASELINE   → TP53 +/+ & BRCA1 +/- (normal, protegida por TP53)
UNSTABLE   → TP53 +/- & BRCA1 +/- (débil, primera instabilidad)
UNPROTECTED→ TP53 -/- (vulnerable, sin protección)
PRIMER     → UNPROTECTED + D1 > 2.0 (pre-tumoral)
TUMORAL    → is_neoplastic_ == true (transformada)

⚠️ NOTA: BRCA1 nativo es SIEMPRE +/-, nunca +/+ (es supresor tumoral)
```

**Regla de oro**: No cambies la lógica de fases sin actualizar esta ADR.

---

---

### ⏳ ADR-0003: Manejo de Mutaciones
**Aplicar cuando**: Modificas genes, probabilidades de mutación o lógica de Genome

**Conceptos clave**:
- 3 estados de gene: `+/+` (wild-type), `+/-` (heterocigoto), `-/-` (mutante)
- Mutaciones son probabilísticas (dependen de ruido aleatorio)
- Gene.live() → verifica si muta
- Genome.liveAllGenes() → hace vivir todos sus genes

**Genes importantes**:
- `TP53`: Guardián del genoma (controla apoptosis intrínseca)
- `BRCA1`: Reparación de ADN (afecta tasa mutación de TP53)

---

### ⏳ ADR-0004: Sistema de Apoptosis
**Aplicar cuando**: Modificas probabilidades de muerte o lógica de apoptosis

**2 fases de apoptosis**:
- Phase 1 (Intrínseca): Controlada por TP53
  - TP53 +/+: 0% apoptosis (protegida)
  - TP53 +/-: ~1% apoptosis (débil)
  - TP53 -/-: ~10% apoptosis (vulnerable)
  
- Phase 2 (Extrínseca): Controlada por D2
  - D2 < 5.0: Susceptible (~5%)
  - D2 >= 5.0: Resistente (0%)

**Implementación**: Lanzan `CellDeathException` capturada por Tissue

---

### ⏳ ADR-0005: Big Bang Mode
**Aplicar cuando**: Modificas tasas de división o comportamiento de neoplasias

**Parámetros**:
- `enable_big_bang_mode`: Flag global (true/false)
- `normal_division_rate`: Tasa célula normal (ej: 0.001 = 0.1%)
- `neoplastic_division_rate`: Tasa neoplasia (ej: 0.01 = 1%)

**Lógica en Phase 4**:
```cpp
if (is_neoplastic_ && enable_big_bang_mode_) {
    usar neoplastic_division_rate  // 10x más rápido
} else {
    usar division_rate  // Tasa normal
}
```

---

### ⏳ ADR-0006: Tissue Manager
**Aplicar cuando**: Trabajas con colección de células, manejo de excepciones

**Responsabilidades de TissueV2**:
- Contenedor de `vector<unique_ptr<ICell>>`
- Ejecutar `live()` en cada célula
- Capturar `CellDeathException` → remover célula
- Capturar `CellDivisionSignal` → agregar hija
- Asignar IDs únicos a cada célula

**Interfaz pública**:
```cpp
void live();                              // Main loop
void addCell(unique_ptr<ICell> cell);   // Agregar
std::size_t size() const;                // Contar
ICell* getCell(size_t idx);             // Acceder
vector<ICell*> getLiveCells();          // Filtrar vivas
vector<ICell*> getCellsByStage(stage);  // Filtrar por stage
```

## Preguntas para hacerse

| Pregunta | Consulta |
|----------|----------|
| ¿Dónde va este código? | ADR-0001 (layers) |
| ¿Cómo decide una célula si divide? | ADR-0002 (agentic phases) |
| ¿Cómo afecta una mutación? | ADR-0003 (cuando se cree) |
| ¿Es correcto este patrón? | Busca en `/docs/adr/` |

---

## Para el Copilot: Instrucciones

Cuando edites code de cellSim:

1. ✅ **Siempre**: Revisa si aplica una ADR existente
2. ✅ **Siempre**: Si la ADR dice algo diferente a tu intención, sigue la ADR
3. ✅ **Siempre**: Si propones cambio arquitectónico, menciona cuál ADR afecta
4. ❌ **Nunca**: Ignores una ADR si crees que es "ineficiente" (fue decidida por razones)

---

## Cómo crear una nueva ADR

```
File: docs/adr/000X-nombre-corto.md

# Título descriptivo
**Status**: Propuesta | Aceptada | Deprecada | Reemplazada
**Date**: YYYY-MM-DD

## Context: ¿Por qué?
## Decision: ¿Qué decidimos?
## Consequences: ¿Implicaciones?
## Alternatives: ¿Qué más consideramos?
## Related ADRs: Enlaces a otras
```

---

**Última actualización**: 2025-12-17  
**Para**: GitHub Copilot y otros LLMs

