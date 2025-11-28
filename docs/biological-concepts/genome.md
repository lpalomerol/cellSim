# 🔬 Genomas en cellSim

## Introducción

El **genoma** es la colección completa de genes de una célula. Mientras que un gen es una entidad individual, el genoma es un **sistema integrado** que coordina todos los genes y gestiona la estabilidad genómica global.

---

## ¿Qué es un Genoma?

En cellSim, el genoma es representado por la clase `Genome` y contiene:

```cpp
class Genome {
private:
    std::unordered_map<std::string, Gene> genes_;
    bool verbose_ = false;
    
public:
    // Acceso a genes
    bool hasGene(const std::string& name) const;
    const Gene* getGene(const std::string& name) const;
    
    // Ciclo de vida
    void liveAllGenes(double genomic_instability = 1.0);
    
    // Estabilidad
    bool isUnstable() const;
    
    // Clonación
    Genome clone() const;
};
```

### Características Clave

- ✅ Mapa de genes con nombres únicos
- ✅ Ciclo de vida coordinado
- ✅ Detección de inestabilidad
- ✅ Factor de inestabilidad multiplicativa
- ✅ Clonación profunda

---

## Estructura: Mapa de Genes

### Genoma Típico

```
Genoma
├─► Gene "TP53"
│   ├─ Estado: +/+
│   ├─ Threshold: 0.15
│   └─ Instability K: 0.10
│
├─► Gene "BRCA1"
│   ├─ Estado: +/+
│   ├─ Threshold: 0.10
│   └─ Instability K: 0.05
│
└─► Gene "..."
    └─ ...
```

### Genoma por Defecto en cellSim

```cpp
// Factory: crear genoma por defecto
Genome genome = Genome::makeDefaultGenome();

// Resultado: genoma con TP53 y BRCA1 sanos (+/+)
```

### Acceso a Genes

```cpp
Genome genome = Genome::makeDefaultGenome();

// Verificar existencia
if (genome.hasGene("TP53")) {
    // TP53 existe
}

// Obtener gen específico
const Gene* tp53 = genome.getGene("TP53");

// Acceder a todos los genes
const auto& genes_map = genome.genes();
for (const auto& [name, gene] : genes_map) {
    std::cout << name << ": " << gene.status() << std::endl;
}
```

---

## Ciclo de Vida del Genoma

### Método: `liveAllGenes()`

El genoma ejecuta un ciclo de vida coordinado en cada tick:

```cpp
void Genome::liveAllGenes(double genomic_instability = 1.0) {
    // Aplicar inestabilidad genómica como factor multiplicativo
    for (auto& [name, gene] : genes_) {
        // Calcular threshold modificado
        double modified_threshold = 
            gene.getMutationThreshold() * genomic_instability;
        
        // Evaluar mutación con instabilidad local del gen
        gene.live(isUnstable(), genomic_instability);
    }
}
```

### Parámetro: `genomic_instability`

Factor multiplicativo que modifica todos los thresholds:

```
threshold_modificado = threshold_base × genomic_instability

Ejemplos:
- genomic_instability = 1.0 → Sin cambio (normal)
- genomic_instability = 1.5 → 50% más mutaciones
- genomic_instability = 2.0 → Doble mutabilidad
```

---

## Detección de Instabilidad: `isUnstable()`

### Concepto

Un genoma se considera **inestable** si su gen TP53 **no está en estado +/+**:

```cpp
bool Genome::isUnstable() const {
    const Gene* tp53 = getGene("TP53");
    if (!tp53) return false;
    
    // Inestable si TP53 es +/- o -/-
    return !tp53->enabled();  // enabled() = true solo si +/+
}
```

### Estados y Estabilidad

| Estado de TP53 | Genoma | Instabilidad Aplicada | Permite tumores |
|---|---|---|---|
| +/+ | Estable ✅ | NO (threshold base) | ❌ NO |
| +/- | Inestable ⚠️ | SÍ (threshold + k) | ❌ NO (protección degradada) |
| -/- | Muy Inestable ❌ | SÍ (threshold + k) | ✅ SÍ |

**Nota importante**: Inestabilidad genómica ≠ Permisión de tumores. Un genoma con TP53 +/- es **inestable** (más mutaciones en otros genes) pero **aún protege** contra transformación neoplástica. Solo TP53 -/- permite formación de tumores.

### Implicación Biológica

**TP53 es el "maestro regulador"** del genoma:
- Su presencia (+/+) mantiene orden genómico
- Su ausencia permite cascadas de mutaciones
- En biología real: TP53 es el "guardián del genoma"

---

## Inestabilidad Multiplicativa

### Aplicación en el Ciclo de Vida

```cpp
// En el ciclo de vida (liveAllGenes)
for (auto& [name, gene] : genes_) {
    // Si el genoma es inestable, pasa el factor multiplicativo
    gene.live(isUnstable(), genomic_instability);
}
```

### Ejemplo Concreto

```
Configuración inicial:
  - TP53: threshold = 0.15, k = 0.10 ← Controla inestabilidad
  - BRCA1: threshold = 0.10, k = 0.05 ← Causa apoptosis si -/-

Escenario 1: Genoma estable (TP53 +/+, BRCA1 +/-)
  TP53:
    threshold_eff = 0.15 × 1.5 = 0.225
  BRCA1:
    threshold_eff = 0.10 × 1.5 = 0.15

Escenario 2: Genoma inestable (TP53 +/-, BRCA1 +/-)
  TP53:
    threshold_eff = (0.15 + 0.10) × 1.5 = 0.375 (!!!)
  BRCA1:
    threshold_eff = (0.10 + 0.05) × 1.5 = 0.225 (!!!)
    
  Pero si BRCA1 muta a -/-:
    BRCA1 -/- = APOPTOSIS (célula muere)
    ─────────────────────────────────
```

