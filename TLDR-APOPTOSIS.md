# TL;DR: Apoptosis Intrínseca vs Extrínseca

## Las 2 Formas de Morir

### 1. APOPTOSIS INTRÍNSECA (I-APOPTOSIS)
```
Trigger:    BRCA1 -/- (cualquier momento)
Quién mata:  La célula misma (sin reparación de DNA)
Cuándo:     Fase 1 (G1 Checkpoint) - SIEMPRE
Escapatoria: NINGUNA (muerte garantizada)
D2 Factor:   NO (no interviene D2)
TP53 Factor: NO (no importa si TP53 es +/+, +/-, o -/-)

Código:
  if (BRCA1 == "-/-") {
    alive() = false;  // → Fase 1 → CellDeathException
  }

Escenarios que disparan:
  - BRCA1 -/- & TP53 +/+ → MUERE (intrínseca)
  - BRCA1 -/- & TP53 +/- → MUERE (intrínseca)
  - BRCA1 -/- & TP53 -/- → MUERE (intrínseca)
```

### 2. APOPTOSIS EXTRÍNSECA (E-APOPTOSIS)
```
Trigger:    PRIMER (TP53 -/-, D1 > 2.0) + ApoptosisSignal
Quién mata:  El tejido (respuesta inmunológica)
Cuándo:     Fase 2 (Endocytosis)
Escapatoria: SÍ (si D2 > 5.0)
D2 Factor:   SÍ (crítico para decidir)
TP53 Factor: SÍ (requiere TP53 -/-)

Código:
  if (ApoptosisSignal && PRIMER && TP53 == "-/-") {
    if (D2 > 5.0) {
      // Resiste → sobrevive
    } else {
      // Acepta → muere
    }
  }

Escenarios que disparan:
  - BRCA1 +/- & TP53 -/- & D1 > 2.0 & D2 ≤ 5.0 → MUERE (extrínseca)
  - BRCA1 +/- & TP53 -/- & D1 > 2.0 & D2 > 5.0 → SOBREVIVE
```

---

## Matriz Rápida: ¿Cuál tipo de apoptosis?

| BRCA1 | TP53 | D1 | D2 | Outcome | Tipo |
|-------|------|----|----|---------|------|
| -/- | +/+ | - | - | DEAD | Intrínseca |
| -/- | +/- | - | - | DEAD | Intrínseca |
| -/- | -/- | - | - | DEAD | Intrínseca |
| +/- | +/+ | - | - | VIVO | Ninguna |
| +/- | +/- | - | - | VIVO | Ninguna |
| +/- | -/- | ≤2 | - | VIVO | Ninguna |
| +/- | -/- | >2 | ≤5 | DEAD | Extrínseca |
| +/- | -/- | >2 | >5 | TUMORAL | Extrínseca (evade) |

---

## Palabras Clave

| Concepto | BRCA1 -/- | PRIMER + D2 ≤5 |
|----------|-----------|----------------|
| **Nombre** | Apoptosis Intrínseca | Apoptosis Extrínseca |
| **Quién decide** | Célula | Tejido |
| **Cuándo** | Fase 1 | Fase 2 |
| **Escapatoria** | NO | SÍ (D2 > 5.0) |
| **Factor clave** | BRCA1 status | D2 level |

---

## En Una Línea

- **Intrínseca:** BRCA1 -/- = muere siempre, Fase 1
- **Extrínseca:** PRIMER + D2 ≤ 5.0 = muere si la detecta el tejido, Fase 2

Si BRCA1 -/-, NUNCA llega a PRIMER (muere en Fase 1).
Si BRCA1 +/- & TP53 -/-, llega a PRIMER y D2 decide.

