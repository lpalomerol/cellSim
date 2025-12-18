# Resumen Visual: Flujo Completo PRIMER → TUMORAL/DEAD

## Timeline de la Decisión Crítica

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         CICLO EN ESTADO PRIMER                              │
└─────────────────────────────────────────────────────────────────────────────┘

CICLO t: Condiciones de entrada a PRIMER
  ├─ TP53 = -/- (sin protección P53)
  ├─ D1 > 2.0 (DNA damage suficiente)
  └─ !is_neoplastic_ (aún no transformada)

  Estado actual: UNPROTECTED (TP53 -/-)
  Detectado por: getCurrentCellLifeStage() retorna UNPROTECTED

  ┌─────────────────────────────────────────────────────────────────┐
  │  FASE 2: ENDOCYTOSIS (Decisión de apoptosis)                   │
  │                                                                 │
  │  Tejido envía: ApoptosisSignal (intenta matar célula)         │
  │                                                                 │
  │  Célula recibe en phase2_Endocytosis():                        │
  │    ├─ Si D2 > 5.0:                                            │
  │    │  └─ RESISTE apoptosis                                    │
  │    │     logger: "Apoptosis blocked: D2=X > 5.0"             │
  │    │     Continúa a Fase 3-4                                 │
  │    │                                                          │
  │    └─ Si D2 ≤ 5.0:                                           │
  │       └─ ACEPTA apoptosis                                    │
  │          logger: "Apoptosis accepted: D2=X <= 5.0"           │
  │          throw CellDeathException("apoptosis@phase2")        │
  │          → DEAD ✗                                            │
  └─────────────────────────────────────────────────────────────────┘

CICLO t+1 (Si sobrevivió apoptosis, D2 > 5.0):

  ┌─────────────────────────────────────────────────────────────────┐
  │  FASE 3: NUCLEAR DYNAMICS (Mutaciones)                         │
  │                                                                 │
  │  - Genes mutan normalmente                                     │
  │  - D1 y D2 NO se actualizan en esta fase                      │
  └─────────────────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────────────────┐
  │  FASE 4: CYTOPLASMIC REMODELING (Transformación a TUMORAL)    │
  │                                                                 │
  │  1. Calcula deltas según BRCA1 × TP53                         │
  │  2. D1_new = min(D1_old² + delta_d1, 999)                    │
  │     logger: "d1_update: X → Y"                               │
  │  3. D2_new = min(D2_old² + delta_d2, 999)                    │
  │     logger: "d2_update: X → Y"                               │
  │  4. Detecta estado:                                           │
  │     if (TP53 -/- && D1 > 2.0):                              │
  │       logger: "Cell entered PRIMER state"                    │
  │  5. Desarrolla neoplasm:                                      │
  │     if (!isNeoplasticProtected()):  // TP53 -/-             │
  │       develop_neoplasm()                                     │
  │       is_neoplastic_ = true                                 │
  │       logger: "Cell transformed to neoplastic"              │
  │                                                              │
  │  Estado actual: TUMORAL ✓                                   │
  └─────────────────────────────────────────────────────────────────┘

CICLO t+2 (Ahora es TUMORAL):

  ┌─────────────────────────────────────────────────────────────────┐
  │  COMPORTAMIENTO NEOPLÁSTICO                                    │
  │                                                                 │
  │  - Phase 2: Recibe ApoptosisSignal pero ignora (immortal)    │
  │  - Phase 4: Divide si random < neoplastic_division_rate      │
  │  - Phase 5: Emite NeoplasmSignal continuamente               │
  │  - Estado: TUMORAL (determinado por is_neoplastic_)          │
  │                                                              │
  │  Resultado: Célula tumoral activa, proliferativa ✓          │
  └─────────────────────────────────────────────────────────────────┘
