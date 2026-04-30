# Copilot Instructions — cellSim

C++20 cellular evolution simulator. Clean architecture, GoogleTest, CMake.

## Build & Test

```bash
# Build (with tests)
make rebuild                        # cmake -S . -B build -DBUILD_TESTS=ON + cmake --build build

# Run all tests
cd build && ctest --output-on-failure
# or
./build/tests/unit_tests

# Run a single test
./build/tests/unit_tests --gtest_filter=AgenticCellTest.SomeName

# Build without tests (faster)
cmake -S . -B build && cmake --build build
```

## Architecture

Three layers with strict dependency direction (no upward deps):

```
src/application/   ← Orchestration, use cases, CLI, SimulationConfig
src/domain/        ← Pure business logic (Cell, Tissue, Genome, Gene, Signals)
src/shared/        ← Cross-cutting (exceptions, utilities)
```

Key subdirectories under `src/domain/`:
- `cell/` — `AgenticCell` + `CellLifeStage` + strategies
- `tissue/` — `Tissue` (container + lifecycle manager)
- `gene/` — `Gene`, `Genome`, `GenomeFactory`
- `ports/` — `ICell`, `INoiseSource`, `ILogger` interfaces
- `adapters/` — `RandomNoise`, `FixedNoise`, `Logger`, `NullLogger`
- `signal/` — `BaseSignal`, `NeoplasmSignal`, `CellDivisionSignal`

**Full ADR documentation**: `.copilot-instructions.md` (root) and `docs/adr/`

## Cell Lifecycle (ADR-0002)

`AgenticCell::live()` executes 5 ordered phases every tick:
1. **Check alive** — baseline checks
2. **Intrinsic apoptosis** — TP53-based; throws `CellDeathException`
3. **Extrinsic apoptosis** — D2-based (immune system); throws `CellDeathException`
4. **Neoplasm development** — TP53 `-/-` + D1 > 2.0 → PRIMER stage
5. **Cytoplasm remodeling / division** — emits `CellDivisionSignal` when dividing

`Tissue::live()` wraps each cell call in try-catch: catches `CellDeathException` (remove cell) and `CellDivisionSignal` (add daughter).

### CellLifeStage (6 states)
`DEAD` → `BASELINE` → `UNSTABLE` → `UNPROTECTED` → `PRIMER` → `TUMORAL`

Computed on-the-fly from gene states + D1 + D2 accumulators.

### Key accumulators
- **D1** (DNA damage): threshold `2.0` triggers PRIMER
- **D2** (Immunosuppression): threshold `5.0` → TUMORAL / immune evasion

## Genetic Model (ADR-0003)

Genes have three states: `+/+` (wild-type) → `+/-` (heterozygous) → `-/-` (homozygous mutant). Mutation is **unidirectional**.

- **TP53**: default `+/+`; controls intrinsic apoptosis
- **BRCA1**: default **`+/-`** ⚠️ (never `+/+` — natively heterozygous); affects D1 accumulation

## Key Conventions

- **`unique_ptr<T>`** for all owned objects; raw pointers only in getters
- **`ICell` interface always** — never cast to `AgenticCell` directly
- **Exception-based death** — `CellDeathException` is the normal flow for cell death
- **Business logic in Domain** — `application/` only orchestrates
- **Logging via `ILoggerPtr`** injected dependency; `NullLogger` in tests
- **Naming**: `PascalCase` classes, `camelCase` variables, `snake_case` namespaces (`domain::`, `application::`)
- **Test files**: `*Test.cpp` in `tests/`; use `FakeNoise`/`TestNoise` helpers from `tests/`
- **Before architectural changes**: consult `docs/adr/README.md` and check for relevant ADR

## Executables

| Binary | Entry point | Purpose |
|--------|-------------|---------|
| `cellSim` | `app/main.cpp` | Batch simulation (100 runs) |
| `cellSim_cli` | `app/main_cli.cpp` | JSON-configured simulation |
| `interactive` | `app/main_interactive.cpp` | Manual mutation control |
| `single_cell_evolution` | `app/single_cell_evolution.cpp` | Single-cell detailed trace |
| `run_all_scenarios` | `app/run_all_scenarios.cpp` | 14-scenario validation suite |

`cellSim_cli` accepts `--config <json>` plus overrides (`--seed`, `--cells`, `--max-t`, `--verbose`). Config templates in `configs/`.
