---
name: "remedy_paper_reproducibility_and_tissue_scale"
description: "Consolidate publication pipeline reproducibility, eliminate hardcoded simulation inputs, resolve Figure 2a and S4 conflicts, integrate missing Supplementary Figures S2 and S3, and realign tissue-scale hierarchy"
created_at: "2026-10-29T06:45:30Z"

created_by:
  tool: "Antigravity CLI"
  model:
    name: "Gemini"
    version: "3.8 Flash (Medium)"
    reasoning_effort: "medium"
---

# 🎯 Goal

Resolve all data regeneration disconnects and hardcoded inputs, settle visual and kinetic conflicts for Figure 2a (canonical 100-year horizon with $d=0.001$) and Figure S4 (adding 1D posterior density vs parameter pair ratios), rescue missing Supplementary Figures S2 and S3, realign tissue-scale figures (Figure S6 for homogeneous restraint and Figure S7 for checkpoint relaxation heatmaps), and synchronize the root `cellSim` and subproject `tissue_scale` build contracts for seamless publication release.

---

# 👀 Context

- **Manuscript Source**: [`/home/luis/Escritorio/abms/v4/Palomero_BRCA1_102126.txt`](file:///home/luis/Escritorio/abms/v4/Palomero_BRCA1_102126.txt) and [`Palomero_BRCA1_102126.docx`](file:///home/luis/Escritorio/abms/v4/Palomero_BRCA1_102126.docx).
- **Target Review Figures**:
  - Main Figures: [`/home/luis/Escritorio/abms/v4/MainFigs_review_102126`](file:///home/luis/Escritorio/abms/v4/MainFigs_review_102126) (Figs 1–6).
  - Supplementary Figures: [`/home/luis/Escritorio/abms/v4/SupplFig-Table-review_102126`](file:///home/luis/Escritorio/abms/v4/SupplFig-Table-review_102126) (Figs S1–S7).
- **Reference Code Packages**: [`/home/luis/Escritorio/abms/v2_BRCA1-ONSET_Ms_092426/BRCA1_ONSET_reproducibility/paper/scripts/`](file:///home/luis/Escritorio/abms/v2_BRCA1-ONSET_Ms_092426/BRCA1_ONSET_reproducibility/paper/scripts/) (original sources for S2 and S3).
- **Cell-Scale Codebase**: [`src/`](file:///home/luis/CLionProjects/cellSim/src), [`app/`](file:///home/luis/CLionProjects/cellSim/app), [`tests/`](file:///home/luis/CLionProjects/cellSim/tests), [`experiments/`](file:///home/luis/CLionProjects/cellSim/experiments).
- **Tissue-Scale Subproject**: [`tissue_scale/`](file:///home/luis/CLionProjects/cellSim/tissue_scale).
- **Orchestration**: [`Makefile`](file:///home/luis/CLionProjects/cellSim/Makefile) and [`CMakeLists.txt`](file:///home/luis/CLionProjects/cellSim/CMakeLists.txt).

---

# 📜 Public Contracts & Architectural Layout

### 1. Project Hierarchy Definition (Initial Modular Structure)
The repository currently has an asymmetrical layout where `cellSim` discrete code is located at the root, and `tissue_scale` is nested as a subfolder:
```text
cellSim/ (Current Asymmetrical Layout)
├── CMakeLists.txt                    # Root CMake compiling cellSim discrete apps and tests
├── Makefile                          # Unified build and experiment orchestrator
├── src/                              # Discrete cellSim domain and application layers
├── app/                              # Discrete cellSim C++ runners
├── tests/                            # Discrete cellSim GoogleTest suite
├── experiments/                      # CellSim experiment slices (exp01 to exp05)
└── tissue_scale/                     # Nested subproject for macro-scale model (Makefile, src, scripts, data)
```

### 2. Final Target Code & Directory Structure Contract (Symmetric Option A)
At the conclusion of this plan, the repository must exhibit this exact, fully symmetric and parallel structure. The two simulation engines (`cell_scale/` and `tissue_scale/`) live side-by-side as equal parallel libraries at the repository root, while `experiments/` is a top-level directory that unifies all 7 publication vertical slices covering every main and supplementary figure of the paper:

```text
cellSim/                                # ROOT REPOSITORY ORCHESTRATOR
├── CMakeLists.txt                      # Top-level CMake compiling BOTH engines & test suites
├── Makefile                            # Top-level orchestrator (build, test, exp01... exp07)
├── README.md                           # Publication and reproducibility master documentation
│
├── cell_scale/                         # ENGINE 1: Discrete Agent-Based Model (500 cells)
│   ├── CMakeLists.txt                  # Dedicated CMake build for cell_scale
│   ├── src/                            # Domain and application layers (Cell, Genome, Tissue)
│   │   ├── domain/                     # Core biological domain model
│   │   └── application/                # Simulation services and configuration
│   ├── app/                            # C++ simulation binaries
│   │   ├── run_bootstrapping.cpp       # Bootstrapping penetrance evaluator
│   │   ├── run_paper_features.cpp      # Emergent feature evaluator
│   │   └── run_paper_interventions.cpp # In silico interception evaluator
│   └── tests/                          # Cell-scale GoogleTest suite
│
├── tissue_scale/                       # ENGINE 2: Macro-Scale Stochastic Model (10^7 cells)
│   ├── CMakeLists.txt                  # Dedicated CMake build (or Makefile) for tissue_scale_sim
│   ├── src/                            # High-performance C++20 OpenMP engine
│   │   ├── tissue_scale_sim.cpp        # Simulation entrypoint matching Methods
│   │   └── TissueScaleSimulator.h      # Standalone domain header
│   ├── configs/                        # Reference configurations
│   │   ├── final_multiobjective.json   # Calibrated reference parameters
│   │   └── homogeneous.json            # Homogeneous tissue restraint baseline
│   └── tests/                          # Stochastic equivalence regression tests
│
├── experiments/                        # UNIFIED PUBLICATION VERTICAL SLICES (9 SLICES)
│   ├── exp01_cs_exploratory_acceleration/ # FIGURE 2 (Pre-transformation vs Accelerated Expansion)
│   │   ├── README.md                   # Kinetic trade-offs documentation
│   │   ├── run_exploratory.py          # Monte Carlo runner (100-year horizon, d=0.001)
│   │   ├── plot_figure2.py             # Canonical plotting script (4 representative + 36 grid models)
│   │   ├── Figure2.pdf                 # Main publication vector figure
│   │   ├── Figure2.png                 # Main publication raster figure (300 DPI)
│   │   └── data/                       # 100-year trajectories & grid summary CSVs
│   │
│   ├── exp02_cs_sensitivity_global/       # SUPPLEMENTARY FIGURE S1 (Global Sensitivity LHS & PRCC)
│   │   ├── README.md                   # Sensitivity analysis methodology
│   │   ├── run_lhs_sensitivity.py      # Space-filling Latin Hypercube Sampler (1,000 pts x 200 sims)
│   │   ├── plot_sfig1.py               # Plotting script (RF relative importance, PRCC heatmap, ECDFs)
│   │   ├── Supplementary_Figure_S1.pdf # Supplementary Figure S1 vector
│   │   ├── Supplementary_Figure_S1.png # Supplementary Figure S1 raster (300 DPI)
│   │   └── data/                       # LHS evaluations, PRCC table & RF importance CSVs
│   │
│   ├── exp03_cs_abc_smc_calibration/      # FIGURE 3 & FIGURE S4 (Bayesian Calibration & Posteriors)
│   │   ├── README.md                   # ABC-SMC calibration guide (250 particles x 20 generations)
│   │   ├── abc_smc.py                  # Calibrator auto-exporting posterior particles & PPC summary
│   │   ├── plot_figure3.py             # Figure 3 plotting script (convergence & PPC penetrance)
│   │   ├── plot_sfig4.py               # Figure S4 plotting script (Variants 4a, 4b, 4c, and 4d)
│   │   ├── Figure3.pdf / .png          # Main Figure 3 outputs
│   │   ├── FigureS4.pdf / .png         # Canonical 4x4 parameter corner matrix (Variant 4a)
│   │   ├── FigureS4b.pdf / .png        # Cross-parameter ratio grid (Variant 4b)
│   │   ├── FigureS4c.pdf / .png        # Self-ratio strip (Variant 4c)
│   │   ├── FigureS4d.pdf / .png        # 1D posterior density (frequency) vs ratio (Variant 4d)
│   │   └── data/                       # Posterior particles, PPC summary, diagnostics CSVs
│   │
│   ├── exp04_cs_emergent_features/        # FIGURE 4 (Emergent Phenotypes & Lineage Clonal Complexity)
│   │   ├── README.md                   # Clonal architecture documentation
│   │   ├── run_emergent_features.py    # 20k virtual tissue simulator across posterior population
│   │   ├── plot_figure4.py             # Figure 4 plotting script (posteriors, genotypes, founders)
│   │   ├── Figure4.pdf / .png          # Main Figure 4 outputs
│   │   └── data/                       # Genotype breakdown, founder lineages & chi-square tests CSVs
│   │
│   ├── exp05_cs_cancer_interception/      # FIGURE 5 & FIGURE S5 (In Silico Cancer Interception)
│   │   ├── README.md                   # Matched cohort intervention design (30, 40, 50 years; 10-50% dose)
│   │   ├── run_interception.py         # 204k matched tissue condition runner
│   │   ├── plot_figure5_sfig5.py       # Plotting script for Figure 5 & Supplementary Figure S5
│   │   ├── Figure5.pdf / .png          # Main Figure 5 outputs (risk curves, ARR, dose-response)
│   │   ├── FigureS5.pdf / .png         # Supplementary Figure S5 outputs (D1-D2 interaction contrasts)
│   │   └── data/                       # Intervention results, ARR summaries & McNemar tests CSVs
│   │
│   ├── exp06_cs_sensitivity_structural/   # SUPPLEMENTARY FIGURES S2 & S3 (RESCUED SLICE)
│   │   ├── README.md                   # Structural & fixed-parameter sensitivity documentation
│   │   ├── run_structural_sensitivity.py # Fixed-parameter perturbations (±10%, ±25%) & TP53 reductions
│   │   ├── plot_sfig2_sfig3.py         # Plotting script for Supplementary Figures S2 & S3
│   │   ├── Supplementary_Figure_S2.pdf # Fixed parameter robustness & TP53 x D2 recalibration
│   │   ├── Supplementary_Figure_S2.png # Supplementary Figure S2 raster (300 DPI)
│   │   ├── Supplementary_Figure_S3.pdf # Structural sensitivity to kappa_BRCA1 & mu_TP53
│   │   ├── Supplementary_Figure_S3.png # Supplementary Figure S3 raster (300 DPI)
│   │   └── data/                       # Perturbation simulation evaluations & recalibrated traces
│   │
│   ├── exp07_ts_multiobjective_cohort/    # FIGURE 6 (Macro Cohort 10^7 Penetrance & Founder Lineages)
│   │   ├── README.md                   # Figure 6 methodology documentation
│   │   ├── run_validation.py           # 10,000 tissue stochastic simulation runner
│   │   ├── simulate_figure6_data.py    # Ad-hoc heterogeneity (sigma) and driver variation runner
│   │   ├── plot_figure6.py             # Complete 5-panel Figure 6 plotting script (Panels a–e)
│   │   ├── Figure6.pdf                 # Main publication vector figure
│   │   ├── Figure6.png                 # Main publication raster figure (300 DPI)
│   │   └── data/                       # 10k validation cohort traces, heterogeneity & driver data
│   │
│   ├── exp08_ts_homogeneous_restraint/    # SUPPLEMENTARY FIGURE S6 (Homogeneous Restraint & Trade-off)
│   │   ├── README.md                   # Homogeneous restraint documentation
│   │   ├── plot_sfig6.py               # Plotting script (homogeneous penetrance & early vs late SSE scatter)
│   │   ├── Supplementary_Figure_S6.pdf # Supplementary Figure S6 vector
│   │   ├── Supplementary_Figure_S6.png # Supplementary Figure S6 raster (300 DPI)
│   │   └── data/                       # 3k homogeneous traces & grid search source CSVs
│   │
│   └── exp09_ts_checkpoint_relaxation/    # SUPPLEMENTARY FIGURE S7 (Checkpoint Bypass & Driver Ordering)
│       ├── README.md                   # Checkpoint relaxation & permissive driver documentation
│       ├── run_checkpoint_grid.py      # Grid evaluator across gamma in [1e-5, 3e-4] & p_bypass in [0.25, 1.0]
│       ├── plot_sfig7.py               # Supplementary Figure S7 heatmaps (SSE & % enabled tumors)
│       ├── Supplementary_Figure_S7.pdf # Supplementary Figure S7 vector
│       ├── Supplementary_Figure_S7.png # Supplementary Figure S7 raster (300 DPI)
│       └── data/                       # Grid evaluation results CSV
│
└── results/
    └── paper_figures/                  # UNIFIED PUBLICATION DELIVERABLES DIRECTORY
        ├── Figure1_schematic.pdf       # Model overview conceptual schematic
        ├── Figure2.pdf                 # Figure 2: Exploratory pre-transformation vs acceleration
        ├── Figure3.pdf                 # Figure 3: Bayesian ABC-SMC calibration
        ├── Figure4.pdf                 # Figure 4: Emergent phenotypes & founder lineages
        ├── Figure5.pdf                 # Figure 5: In silico cancer interception
        ├── Figure6.pdf                 # Figure 6: Tissue-scale penetrance & founder complexity
        ├── FigureS1.pdf                # Supplementary Figure S1: Global sensitivity LHS & PRCC
        ├── FigureS2.pdf                # Supplementary Figure S2: Fixed parameter robustness
        ├── FigureS3.pdf                # Supplementary Figure S3: Structural sensitivity
        ├── FigureS4.pdf                # Supplementary Figure S4: Posterior parameter distributions
        ├── FigureS5.pdf                # Supplementary Figure S5: D1-D2 interception interaction
        ├── FigureS6.pdf                # Supplementary Figure S6: Homogeneous restraint trade-off
        └── FigureS7.pdf                # Supplementary Figure S7: Checkpoint relaxation heatmaps
```

### 3. Compilation and Build Contract
- `make build`: Must build both simulation engines in a single step via top-level CMake (or dual invocation):
  1. `cell_scale` binaries: `build/cell_scale/run_bootstrapping`, `run_paper_features`, `run_paper_interventions`, and `unit_tests`.
  2. `tissue_scale` binary: `build/tissue_scale/tissue_scale_sim`.
- `make test`: Runs `cell_scale` unit tests and `tissue_scale` regression equivalence tests.

### 4. Execution Interface Contract (Makefile)
- **Cell Scale Experiments (`cs_`)**:
  - `make exp01-cs-fig2`: Renders canonical Figure 2 (100-year horizon, $d=0.001$, representative curves + 36-model grid ensemble, 100% simulated).
  - `make exp01-cs-simulate`: Executes Monte Carlo simulations generating 100-year trajectories.
  - `make exp02-cs-sfig1`: Renders Supplementary Figure S1 (RF importance computed dynamically without hardcoded tables).
  - `make exp03-cs-fig3`: Renders Figure 3 (ABC-SMC convergence and posterior predictive).
  - `make exp03-cs-sfig4`: Renders Supplementary Figure S4 (Variant 4a: canonical 4x4 corner matrix).
  - `make exp03-cs-sfig4d`: Renders Supplementary Figure S4d (1D posterior density vs parameter pair ratios).
  - `make exp04-cs-fig4`: Renders Figure 4 (emergent features, clonal genotypes, founder lineages).
  - `make exp05-cs-fig5`: Renders Figure 5 & Figure S5 (cancer interception ARR and dose-response).
  - `make exp06-cs-sfig2`: Renders Supplementary Figure S2 (fixed parameter sensitivity).
  - `make exp06-cs-sfig3`: Renders Supplementary Figure S3 (structural sensitivity to $\kappa_{BRCA1}$ and $\mu_{TP53}$).
- **Tissue Scale Experiments (`ts_`)**:
  - `make exp07-ts-fig6`: Renders Figure 6 (Panels a–e, 5-panel publication layout).
  - `make exp08-ts-sfig6`: Renders Supplementary Figure S6 (homogeneous restraint penetrance and early vs late SSE trade-off).
  - `make exp09-ts-sfig7`: Renders Supplementary Figure S7 (heatmaps of $\gamma$ driver rate vs checkpoint-bypass probability $p_{\text{bypass}}$).
  - `make exp07-ts-simulate`: Runs 10k virtual cohort simulation for Figure 6.
  - `make exp09-ts-simulate`: Runs grid simulations for checkpoint relaxation heatmaps.
  - `make exp-smoke-all`: Quick execution check across all 9 experimental vertical slices ($<30$ seconds).

---

# 🪜 Phases

### Phase 1: Figure 2a Canonical Consolidation & Intermediate Trial Purge
*(Recommended model: Gemini 3.8 Flash Low)*
**Description:** Consolidate Figure 2 to its definitive canonical version (100-year horizon, $d=0.001$, 4 representative curves overlaid on the 36-model faint gray grid ensemble, 100% data-driven simulation). Completely purge all intermediate trial files, 12-variant scripts, redundant PDFs/PNGs, and temporary CSVs.
- [x] Ask and confirm with the user that the recommended model is active.
- [x] Refactor [`experiments/exp01_cs_exploratory_acceleration/run_exploratory.py`](file:///home/luis/CLionProjects/cellSim/experiments/exp01_cs_exploratory_acceleration/run_exploratory.py) to add a single clean `--max-t 100` mode that generates the representative 100-year trajectories and the 36-model grid trajectories without ad-hoc file renaming.
- [x] Refactor [`experiments/exp01_cs_exploratory_acceleration/plot_figure2.py`](file:///home/luis/CLionProjects/cellSim/experiments/exp01_cs_exploratory_acceleration/plot_figure2.py) into a clean, PEP8 script that renders only the canonical [`Figure2.pdf`](file:///home/luis/CLionProjects/cellSim/experiments/exp01_cs_exploratory_acceleration/Figure2.pdf) and `Figure2.png` (Panel a: 4 representative curves + 36 grid models at $d=0.001$ up to age 100; Panel b: basal vs accelerated expansion $\nu=0.10$).
- [x] Purge temporary trial artifacts from `experiments/exp01_cs_exploratory_acceleration/`:
  - Delete `panel_a_12_variants/` directory and `panel_a_12_variants.zip`.
  - Delete `generate_12_figures_panel_a.py`.
  - Delete scratch figure files (`Figure2_extra_grid_d0.01.*`, `Figure2_extra_grid.*`, `Figure2_100years.*`, `Figure2_extra_grid_100years.*`, `Figure2_simulated.*`, `Figure2_paper.*`).
  - Delete intermediate trial CSVs in `data/` (`*_d001.csv`, `*_d0.01_100y_n500.csv`, etc.).
- [x] Streamline Makefile targets to only: `exp01-cs-fig2`, `exp01-cs-simulate`, and `exp01-cs-smoke` (plus compatibility aliases).
- [x] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make exp01-cs-smoke && make exp01-cs-fig2`). Fix issues if any.
- [x] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

---

### Phase 2: Parameter Pair Ratios & Supplementary Figure S4 Modularization
*(Recommended model: Gemini 3.8 Flash Low)*
**Description:** Implement the boss's request: compute and visualize 1D posterior distributions (frequency / KDE) versus parameter ratios across all 6 parameter pairs. Modularize Supplementary Figure S4 variants (`4a` canonical corner plot, `4b` cross-parameter ratios, `4c` self-ratios, `4d` frequency vs ratio grid).
- [x] Ask and confirm with the user that the recommended model is active.
- [x] Add Variant 4d calculation in [`experiments/exp03_cs_abc_smc_calibration/plot_sfig4.py`](file:///home/luis/CLionProjects/cellSim/experiments/exp03_cs_abc_smc_calibration/plot_sfig4.py) for the 6 parameter pair ratios:
  - $R_1 = \delta_{\rm high} / \delta_{\rm low}$
  - $R_2 = \delta_{\rm low} / \beta$
  - $R_3 = \nu / \beta$
  - $R_4 = \nu / \delta_{\rm low}$
  - $R_5 = \delta_{\rm high} / \beta$
  - $R_6 = \nu / \delta_{\rm high}$
- [x] Render a publication-quality $2 \times 3$ grid figure (`FigureS4d.pdf` and `FigureS4d.png`) showing:
  - Eje X: Ratio value.
  - Eje Y: Weighted posterior density / frequency (SIS KDE).
  - Vertical solid line: Weighted posterior median.
  - Shaded area / dashed lines: 95% credible intervals.
- [x] Ensure `--variant 4a` continues to generate the canonical manuscript 4x4 corner matrix as `FigureS4.pdf`.
- [x] Add Makefile targets `exp03-cs-sfig4d` and keep `exp03-cs-sfig4` (which defaults to canonical `4a` or `--variant all` if explicitly requested).
- [x] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`python3 experiments/exp03_cs_abc_smc_calibration/plot_sfig4.py --variant 4d && make exp03-cs-sfig4`). Fix issues if any.
- [x] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

---

### Phase 3: Rescuing Missing Supplementary Figures S2 & S3 (Structural Sensitivity)
*(Recommended model: Gemini 3.8 Thinking Middle)*
**Description:** Integrate the missing structural and fixed-parameter sensitivity analyses from the original paper scripts into a clean vertical slice `experiments/exp06_cs_sensitivity_structural/` to reproduce Supplementary Figures S2 and S3 matching review PDFs.
- [x] Ask and confirm with the user that the recommended model is active.
- [x] Create directory structure `experiments/exp06_cs_sensitivity_structural/` (`data/`, `README.md`).
- [x] Import and beautify runner script `run_structural_sensitivity.py` (interfacing with `build/run_bootstrapping`):
  - Fixed-parameter sensitivity: $\pm 10\%$ and $\pm 25\%$ perturbations on $\mu_{TP53}, \theta_{D2}, \theta_{D1}, d, \kappa_{BRCA1}, A_{it}$.
  - Structural sensitivity: varying $\kappa_{BRCA1} \in [1, 3]$ and reducing $\mu_{TP53}$ threefold and tenfold.
  - Support `--smoke-test` mode completing in $<2$ seconds.
- [x] Implement plotting scripts `plot_sfig2.py` and `plot_sfig3.py`:
  - Supplementary Figure S2: Fixed parameter robustness (max penetrance shift, weighted SSE, and trajectory overlays matching review PDFs).
  - Supplementary Figure S3: Structural sensitivity to $\kappa_{BRCA1}$ and $\mu_{TP53}$ (matching review PDFs).
- [x] Wire Makefile targets: `exp06-cs-sfig2`, `exp06-cs-sfig3`, `exp06-cs-smoke`, and `exp06-cs-simulate`.
- [x] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make exp06-cs-smoke && make exp06-cs-sfig2 && make exp06-cs-sfig3`). Fix issues if any.
- [x] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

---

### Phase 4: Symmetrical Engine Layout Migration & Tissue-Scale Realignment (exp07_ts, exp08_ts, exp09_ts)
*(Recommended model: Gemini 3.8 Thinking Middle)*
**Description:** Establish the symmetric parallel architecture (Option A): migrate discrete simulation code into `cell_scale/`, package all tissue-scale experiment scripts and outputs into three dedicated vertical slices (`exp07_ts_multiobjective_cohort` for Fig 6, `exp08_ts_homogeneous_restraint` for Fig S6, and `exp09_ts_checkpoint_relaxation` for Fig S7), and unify CMake compilation for both simulation engines.
- [x] Ask and confirm with the user that the recommended model is active.
- [x] Migrate discrete simulation components into `cell_scale/` using `git mv` to preserve git history:
  - `git mv src cell_scale/src`
  - `git mv app cell_scale/app`
  - `git mv tests cell_scale/tests`
  - Create `cell_scale/CMakeLists.txt` building the cell-scale libraries and executables.
- [x] Create `experiments/exp07_ts_multiobjective_cohort/` (FIGURE 6):
  - Migrate and refactor `make_figure6_sfig6.py` $\to$ `experiments/exp07_ts_multiobjective_cohort/plot_figure6.py` (generating `Figure6.pdf` and `Figure6.png`).
  - Move runners (`run_validation.py`, `simulate_figure6_data.py`, `summarize_validation.py`, `score_multiobjective.py`) and 10k traces dataset into `experiments/exp07_ts_multiobjective_cohort/`.
- [x] Create `experiments/exp08_ts_homogeneous_restraint/` (SUPPLEMENTARY FIGURE S6):
  - Migrate and rename `make_sfig7.py` $\to$ `experiments/exp08_ts_homogeneous_restraint/plot_sfig6.py` (generating `Supplementary_Figure_S6.pdf` and `Supplementary_Figure_S6.png` matching `Palomero_SFig6_100126.pdf`).
  - Move 3k homogeneous traces and grid search source data into `experiments/exp08_ts_homogeneous_restraint/data/`.
- [x] Create `experiments/exp09_ts_checkpoint_relaxation/` (SUPPLEMENTARY FIGURE S7):
  - Implement `experiments/exp09_ts_checkpoint_relaxation/plot_sfig7.py` generating `Supplementary_Figure_S7.pdf` and `Supplementary_Figure_S7.png` matching `Palomero_SFig7_100126.pdf` (heatmaps of SSE and % enabled tumors across driver acquisition rate $\gamma \in [10^{-5}, 3\times 10^{-4}]$ and bypass probability $p_{\text{bypass}} \in [0.25, 1.0]$).
  - Add checkpoint grid simulation runner `run_checkpoint_grid.py`.
- [x] Keep `tissue_scale/` strictly as the macro simulation engine backend (`src/`, `configs/`, `tests/`, and `CMakeLists.txt`).
- [x] Delete orphaned/superseded scripts from old tissue-scale locations (`make_figures.py`, `make_sfig5.py`).
- [x] Update root `CMakeLists.txt` to include both subdirectories:
  ```cmake
  add_subdirectory(cell_scale)
  add_subdirectory(tissue_scale)
  ```
- [x] Update Makefile targets: `exp07-ts-fig6`, `exp08-ts-sfig6`, `exp09-ts-sfig7`, and their respective simulate/smoke targets.
- [x] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make build && make test && make exp07-ts-smoke && make exp08-ts-smoke && make exp09-ts-smoke`). Fix issues if any.
- [x] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

---

### Phase 5: Centralized Results Synchronization, Vector EPS Export & Data Pipeline Integrity
*(Recommended model: Gemini 3.8 Flash Low)*
**Description:** Centralize all publication deliverables into `results/paper_figures/`, export vector EPS alongside PDF and PNG for journal publication requirements, fix figure layout gaps (e.g. S4d subtitle spacing), and resolve relative data script paths across all vertical slices.
- [x] Ask and confirm with the user that the recommended model is active.
- [x] Implement vector EPS export across all 15 figures (main Figures 2–6 and Supplementary Figures S1–S7).
- [x] Resolve vertical spacing conflict in Supplementary Figure S4d (adjust `top=0.84`, `hspace=0.42` leaving 49px clean separation).
- [x] Fix relative directory defaults in `exp02_cs_sensitivity_global`, `exp04_cs_emergent_features`, and `exp05_cs_cancer_interception`.
- [x] Create `results/paper_figures/export_all_figures.py` and synchronize 45 publication-grade files with 0 missing.
- [x] Add `make paper-figures` and `make exp-smoke-all` to root `Makefile`.
- [x] Verify that all smoke tests pass across all experiments (`make exp-smoke-all` in <15s).
- [x] Verify the suite of 144 unit tests passes at 100% (`make test`).
- [x] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

---

### Phase 6: Repository Final Polish & Master Documentation
*(Recommended model: Gemini 3.8 Thinking Middle)*
**Description:** Update repository master documentation `README.md`, perform cross-checks, review status of `archive_backup/`, and conclude reproducibility remedy.
- [x] Ask and confirm with the user that the recommended model is active.
- [x] Update root `README.md` documenting the symmetric architecture (`cell_scale/` and `tissue_scale/`), the 9 experiment slices, and commands to run smoke tests and reproduce all paper figures.
- [x] Request explicit user confirmation to permanently delete `archive_backup/`.
- [x] Remove `archive_backup/` upon confirmation.
- [x] Verify clean repository state (`git status`, `make test`, `make exp-smoke-all`, `make paper-figures`).
- [x] Present final completion summary to the user.

---

# 🏁 Concluded & Verified

All 6 phases of the remediation plan have been executed, verified, and committed. The repository is ready for official publication release.

Plan verified and guided by 🐢 💨 (Turbotuga™, [Codely](https://codely.com)’s mascot) 🔍 📦 📊 ✨.