**Observación**: La inestabilidad se amplifica multiplicativamente.

---

## Clonación: `clone()`

### Concepto

Crear una copia profunda del genoma (todos los genes se clonan):

```cpp
Genome original = Genome::makeDefaultGenome();
Genome clon = original.clone();

// Ahora clon es idéntico a original, pero independiente
// Cambios en uno no afectan el otro
```

### Uso en Simulación

Cuando una célula se divide, su genoma se clona:

```
Célula Madre
  ├─ Genoma original
  │
  ├─► División
  │
  ├─ Célula Hija 1
  │   └─ Genoma clonado (copia profunda)
  │
  └─ Célula Hija 2
      └─ Genoma clonado (copia profunda)
```

---

## Visualización: Ciclo de Vida del Genoma

```
┌────────────────────────────────────────────────┐
│        CICLO DE VIDA: liveAllGenes()           │
└────────────────────────────────────────────────┘

ENTRADA:
  - genomic_instability (factor multiplicativo)

PASO 1: Detectar inestabilidad
  │
  ├─► TP53 == +/+ ?
  │   ├─ SÍ: genoma_inestable = false
  │   └─ NO: genoma_inestable = true
  │

PASO 2: Para cada gen en el genoma
  │
  ├─► Calcular threshold modificado
  │   threshold_eff = 
  │     threshold_base × genomic_instability +
  │     (instability_k si genoma_inestable)
  │
  ├─► Ejecutar gene.live(genoma_inestable, genomic_instability)
  │   │
  │   ├─► Muestrear random()
  │   ├─► Comparar con threshold_eff
  │   ├─► Aplicar mutación si procede
  │   └─► Registrar (si verbose)
  │

PASO 3: Repetir para siguiente gen
  │

SALIDA:
  - Todos los genes actualizados
  - Genoma listo para siguiente tick
```

---

## Código: Ejecución del Genoma

```cpp
// Ejemplo completo de ciclo de vida
Genome genome = Genome::makeDefaultGenome();
genome.setNoiseSourceForAll(&noise_source);
genome.setVerbose(true);

// Simular 10 años
for (int year = 0; year < 10; ++year) {
    std::cout << "Year " << year << ":\n";
    
    // Ciclogenómico normal
    genome.liveAllGenes(1.0);
    
    // Mostrar estado actual
    genome.details();
}
```

### Salida Esperada (verbose)

```
Year 0:
  TP53[+/+]
  BRCA1[+/-]  ← Estado inicial (células de mama luminal)

Year 1:
  TP53[+/+]
  BRCA1[+/-]

Year 2:
  TP53[+/-]   <-- TP53 mutó! Inestabilidad comienza
  BRCA1[+/-]

Year 3:
  TP53[+/-]
  BRCA1[-/-]  <-- BRCA1 mutó a -/- → CÉLULA MUERE (apoptosis)

Year 4: (no hay, célula murió en Year 3)
```

---

## Métodos Útiles

### Detección de Genes

```cpp
// ¿Existe el gen TP53?
if (genome.hasGene("TP53")) {
    const Gene* tp53 = genome.getGene("TP53");
    std::cout << "TP53 status: " << tp53->status() << std::endl;
}
```

### Información Detallada

```cpp
// Imprimir todos los genes y sus estados
genome.details();
// Output:
// TP53[+/+] BRCA1[+/-] ...
```

### Mutación Forzada

```cpp
// Forzar mutación de TP53
genome.mutate("TP53");  // TP53 +/+ → +/-

// Intentar mutar gen inexistente (sin efecto)
genome.mutate("UNKNOWN");
```

### Verbosidad

```cpp
// Activar trazas detalladas
genome.setVerbose(true);

// Los siguientes ciclos mostrarán detalles de mutaciones
genome.liveAllGenes();
```

---

## Recapitulación

| Concepto | Explicación |
|----------|-------------|
| **Genoma** | Colección de genes de una célula |
| **Mapa** | Acceso por nombre a genes individuales |
| **Ciclo de vida** | `liveAllGenes()` ejecuta mutaciones coordinadas |
| **Inestabilidad** | `isUnstable()` determina si TP53 está dañado |
| **Factor multiplicativo** | `genomic_instability` amplifica mutabilidad |
| **Clonación** | Copias profundas para división celular |

---

## Diagrama: Estado Genómico

```
        Genoma Sano
        (TP53 +/+)
            │
            │ Mutación TP53
            ▼
        Genoma Inestable
        (TP53 +/- o -/-)
            │
            ├─────────────────────────┐
            │                         │
            │ Threshold base          │ + Inestabilidad K
            │ (todos los genes)       │ (todos los genes)
            │                         │
            ▼                         ▼
        Tasa de mutación AUMENTADA
        (cascada de mutaciones adicionales)
```

---

## Siguiente Paso

👉 **Lee [Células](./cells.md)** para entender cómo el genoma se integra en una célula viva que puede mutar, reproducirse y morir.

---

## Referencias Cruzadas

- 🔗 [Genes](./genes.md) - Unidades individuales
- 🔗 [Células](./cells.md) - Dónde viven los genomas
- 🔗 [Neoplasias](./neoplasm.md) - Cascadas genómicas hacia cáncer

