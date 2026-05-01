# cellSim

[![CI](https://github.com/lpalomerol/cellSim/actions/workflows/ci.yml/badge.svg)](https://github.com/lpalomerol/cellSim/actions)

A domain-driven, open-source C++20 framework for stochastic tumour evolution simulation.
CellSim models the progressive genomic corruption of a cell population through a biologically
grounded lifecycle (TP53 / BRCA1 two-hit inactivation, DNA damage accumulation, immune evasion)
within a clean hexagonal architecture designed for reproducibility and extensibility.

> **Academic context:** CellSim is a proof-of-concept at the intersection of software engineering
> and computational oncology. See `docs/paper2/intro_related_work.pdf` for the full technical description.

---

## Requirements

- **CMake** ≥ 3.14
- **g++** or **clang++** with C++20 support
- **make**

```bash
# Ubuntu/Debian
sudo apt install cmake g++ make

# Fedora/RHEL
sudo dnf install cmake gcc-c++ make

# macOS
brew install cmake
```

---

## Build

```bash
# Build with tests (recommended)
make rebuild          # cmake -S . -B build -DBUILD_TESTS=ON + cmake --build build

# Build without tests
cmake -S . -B build && cmake --build build
```

### Run tests

```bash
cd build && ctest --output-on-failure
# or directly:
./build/tests/unit_tests

# Single test
./build/tests/unit_tests --gtest_filter=AgenticCellTest.SomeName
```

---

## Executables

| Binary | Description |
|--------|-------------|
| `cellSim` | Batch simulation — 100 runs with default parameters |
| `cellSim_cli` | JSON-configurable simulation (recommended entry point) |
| `interactive` | Manual mutation control, verbose per-year logging |
| `single_cell_evolution` | Single-cell detailed trace → `single_cell_evolution_log.txt` |
| `run_all_scenarios` | 17-scenario validation suite → Markdown + CSV traces |

---

## cellSim_cli — Configurable Entry Point

```bash
./build/cellSim_cli --config configs/default.json
./build/cellSim_cli --config configs/default.json --seed 42 --cells 1000 --max-t 80
./build/cellSim_cli --config configs/high_tp53.json --verbose
```

### CLI options

| Option | Description |
|--------|-------------|
| `--config <path>` | JSON configuration file (required) |
| `--seed <n>` | Random seed; -1 = random |
| `--cells <n>` | Cell population size |
| `--max-t <n>` | Simulation length in ticks (≈ years) |
| `--verbose` | Enable detailed logging |
| `--help` | Show full help |

### JSON structure

```json
{
  "config": {
    "seed": -1,
    "verbose": false,
    "use_random_noise": true
  },
  "simulation_context": {
    "max_t": 80,
    "n_cells": 1000
  },
  "tissue_parameters": {
    "neoplasm_k": 0.01,
    "division_rate": 0.05,
    "neoplastic_division_rate": 0.15,
    "enable_big_bang_mode": false,
    "genes": {
      "TP53":  { "mutation_rate": 0.001, "instability_rate": 0.002 },
      "BRCA1": { "mutation_rate": 0.001, "instability_rate": 0.002 }
    }
  }
}
```

Config templates: `configs/default.json`, `configs/high_tp53.json`, `configs/no_mutations.json`.

---

## Scenario Validation Suite

`run_all_scenarios` executes 17 predefined biological scenarios and writes traces to
`cmake-build-debug/traces/`:

| Group | Scenarios | Description |
|-------|-----------|-------------|
| Baseline controls | 01–09 | No mutations → high TP53 load; establishes behaviour boundaries |
| Big Bang | 10–12 | TP53-/- cells acquire accelerated division; clonal dominance validation |
| Calibrated + lognormal noise | 13–15 | BRCA1 carrier profile (CV 0.3–0.5, N=1000–2000), 10 replicates each |
| Calibrated + constant noise | 16–17 | Same parameters, deterministic increments; isolates stochastic contribution |

```bash
./build/run_all_scenarios
```

---

## Architecture

Three layers with strict inward dependency (no upward deps):

```
src/
├── application/        # Orchestration, CLI, SimulationConfig
├── domain/             # Pure business logic — no external dependencies
│   ├── cell/           # AgenticCell, CellLifeStage
│   ├── gene/           # Gene, Genome, GenomeFactory
│   ├── tissue/         # Tissue (container + lifecycle manager)
│   ├── signal/         # CellDivisionSignal, NeoplasmSignal
│   ├── ports/          # ICell, INoiseSource, ILogger interfaces
│   └── adapters/       # RandomNoise, FixedNoise, Logger, NullLogger
└── shared/             # Cross-cutting utilities, exceptions
```

Architectural decisions are documented as ADRs in `docs/adr/`.

---

## Biological Model (summary)

| Concept | Representation |
|---------|---------------|
| Tumour suppressors | TP53, BRCA1 — three allelic states (+/+, +/-, -/-), unidirectional |
| BRCA1 initial state | +/- (germline carrier) |
| DNA damage | Continuous accumulator D₁; threshold θ_{D₁} → PRIMER stage |
| Immunosuppression | Continuous accumulator D₂; threshold θ_{D₂} → TUMORAL stage |
| Cell stages | BASELINE → UNSTABLE → UNPROTECTED → PRIMER → TUMORAL (+ DEAD) |
| Apoptosis | Intrinsic (TP53-mediated) and extrinsic (immune-mediated) |

Thresholds θ_{D₁} and θ_{D₂} are calibrated against BRCA1 carrier penetrance data.
See `docs/paper2/intro_related_work.pdf` §4.3 for the calibration procedure.

---

## Test Suite

82 unit tests across 11 test files (GoogleTest), running on every commit under GCC and Clang:

```bash
./build/tests/unit_tests
```

Coverage: gene mutation, genome construction, cell lifecycle ordering, apoptosis triggering,
life stage transitions, tissue population dynamics, stochastic noise behaviour.

---

## Documentation

| Path | Content |
|------|---------|
| `docs/paper2/intro_related_work.pdf` | Full technical paper (introduction, related work, model, reproducibility) |
| `docs/adr/` | Architecture Decision Records |
| `docs/diagrams_luis/` | Biological model diagrams |
| `configs/` | JSON configuration templates |

