# Section 2: Software Architecture and Design
*Draft — cellSim paper*

---

## 2.1 Architectural Overview

cellSim is structured according to **Clean Architecture** (Martin, 2017), a layered architectural style that enforces a strict dependency rule: inner layers (domain) are independent of outer layers (application, infrastructure). This guarantees that the biological model can evolve or be tested in complete isolation from simulation orchestration, output formatting, or configuration loading.

The project is organised into four layers:

| Layer | Responsibility | Key components |
|-------|---------------|----------------|
| **Domain** | Pure biological logic | `AgenticCell`, `Genome`, `Gene`, `Tissue`, signals |
| **Application** | Use-case orchestration | `Simulation`, `SimulationConfig`, scenarios |
| **Adapters** | Concrete implementations | `RandomNoise`, `FixedNoise`, `Logger` |
| **Ports** | Interface contracts | `ICell`, `INoiseSource`, `ILogger` |

No dependency ever points inward: the domain layer imports nothing from application or infrastructure. This architecture enables mocking of any external concern (noise, logging, I/O) without modifying biological logic, which is essential for deterministic unit testing.

## 2.2 The Agentic Cell Pattern

The central design decision of cellSim is to model each cell as a **fully autonomous agent** that encapsulates its own state, applies its own rules, and communicates with the tissue through a signal-based protocol.

### 2.2.1 Cell Internal State

Each `AgenticCell` instance holds:

```
AgenticCell {
    genome:   Genome { BRCA1: GeneState, TP53: GeneState }
    D1:       float   // genomic instability [1, 999]
    D2:       float   // apoptotic evasion capacity (phenomenological) [1, 999]
    alive:    bool
    neoplastic: bool
    immortal:   bool
}
```

The life stage (`CellLifeStage`) is derived on demand from these variables, not stored. This eliminates state synchronisation bugs: there is a single source of truth.

### 2.2.2 The Cell Cycle: Six Phases

Each simulation tick invokes `AgenticCell::live()`, which executes a **deterministic, ordered six-phase cycle**:

1. **Phase 0 — Genome mutation check**: Stochastic mutation of BRCA1 and TP53 alleles, driven by configurable per-gene mutation rates and an `INoiseSource`.
2. **Phase 1 — Intrinsic apoptosis**: If BRCA1 becomes `-/-` while TP53 is functional (`+/+` or `+/-`), TP53 detects the genomic damage and triggers a `CellDeathException`. This models the tumour suppressor's canonical checkpoint function.
3. **Phase 2 — Extrinsic apoptosis**: If the cell is in PRIMER stage (TP53 `-/-`, D1 > threshold) and D2 is below the immune evasion threshold, the tissue-derived signal kills the cell.
4. **Phase 3 — Neoplastic transformation**: If the cell is in PRIMER stage and D2 is above threshold, it evades apoptosis and undergoes neoplastic transformation (`neoplastic = true`, `immortal = true`).
5. **Phase 4 — Big Bang division**: If Big Bang mode is enabled, neoplastic cells may divide at an accelerated rate (`neoplastic_division_rate`), emitting a `CellDivisionSignal` carrying a copy of the parent genome.
6. **Phase 5 — Instability accumulation**: D1 and D2 are incremented according to the current TP53 and BRCA1 state (see Section 3.3).

### 2.2.3 Signal-Based Tissue Communication

Cells do not directly modify the tissue container. Instead, they emit typed signals during their cycle:

- `CellDeathException` — cell requests removal from the tissue
- `CellDivisionSignal` — cell requests addition of a daughter cell
- `NeoplasmSignal` — cell signals its neoplastic transformation to the tissue tracker

The `Tissue` class captures these signals and updates the population accordingly. This decoupling means the cell's logic requires no knowledge of the tissue's internal data structure, preserving single-responsibility and enabling independent testing.

## 2.3 Genome and Mutation Model

Genes are implemented as value types with three discrete allele states (`+/+`, `+/-`, `-/-`). Mutations are stochastic events driven by per-gene rates:

```
P(+/+ → +/-) = mutation_rate_TP53
P(+/- → -/-) = mutation_rate_TP53 × instability_factor
```

The instability factor is applied only when TP53 is already compromised (`+/-` or `-/-`), capturing the biological phenomenon of mutational acceleration under genomic instability. A `GenomeFactory` provides standardised initial configurations for each scenario.

## 2.4 Configurability and Reproducibility

All simulation parameters are encapsulated in a `SimulationConfig` struct loaded from JSON:

- **Gene parameters**: `mutation_rate`, `instability_rate` (per gene)
- **Tissue parameters**: `neoplasm_k`, `division_rate`, `neoplastic_division_rate`, `enable_big_bang_mode`
- **Simulation context**: `n_cells`, `max_t`, `seed`

Setting `seed` to any non-negative integer guarantees deterministic simulation output across executions and platforms, which is a prerequisite for scientific reproducibility. Setting `seed = -1` enables stochastic mode.

## 2.5 Testing Strategy

The simulator ships with **53 unit tests** implemented with the GoogleTest framework, organised by component:

| Component | Tests | Coverage focus |
|-----------|-------|---------------|
| Gene / Genome | 8 | State transitions, mutation rules |
| AgenticCell | 14 | Phase-by-phase cycle correctness |
| Tissue / Tracking | 11 | Population management, signal handling |
| Simulation scenarios | 12 | End-to-end scenario regression |
| Configuration | 8 | JSON parsing, validation, defaults |

All tests run in under 5 seconds. The CI pipeline (GitHub Actions) executes the full test suite on every push, ensuring no regression in biological or architectural logic.

---

## References (placeholder)

- Martin, R.C. (2017). *Clean Architecture: A Craftsman's Guide to Software Structure and Design*. Prentice Hall.
- [GoogleTest reference]
- [CMake reference]
