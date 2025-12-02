# cellSim

[![CI](https://github.com/lpalomerol/cellSim/actions/workflows/ci.yml/badge.svg)](https://github.com/lpalomerol/cellSim/actions)

Simulador de evolución celular y neoplasia. Permite simular múltiples células a lo largo de los años, aplicando reglas de mutación genética y aparición de neoplasias.

## Estructura del proyecto

- `src/`
  - `application/`
    - `simulation/Simulations.h` y `.cpp`: lógica de simulaciones y configuración global.
  - `domain/`
    - `cell/`: lógica de células, fábrica de células (`CellFactory`, `AgenticCell`).
    - `gene/`: lógica de genes (`Gene`), `Genome` y mutaciones.
    - `adapters/`: adaptadores como los generadores de ruido (`RandomNoise`, `FixedNoise`).
- `app/`: puntos de entrada (`main.cpp`, `main_interactive.cpp`).
- `tests/`: tests unitarios (GoogleTest).
- `CMakeLists.txt`, `Makefile`: configuración de compilación.

## Resumen de cambios recientes

Este README se actualiza para reflejar las siguientes modificaciones en el código:

- `main_interactive.cpp` ahora permite elegir entre una fuente de ruido aleatoria (`RandomNoise`) o fija (`FixedNoise`) mediante un booleano (`use_random_noise`).
- Los thresholds de mutación por defecto y la inestabilidad pueden configurarse por gen al crear el `Genome` (ahora `makeDefaultGenome` acepta mapas de thresholds e inestabilidad).
  - Ejemplo usado en `main_interactive`: `BRCA1 = 0.1`, `TP53 = 0.15`, e inestabilidad `k = 0.1` si se desea.
- Se añadió `Genome::mutate(name)` para forzar la mutación de un gen por nombre.
- Se añadió `Genome::isUnstable()` que devuelve true si `TP53` está en estado `+/-` o `-/-`.
- La inestabilidad (componente `k`) ahora se aplica a la mutación de genes sólo cuando `Genome::isUnstable()` es true (es decir, la condición de inestabilidad depende del estado de `TP53`).
- En `AgenticCell`:
  - Nueva función `develop_neoplasm()` que decide si la célula adquiere estado neoplásico muestreando `noise->next().u01` y comparando con `neoplasm_k`.
  - `live()` fue refactorizada para: 1) avanzar genes, 2) comprobar si la célula sigue viva, 3) si TP53 no protege, llamar a `develop_neoplasm()`; si TP53 protege se deja constancia en traza.
  - Se añadieron trazas (`std::cout`) para registrar si la célula estaba protegida, si muestreó y no se volvió neoplásica, o si sí se volvió neoplásica.
- Menú interactivo en `InteractiveSimulation` / `main_interactive`:
  - Opciones: `1` -> mutar `BRCA1`; `2` -> mutar `TP53`; `q` -> salir; otra tecla / Enter -> no mutar.
  - La opción elegida se aplica a todas las células antes de ejecutar el tick (cada año).

## Reglas y comportamiento (detallado)

- Genes: cada `Gene` tiene un threshold base (`mutation_threshold`) y un componente de inestabilidad (`mutation_instability_k`).
- Mutación de genes: en cada tick `Genome::liveAllGenes()` llama a `Gene::live(apply_instability)` pasando `apply_instability = Genome::isUnstable()`. Si `apply_instability` es true se añade `mutation_instability_k` al threshold al muestrear.
- Desarrollo de neoplasia: en `AgenticCell::develop_neoplasm()` se muestrea `u01` desde la fuente de ruido inyectada. Si `u01 < neoplasm_k`, la célula se marca como neoplásica. Si `TP53` protege (TP53 `+/+`), no se muestrea y se registra la protección.

## Ejemplo de configuración (interactivo)

En `app/main_interactive.cpp` se inicializa el genoma así (ejemplo):

```cpp
std::unordered_map<std::string, double> gene_thresholds{{"BRCA1", 0.1}, {"TP53", 0.15}};
std::unordered_map<std::string, double> gene_instability_k{{"BRCA1", 0.1}, {"TP53", 0.1}};
domain::Genome genome = domain::genome_factory::makeDefaultGenome(gene_thresholds, gene_instability_k);
```

Y la selección de ruido se hace por un booleano:

```cpp
bool use_random_noise = true; // true = RandomNoise(seed), false = FixedNoise({0.0})
```

## Cómo compilar y ejecutar

Configurar y compilar (ejemplo con build en `cmake-build-debug`):

```bash
cmake -S . -B cmake-build-debug -DBUILD_TESTS=ON
cmake --build cmake-build-debug --target interactive -j 2
```

Ejecutar el modo interactivo:

```bash
./cmake-build-debug/interactive
```

Durante la ejecución interactiva verás el menú en cada tick; las trazas indicarán si la célula estuvo protegida por TP53, si se realizó el muestreo para neoplasia y el resultado del muestreo.

## Tests

- Los tests unitarios están en `tests/`. Se añadieron tests para `Genome::mutate` (mutar gen existente y mutar gen no existente sin efectos).
- Para compilar y ejecutar los tests (requiere BUILD_TESTS=ON):

```bash
cmake -S . -B cmake-build-debug -DBUILD_TESTS=ON
cmake --build cmake-build-debug --target unit_tests -j 2
./cmake-build-debug/unit_tests --gtest_color=no
```

## Cómo empezar desde cero

Sigue estos pasos para clonar, compilar y ejecutar la simulación desde cero en un entorno Linux con CMake y un compilador C++20:

1. Clonar el repositorio y situarse en la raíz del proyecto:

```bash
git clone <tu-repo-url> cellSim
cd cellSim
```

2. Crear un directorio de build y configurar con CMake (se habilitan los tests):

```bash
cmake -S . -B cmake-build-debug -DBUILD_TESTS=ON
```

3. Compilar el ejecutable interactivo:

```bash
cmake --build cmake-build-debug --target interactive -j 2
```

4. Ejecutar la simulación interactiva:

```bash
./cmake-build-debug/interactive
```

5. (Opcional) Ejecutar los tests unitarios:

```bash
cmake --build cmake-build-debug --target unit_tests -j 2
./cmake-build-debug/unit_tests --gtest_color=no
```

---

Para más detalles revisa los archivos fuente en `src/` y los tests en `tests/`.
