# Contexto para sesiones futuras — cellSim & argumento de relevancia científica

> Creado: 2026-04-29. Actualizar este fichero al final de cada sesión.

---

## 1. Qué es cellSim

**cellSim** es un simulador de evolución celular en C++20 para investigación en oncología. Repositorio: `lpalomerol/cellSim`.

- Arquitectura hexagonal en 3 capas: `application/`, `domain/`, `shared/`
- Build: `make rebuild` → compila tests incluidos
- Tests: `cd build && ctest --output-on-failure` o `./build/tests/unit_tests`
- CI en GitHub Actions con matrix gcc + clang (ubuntu-latest)
- Dependencias: GoogleTest (FetchContent), nlohmann/json, cxxopts

**Modelo biológico core:**

El método `AgenticCell::live()` tiene 5 fases ordenadas:
1. Check alive
2. Apoptosis intrínseca (TP53)
3. Apoptosis extrínseca (D2 / inmune)
4. Desarrollo neoplásico
5. Remodelación citoplasmática / división

6 etapas de vida celular (`CellLifeStage`): `DEAD → BASELINE → UNSTABLE → UNPROTECTED → PRIMER → TUMORAL`

Umbrales clave:
- D1 (daño DNA) ≥ 2.0 → PRIMER
- D2 (inmunoevasión) ≥ 5.0 → TUMORAL
- BRCA1 nativa es SIEMPRE +/- (heterocigota), NUNCA +/+
- Estados génicos: +/+ → +/- → -/- (unidireccional)
- Modo Big Bang: células neoplásicas se dividen a 10x la tasa normal

**Patrones de diseño críticos:**
- Muerte celular por excepción: `CellDeathException`
- División por señal: `CellDivisionSignal`
- Siempre usar interfaz `ICell` — nunca hacer cast a `AgenticCell` directamente
- `unique_ptr` para ownership, raw pointers solo en getters
- Consultar `docs/adr/` (ADRs 0001–0006) antes de cualquier cambio arquitectónico

---

## 2. El proyecto paper / argumento de relevancia

El objetivo es construir el argumento científico para publicar cellSim y justificar sus derivados planeados (Moran, Gillespie, tau-leaping). El contexto es un artículo o propuesta que combine matemática, informática y desarrollo de software para investigación en oncología.

**Los derivados planeados:**
- **Proceso de Moran** — competencia clonal en tejido de tamaño fijo (homeostasis tisular, iniciación tumoral)
- **Gillespie SSA** — dinámica estocástica en tiempo continuo exacto (eventos raros, mutaciones de baja frecuencia)
- **Tau-leaping** — versión escalable de Gillespie para poblaciones grandes (régimen Big Bang)

---

## 3. Corpus de papers — `docs/paper2/refs/software/`

### Papers originales (8 papers)

| Fichero | Referencia | Relevancia para el argumento |
|---------|-----------|------------------------------|
| `s12609-010-0020-6.pdf` | Chauviere et al. (2010) — *Mathematical oncology and breast cancer*, Cell Prolif. | Historia 60 años de oncología matemática; el problema es real y cuantificable |
| `s11538-025-01544-9.pdf` | Pugh et al. (2025) — *Bibliometric study of mathematical oncology*, Bull Math Biol | Bibliometría: el campo crece, pero software de calidad es el cuello de botella |
| `s11538-025-01544-9-1.pdf` | **Duplicado exacto** del anterior — mismo tamaño de fichero | ⚠️ Ignorar |
| `949344.949346.pdf` | Thomas & Barry — *MDD / Domain-Oriented Programming* | Ingeniería del software: DDD/hexagonal es el camino para código científico mantenible |
| `Bioinformaticssoftwaredevelopment_Principlesandfuturedirections.pdf` | Ma et al. (2024) — *Bioinformatics SW development principles* | Principios de calidad de software en bioinformática; la deuda técnica es el problema |
| `biomolecules-15-01409.pdf` | Veneziano et al. (2025) — *Bioinformatics in breast cancer* | Brecha actual: datos abundantes, herramientas integradoras escasas |
| `s41392-024-01888-z.pdf` | Hassan et al. (2024) — *ML + mathematical modeling in cancer* | La combinación ML + modelos mecanísticos es el estado del arte |
| `pharmaceutics-16-00260.pdf` | Su et al. (2024) — *Cell-cell communication* | Comunicación celular: señalización tumor-microambiente, justifica el módulo D2 |

