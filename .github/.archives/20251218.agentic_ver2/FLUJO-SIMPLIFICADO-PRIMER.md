# Flujo: PRIMER → TUMORAL (N, N+1, N+2)

## Modelo Correcto

- **Ciclo N:** Entra PRIMER (TP53 -/-, D1 > 2.0) - AÚN NO es neoplástica
- **Ciclo N+1:** Recibe apoptosis, D2 decide vida/muerte
  - D2 > 5.0 → SOBREVIVE Y PASA A TUMORAL
  - D2 ≤ 5.0 → MUERE
- **Ciclo N+2:** Ya es TUMORAL (comportamiento neoplástico)

---

## Timeline Exacto

```
CICLO N (Entra PRIMER):
  Fase 4:
    ├─ Actualiza D1 y D2
    ├─ Detecta: TP53 -/- && D1 > 2.0 → PRIMER
    ├─ Estado = PRIMER (visible para tejido)
    └─ **NO desarrolla neoplasm aún** (es_neoplastica_ = false)

CICLO N+1 (Recibe Apoptosis):
  Fase 2:
    ├─ Recibe ApoptosisSignal (tejido intenta matar)
    ├─ Si D2 > 5.0:
    │  ├─ develop_neoplasm() ejecuta
    │  ├─ is_neoplastic_ = true
    │  └─ Estado = TUMORAL ✓ (sobrevive)
    └─ Si D2 ≤ 5.0:
       └─ throw CellDeathException (muere)

CICLO N+2+ (Comportamiento Neoplástico):
  Si sobrevivió en N+1:
    ├─ Estado = TUMORAL (ya es neoplástica)
    ├─ Fase 2: Ignora apoptosis (es inmortal)
    ├─ Fase 5: Emite NeoplasmSignal
    └─ Puede dividirse si random < neoplastic_division_rate
```

---

## Código (Pseudo)

### Phase 4 (Ciclo N):

```cpp
void phase4_CytoplasmicRemodeling() {
    // 1. Actualizar D1 y D2
    d1 = min(d1² + delta_d1, 999);
    d2 = min(d2² + delta_d2, 999);
    
    // 2. Detectar PRIMER (AÚN SIN DESARROLLAR NEOPLASM)
    CellLifeStage stage = getCurrentCellLifeStage();
    
    if (stage == CellLifeStage::PRIMER && !is_neoplastic_) {
        // IMPORTANTE: Solo detectar, NO desarrollar aún
        logger_->logCell("Cell DETECTED in PRIMER state (TP53 -/-, D1 > 2.0)");
        logger_->logCell("Waiting for apoptosis signal in next cycle...");
        // is_neoplastic_ sigue siendo false
        // El tejido ahora enviará ApoptosisSignal
    }
}
```

### Phase 2 (Ciclo N+1):

```cpp
void phase2_Endocytosis() {
    while (!incoming_messages_.empty()) {
        auto signal = pop_message();
        
        if (signal is ApoptosisSignal && getCurrentCellLifeStage() == PRIMER) {
            // Decisión crítica basada en D2
            if (d2 > 5.0) {
                // SOBREVIVE: Desarrolla neoplasm y se vuelve inmortal
                develop_neoplasm();  // is_neoplastic_ = true
                logger_->logCell("Apoptosis BLOCKED: D2=" + d2 + " > 5.0");
                logger_->logCell("Cell TRANSFORMED to TUMORAL (neoplastic)");
            } else {
                // MUERE: Apoptosis exitosa
                logger_->logCell("Apoptosis ACCEPTED: D2=" + d2 + " <= 5.0");
                throw CellDeathException("extrinsic_apoptosis@phase2");
            }
        }
    }
}
```

---

## Resumen del Cambio

| Aspecto | Antes | Ahora |
|--------|-------|-------|
| Ciclos | 3 | 3 (N, N+1, N+2) |
| Ciclo N | Entra PRIMER | Entra PRIMER (sin neoplasm) |
| Ciclo N+1 | Recibe apoptosis | Recibe apoptosis, D2 decide |
| Ciclo N+2 | Comportamiento neoplástico | TUMORAL si sobrevivió |
| Neoplasm development | Ciclo N+2 | Ciclo N+1 (si D2 > 5.0) |
| Apoptosis decision | Ciclo N+1 | Ciclo N+1 |

---

## Orden en PASO 3.4

**Phase 4 (Ciclo N):**
1. Actualizar D1 y D2
2. Detectar PRIMER (TP53 -/-, D1 > 2.0)
3. **NO desarrollar neoplasm** - Solo log "Cell DETECTED in PRIMER state"
4. El tejido enviará ApoptosisSignal en próximo ciclo

**Phase 2 (Ciclo N+1):**
1. Recibir ApoptosisSignal
2. Si D2 > 5.0 → `develop_neoplasm()` → `is_neoplastic_ = true` → TUMORAL
3. Si D2 ≤ 5.0 → `throw CellDeathException`

**Test 5:** PRIMER en Ciclo N - detectado pero AÚN NO neoplástica
```cpp
cell.live();  // Ciclo N: TP53 -/-, D1 > 2.0
ASSERT_EQ(cell.getCurrentCellLifeStage(), CellLifeStage::PRIMER);
ASSERT_FALSE(cell.isNeoplastic());  // AÚN NO es neoplástica
```

**Test 6:** Ciclo N+1: PRIMER + Apoptosis + D2 alto = TUMORAL (sobrevive)
```cpp
// Ciclo N: entró en PRIMER
cell.receiveMessage(apoptosis_signal);  // Ciclo N+1: recibe apoptosis
cell.live();  // Fase 2 recibe signal, D2 > 5.0
ASSERT_TRUE(cell.isNeoplastic());  // Ahora SÍ es neoplástica
ASSERT_TRUE(cell.alive());  // Sobrevive
```

**Test 7:** Ciclo N+1: PRIMER + Apoptosis + D2 bajo = MUERE
```cpp
// Ciclo N: entró en PRIMER
// Cambiar D2 a bajo (< 5.0)
cell.receiveMessage(apoptosis_signal);  // Ciclo N+1: recibe apoptosis
ASSERT_THROWS(cell.live(), CellDeathException);  // Muere
ASSERT_FALSE(cell.alive());
```

---

## Resumen del Cambio

| Aspecto | Antes | Ahora |
|--------|-------|-------|
| Ciclos | 3 | 2 |
| PRIMER | Detectado | Detectado + Transforma |
| Neoplasm development | Ciclo N+2 | Ciclo N |
| Apoptosis decision | Ciclo N+1 | Ciclo N+1 |
| Complejidad | Media | Baja |

---

## ¿Listo para implementar PASO 3.4?

Con este modelo simplificado, la implementación de Phase 4 es más directa:
- Actualiza D1 y D2
- Detecta PRIMER
- Desarrolla neoplasm inmediatamente
- Fin

¿Continuamos?

