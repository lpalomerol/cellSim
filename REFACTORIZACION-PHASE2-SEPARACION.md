# Refactorización: Separación de Concerns en Phase 2

## Problema Original

La lógica estaba **acoplada**:

```cpp
if (PRIMER && D2 > 5.0) {
    develop_neoplasm();
} else if (PRIMER && D2 <= 5.0) {
    throw CellDeathException();
} else if (!PRIMER && D2 > 5.0) {
    // survive
} else {
    // die
}
```

**Issue:** La decisión de NEOPLASM estaba mezclada con APOPTOSIS.

---

## Solución: Separación de Responsabilidades

### PART 1: Apoptosis Extrínseca (Applies to ALL cells)

```cpp
// D2 decide universalmente: muerte o supervivencia
if (D2 > 5.0) {
    // Cell survives (immune evasion)
    log: "Extrinsic BLOCKED"
} else {
    // Cell dies (immune clearance)
    throw CellDeathException();
}
```

### PART 2: Neoplasm Development (Only if PRIMER + survived)

```cpp
// SOLO si cell sobrevivió apoptosis AND está en PRIMER
if (survived_apoptosis && PRIMER && !is_neoplastic_) {
    develop_neoplasm();
    log: "Cell TRANSFORMED to TUMORAL"
}
```

---

## Código Nuevo (Separado)

```cpp
void phase2_Endocytosis() {
    while (!messages.empty()) {
        signal = pop_message();
        
        if (signal is ApoptosisSignal) {
            // ==================
            // PART 1: APOPTOSIS (universal)
            // ==================
            if (D2 > 5.0) {
                log: "Extrinsic BLOCKED: D2=" + D2;
                
                // ==================
                // PART 2: NEOPLASM (conditional)
                // ==================
                if (PRIMER && !is_neoplastic_) {
                    develop_neoplasm();
                    log: "Cell TRANSFORMED to TUMORAL";
                }
            } else {
                // Muere
                throw CellDeathException();
            }
        }
    }
}
```

---

## Flujo Resultante

```
ApoptosisSignal llega en Phase 2:
  ├─ PART 1: Check D2 (universal rule)
  │  ├─ D2 > 5.0? → Survive
  │  └─ D2 ≤ 5.0? → Die (exception)
  │
  └─ PART 2 (solo si survived): Check PRIMER (conditional rule)
     ├─ PRIMER && !neoplastic? → develop_neoplasm()
     └─ Else → nothing (maybe non-PRIMER cell)
```

---

## Ventajas

✅ **Claridad:** Apoptosis y Neoplasm son responsabilidades separadas
✅ **Extensibilidad:** Fácil añadir otras reglas de apoptosis
✅ **Mantenibilidad:** D2 es el único factor en PART 1
✅ **Corrección:** Non-PRIMER cells también pueden resistir apoptosis (por D2)

---

## Cambios Implementados

✅ Refactorizada Phase 2
✅ Lógica separada: PART 1 (apoptosis) y PART 2 (neoplasm)
✅ Compilación exitosa

---

## Beneficio Clave

Ahora la lógica es **composable**:
- Cualquier célula (PRIMER o no) puede sobrevivir apoptosis si D2 > 5.0
- Solo PRIMER + sobrevivida → desarrolla neoplasm
- No hay reglas mágicas acopladas

Próximo: **SUB-PASO 3.5** (Phases 0,1,3,5 + helpers)

