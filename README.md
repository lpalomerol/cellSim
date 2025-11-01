# cellSim

Simulador de evolución celular y neoplasia. Permite simular múltiples células a lo largo de los años, aplicando reglas de mutación genética y aparición de neoplasias.

## Estructura del proyecto

- `src/`
  - `application/`
    - `simulation/Simulations.h` y `.cpp`: lógica de simulaciones y configuración global.
  - `domain/`
    - `cell/`: lógica de células, fábrica de células.
    - `gene/`: lógica de genes, genoma y mutaciones.
    - `adapters/`: adaptadores como el generador de ruido aleatorio.
- `app/`: punto de entrada principal.
- `tests/`: tests unitarios (GoogleTest).
- `CMakeLists.txt`, `Makefile`: configuración de compilación.

## Ejemplo de célula y reglas básicas

Cada célula tiene un genoma con genes como TP53 y BRCA1. Cada gen puede estar en uno de estos estados:
- `+/+` (ambos alelos sanos)
- `+/-` (uno mutado)
- `-/-` (ambos mutados)

### Ejemplo de célula
```json
{
  "TP53": "+/+",
  "BRCA1": "+/-"
}
```

### Reglas básicas
- En cada tick (año), cada gen puede mutar según su probabilidad de mutación (definida en `SimulationsConfig`).
- Si el valor aleatorio es menor que el threshold, el gen muta (por ejemplo, si el threshold es 0.1, muta 10% de las veces).
- Si TP53 está inactivo (`-/-`), la célula puede volverse neoplásica con probabilidad `neoplasm_k`.
- Una célula muerta no puede revivir.

### Ejemplo de configuración
```cpp
SimulationsConfig cfg;
cfg.gene_mutation_thresholds["TP53"] = 0.1; // TP53 muta 10% de las veces
cfg.gene_mutation_thresholds["BRCA1"] = 0.05; // BRCA1 muta 5% de las veces
cfg.neoplasm_k = 0.005; // Probabilidad de neoplasia si TP53 está inactivo
```

## Ejecución
Compila con CMake y ejecuta el binario principal o los tests:
```bash
cmake -B build
cmake --build build
./build/cellSim
```

## Tests
Los tests unitarios están en la carpeta `tests/` y cubren mutaciones, reglas de neoplasia y comportamiento de las células.

---
Para más detalles, revisa los archivos fuente y la configuración en `src/application/simulation/Simulations.h`.
