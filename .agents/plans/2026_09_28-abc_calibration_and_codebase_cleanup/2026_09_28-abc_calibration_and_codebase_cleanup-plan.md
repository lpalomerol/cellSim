---
name: "abc_calibration_and_codebase_cleanup"
description: "Work plan to audit cellSim against the manuscript, isolate legacy code, implement standalone ABC-SMC experiment scripts for canonical, extended, and locked-tumoral-division setups with dry-run support, and verify 4-parameter reproducibility."
created_at: "2026-09-28T17:50:03+02:00"

created_by:
  tool: "Antigravity"
  model:
    name: "Gemini"
    version: "3.8 Flash"
    reasoning_effort: "low"

implemented_by:
  tool: "Antigravity"
  model:
    name: "Gemini"
    version: "3.8 Flash"
    reasoning_effort: "low"

last_implementation_at: "2026-09-28T18:48:00+02:00"
has_completed_all_phases: false
---

# 🎯 Goal

Audit and clean the `cellSim` codebase by establishing exact traceability with the manuscript sentences ([`MANUSCRIPT_SENTENCES.md`](file:///home/luis/Escritorio/abms/v2_BRCA1-ONSET_Ms_092426/MANUSCRIPT_SENTENCES.md)), isolating unused legacy modules, and creating modular, standalone executable experiment scripts structured across the 4 key experimental pillars of the paper:
1. **ABC-SMC Parameter Calibration**: General calibration across all free parameters (with continuous expansion rate $\nu$) and baseline support script without accelerated division.
2. **Bootstrapping**: Robustness, emergent parameter relationships, and tumor feature generation.
3. **Attenuation / Interception**: Tissue-level restraint and dynamic interception scenarios.
4. **Cell Scaling**: Scale-invariance and tissue size sensitivity experiments.

Every experiment script supports `--dry-run`, saves structured outputs in `results/20260928_draft_map/<experiment_id>/`, generates diagnostic tables, manifests, and minimal verification plots.

# 👀 Context

- **Manuscript sentences audit source**: [`/home/luis/Escritorio/abms/v2_BRCA1-ONSET_Ms_092426/MANUSCRIPT_SENTENCES.md`](file:///home/luis/Escritorio/abms/v2_BRCA1-ONSET_Ms_092426/MANUSCRIPT_SENTENCES.md).
- **Core C++ Monte Carlo Engine**: [`app/run_bootstrapping.cpp`](file:///home/luis/CLionProjects/cellSim/app/run_bootstrapping.cpp) and [`src/application/bootstrapping/BootstrappingMetrics.cpp`](file:///home/luis/CLionProjects/cellSim/src/application/bootstrapping/BootstrappingMetrics.cpp).
- **Cell Lifecycle and Genomic Dynamics**: [`src/domain/cell/AgenticCell.cpp`](file:///home/luis/CLionProjects/cellSim/src/domain/cell/AgenticCell.cpp), [`src/domain/cell/strategies/GenomicInstabilityDeltaStrategy.cpp`](file:///home/luis/CLionProjects/cellSim/src/domain/cell/strategies/GenomicInstabilityDeltaStrategy.cpp), and [`src/domain/cell/strategies/GenomicViabilityStrategy.cpp`](file:///home/luis/CLionProjects/cellSim/src/domain/cell/strategies/GenomicViabilityStrategy.cpp).
- **Manuscript Sections & Experiments Mapping**:
  - *Result 1*: "BRCA1-ONSET: A stochastic agent-based model..." (Conceptual architecture).
  - *Result 2*: "Bayesian calibration establishes the requirement for accelerated tumoral-cell expansion and reproduces clinical penetrance" -> ABC-SMC calibration (`exp_01_abc_smc_global.py` & support `exp_01_abc_smc_locked_division.py`).
  - *Result 3*: "Emergent parameter relationships and tumor features in BRCA1-ONSET" -> Bootstrapping (`exp_02_bootstrapping_features.py`).
  - *Result 4*: "BRCA1-ONSET tissue-level control provides substantial interception benefit" -> Attenuation/Interception (`exp_03_attenuation_interception.py`).
  - *Support / Sensitivity*: Tissue cell scaling (`exp_04_cell_scaling.py`).
- **Verification Commands**:
  - Build command: `cmake --build build` or `make rebuild`
  - Test command: `./build/tests/unit_tests` or `make test`

# 🪜 Phases

## Phase 1: Manuscript-Codebase Audit and Unused Code Isolation

### Description
Map every relevant simulation statement in [`MANUSCRIPT_SENTENCES.md`](file:///home/luis/Escritorio/abms/v2_BRCA1-ONSET_Ms_092426/MANUSCRIPT_SENTENCES.md) to its implementing class in `cellSim`. Formally document which features are active versus which are unused legacy components. Isolate/deprecate unused components (e.g. `ISignal` queue, lognormal noise decorator, early exploration apps) so that the codebase is clean, without breaking compilation or existing test suites.

### Public Contracts
- **Documentation**: Create [`docs/CODEBASE_MANUSCRIPT_AUDIT.md`](file:///home/luis/CLionProjects/cellSim/docs/CODEBASE_MANUSCRIPT_AUDIT.md).
- **C++ Deprecations**: Mark legacy message-passing signal headers in [`src/domain/signal/`](file:///home/luis/CLionProjects/cellSim/src/domain/signal/) and [`src/domain/cell/strategies/LognormalNoiseInstabilityStrategy.h`](file:///home/luis/CLionProjects/cellSim/src/domain/cell/strategies/LognormalNoiseInstabilityStrategy.h) with `[[deprecated]]` notices or archive them into a legacy namespace.

### To-do actions list
- [x] Create [`docs/CODEBASE_MANUSCRIPT_AUDIT.md`](file:///home/luis/CLionProjects/cellSim/docs/CODEBASE_MANUSCRIPT_AUDIT.md) detailing exact manuscript paragraph references vs. C++ and Python source files, historical experiments in `results/` and `traces/`, Makefile targets, and legacy scripts marked for archive.
- [x] Document active code: `run_bootstrapping.cpp`, `AgenticCell.cpp`, `GenomicInstabilityDeltaStrategy.cpp`, `GenomicViabilityStrategy.cpp`, `BootstrappingMetrics.cpp`.
- [x] Document unused/legacy code: `src/domain/signal/*` (synchronous execution does not route `ISignal` queues), `LognormalNoiseInstabilityStrategy` (manuscript fixes `delta-cv = 0`), `GeneticSummaryPresenter`, and early exploratory binaries in `app/`.
- [x] Document historical experiments in `results/` and `traces/` and mark them for archive.
- [x] Document Makefile target audit and mark legacy CLI, exploratory, and packaging targets for deprecation/archiving.
- [x] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make test` passed 138/138 tests). Fix issues if any.
- [x] STOP. Present the changes to the user for review and suggest commit messages (or pull request titles, when the phases are implemented through pull requests). Do NOT proceed to the next phase until the user explicitly asks.

---

## Phase 2: Enhanced C++ CLI Contracts for Extended & Constrained Division Simulations

### Description
Enhance [`app/run_bootstrapping.cpp`](file:///home/luis/CLionProjects/cellSim/app/run_bootstrapping.cpp) and domain configuration to expose all parameters explored during sensitivity/structural validations (`--kappa-brca1`, `--age-drift`, and `--division-rate`), and use the native `--big-bang` flag directly to control whether neoplastic cells divide at accelerated rate $\nu$ (`--big-bang` present) or at normal tissue division rate $d$ (`--big-bang` omitted).

### Public Contracts
- **CLI Options in `run_bootstrapping`**:
  - `--division-rate <double>`: Normal cell division rate per tick (default: 0.001).
  - `--kappa-brca1 <double>`: BRCA1 D2 accumulation structural multiplier (default: 2.0).
  - `--age-drift <double>`: Cellular aging drift coefficient per tick (default: 1e-5).
  - `--big-bang`: When omitted/false, forces neoplastic cell division rate to match normal tissue division rate (`division_rate`).
- **C++ Methods / Structs**:
  - Update `domain::InstabilityConfig` and `domain::CellConfig` with `kappa_brca1` and `age_drift`.
  - Update `CellFactory::createCustomCell` to accept and pass `division_rate`, `kappa_brca1`, and `age_drift`.
  - Update `runOnce` and `runBatch` in [`app/run_bootstrapping.cpp`](file:///home/luis/CLionProjects/cellSim/app/run_bootstrapping.cpp) to accept and forward these parameters while preserving defaults.
- **Unit Tests**:
  - Add test cases in [`tests/RunBootstrappingTest.cpp`](file:///home/luis/CLionProjects/cellSim/tests/RunBootstrappingTest.cpp) verifying kappa scaling, age drift, and big bang division control.

### To-do actions list
- [x] Add `--kappa-brca1`, `--age-drift`, and `--division-rate` flags to [`app/run_bootstrapping.cpp`](file:///home/luis/CLionProjects/cellSim/app/run_bootstrapping.cpp).
- [x] Update `runOnce` and `runBatch` in `run_bootstrapping.cpp` to forward the new parameters to `CellFactory::createCustomCell`.
- [x] Use `--big-bang` mode switch directly to control accelerated neoplastic division vs. equal division rate matching normal cells.
- [x] Add unit test assertions in [`tests/RunBootstrappingTest.cpp`](file:///home/luis/CLionProjects/cellSim/tests/RunBootstrappingTest.cpp) to verify default invariance, kappa scaling, age drift, and big bang mode division control (141/141 passed).
- [x] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make test` passed 141/141 tests). Fix issues if any.
- [x] STOP. Present the changes to the user for review and suggest commit messages (or pull request titles, when the phases are implemented through pull requests). Do NOT proceed to the next phase until the user explicitly asks.

---

## Phase 3: Standalone Executable ABC-SMC Calibration Scripts (Pillar 1)

### Description
Implement the ABC-SMC calibration infrastructure in `scripts/experiments/` with an adaptive sequential Monte Carlo engine, supporting the unified Bayesian calibration covering all variables (canonical & extended) and the unaccelerated baseline support script. Every script supports instant `--dry-run` and outputs structured diagnostics and summary statistics to `results/20260928_draft_map/`.

### Public Contracts
- **Scripts**:
  - `scripts/experiments/exp_01_abc_smc_global.py`: Unified Bayesian calibration estimating parameters ($\beta, \delta_{\text{low}}, \delta_{\text{high}}, \nu, \mu_{\text{TP53}}, \dots$) reproducing clinical penetrance while showing posterior exclusion of basal proliferation.
  - `scripts/experiments/exp_01_abc_smc_locked_division.py`: Methodological support script evaluating the model without accelerated neoplastic expansion ($\nu = d$).
  - `scripts/experiments/abc_smc_framework.py`: Reusable ABC-SMC core (adaptive tolerance $Q_{0.75}$, Beaumont/Toni weighted covariance perturbation, SIS weights, weighted SSE distance against clinical penetrance, OpenMP concurrency).
- **Execution CLI Flags**:
  - `--dry-run`: Runs an instant smoke test (2 particles, 2 generations, 5 bootstrap simulations per particle).
  - `--output-dir <path>`: Output destination defaulting to `results/20260928_draft_map/<experiment_id>/`.
  - `--workers <int>`: Concurrency worker thread count.
- **Unit Tests**:
  - Add `tests/test_experiment_scripts.py` verifying `--dry-run` execution for both scripts.

### To-do actions list
- [x] Create `scripts/experiments/abc_smc_framework.py` with adaptive tolerance, Gaussian perturbation kernel, SIS weight computation, and biological constraints ($\delta_{\text{high}} > \delta_{\text{low}}$).
- [x] Implement `scripts/experiments/exp_01_abc_smc_global.py` with standalone CLI, all free parameters predicted without fixed params, and `--dry-run` flag.
- [x] Implement `scripts/experiments/exp_01_abc_smc_locked_division.py` predicting all free parameters without `--big-bang` flag.
- [x] Implement `tests/test_experiment_scripts.py` verifying dry-run runs across ABC calibration scripts.
- [x] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make test` and `python3 tests/test_experiment_scripts.py`). Fix issues if any.
- [x] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

---

## Phase 4: Bootstrapping, Attenuation, and Cell Scaling Experiment Scripts (Pillars 2, 3, 4)

### Description
Implement standalone experiment runners for the remaining 3 manuscript pillars:
1. **Bootstrapping** (`exp_02_bootstrapping_features.py`): Runs Monte Carlo cohorts using calibrated parameters, extracts emergent parameter relationships and tumor features (age at onset, cell counts, stage distributions).
2. **Attenuation / Interception** (`exp_03_attenuation_interception.py`): Evaluates tissue-level restraint and pharmacological/interception interventions (attenuation factors, delay of onset).
3. **Cell Scaling** (`exp_04_cell_scaling.py`): Evaluates tissue size scaling and incomplete penetrance dynamics across cell population sizes.
All scripts generate structured CSVs, JSON manifests, and minimal verification plots (`penetrance_curve.png`, `correlation_matrix.png`, etc.) directly into `results/20260928_draft_map/`.

### Public Contracts
- **New Scripts**:
  - `scripts/experiments/exp_02_bootstrapping_features.py`: Generates cohort synthetic penetrance and emergent parameter correlations.
  - `scripts/experiments/exp_03_attenuation_interception.py`: Tests tissue restraint and intervention regimens.
  - `scripts/experiments/exp_04_cell_scaling.py`: Tests cohort behavior across tissue scale variations.
- **Verification Tests**:
  - Update `tests/test_experiment_scripts.py` to cover `--dry-run` across all 5 experiment scripts.

### To-do actions list
- [ ] Implement `scripts/experiments/exp_02_bootstrapping_features.py` with `--dry-run` support and automated summary metrics generation.
- [ ] Implement `scripts/experiments/exp_03_attenuation_interception.py` with interception scenario parameters and summary metrics.
- [ ] Implement `scripts/experiments/exp_04_cell_scaling.py` with cell scaling sweep and penetrance evaluation.
- [ ] Add minimal figure rendering utilities (penetrance curves, parameter distributions, trade-off matrices) in each script.
- [ ] Update `tests/test_experiment_scripts.py` to assert dry-run execution on all 5 experiment scripts.
- [ ] Verify using `make test` and `python3 tests/test_experiment_scripts.py`. Fix issues if any.
- [ ] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

---

## Phase 5: Verification, Benchmarking Against Frozen Results & Outcome Reports

### Description
Verify that newly calibrated ABC-SMC and bootstrapping results reproduce the reference penetrance curves and parameter posterior ranges from the manuscript frozen data (`paper/results/` or `results/abc_final_v4/`). Generate comprehensive discrepancy and outcome reports in `results/20260928_draft_map/`.

### Public Contracts
- **Verification Script**:
  - `scripts/experiments/verify_canonical_consistency.py`: CLI script comparing newly generated posteriors and penetrance metrics against manuscript reference values.
- **Reporting**:
  - `results/20260928_draft_map/reproducibility_benchmark_report.md`: Comparative table of parameters, penetrance SSE, and KS test statistics.

### To-do actions list
- [ ] Implement `scripts/experiments/verify_canonical_consistency.py`.
- [ ] Run benchmark verification comparing `results/20260928_draft_map/` against frozen manuscript data.
- [ ] Produce `results/20260928_draft_map/reproducibility_benchmark_report.md`.
- [ ] Verify the changes using the project's verification command (`make test`). Fix issues if any.
- [ ] STOP. Present the final status to the user for review and suggest commit messages.

# ⏭️ Next step

The next step is to implement **Phase 4: Bootstrapping, Attenuation, and Cell Scaling Experiment Scripts (Pillars 2, 3, 4)** (`scripts/experiments/exp_02_bootstrapping_features.py`, `scripts/experiments/exp_03_attenuation_interception.py`, `scripts/experiments/exp_04_cell_scaling.py`) with `--dry-run` modes, output artifacts, and minimal figure generation.

Contracts expanded and calibrated thanks to [Codely](https://codely.com) tooling 🐢 💨 🔍 📜 🔬 🚀 🧬 🧪.