### Papers añadidos (fundacionales)

| Fichero | Referencia | Relevancia |
|---------|-----------|------------|
| `s0305004100033193.pdf` | **Moran (1958)** — *Random processes in genetics*, Math Proc Cambridge Phil Soc 54(1):60–71. DOI: 10.1017/S0305004100033193 ✅ | El proceso de Moran fundacional: población fija N, reemplazos individuales, probabilidad de fijación, 2× más rápido que Wright |
| `j100540a008.pdf` | **Gillespie (1977)** — *J. Phys. Chem.* ✅ | SSA fundacional; argumento filosófico clave: la formulación estocástica tiene base física más sólida que la determinista |
| `Science04.pdf` | **Nowak & Sigmund (2004)** — *Evolutionary Dynamics of Biological Games*, Science 303:793 ✅ + intro de May "Mathematics in Biology" | Dinámica evolutiva dependiente de frecuencia; advierte contra modelos con "detalle excesivo en algunos aspectos y neblina en otros" |
| `EvolutionaryDynamics.pdf` | Nowak (libro) — escaneado, 43 páginas, texto NO extraíble ⚠️ | Solo como referencia bibliográfica; no leer con pdftotext |

---

## 4. Estructura del argumento (5 pilares)

### Pilar 1 — El problema es real y no está resuelto
- Chauviere (2010): 60 años de oncología matemática, mortalidad por cáncer de mama sigue siendo inaceptable
- Veneziano (2025): datos ómicos abundantes, herramientas integradoras insuficientes
- Hassan (2024): el gap entre modelos biológicos y clínica práctica persiste

### Pilar 2 — La simulación estocástica es el camino
- Gillespie (1977): argumento filosófico directo — *"la formulación estocástica tiene una base física más sólida que la determinista"*. Aplicable directamente a justificar simulación estocástica de tumores vs. ecuaciones diferenciales.
- Moran (1958): la aleatoriedad de los reemplazos individuales acelera 2× la pérdida de heterocigosidad respecto al modelo determinista de Wright. En oncología: la selección clonal en tejido regulado es un proceso de Moran.
- Nowak & Sigmund (2004): cuando la fitness es dependiente de frecuencia (lo que ocurre en tumores: competencia por nicho, señalización paracrina), la dinámica requiere modelos evolutivos, no solo cinéticos.

### Pilar 3 — La calidad del software es el eslabón perdido
- Ma et al. (2024): la deuda técnica en software bioinformático es el cuello de botella para la reproducibilidad
- Thomas & Barry (MDD): la arquitectura orientada a dominio permite que matemáticos y biólogos contribuyan al mismo codebase sin accidentes
- Pugh (2025): el crecimiento del campo no está acompañado de mejoras en infraestructura de software

### Pilar 4 — La multidisciplinariedad es el requisito, no el lujo
- Ningún paper del corpus puede leerse sin cruzar matemáticas, biología y software
- cellSim como artefacto: implementa modelos de la literatura (Moran, Gillespie) con arquitectura de software de calidad (ADRs, DDD, CI)
- La combinación no es cosmética — es estructural: sin el modelo matemático, el software no tiene semántica; sin el software, el modelo no es reproducible ni escalable

### Pilar 5 — Los derivados tienen impacto clínico concreto
- **Moran**: cuantifica la probabilidad de que una sola célula TP53 -/- fije en tejido mamario normal. Predice tiempo hasta iniciación tumoral. Conecta con epidemiología de Chauviere.
- **Gillespie**: permite simular la aparición de mutaciones raras (segundo hit de BRCA1) con exactitud matemática. Sin aproximación.
- **Tau-leaping**: escala Gillespie a poblaciones grandes (~10⁶ células), necesario para simular el régimen Big Bang de cellSim.

