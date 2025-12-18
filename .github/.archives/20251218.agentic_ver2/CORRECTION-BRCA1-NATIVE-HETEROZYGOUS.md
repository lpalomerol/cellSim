# ⚠️ CORRECCIÓN BIOLÓGICA IMPORTANTE: BRCA1 Nativo

**Fecha**: 2025-12-17  
**Impacto**: CRÍTICO para la exactitud biológica del modelo

## El Error

Inicialmente, las ADRs y documentación mencionaban que BRCA1 en células normales podía ser:
- `+/+` (wild-type, ambas copias funcionales)
- `+/-` (heterocigoto, una funcional, una no)
- `-/-` (ambas mutantes)

**PERO ESTO ES INCORRECTO BIOLÓGICAMENTE**

## La Corrección

**BRCA1 en células normales es SIEMPRE `+/-` (heterocigoto nativo), NUNCA `+/+`**

### Por qué:

1. **BRCA1 es un supresor tumoral**: Su función es reparar daño de ADN
2. **Dosis génica**: En humanos, tener una copia de BRCA1 es suficiente para función normal
3. **Equilibrio evolutivo**: Las células no necesitan 2 copias funcionales
4. **El estado wild-type +/+ no existe en células reales**: Es un artefacto del modelo

### Implicación en cellSim:

```
Célula Normal (BASELINE):
  - TP53: +/+ (ambas copias, ambas funcionales)
  - BRCA1: +/- (una copia, es suficiente) ← SIEMPRE
  
Célula Inestable (UNSTABLE):
  - TP53: +/- (una copia afectada)
  - BRCA1: +/- (nativa)
  
Célula Desprotegida (UNPROTECTED):
  - TP53: -/- (ambas copias perdidas) ← Vulnerable
  - BRCA1: +/- o -/- (puede haber mutado más)
```

## Verificación en Código

**Buena noticia**: La implementación en C++ YA ERA CORRECTA ✅

### GenomeFactory.cpp (línea 44)
```cpp
Gene brca1(GENE_BRCA1, Gene::State::PlusMinus, ...);
// PlusMinus = +/- (heterocigoto)  ✅ CORRECTO
```

### GenomeFactory.h (comentarios)
```cpp
// Gene states:
//   TP53: initialized as +/+ (wild-type, protected)
//   BRCA1: initialized as +/- (heterozygous)  ✅ Ya tenía comentario correcto
```

**Lo que se corrigió**: La documentación (ADRs) para que sea explícito y consistente con el código.

## Para el Copilot / LLMs

**REGLA DE ORO**:
```
BRCA1 nativo en células reales = +/- (heterocigoto)
NUNCA = +/+ (wild-type)
NUNCA = -/- (excepto mutaciones somáticas)
```

Si ves código o documentación que asume BRCA1 +/+ como default:
❌ **NO ES CORRECTO BIOLÓGICAMENTE**
✅ **Debe ser +/-**

## Impacto en Simulación

### Sin esta corrección:
- Modelo menos realista
- Tasas de mutación TP53 podrían ser erradas
- Progresión a neoplasia no refleja biología real

### Con esta corrección:
- ✅ Más exacto biológicamente
- ✅ BRCA1 +/+ es solo resultado de mutación POSITIVA (raro)
- ✅ Modelo más realista

## Referencias Biológicas

- BRCA1 es un supresor tumoral: un alelo es suficiente
- En humanos, la mayoría de células tiene BRCA1 +/-
- Pérdida de heterocigosidad (LOH) de BRCA1 lleva a cáncer
- La mutación Ashkenazi de BRCA1 es común en poblaciones específicas

---

**Archivado en**: `CORRECTION-BRCA1-NATIVE-HETEROZYGOUS.md`  
**Aplicable desde**: 2025-12-17  
**Criticidad**: 🔴 ALTA - Afecta exactitud biológica

