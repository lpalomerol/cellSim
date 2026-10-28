---
name: "exp07_tissue_scale_refactoring"
description: "Refactoring, beautification and integration of the 10^7 cell tissue-scale model, simulation engine, and publication figures (Figure 6, Figure S6, Figure S7)"
created_at: "2026-10-28T08:52:00+01:00"

created_by:
  tool: "Antigravity CLI"
  model:
    name: "Gemini"
    version: "3.8 Flash (Medium)"
    reasoning_effort: "medium"

implemented_by:
  tool: "Antigravity CLI"
  model:
    name: "Gemini"
    version: "3.8 Flash (Medium)"
    reasoning_effort: "medium"

last_implementation_at: "2026-10-28T09:24:00+01:00"
has_completed_all_phases: true
---

# 🎯 Goal

Import, refactor, and integrate the standalone **BRCA1-ONSET tissue-scale model** ($10^7$ susceptible cells, $5 \times 10^5$ tumor-positive threshold, finite doubling time $\tau_T$, and log-normal tissue restraint $\theta_{D2}$) into `tissue_scale/`.
Beautify the compressed C++ simulator (`tissue_scale_sim.cpp`) into modern, highly readable, well-documented C++20 code matching our domain naming conventions. Package reproduction scripts and Makefile targets to instantly reproduce **Figure 6**, **Supplementary Figure S6**, and **Supplementary Figure S7** from frozen validation records, with fast smoke testing support.

---

# 👀 Context

- **Source Codebase**: `/home/luis/Escritorio/abms/v2_BRCA1-ONSET_Ms_092426/code_tissue-scale/code_tissue-scale/BRCA1_ONSET_tissue_scale_GitHub/`
  - Simulator: `src/tissue_scale_sim.cpp` (58 lines of heavily compressed code)
  - Configs: `configs/final_multiobjective.json`, `configs/homogeneous.json`
  - Frozen Validation Data: `data/final_multiobjective_validation_10000.csv`, `data/homogeneous_validation_3000.csv`, `data/homogeneous_search_source.csv`
  - Analysis Scripts: `scripts/make_figures.py`, `scripts/make_sfig5.py`, `scripts/summarize_validation.py`, `scripts/run_validation.py`, `scripts/score_multiobjective.py`
  - Methods: `docs/METHODS_tissue_scale.md`