```

---

## Matriz de Decisión: 4 Escenarios

```
┌────────────────────────────────────────────────────────────────────────┐
│                       ESCENARIO 1: D2 BAJO (≤ 5.0)                     │
├────────────────────────────────────────────────────────────────────────┤
│                                                                        │
│  Ciclo t:   UNPROTECTED (TP53 -/-, D1 > 2.0)                         │
│  Ciclo t+1: Fase 2 recibe ApoptosisSignal                            │
│             D2 = 3.0 (BAJO)                                          │
│             → ACEPTA apoptosis                                       │
│             → throw CellDeathException                               │
│                                                                       │
│  RESULTADO: DEAD ✗                                                  │
│  Razón: Inmunosupresión insuficiente para escapar                   │
│                                                                        │
└────────────────────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────────────────────┐
│                       ESCENARIO 2: D2 ALTO (> 5.0)                     │
├────────────────────────────────────────────────────────────────────────┤
│                                                                        │
│  Ciclo t:   UNPROTECTED (TP53 -/-, D1 > 2.0)                         │
│  Ciclo t+1: Fase 2 recibe ApoptosisSignal                            │
│             D2 = 7.0 (ALTO)                                          │
│             → RESISTE apoptosis                                      │
│             → Continúa a Fase 3-4                                    │
│  Ciclo t+1 (cont): Fase 4 ejecuta develop_neoplasm()                │
│                    is_neoplastic_ = true                             │
│                                                                       │
│  RESULTADO: TUMORAL ✓                                               │
│  Razón: Inmunosupresión permite escapar y transformarse               │
│                                                                        │
└────────────────────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────────────────────┐
│                    ESCENARIO 3: Nunca llega a PRIMER                   │
├────────────────────────────────────────────────────────────────────────┤
│                                                                        │
│  UNPROTECTED + D1 ≤ 2.0                                              │
│  ├─ TP53 -/- (sin protección)                                       │
│  ├─ D1 bajo (aún acumulando mutaciones)                             │
│  └─ Sigue en UNPROTECTED (waiting state)                            │
│                                                                       │
│  RESULTADO: Sigue esperando a que D1 suba > 2.0                    │
│  Comportamiento: Acumula D1 y D2 cada ciclo                         │
│                                                                        │
└────────────────────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────────────────────┐
│            ESCENARIO 4: BRCA KO antes que TP53 KO                     │
├────────────────────────────────────────────────────────────────────────┤
│                                                                        │
│  Si BRCA1 -/- ANTES que TP53 -/-:                                   │
│  └─ Célula muere por apoptosis INTERNA (en Fase 2 normal)          │
│     Razón: BRCA1 -/- = alive() == false                            │
│             No llega a PRIMER nunca                                 │
│                                                                       │
│  RESULTADO: DEAD (antes de PRIMER)                                 │
│                                                                        │
└────────────────────────────────────────────────────────────────────────┘
```

---

## Tabla Resumen: PRIMER en 1 Ciclo

| Factor | Valor | Outcome |
|--------|-------|---------|
| TP53 | -/- | Entra en UNPROTECTED |
| D1 | > 2.0 | Detectado como PRIMER |
| D2 | ≤ 5.0 | **MUERE en Fase 2** |
| D2 | > 5.0 | **SURVIVES → TUMORAL en Fase 4** |

---

## Código Simplificado: La Decisión

```cpp
// Fase 2: Endocytosis
if (apoptosis_signal_received && cell_state == PRIMER) {
    if (d2_immunosuppression_ > 5.0) {
        // VIVE → Continúa a Fase 3-4
        logger_->logCell("Apoptosis blocked: D2=" + d2 + " > 5.0");
    } else {
        // MUERE ahora
        logger_->logCell("Apoptosis accepted: D2=" + d2 + " <= 5.0");
        alive_ = false;
        throw CellDeathException("apoptosis@phase2");
    }
}

// Fase 4: Si sobrevivió...
if (!isNeoplasticProtected()) {  // TP53 -/-
    develop_neoplasm();  // is_neoplastic_ = true → TUMORAL
}
```

---

## Umbrales Críticos (Memorizables)

```
D1_PRIMER_THRESHOLD = 2.0  (entra en PRIMER cuando D1 > 2.0)
D2_APOPTOSIS_THRESHOLD = 5.0  (resiste apoptosis si D2 > 5.0)
```

Si recordás estos 2 umbrales, recordás todo el modelo.

---

## ¿Por qué es importante?

1. **PRIMER es transición obligatoria** para células TP53 -/-
2. **D2 decide el futuro** (DEAD o TUMORAL)
3. **Todo ocurre en 1 ciclo** (Fase 2 → Fase 4)
4. **Apoptosis extrínseca** es el "último guardián" del tejido

Si el tejido puede enviar ApoptosisSignal a tiempo, y D2 es bajo → Célula tumoral se detiene.
Si D2 es alto → Tumor escapa y prolifera.

