# 🔄 Cell State Machine - Máquina de Estados Celular

**Basado en:** Documentación de `docs/diagrams_luis/`  
**Última actualización:** Diciembre 2025  
**Propósito:** Guía visual y conceptual del ciclo de vida celular en cellSim

---

## 📊 Resumen Ejecutivo

La célula en cellSim no transiciona de **estable → tumoral de forma inmediata**. En su lugar, pasa por **5 estadios diferenciados** controlados por sus **4 atributos genéticos internos**:

- **BRCA1** (DNA repair capacity)
- **TP53** (Apoptosis protector)
- **D1** (Genomic instability level 1)
- **D2** (Genomic instability level 2)

---

## 🧬 Atributos Genéticos Internos

Estos atributos **evolucionan independientemente** del estado del tejido, afectados solo por:
- Edad de la célula
- Mutaciones acumuladas
- Inestabilidad presente

### Estados de cada gen

```
Cada gen puede estar en uno de 3 estados:
+/+ → +/- → -/-
(wild-type → heterozygous → homozygous recessive)

Transición: UNIDIRECCIONAL (nunca retrocede)
```

---

## 🎭 Los 5 Estadios Celulares

```
┌─────────────────────────────────────────────────────────────┐
│                    CICLO VITAL CELULAR                      │
└─────────────────────────────────────────────────────────────┘

     BASELINE
        ▲
        │ (normal growth conditions)
        │
        ▼
   
   ESTADIO 1 (Intermedio temprano)
   └─ D1 bajo, D2 bajo
   └─ Célula sigue acumulando mutaciones gradualmente
   
        ▼
   
   ESTADIO 2 (Intermedio avanzado)
   └─ D1 bajo→moderado, D2 bajo→moderado
   └─ Empieza a mostrar inestabilidad detectada
   
        ▼
   
   ESTADO PRIMER (Pre-tumoral)
   ┌─────────────────────────────────┐
   │ Solo si: TP53 = -/- (sin P53)   │
   │ Y: D1 ALTO                       │
   │                                 │
   │ En este estado:                 │
   │ - Tejido intenta matar célula   │
   │ - Si D2 BAJO → Muere (apoptosis)│
   │ - Si D2 ALTO → Puede sobrevivir │
   └─────────────────────────────────┘
   
        ▼ (si D2 ALTO)
   
   TUMORAL
   └─ La célula se transforma
   └─ Gana capacidad proliferativa
   └─ Evade mecanismos normales
```

---

## 🔀 Matriz de Decisión: ¿Qué pasa con cada célula?

### Caso 1: Pierde BRCA antes que TP53
```
BRCA1: -/-  (perdido primero)
TP53:  +/+  (intacto)

Resultado: ✗ MUERE por apoptosis interna
Razón: P53 funciona correctamente, detecta el daño masivo y activa muerte programada
```

### Caso 2: Pierde TP53 antes que BRCA1
```
TP53:  -/-  (perdido primero)
BRCA1: +/+  (intacto)

Resultado: Entra en fase INTERMEDIA DE ACUMULACIÓN
Comportamiento según D1 y D2:
```

#### Subcase 2a: POCO D1 Y POCO D2
```
Estado: BASELINE / ESTADIO 1
└─ Célula sigue acumulando mutaciones según su perfil genético (TP53-/-, BRCA1+/+)
└─ No hay señal de peligro inmediato
└─ Continúa evolucionando
```

#### Subcase 2b: MUCHO D1 Y POCO D2
```
Estado: PRIMER (Pre-tumoral)
└─ D1 alto dispara transición a pre-tumoral
└─ D2 bajo significa: Baja resistencia a apoptosis
└─ Resultado: Tejido intenta matar → MUERE (apoptosis extrínseca)
```

#### Subcase 2c: POCO D1 Y MUCHO D2
```
Estado: BASELINE / ESTADIO 1-2
└─ D1 bajo: No hay activación de pre-tumoral
└─ D2 alto: Acumula resistencia pasivamente
└─ Resultado: Sigue evolucionando, esperando que D1 suba
```

#### Subcase 2d: MUCHO D1 Y MUCHO D2
```
Estado: PRIMER → TUMORAL
└─ D1 alto: Transita a PRIMER (pre-tumoral)
└─ D2 alto: Resiste intento de apoptosis extrínseca
└─ Resultado: ✓ SOBREVIVE → Transita a TUMORAL
└─ La célula ahora es neoplástica con capacidad de proliferación
```

---

## ⚡ Notas Implementativas

### 1. Transiciones Consecutivas
```
La cadena pretumoral → primer → tumoral ES CONSECUTIVA:

Si D1 ↑ (muy alto) Y D2 ↑ (muy alto):
  Fase t:     BASELINE
  Fase t+1:   PRIMER (detectable)
  Fase t+2:   TUMORAL (confirmado)
```

### 2. Crecimiento de D1 y D2
```
Actualmente: LINEAL

D1(t+1) = D1(t) + δ1  (si condición de inestabilidad se cumple)
D2(t+1) = D2(t) + δ2  (si condición de inestabilidad se cumple)

Donde:
- δ1, δ2 son constantes según estado mutacional
- Incremento diferente si TP53 -/- vs +/- vs +/+
```

### 3. Independencia Interna vs Externa
```
INTERNO (dentro de célula): D1, D2 evolucionan por edad + mutaciones
EXTERNO (tisular): P53KO = "Sin protección" → detecta PRIMER

La célula "sabe internamente" que está mal,
pero el TEJIDO solo puede verlo cuando transita a PRIMER
```

---

## 🗺️ Diagrama Simplificado de Transiciones

```
                      ┌─────────────────────────────┐
                      │   BRCA -/-  antes que P53   │
                      │        ↓                    │
                      │    APOPTOSIS ✗              │
                      └─────────────────────────────┘

BASELINE ──────────→ INTERMEDIO ──────────→ PRIMER ──────────→ TUMORAL
                         │                    │
                         │                    └─ (si D2 BAJO → APOPTOSIS)
                         │                    └─ (si D2 ALTO → TUMORAL)
                         │
                         └─ (si BRCA -/- primero → APOPTOSIS)

Controladores internos de D1, D2:
  - Edad de célula
  - Estado mutacional (TP53, BRCA1)
  - Inestabilidad acumulada
```

---

## 📝 Referencias

- `docs/diagrams_luis/estado_1.png` - Versión inicial (5 estadios)
- `docs/diagrams_luis/estado_2_base.png` - Versión con duplicidades (BRCA branches)
- `docs/diagrams_luis/estado_2_simplificado.png` - Versión final optimizada
- `docs/diagrams_luis/celula.png` - Comportamiento por atributos (D1, D2)
- `docs/diagrams_luis/resumen.txt` - Notas técnicas del diseño

---

## ✅ Checklist para Copilot

Al trabajar con lógica celular, verifica que:

- [ ] Las mutaciones genéticas son **unidireccionales** (+/+ → +/- → -/-)
- [ ] **D1 y D2** evolucionan independientemente del estado tisular
- [ ] La transición a **PRIMER** solo ocurre si **TP53 -/-** Y **D1 alto**
- [ ] La apoptosis extrínseca en PRIMER solo falla si **D2 alto**
- [ ] Las transiciones consecutivas (BASELINE → PRIMER → TUMORAL) respetan timing
- [ ] Se respeta la **independencia interna vs externa** del estado celular