---

## 5. Cadena de citación recomendada

```
Moran (1958)
    ↓ marco teórico: fijación estocástica en población finita
Gillespie (1977)
    ↓ cómo simular ese proceso en tiempo continuo exacto
Nowak & Sigmund (2004)
    ↓ cuando la fitness es dependiente de frecuencia
cellSim (hoy)
    ↓ implementación con arquitectura robusta (DDD/hexagonal)
Derivados: Moran-cellSim, Gillespie-cellSim, tau-leaping
    ↓ aplicación clínica: iniciación, mutaciones raras, Big Bang
```

---

## 6. Frases citables directamente

**Gillespie (1977):**
> "the stochastic formulation of chemical kinetics has a firmer physical basis than the deterministic formulation"

Uso: justificar por qué cellSim simula estocasticidad en lugar de usar ODEs.

**May / Nowak Science04:**
> Models with "excruciating abundance of detail in some aspects, whilst other important facets of the problem are misty"

Uso: reconocer honestamente que los umbrales D1=2.0 y D2=5.0 son valores razonables no calibrados con datos biológicos reales. Esto es una **debilidad anticipada** que los reviewers señalarán.

**Moran (1958)** (parafraseado):
> En lugar de generaciones simultáneas, los eventos de nacimiento y muerte ocurren de forma individual y aleatoria — introduciendo una fuente adicional de estocasticidad que duplica la tasa de fijación respecto al modelo de Wright.

Uso: justificar el proceso de Moran como el modelo correcto para homeostasis tisular (tejido de tamaño fijo, competencia clonal).

---

## 7. Deuda pendiente / próximas acciones

- [ ] **Calibración D1/D2**: los umbrales 2.0 y 5.0 necesitan justificación biológica o mención explícita como parámetros libres. Los reviewers lo pedirán.
- [ ] **Redactar sección Background / Related Work** usando el corpus completo. La estructura del argumento ya está lista (ver §4).
- [ ] **Verificar si Nowak libro (EvolutionaryDynamics.pdf) es citable** — solo tiene 43 páginas escaneadas, no es el libro completo. Citar la edición estándar: Nowak MA (2006) *Evolutionary Dynamics*, Harvard University Press.
- [ ] **Añadir referencia crítica/escéptica** — el corpus actual es 100% pro-computacional. Un reviewer exigirá al menos una referencia que desafíe los límites de los modelos. Candidatos: Weinberg (2014) sobre simplicidad vs. complejidad en cáncer, o cualquier crítica al modelo matematizado de oncología.
- [ ] **Implementar proceso de Moran** en cellSim como primer derivado.
- [ ] **Revisar `docs/paper2/refs/software/s11538-025-01544-9-1.pdf`** — es duplicado del `s11538-025-01544-9.pdf`, eliminar para no confundir.

---

## 8. Ficheros clave del repositorio

```
cellSim/
├── .github/copilot-instructions.md     ← instrucciones concisas para Copilot (creado esta sesión)
├── .copilot-instructions.md            ← instrucciones detalladas con todos los ADRs (560 líneas, no tocar)
├── .cursorrules                        ← equivalente para Cursor
├── CMakeLists.txt                      ← BUILD_TESTS=OFF por defecto; Makefile lo activa
├── Makefile                            ← punto de entrada: make rebuild, make test
├── src/
│   ├── application/                    ← orquestación, SimulationConfig, CLI, JSON
│   ├── domain/                         ← lógica de negocio pura
│   │   ├── cell/                       ← AgenticCell, ICell, CellLifeStage
│   │   ├── tissue/
│   │   ├── gene/
│   │   ├── ports/ y adapters/
│   │   └── signal/
│   └── shared/
├── tests/
├── docs/
│   ├── adr/                            ← ADRs 0001–0006, leer antes de cambios arquitectónicos
│   └── paper2/
│       ├── contexto_sesiones.md        ← este fichero
│       ├── bibliografia_revision.md
│       └── refs/software/              ← todos los papers del corpus
```

---

*Fin del resumen de sesión. Próxima sesión: leer este fichero primero.*