- **Destination Module**: [`tissue_scale/`](file:///home/luis/CLionProjects/cellSim/tissue_scale/)
- **Master Plan Reference**: [`Phase 7 of .agents/plans/2026_10_25-vertical_refactor_paper_reproducibility-plan.md`](file:///home/luis/CLionProjects/cellSim/.agents/plans/2026_10_25-vertical_refactor_paper_reproducibility/2026_10_25-vertical_refactor_paper_reproducibility-plan.md#L205-L216)
- **Top-level Makefile**: [`Makefile`](file:///home/luis/CLionProjects/cellSim/Makefile)

---

# 📜 Public Contracts & Deliverables

### Directory Structure: `tissue_scale/`
```text
tissue_scale/
├── CMakeLists.txt / Makefile     # Dedicated compilation with OpenMP optimization
├── README.md                     # Tissue-scale methodology, quickstart & CLI usage
├── configs/
│   ├── final_multiobjective.json # Final representative parameters
│   └── homogeneous.json          # Homogeneous comparison parameters
├── data/
│   ├── final_multiobjective_validation_10000.csv
│   ├── homogeneous_validation_3000.csv
│   └── homogeneous_search_source.csv
├── results/                      # Generated figures & outputs
│   ├── Figure6.pdf / Figure6.png
│   ├── FigureS6.pdf / FigureS6.png
│   └── FigureS7.pdf / FigureS7.png
├── scripts/
│   ├── make_figure6_sfig6.py     # Cleaned make_figures.py
│   ├── make_sfig7.py             # Cleaned make_sfig5.py (manuscript Figure S7)
│   ├── run_validation.py         # Stochastic runner with fast smoke-test flag
│   └── summarize_validation.py  # Validation metrics calculator
└── src/
    └── tissue_scale_sim.cpp      # Beautified modern C++20 simulation engine
```

### CLI Contracts & Execution
- **C++ Simulation Binary** (`build/tissue_scale_sim` or `tissue_scale/bin/tissue_scale_sim`):
  `./tissue_scale_sim <mu_tp53> <beta_brca1> <delta_low> <delta_high> <theta_d1> <theta_d2> <tau> <sigma> <nsim> <seed>`
- **Figure Generation CLI**:
  - `python3 tissue_scale/scripts/make_figure6_sfig6.py`: Renders Figure 6 and Figure S6.
  - `python3 tissue_scale/scripts/make_sfig7.py`: Renders Figure S7.
- **Root Makefile Targets**:
  - `make exp07-fig6-tissue`: Generate Figure 6 and Figure S6.
  - `make exp07-sfig7-tissue`: Generate Figure S7.
  - `make exp07-smoke`: Fast (<5s) verification compiling and running N=10 simulations.
  - `make tissue-scale-test`: Validate summary metrics against frozen manuscript values.

---

# 🪜 Phases

## Phase 1: Directory Setup, Asset Ingestion & Dedicated Build Harness

### Description
Establish the `tissue_scale/` directory structure, import frozen validation CSVs and JSON configurations from the external package, and set up a dedicated `Makefile` / `CMakeLists.txt` for compiling the tissue simulator with C++20 and OpenMP.

### To-do Actions List
- [x] Create `tissue_scale/` directory structure (`src/`, `scripts/`, `data/`, `configs/`, `results/`).
- [x] Copy frozen datasets (`final_multiobjective_validation_10000.csv`, `homogeneous_validation_3000.csv`, `homogeneous_search_source.csv`) into `tissue_scale/data/`.
- [x] Copy parameter configuration files (`final_multiobjective.json`, `homogeneous.json`) into `tissue_scale/configs/`.
- [x] Create `tissue_scale/Makefile` to compile `tissue_scale_sim` using `-std=c++20 -O3 -fopenmp`.
- [x] Wire top-level Makefile to delegate build and clean targets to `tissue_scale/Makefile`.
- [x] Verify compilation of the baseline `tissue_scale_sim.cpp` without warnings or linking errors.
- [x] STOP. Present the changes to the user for review.

---

## Phase 2: C++ Engine Beautification & Refactoring (C++20 Domain Cleanliness)

### Description
Refactor `tissue_scale/src/tissue_scale_sim.cpp` completely. Replace single-letter and abbreviated variable names (`bl`, `tp`, `dl`, `dh`, `th1`, `th2`, `ad`, `cnt`, `bpp`, `rr`) with explicit domain names (`brca1_loss`, `tp53_hits`, `delta_low`, `delta_high`, `theta_d1`, `theta_d2`, `age_dependent_rate`, etc.). Document formulas referencing the Methods section (`D1`, `D2`, binom/poisson approximations, doubling time $G = 2^{1/\tau_T}$, and founder clonality). Ensure zero functional divergence from the mathematical formulation.

### To-do Actions List
- [x] Expand and document structs:
  - `CellCohortKey`: `brca1_loss`, `tp53_hits`, `cohort_age`, `d1_low_events`, `d1_high_events`, `d2_low_events`, `d2_high_events`, `d1_age_accum`, `d2_age_accum`.
  - `CohortHash`: 64-bit FNV-1a state hasher with clear documentation.
  - `SimulationParameters`: all 8 biological and tissue-restraint parameters.
  - `FounderClone`: `unmutated_cells`, `brca1_null_cells`, `damage_d1_accumulator`.
  - `TissueSimulationResult`: `age_at_detection`, `total_founders`, `null_founders`, `final_unmutated_cells`, `final_null_cells`, `major_lineages_1pct`, `major_lineages_5pct`, `effective_founders`.
- [x] Clean up and document mathematical helper functions (`compute_D1`, `compute_D2`, `draw_binomial`, `draw_poisson`).
- [x] Refactor `simulate_single_tissue` with clear modular sections:
  1. Initialization of $N_0 = 10^7$ unmutated cohort.
  2. Annual loop (somatic BRCA1 and TP53 hits, binomial partitioning).
  3. Transformation criteria: $TP53 = 2$, $D_1 > \theta_{D1}$, and $D_2 > \theta_{D2, \text{individual}}$.
  4. Post-transformation clonal expansion with doubling time $\tau_T$ ($G = 2^{1/\tau_T}$) and second-hit BRCA1 somatic conversion.
  5. Detection threshold evaluation ($5 \times 10^5$ cells) and clonal diversity metrics (Simpson effective founders).
- [x] Verify identical numerical outputs between beautified version and original engine given the same seed.
- [x] Create comprehensive unit test suite in [`tests/TissueScaleSimulatorTest.cpp`](file:///home/luis/CLionProjects/cellSim/tests/TissueScaleSimulatorTest.cpp) integrated with GoogleTest and CTest.
- [x] Verify that all 144 unit and regression tests pass (`make test`).
- [x] STOP. Present the changes to the user for review.

---

## Phase 3: Python Analysis Scripts Refactoring, Figure Generation & Verification

### Description
Import, clean, and standardize Python scripts (`make_figures.py`, `make_sfig5.py`, `summarize_validation.py`, `run_validation.py`). Implement the publication aesthetic consistent with Figures 2–5. Wire top-level Makefile targets and write thorough documentation in `tissue_scale/README.md`.

### To-do Actions List
- [x] Adapt `make_figures.py` to `tissue_scale/scripts/make_figure6_sfig6.py`:
  - Render `Figure6.pdf` / `Figure6.png` (penetrance comparison, BRCA1 composition, and founder lineage distribution).
- [x] Adapt `make_sfig5.py` to `tissue_scale/scripts/make_sfig7.py`:
  - Render `FigureS7.pdf` / `FigureS7.png` (homogeneous fitting trade-offs).
- [x] Refactor `summarize_validation.py` to output formatted verification metrics (penetrance, SSE=4.15, BRCA1 majority=92.5%, >=2 founders=28.1%).
- [x] Refactor `run_validation.py` to support `--smoke-test` (N=10 in <2s) and full validation runs.
- [x] Add Makefile targets:
  - `exp06-fig6`: Generates Figure 6.
  - `exp06-sfig7`: Generates Figure S7.
  - `exp06-smoke`: Compiles and runs 10 tissues in <3s.
  - `exp06-validate`: Validates summary stats from frozen data.
- [x] Create `tissue_scale/README.md` with complete documentation, mathematical rationale, parameter tables, and reproduction commands.
- [x] Run complete verification suite (`make exp06-fig6`, `make exp06-sfig7`, `make exp06-smoke`, `make exp06-validate`, `make test`).
- [x] STOP. Present the changes to the user for review and suggest commit messages.

---

# ⏭️ Next Step

The next step is to execute **Phase 1: Directory Setup, Asset Ingestion & Dedicated Build Harness** to create the `tissue_scale/` directory, copy the frozen datasets and configs, and wire the dedicated Makefile.

Plan prepared with Codely discipline by 🐢 💨 🏆 ✨ (Turbotuga™, [Codely](https://codely.com)’s mascot).
