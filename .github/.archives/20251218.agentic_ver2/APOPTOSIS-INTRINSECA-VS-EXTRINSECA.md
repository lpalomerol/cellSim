# Apoptosis: Intrínseca vs Extrínseca

## Definiciones

### Apoptosis INTRÍNSECA
**Origen:** Detectada internamente por la célula (daño irreparable)
**Mecanismo:** BRCA1 -/- = Reparación de DNA imposible = Célula muere
**Tipo:** Automática, no requiere señal externa
**TP53 requerido:** Puede ser +/+, +/-, o -/-

### Apoptosis EXTRÍNSECA
**Origen:** Enviada por el tejido (respuesta inmunológica)
**Mecanismo:** Tejido detecta PRIMER + envía ApoptosisSignal
**Tipo:** Requiere D2 ≤ 5.0 para ser efectiva
**TP53 requerido:** Debe ser -/- (célula en UNPROTECTED/PRIMER)

---

## Matriz de Apoptosis: 6 Escenarios

```
┌─────────────────────────────────────────────────────────────────────┐
│ ESCENARIO 1: BRCA1 -/- & TP53 +/+                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│ Genética: BRCA1 KO, TP53 normal (protegido)                       │
│                                                                     │
│ Flujo:                                                             │
│   Fase 1: alive() = false                                         │
│   → throw CellDeathException("dead@phase1")                       │
│                                                                     │
│ Apoptosis: INTRÍNSECA ✓                                           │
│ Razón: BRCA1 -/- = sin reparación de DNA                         │
│        TP53 +/+ también intenta matar pero BRCA ya lo hizo       │
│                                                                     │
│ Estado: DEAD                                                      │
│ Mensaje: "Cell died from intrinsic apoptosis (BRCA1 -/-)"       │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│ ESCENARIO 2: BRCA1 -/- & TP53 +/-                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│ Genética: BRCA1 KO, TP53 heterozygous                             │
│                                                                     │
│ Flujo:                                                             │
│   Fase 1: alive() = false                                         │
│   → throw CellDeathException("dead@phase1")                       │
│                                                                     │
│ Apoptosis: INTRÍNSECA ✓                                           │
│ Razón: BRCA1 -/- mata la célula antes que TP53 pueda             │
│                                                                     │
│ Estado: DEAD                                                      │
│ Mensaje: "Cell died from intrinsic apoptosis (BRCA1 -/-)"       │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│ ESCENARIO 3: BRCA1 -/- & TP53 -/-                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│ Genética: Ambos KO                                                 │
│                                                                     │
│ Flujo:                                                             │
│   Fase 1: alive() = false                                         │
│   → throw CellDeathException("dead@phase1")                       │
│                                                                     │
│ Apoptosis: INTRÍNSECA ✓ (causada por BRCA1, no por TP53)         │
│ Razón: BRCA1 -/- es letal sin importar TP53                      │
│                                                                     │
│ Estado: DEAD                                                      │
│ Mensaje: "Cell died from intrinsic apoptosis (BRCA1 -/-)"       │
│ Nota: Nunca llega a PRIMER porque muere en Fase 1               │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│ ESCENARIO 4: BRCA1 +/- & TP53 +/-                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│ Genética: Ambos heterozygous (UNSTABLE)                           │
│                                                                     │
│ Flujo:                                                             │
│   Estado: UNSTABLE (viva, puede dividirse)                       │
│   Acumula D1 y D2 lentamente                                      │
│   Sigue evolucionando sin llegar a PRIMER                        │
│   (porque TP53 aún es +/-, no -/-)                               │
│                                                                     │
│ Apoptosis: NINGUNA (por ahora)                                   │
│ Protección: TP53 +/- la protege de neoplasia                     │
│                                                                     │
│ Estado: UNSTABLE (indefinidamente, a menos que mute más)        │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│ ESCENARIO 5: BRCA1 +/- & TP53 -/-                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│ Genética: BRCA1 heterozygous, TP53 KO (UNPROTECTED)              │
│                                                                     │
│ Flujo:                                                             │
│   Estado: UNPROTECTED (viva, acumula D1 y D2)                    │
│   Si D1 ≤ 2.0:                                                   │
│     → Sigue en UNPROTECTED (esperando)                           │
│   Si D1 > 2.0:                                                   │
│     → Entra en PRIMER                                            │
│     → Fase 2: Recibe ApoptosisSignal del tejido                 │
│       ├─ Si D2 ≤ 5.0: MUERE (apoptosis extrínseca)             │
│       └─ Si D2 > 5.0: SOBREVIVE → Fase 4 → TUMORAL             │
│                                                                     │
│ Apoptosis: EXTRINSECA (controlada por D2) ✓                     │
│ Razón: Tejido intenta matar en PRIMER basado en D2               │
│                                                                     │
│ Estados: UNPROTECTED → PRIMER → (DEAD o TUMORAL)               │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│ ESCENARIO 6: BRCA1 -/- & TP53 -/-                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│ Genética: Ambos KO                                                 │
│                                                                     │
│ Flujo:                                                             │
│   Fase 1: alive() = false                                         │
│   → throw CellDeathException("dead@phase1")                       │
│                                                                     │
│ Apoptosis: INTRÍNSECA ✓ (BRCA1 -/- mata antes)                  │
│ Razón: BRCA1 -/- es letal SIEMPRE                               │
│        Nunca llega a PRIMER aunque TP53 -/-                       │
│                                                                     │
│ Estado: DEAD                                                      │
│ Mensaje: "Cell died from intrinsic apoptosis (BRCA1 -/-)"       │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Tabla Resumen: Tipo de Apoptosis

| BRCA1 | TP53 | Estado | Apoptosis | Trigger | D2 Factor |
|-------|------|--------|-----------|---------|-----------|
| -/- | +/+ | DEAD | INTRÍNSECA | BRCA1 KO | NO |
| -/- | +/- | DEAD | INTRÍNSECA | BRCA1 KO | NO |
| -/- | -/- | DEAD | INTRÍNSECA | BRCA1 KO | NO |
| +/- | +/- | UNSTABLE | NINGUNA | - | NO |
| +/- | -/- (D1≤2) | UNPROTECTED | NINGUNA | - | NO |
| +/- | -/- (D1>2) | PRIMER | EXTRÍNSECA | Tejido | **SÍ** |

---

## Código: Diferenciación en el Modelo

### Apoptosis INTRÍNSECA (BRCA1 -/-)
```cpp
// En alive():
bool AgenticCell_v2::alive() const {
    const Gene* brca1 = genome_.getGene("BRCA1");
    
    if (!has_evaded_apoptosis_) {
        if (!brca1) return false;
        std::string brca1_status = brca1->status();
        // BRCA1 -/- → alive = false (apoptosis intrínseca)
        return brca1_status != "-/-";
    }
    return true;
}

