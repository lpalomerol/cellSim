# 🚀 QUICK START: Usar ADRs con Copilot

**Archivo de referencia**: Guarda esto en tus favoritos

## Antes de Pedir Ayuda a Copilot

Menciona una de estas líneas en tu prompt:

```
Opción 1 (Más directo):
"Considera las ADRs en /docs/adr/, especialmente ADR-0001 y ADR-0002"

Opción 2 (Más corto):
"Consulta /docs/adr/QUICK-REFERENCE.md"

Opción 3 (Máximo contexto):
"Por favor sigue las decisiones en CONTEXT.md y .copilot-instructions.md"
```

## Las 3 ADRs Más Críticas

### 🏗️ ADR-0001: 3 Capas
```
Application (orquestación)
        ↓
Domain (lógica de negocio) ← AQUÍ va todo lo importante
        ↓
Shared (utilidades)
```

### 🧬 ADR-0002: Agentic Cell
```
5 Fases en live():
  1. Check alive
  2. Intrinsic apoptosis (TP53)
  3. Extrinsic apoptosis (D2)
  4. Neoplasm decision
  5. Cytoplasm remodeling + division

Estados (CellLifeStage):
  DEAD → BASELINE → UNSTABLE → UNPROTECTED → PRIMER → TUMORAL
```

### 🧬 ADR-0003: Mutaciones
```
IMPORTANTE: BRCA1 nativo es +/- (NO +/+)

TP53 estados: +/+ (normal) → +/- (débil) → -/- (vulnerable)
BRCA1 estados: +/- (nativo) → -/- (raro)
```

## Archivos Clave

| Archivo | Usa cuando... |
|---------|--------------|
| `/docs/adr/QUICK-REFERENCE.md` | Necesitas recordar decisiones rápido |
| `.copilot-instructions.md` | Copilot actúa incorrectamente |
| `CONTEXT.md` | Necesitas contexto general |
| `/docs/adr/000X-*.md` | Necesitas profundidad en un tema |

## Reglas de Oro (Copilot Must Follow)

```
✅ Lógica de negocio → Domain Layer
❌ Lógica de negocio → Application Layer

✅ Excepciones para cell death
❌ Código de error normal

✅ D1 + D2 son estado multidimensional
❌ Un solo "health" o "instability"

✅ BRCA1 nativo = +/-
❌ BRCA1 nativo = +/+

✅ Células toman decisiones en live()
❌ Tissue toma decisiones sobre células
```

## Ejemplo: Pedir Cambio

### Mal (sin contexto):
```
"Haz que las células se dividan más rápido"
```

### Bien (con ADR):
```
"Aumenta neoplastic_division_rate en ADR-0005 (Big Bang Mode)
para que las neoplasias se dividan más rápido.
Asegúrate de seguir ADR-0001 (layers) y ADR-0002 (agentic phases)"
```

## Una Línea Para Recordar

> "Las células son agentes autónomos que progresan por 5 fases impulsadas por D1 (DNA) y D2 (Immunosuppression), en un tissue que maneja excepciones y divisiones"

---

**Sálvalo y úsalo como referencia rápida** 🔖

