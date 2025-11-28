# Genes y Mutaciones en cellSim

## Introducción

Los **genes** son las unidades fundamentales de herencia en cellSim. Cada gen puede cambiar de estado a través de **mutaciones**, lo que afecta el comportamiento y la viabilidad de la célula.

---

## Representación y API (resumen)

En el código, un gen se modela con la clase `Gene` que expone, entre otros, los siguientes elementos:

```cpp
class Gene {
public:
    enum class State { PlusPlus, PlusMinus, MinusMinus };
    
    explicit Gene(std::string name, 
                  State initial = State::PlusPlus, 
                  double mutation_threshold = 0.1, 
                  double mutation_instability_k = 0.0, 
                  bool verbose = false);
    
    const std::string& name() const;
    void mutate();
    void live(bool apply_instability);
    bool enabled() const;
};
```

> Nota: la API es genérica; reglas biológicas específicas (por ejemplo, estados iniciales o efectos particulares como apoptosis) se describen en esta documentación y aplican según la configuración y la lógica del modelo.

---

## Máquina de Estados

Cada gen puede estar en uno de tres estados:

- `+/+` (PlusPlus): ambas copias funcionales.
- `+/-` (PlusMinus): una copia funcional, otra no.
- `-/-` (MinusMinus): ambas copias no funcionales (estado absorbente).

Las transiciones sólo avanzan hacia estados más dañados: +/+ → +/- → -/-. Una vez en -/-, el gen permanece allí.

Ejemplo (implementación de la transición):

```cpp
void Gene::mutate() {
    switch (state_) {
        case State::PlusPlus:
            state_ = State::PlusMinus;
            break;
        case State::PlusMinus:
            state_ = State::MinusMinus;
            break;
        case State::MinusMinus:
            // Estado absorbente: permanece -/-
            break;
    }
}
```

---

## Umbrales y Inestabilidad

- `mutation_threshold`: probabilidad base por tick (año) de que ocurra una mutación en el gen.
- `mutation_instability_k`: incremento del umbral cuando el genoma está inestable.

Regla práctica: si `Genome::isUnstable()` es `true`, el umbral efectivo se calcula como `threshold + instability_k`.

---

## Genes Clave: TP53 y BRCA1 (corrección importante sobre BRCA1)

### TP53

- Función: supresor de tumores; detecta daño, induce detención del ciclo y apoptosis cuando procede.
- En el modelo: cuando TP53 deja de estar en `+/+` (p. ej. pasa a `+/-` o `-/-`), el genoma se considera inestable y la probabilidad de mutación de otros genes aumenta.

### BRCA1

IMPORTANTE: por diseño del modelo y por el origen biológico de las células simuladas (células mamarias progenitoras luminales), **BRCA1 nunca se modela como `+/+`** en estas células. Su estado inicial por defecto en este contexto es `+/-`.

- Estado inicial en células luminales progenitoras: `+/-` (heterocigoto).
- Efecto de una mutación adicional:
  - Si BRCA1 pasa a `-/-` (homocigoto recesivo), el modelo aplica apoptosis celular (la célula pierde viabilidad y muere).
  - Por tanto, BRCA1 `-/-` es letal en el modelo y no hay etapa funcional `+/+` para estas células en el flujo normal.

No incluyas en la documentación ni en los ejemplos que BRCA1 pueda aparecer como `+/+` para estas células; eso sería incorrecto respecto al comportamiento previsto.

---

## Ciclo de Vida de una Mutación (resumen)

1. Cada tick, para cada gen se calcula el umbral efectivo (aplica inestabilidad si corresponde).
2. Se muestrea una variable aleatoria u ∈ [0,1).
3. Si u < umbral_efectivo, se llama a `mutate()` y el gen avanza al siguiente estado.
4. Si el gen es BRCA1 y llega a `-/-`, la célula se marca para apoptosis según la lógica del modelo.

---

## Resumen Rápido

| Concepto | Explicación |
|----------|-------------|
| Gen | Unidad de herencia con 3 estados posibles |
| Estados | +/+ (sano), +/- (parcial), -/- (dañado; absorbente) |
| Mutación | Transición irreversible a estado más dañado |
| Threshold | Probabilidad base de mutación por tick |
| Inestabilidad | Factor que aumenta mutabilidad cuando TP53 no es +/+ |
| TP53 | Regulador maestro de estabilidad genómica |
| BRCA1 | En células luminales progenitoras: inicial `+/-`; si `-/-` ⇒ apoptosis |

---

## Siguientes lecturas

- Lee `genome.md` para ver cómo se agrupan genes y cómo se determina la inestabilidad genómica.
- Lee `cells.md` para entender cómo la pérdida de BRCA1 se conecta con la viabilidad celular y apoptosis.

---

## Notas finales

He eliminado cualquier ejemplo que presente BRCA1 como `+/+`. Si en algún otro contexto del repositorio (p. ej. en pruebas unitarias o en otros ficheros de docs) existe un ejemplo que arma BRCA1 como `+/+`, indícalo y lo corrijo también para mantener consistencia.
