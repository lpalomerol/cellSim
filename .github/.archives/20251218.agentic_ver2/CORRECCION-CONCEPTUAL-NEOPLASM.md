# Corrección Conceptual: Flujo PRIMER → TUMORAL

## Flujo Correcto (Confirmado)

**1. Célula muta TP53 -/-**
   - Entra estado **UNPROTECTED** (sin protección p53)

**2. D1 sube (> 2.0)** en Phase 4
   - Transita a **PRIMER** (pretumoral, visible para tejido)
   - **Solo se DETECTA** en Phase 4, no se desarrolla

**3. Ciclo siguiente: Recibe ApoptosisSignal en Phase 2**
   - D2 > 5.0 → **SOBREVIVE** + **DESARROLLA NEOPLASM** → **TUMORAL**
   - D2 ≤ 5.0 → **MUERE**

---

## Fases: Responsabilidades Claras

### Phase 4 (Cytoplasmic Remodeling)

```cpp
1. Actualiza D1 y D2 (contadores de inestabilidad)
2. Detecta PRIMER: TP53 -/- && D1 > 2.0
3. LOG: "Cell DETECTED in PRIMER state"
4. Espera: Tejido enviará ApoptosisSignal próximo ciclo

// NO desarrolla neoplasm aquí
```

### Phase 2 (Endocytosis) - CICLO SIGUIENTE

```cpp
// Si en PRIMER + recibe ApoptosisSignal:
if (D2 > 5.0) {
    // SOBREVIVE apoptosis
    develop_neoplasm();      // <-- NEOPLASM AQUÍ
    is_neoplastic_ = true;
    // Estado: TUMORAL
} else {
    // NO sobrevive
    throw CellDeathException();
    // Estado: DEAD
}
```

---

## Timeline: Ciclo por Ciclo

```
CICLO N: Detección PRIMER
  Phase 1-3: Operaciones normales
  Phase 4:
    ├─ D1 = min(D1² + delta_d1, 999)
    ├─ D2 = min(D2² + delta_d2, 999)
    ├─ Detecta: TP53 -/-, D1 > 2.0 → PRIMER
    └─ LOG: "Cell DETECTED in PRIMER state"
    
  Estado: PRIMER (porque D1 > 2.0)
  Neoplasm: NO (es_neoplastic_ = false)
  Vivo: SÍ (awaiting tissue response)

CICLO N+1: Decisión Apoptosis
  Phase 1-2:
    ├─ Recibe ApoptosisSignal (tejido intenta matar)
    └─ Si D2 > 5.0:
         ├─ develop_neoplasm()
         ├─ is_neoplastic_ = true
         └─ Estado: TUMORAL ✓
       Si D2 ≤ 5.0:
         └─ throw CellDeathException() → DEAD

CICLO N+2+: Comportamiento Neoplástico (si sobrevivió)
  ├─ Ignora apoptosis (es inmortal)
  ├─ Emite NeoplasmSignal
  └─ Puede dividirse
```

---

## Conceptos Clave

| Concepto | Qué Es | Cuándo |
|----------|--------|--------|
| **TP53 -/-** | Mutación genómica | Fase 3 (mutaciones acumulan) |
| **UNPROTECTED** | Estado sin protección p53 | Cuando TP53 -/- |
| **D1** | Contador de DNA damage | Crece cada ciclo en Fase 4 |
| **D2** | Contador de immune evasion | Crece cada ciclo en Fase 4 |
| **PRIMER** | Pretumoral detectado | Cuando UNPROTECTED + D1 > 2.0 |
| **ApoptosisSignal** | Intento del tejido | Fase 2 del ciclo siguiente |
| **TUMORAL** | Transformación completada | Si sobrevive apoptosis (D2 > 5.0) |

---

## Cambios Implementados

✅ **Phase 4**: Detecta PRIMER (D1>2.0 + TP53 -/-), SIN desarrollar neoplasm
✅ **Phase 2**: Si sobrevive apoptosis (D2>5.0) → develop_neoplasm() → TUMORAL
✅ **Compilación**: Exitosa sin errores
✅ **Concepto**: TP53 -/- es la causa, D1/D2 son contadores, apoptosis es el gatillo

---

## ¿Tiene sentido ahora?

✅ **SÍ:**
- Célula muta TP53 → UNPROTECTED
- D1 alto → PRIMER (visible)
- Apoptosis + D2 decide → TUMORAL o DEAD
- D1/D2 son síntomas, no causas
- Neoplasm desarrolla cuando sobrevive apoptosis

Próximo: **SUB-PASO 3.5** (Phases 0,1,3,5 + helpers)