// En phase1_G1IntegrityCheckpoint():
void phase1_G1IntegrityCheckpoint() const {
    if (!alive()) {
        // Intrinsic apoptosis: BRCA1 -/- detected
        logger_->logCell("[Apoptosis] Cell dead: BRCA1 -/- (intrinsic)");
        throw CellDeathException("intrinsic_apoptosis@phase1");
    }
}
```

### Apoptosis EXTRÍNSECA (D2 ≤ 5.0 en PRIMER)
```cpp
// En phase2_Endocytosis():
void phase2_Endocytosis() {
    while (!incoming_messages_.empty()) {
        auto signal = pop_message();
        
        if (signal is ApoptosisSignal) {
            // Extrinsic apoptosis: tissue sends signal, D2 decides
            if (d2_immunosuppression_ > 5.0) {
                // Resists
                logger_->logCell("[Apoptosis] Extrinsic blocked: D2=" + 
                               d2 + " > 5.0 (immune evasion)");
            } else {
                // Dies
                logger_->logCell("[Apoptosis] Extrinsic accepted: D2=" + 
                               d2 + " <= 5.0 (immune clearance)");
                throw CellDeathException("extrinsic_apoptosis@phase2");
            }
        }
    }
}
```

---

## Timeline: ¿Cuándo ocurre cada tipo?

### INTRÍNSECA (BRCA1 -/-)
```
Cualquier ciclo → Fase 1
Si BRCA1 -/- → alive() = false → CellDeathException
Timing: **Temprano, Fase 1**
```

### EXTRÍNSECA (D2 ≤ 5.0 en PRIMER)
```
Ciclo N: TP53 -/-, D1 > 2.0 → Estado = PRIMER
Ciclo N+1: Fase 2 recibe ApoptosisSignal
Si D2 ≤ 5.0 → CellDeathException
Timing: **Tarde, Fase 2, solo en PRIMER**
```

---

## Implicaciones Biológicas

| Tipo | Mecanismo | Control | Escapatoria |
|------|-----------|---------|-----------|
| **INTRÍNSECA** | Reparación DNA rota | BRCA1 status | NO (muerte automática) |
| **EXTRÍNSECA** | Respuesta inmune | D2 status | SÍ (si D2 > 5.0) |

**Clave:** 
- Apoptosis INTRÍNSECA = garantizada (no escapatoria)
- Apoptosis EXTRÍNSECA = evitable (con D2 alto)

---

## ¿El modelo lo tiene en cuenta?

✅ **SÍ**, el modelo diferencia ambas:
- **BRCA1 -/- → alive() = false** (intrínseca, fase 1)
- **PRIMER + ApoptosisSignal + D2 ≤ 5.0** (extrínseca, fase 2)

❌ **NO está claramente documentado** en el código actual como "intrínseca" vs "extrínseca"

---

## Próximo paso: Documentar en el código

Necesitamos añadir comentarios claros en:
1. `alive()`: "BRCA1 -/- detects intrinsic apoptosis"
2. `phase1_G1IntegrityCheckpoint()`: "Throws if intrinsic apoptosis"
3. `phase2_Endocytosis()`: "Extrinsic apoptosis: tissue signal + D2 decision"

