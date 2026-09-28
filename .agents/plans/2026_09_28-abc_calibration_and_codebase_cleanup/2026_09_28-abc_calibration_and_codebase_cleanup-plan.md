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

last_implementation_at: "2026-09-28T18:00:00+02:00"
has_completed_all_phases: false
---

# 🎯 Goal

Audit and clean the `cellSim` codebase by establishing exact traceability with the manuscript sentences ([`MANUSCRIPT_SENTENCES.md`](file:///home/luis/Escritorio/abms/v2_BRCA1-ONSET_Ms_092426/MANUSCRIPT_SENTENCES.md)), isolating unused legacy modules, and creating standalone executable ABC-SMC calibration scripts for three specific paper experimental scenarios (canonical 4-parameter, extended all-parameter, and tumoral-division-locked ablation) with dry-run capabilities, dedicated outcome summaries in `results/20260928_draft_map/`, and reproducibility verification against prior manuscript results.

# 👀 Context

- **Manuscript sentences audit source**: [`/home/luis/Escritorio/abms/v2_BRCA1-ONSET_Ms_092426/MANUSCRIPT_SENTENCES.md`](file:///home/luis/Escritorio/abms/v2_BRCA1-ONSET_Ms_092426/MANUSCRIPT_SENTENCES.md).
- **Core C++ Monte Carlo Engine**: [`app/run_bootstrapping.cpp`](file:///home/luis/CLionProjects/cellSim/app/run_bootstrapping.cpp) and [`src/application/bootstrapping/BootstrappingMetrics.cpp`](file:///home/luis/CLionProjects/cellSim/src/application/bootstrapping/BootstrappingMetrics.cpp).
- **Cell Lifecycle and Genomic Dynamics**: [`src/domain/cell/AgenticCell.cpp`](file:///home/luis/CLionProjects/cellSim/src/domain/cell/AgenticCell.cpp), [`src/domain/cell/strategies/GenomicInstabilityDeltaStrategy.cpp`](file:///home/luis/CLionProjects/cellSim/src/domain/cell/strategies/GenomicInstabilityDeltaStrategy.cpp), and [`src/domain/cell/strategies/GenomicViabilityStrategy.cpp`](file:///home/luis/CLionProjects/cellSim/src/domain/cell/strategies/GenomicViabilityStrategy.cpp).
- **Existing ABC Implementations & Paper Scripts**:
  - Legacy ABC script: [`scripts/abc_final_v4.py`](file:///home/luis/CLionProjects/cellSim/scripts/abc_final_v4.py).
  - Frozen manuscript reproducibility scripts: [`/home/luis/Escritorio/abms/v2_BRCA1-ONSET_Ms_092426/BRCA1_ONSET_reproducibility/paper/scripts/`](file:///home/luis/Escritorio/abms/v2_BRCA1-ONSET_Ms_092426/BRCA1_ONSET_reproducibility/paper/scripts/).
- **Architectural & Agent Guidelines**:
  - [`.copilot-instructions.md`](file:///home/luis/CLionProjects/cellSim/.copilot-instructions.md): Master instructions defining the 3-layer architecture, 6 cell stages, dual accumulators ($D_1, D_2$), and ADR summaries.
  - [`.github/copilot/agent_roles.md`](file:///home/luis/CLionProjects/cellSim/.github/copilot/agent_roles.md): Agent matrix for Tissue, AgenticCell, Genome, and Gene.
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
Enhance [`app/run_bootstrapping.cpp`](file:///home/luis/CLionProjects/cellSim/app/run_bootstrapping.cpp) and [`src/application/config/SimulationConfig.h`](file:///home/luis/CLionProjects/cellSim/src/application/config/SimulationConfig.h) to expose all parameters explored during sensitivity/structural validations, and add a flag to force tumoral cell reproduction rate to the healthy cell division rate ($\nu = d = 0.001$).

### Public Contracts
- **CLI Options in `run_bootstrapping`**:
  - `--kappa-brca1 <double>`: BRCA1 D2 accumulation structural multiplier (default: 2.0).
  - `--age-drift <double>`: Cellular aging drift coefficient per tick (default: 1e-5).
  - `--lock-tumoral-division`: Boolean flag forcing tumoral cell division rate to match normal tissue division rate (`neoplastic_div_rate = division_rate = 0.001`).
- **C++ Methods / Structs**:
  - Update `application::SimulationConfig` with `kappa_brca1`, `age_drift`, and `lock_tumoral_division`.
  - Update `runOnce` and `runBatch` signatures in [`app/run_bootstrapping.cpp`](file:///home/luis/CLionProjects/cellSim/app/run_bootstrapping.cpp) to accept these parameters while preserving default values for backwards compatibility.
- **Unit Tests**:
  - Add test cases in [`tests/RunBootstrappingTest.cpp`](file:///home/luis/CLionProjects/cellSim/tests/RunBootstrappingTest.cpp) verifying parameter passing and checking that `--lock-tumoral-division` strictly maintains equal division rates.

### To-do actions list
- [ ] Add `--kappa-brca1`, `--age-drift`, and `--lock-tumoral-division` flags to [`app/run_bootstrapping.cpp`](file:///home/luis/CLionProjects/cellSim/app/run_bootstrapping.cpp).
- [ ] Update `runOnce` and `runBatch` in `run_bootstrapping.cpp` to forward the new parameters to `CellFactory::createCustomCell`.
- [ ] Implement the division rate lock logic: when `lock_tumoral_division` is enabled, enforce `neoplastic_div_rate = division_rate`.
- [ ] Add unit test assertions in [`tests/RunBootstrappingTest.cpp`](file:///home/luis/CLionProjects/cellSim/tests/RunBootstrappingTest.cpp) to verify default invariance and division lock.
- [ ] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make test` or `./build/tests/unit_tests`). Fix issues if any.
- [ ] STOP. Present the changes to the user for review and suggest commit messages (or pull request titles, when the phases are implemented through pull requests). Do NOT proceed to the next phase until the user explicitly asks.

---

## Phase 3: Standalone Executable ABC-SMC Scripts with Dry-Run Mode

### Description
Implement a new series of standalone executable Python scripts in `scripts/experiments/`. Each script corresponds to a concrete manuscript experimentation stage, supports an instant `--dry-run` verification mode, and saves structured outputs into `results/20260928_draft_map/<experiment_id>/`.

### Public Contracts
- **New Scripts**:
  - `scripts/experiments/01_abc_canonical_4p.py`: Calibrates 4 canonical parameters ($\beta, \delta_{\text{low}}, \delta_{\text{high}}, \nu$) with priors matching Figure 3a.
  - `scripts/experiments/02_abc_extended_all.py`: Calibrates full parameter set from sensitivity/structural validations ($\beta, \delta_{\text{low}}, \delta_{\text{high}}, \nu, \mu_{\text{TP53}}, \theta_{D1}, \theta_{D2}, \kappa_{\text{BRCA1}}$).
  - `scripts/experiments/03_abc_locked_tumoral_division.py`: Calibrates full parameter set with tumoral division forced to healthy basal rate ($\nu = d = 0.001$).
- **Shared Module**:
  - `scripts/experiments/abc_smc_framework.py`: Core adaptive ABC-SMC engine (prior sampling, Beaumont/Toni weighted covariance perturbation, SIS weights, weighted SSE distance, OpenMP worker orchestration).
- **Execution CLI Flags**:
  - `--dry-run`: Runs a fast pipeline smoke test (2 particles, 2 generations, 5 bootstrap simulations per particle).
  - `--workers <int>`: Concurrency worker thread count.
  - `--output-dir <path>`: Defaults to `results/20260928_draft_map/<experiment_id>/`.
- **Unit Tests**:
  - Add test script `tests/test_experiment_scripts.py` verifying `--dry-run` execution for all three experiment scripts.

### To-do actions list
- [ ] Create `scripts/experiments/abc_smc_framework.py` with adaptive tolerance ($Q_{0.75}$), Gaussian perturbation kernel with optimal variance, SIS weight computation, and biological constraints ($\delta_{\text{high}} > \delta_{\text{low}}$).
- [ ] Implement `scripts/experiments/exp_01_01_abc_canonical_4p.py` with standalone CLI and `--dry-run` flag.
- [ ] Implement `scripts/experiments/exp_01_02_abc_extended_all.py` with expanded priors and standalone CLI.
- [ ] Implement `scripts/experiments/exp_01_03_abc_locked_tumoral_division.py` passing `--lock-tumoral-division` to `run_bootstrapping`.
- [ ] Implement `tests/test_experiment_scripts.py` to test dry-run runs across all three scripts.
- [ ] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make test` and `python3 tests/test_experiment_scripts.py`). Fix issues if any.
- [ ] STOP. Present the changes to the user for review and suggest commit messages (or pull request titles, when the phases are implemented through pull requests). Do NOT proceed to the next phase until the user explicitly asks.

---

## Phase 4: Experiment Execution & Outcome Logging Pipeline

### Description
Execute the experiment runs and establish structured outcome logging for each individual experiment under `results/20260928_draft_map/`. Each outcome contains pass-by-pass statistics, rejection counts, error scoring ($SSE_w$ percentiles), and parameter estimation summaries (median, MAP, 95% CIs), with metadata prepared for future figure generation without plotting marginal densities at this time.

### Public Contracts
- **Output Artifacts Schema** in `results/20260928_draft_map/<experiment_id>/`:
  - `generation_XX.csv`: Particle values, weights, and distances for generation `XX`.
  - `diagnostics.csv`: Generation-by-generation acceptance rate, epsilon threshold, effective sample size (ESS), and elapsed time.
  - `parameter_estimation_summary.json`: Parameter median, MAP, and 95% credible intervals.
  - `summary_report.md`: Human-readable summary detailing pass-by-pass error scoring, convergence statistics, and parameter estimations.
  - `run_manifest.json`: Full environment, git commit, seed, and CLI parameter configuration for reproducibility.

### To-do actions list
- [ ] Validate dry-run execution and directory creation under `results/20260928_draft_map/`.
- [ ] Add outcome reporting generator in `scripts/experiments/abc_smc_framework.py` to automatically produce `summary_report.md` and `parameter_estimation_summary.json`.
- [ ] Execute the production runs (or user-defined particle/generation profiles) for:
  - `exp_01_01_abc_canonical_4p`
  - `exp_01_02_abc_extended_all`
  - `exp_01_03_abc_locked_tumoral_division`
- [ ] Verify that all outcome directories contain the required diagnostic tables, manifests, and error scoring summaries.
- [ ] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make test`). Fix issues if any.
- [ ] STOP. Present the changes to the user for review and suggest commit messages (or pull request titles, when the phases are implemented through pull requests). Do NOT proceed to the next phase until the user explicitly asks.

---

## Phase 5: Canonical 4-Parameter ABC Verification Against Prior Paper Results

### Description
Verify that the freshly calibrated 4-parameter ABC-SMC results (`results/20260928_draft_map/canonical_4p/`) closely match the previous paper frozen results. Perform automated discrepancy checking and generate a verification report.

### Public Contracts
- **Verification Script**:
  - `scripts/experiments/verify_canonical_consistency.py`: CLI script comparing newly generated `canonical_4p` posteriors against the reference posterior file in the paper repository (`paper/results/` or `results/abc_final_v4/`).
- **Discrepancy Report**:
  - `results/20260928_draft_map/canonical_4p/canonical_reproducibility_report.md`: Table comparing parameter medians, 95% CIs, and minimum $SSE_w$ between previous and new runs.

### To-do actions list
- [ ] Implement `scripts/experiments/verify_canonical_consistency.py` to compare parameter distributions (median, 2.5%, 97.5% quantiles) and distance distributions.
- [ ] Run consistency verification comparing `results/20260928_draft_map/canonical_4p/` against the reference posterior.
- [ ] Generate `results/20260928_draft_map/canonical_4p/canonical_reproducibility_report.md` summarizing the consistency check.
- [ ] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make test`). Fix issues if any.
- [ ] STOP. Present the changes to the user for review and suggest commit messages (or pull request titles, when the phases are implemented through pull requests). Do NOT proceed to the next phase until the user explicitly asks.

# ⏭️ Next step

The next step is to implement **Phase 2: Enhanced C++ CLI Contracts for Extended & Constrained Division Simulations** by adding `--kappa-brca1`, `--age-drift`, and `--lock-tumoral-division` to `app/run_bootstrapping.cpp` and verifying with unit tests.

Audited and catalogued thanks to [Codely](https://codely.com) tooling 🐢 💨 🔍 📜.
