---
name: "vertical_refactor_paper_reproducibility"
description: "Vertical refactoring of cellSim and tissue_scale experiments, code beautification, and publication pipeline for BRCA1-ONSET paper reproducibility"
created_at: "2026-10-25T17:47:00Z"

created_by:
  tool: "Antigravity CLI"
  model:
    name: "Gemini"
    version: "3.8 Flash (Low)"
    reasoning_effort: "low"
---

# 🎯 Goal

Refactor the repository into a clean, publication-ready structure with two clearly decoupled modules: `cell_scale/` (the discrete agent-based CellSim engine for 500 cells and Figures 2–5 + S1–S5) and `tissue_scale/` (the macro-scale stochastic cohort engine for $10^7$ cells and Figures 6 + S6–S7). All code (C++20 and Python) will be beautified and explicitly documented according to the manuscript equations, legacy components will be quarantined in `archive_backup/` until final validation, and the Git history will be structured for release.

Execution follows an optimized **Architect vs. Worker** model strategy (Middle/High reasoning for critical plan review and algorithmic refactoring; Low/Flash for mechanical file ops and script wiring) to maximize technical accuracy while minimizing token cost.

---

# 👀 Context

- **Manuscript Source**: [`/home/luis/Escritorio/abms/v4/Palomero_BRCA1_102126.txt`](file:///home/luis/Escritorio/abms/v4/Palomero_BRCA1_102126.txt) and [`Palomero_BRCA1_102126.docx`](file:///home/luis/Escritorio/abms/v4/Palomero_BRCA1_102126.docx).
- **Target Manuscript Figures**: [`/home/luis/Escritorio/abms/v4/MainFigs_review_102126`](file:///home/luis/Escritorio/abms/v4/MainFigs_review_102126) and [`/home/luis/Escritorio/abms/v4/SupplFig-Table-review_102126`](file:///home/luis/Escritorio/abms/v4/SupplFig-Table-review_102126).
- **Cell-Scale Codebase**: [`src/domain/`](file:///home/luis/CLionProjects/cellSim/src/domain), [`src/application/`](file:///home/luis/CLionProjects/cellSim/src/application), [`tests/`](file:///home/luis/CLionProjects/cellSim/tests).
- **Paper Reproducibility Assets**: [`/home/luis/Escritorio/abms/v2_BRCA1-ONSET_Ms_092426/BRCA1_ONSET_reproducibility`](file:///home/luis/Escritorio/abms/v2_BRCA1-ONSET_Ms_092426/BRCA1_ONSET_reproducibility).
- **Tissue-Scale Source**: [`/home/luis/Escritorio/abms/v2_BRCA1-ONSET_Ms_092426/code_tissue-scale/code_tissue-scale/BRCA1_ONSET_tissue_scale_GitHub`](file:///home/luis/Escritorio/abms/v2_BRCA1-ONSET_Ms_092426/code_tissue-scale/code_tissue-scale/BRCA1_ONSET_tissue_scale_GitHub).
- **Audit Documentation**: [`docs/CODEBASE_MANUSCRIPT_AUDIT.md`](file:///home/luis/CLionProjects/cellSim/docs/CODEBASE_MANUSCRIPT_AUDIT.md).

---

# 🤖 Execution & Model Allocation Strategy (Cost-Complexity-Tokens)

To balance rigorous scientific consistency against token and computation budgets:
- **Architect Role (Deep Reasoning - Gemini 3.8 Middle / High)**:
  - Phase 0: Critical plan review & mathematical consistency audit.
  - Phase 7: Algorithm beautification & stochastic decomposition of `tissue_scale_sim.cpp`.
  - Phase 8: Final regression validation against review PDFs.
- **Worker Role (Fast & Token Efficient - Gemini 3.8 Flash Low)**:
  - Phase 1: File movement, directory scaffolding, legacy quarantining.
  - Phases 2, 3, 4, 5, 6: Mechanical python runners, boilerplate wrappers, and Makefile wiring.
- **Specialist Escalation (4.0 Argon)**:
  - Reserved strictly as fallback if subtle OpenMP race conditions or complex C++ template regressions occur.

---

# 📜 Public Contracts & Architectural Interfaces

### 1. Repository Directory Contract
The repository will contain two independent core simulation components and a vertical experiment layout:
```
cellSim/
├── Makefile                          # Top-level orchestrator (build, test, paper-figures)
├── README.md                         # Publication & reproducibility guide
├── archive_backup/                   # Temporary quarantine for superseded code & traces
│
├── cell_scale/                       # Discrete Agent-Based Model (500 cells, CellSim framework)
│   ├── CMakeLists.txt                # Independent CMake build for cell_scale
│   ├── src/                          # Domain & application engine
│   ├── tests/                        # GoogleTest suite
│   ├── app/                          # C++ runners (run_bootstrapping, run_paper_features, run_paper_interventions)
│   └── experiments/                  # Vertical slices per experiment & figure
│       ├── exp01_exploratory_acceleration/ # Figure 2
│       ├── exp02_sensitivity_global/       # Supplementary Figure S1
│       ├── exp03_abc_smc_calibration/      # Figure 3 & Supplementary Figure S4
│       ├── exp04_emergent_features/        # Figure 4
│       ├── exp05_cancer_interception/      # Figure 5 & Supplementary Figure S5
│       └── exp06_sensitivity_structural/   # Supplementary Figures S2 & S3
│
└── tissue_scale/                     # Macro-Scale Stochastic Model (10^7 cells, cohort-based)
    ├── Makefile                      # Dedicated compilation & execution Makefile
    ├── src/
    │   └── tissue_scale_sim.cpp      # Beautified, self-documenting C++20 OpenMP engine
    ├── scripts/
    │   ├── make_figures.py           # Figure 6 & Figure S6
    │   ├── make_sfig5.py             # Figure S7 (alternative event ordering)
    │   └── run_validation.py         # Multi-objective & homogeneous validation
    ├── data/                         # Target penetrance & empirical validation data
    └── results/                      # Output validation traces & generated figures
```

### 2. Code Beautification & Style Contract
- **C++ Standards**: All C++ code in `tissue_scale/` and `cell_scale/` will use readable variable names matching paper notation ($\beta$, $\mu$, $\delta_{\text{low}}$, $\delta_{\text{high}}$, $\theta_{D1}$, $\theta_{D2}$, $\tau_T$, $\sigma_{\theta_{D2}}$), explicit types, and in-code formula citations from Methods.
- **Python Scripts**: PEP8 compliant, modular functions with typed docstrings, argparse interfaces, and unified publication-quality matplotlib styling matching the journal specifications.

### 3. Execution Interface Contract (`Makefile`)
- Root targets:
  - `make build`: Compiles `cell_scale` and `tissue_scale`.
  - `make test`: Runs `cell_scale` unit tests and `tissue_scale` sanity test.
  - `make exp01-fig2` ... `make exp06-sfig2-3`: Runs specific `cell_scale` experiments and renders figures.
  - `make exp07-fig6-tissue`: Runs `tissue_scale` simulations and renders Figures 6, S6, and S7.
  - `make paper-figures-all`: End-to-end execution of all paper figures.

---

# 🪜 Phases

### Phase 0: Critical Plan Review & Mathematical Cross-Check
*(Recommended model: Gemini 3.8 Thinking Middle or High)*
**Description:** Perform a deep-reasoning verification pass across manuscript equations, parameter priors, confidence intervals, and C++ runner interfaces to preempt any scientific regressions before file operations begin.
- [ ] Cross-check parameter bounds ($\beta, \delta_{\text{low}}, \delta_{\text{high}}, \nu, \mu$) in `run_bootstrapping.cpp` and `exp_01_abc_smc_global.py` against text lines 112–136 of `Palomero_BRCA1_102126.txt`.
- [ ] Confirm alignment of $SSE_w$ formula: $\sum \left(\frac{R_{\text{sim}}(a) - R_{\text{obs}}(a)}{\sigma_a}\right)^2$ with Kuchenbaecker 2017 CI weights.
- [ ] Audit `run_paper_features.cpp` and `run_paper_interventions.cpp` to verify exact signature match with `src/domain/` interfaces.
- [ ] Verify that all 6 main figures and 7 supplementary figures have a dedicated, identified generation path.
- [ ] Present audit findings and refine plan parameters if discrepancies are found.
- [ ] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

### Phase 1: Quarantine Backup & Repository Restructuring
*(Recommended model: Gemini 3.8 Flash Low)*
**Description:** Isolate all legacy apps, intermediate scripts, and historical traces into `archive_backup/`. Move active cell simulation components into `cell_scale/` and verify that compilation and tests pass cleanly without breaking any dependencies.
- [ ] Create `archive_backup/` directory structure (`archive_backup/app`, `archive_backup/scripts`, `archive_backup/traces`, `archive_backup/results`).
- [ ] Move legacy apps (`main_interactive.cpp`, `single_cell_evolution.cpp`, `run_all_scenarios.cpp`, `calibration_sweep.cpp`, `main_cli.cpp`, `main.cpp`) into `archive_backup/app/`.
- [ ] Move obsolete python scripts (`scripts/abc_final_v4.py`, `scripts/analyse_abc_final.py`, `scripts/analyze_bimodality.py`, `scripts/generate_figures.py`, `scripts/generate_paper7_figures.py`, `scripts/paper_figures_gen2.py`, `scripts/paper_figures_v4.py`, `scripts/generar_memoria.py`, `scripts/qa_report.py`) into `archive_backup/scripts/`.
- [ ] Move historical traces (`traces/`) and older result runs (`results/abc_*`, `results/paper*`) into `archive_backup/`.
- [ ] Set up `cell_scale/` folder containing `src/`, `tests/`, and active `app/` files (`run_bootstrapping.cpp`).
- [ ] Configure `cell_scale/CMakeLists.txt` and root `Makefile` so that `make build` and `make test` pass.
- [ ] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make build && make test`). Fix issues if any.
- [ ] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

### Phase 2: Vertical Slice 1 – Exploratory Grid & Tumoral Acceleration (Figure 2)
*(Recommended model: Gemini 3.8 Flash Low)*
**Description:** Build the vertical slice for the exploratory grid search ($\beta$ vs $\delta_{\text{low}}$) comparing basal division ($\nu = d$) vs accelerated tumoral expansion ($\nu = 0.10$).
- [ ] Create directory `cell_scale/experiments/exp01_exploratory_acceleration/`.
- [ ] Add clean, beautified runner script `run_exploratory.py` interfacing with `run_bootstrapping`.
- [ ] Add publication-ready plotting script `plot_figure2.py` matching Figure 2 against Kuchenbaecker et al. (2017) clinical targets.
- [ ] Add Makefile target `exp01-fig2`.
- [ ] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make exp01-fig2` smoke test). Fix issues if any.
- [ ] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

### Phase 3: Vertical Slice 2 – Global Sensitivity Analysis LHS & PRCC (Supplementary Figure S1)
*(Recommended model: Gemini 3.8 Flash Low)*
**Description:** Implement the Latin Hypercube Sampling (LHS) global sensitivity analysis across 1,000 parameter combinations evaluated on 200 simulated tissues.
- [ ] Create directory `cell_scale/experiments/exp02_sensitivity_global/`.
- [ ] Add beautified LHS sampler and batch runner `run_lhs_sensitivity.py`.
- [ ] Add feature ranking and plotting script `plot_sfig1.py` (Random Forest regression importance and PRCC metrics).
- [ ] Add Makefile target `exp02-sfig1`.
- [ ] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make exp02-sfig1` smoke test). Fix issues if any.
- [ ] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

### Phase 4: Vertical Slice 3 – Bayesian ABC-SMC Calibration & Posterior Predictive (Figure 3 & Figure S4)
*(Recommended model: Gemini 3.8 Flash Low)*
**Description:** Implement the canonical 4-parameter ABC-SMC calibration pipeline (20 generations, 250 particles, adaptive $Q_{0.75}$ tolerance, weighted SSE).
- [ ] Create directory `cell_scale/experiments/exp03_abc_smc_calibration/`.
- [ ] Consolidate and beautify the ABC-SMC algorithm into `abc_smc.py` using `run_bootstrapping` for particle scoring.
- [ ] Add convergence and posterior predictive generation script `plot_figure3.py` (Figure 3: tolerance, SSE IQR, ESS, PPC penetrance curve).
- [ ] Add joint and marginal posterior distribution plotting script `plot_sfig4.py` (Figure S4: KDEs, medians, 95% CIs, pairwise correlations).
- [ ] Add Makefile target `exp03-fig3`.
- [ ] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make exp03-fig3` dry-run). Fix issues if any.
- [ ] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

### Phase 5: Vertical Slice 4 – Emergent Tumor Features & Clonal Lineages (Figure 4)
*(Recommended model: Gemini 3.8 Flash Low)*
**Description:** Integrate `run_paper_features.cpp` into `cell_scale/app/` and package the evaluation of emergent tumor properties across the posterior population.
- [ ] Add `cell_scale/app/run_paper_features.cpp` and update `cell_scale/CMakeLists.txt`.
- [ ] Create directory `cell_scale/experiments/exp04_emergent_features/`.
- [ ] Add beautified runner script `run_emergent_features.py` executing `run_paper_features` across the 250 retained posterior combinations.
- [ ] Add plotting script `plot_figure4.py` generating Figure 4 (posterior parameter correlations, $BRCA1^{+/-}$ vs. $BRCA1^{-/-}$ composition, founder lineages vs age).
- [ ] Add Makefile target `exp04-fig4`.
- [ ] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make exp04-fig4` smoke test). Fix issues if any.
- [ ] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

### Phase 6: Vertical Slice 5 – Cancer Interception & Attenuation Protocols (Figure 5 & Supplementary Figure S5)
*(Recommended model: Gemini 3.8 Flash Low)*
**Description:** Integrate `run_paper_interventions.cpp` to evaluate $D_1$ and $D_2$ attenuation (10%, 25%, 50%) initiated at ages 30, 40, and 50.
- [ ] Add `cell_scale/app/run_paper_interventions.cpp` and update `cell_scale/CMakeLists.txt`.
- [ ] Create directory `cell_scale/experiments/exp05_cancer_interception/`.
- [ ] Add runner script `run_interception.py` evaluating matched no-intervention vs intervention cohorts.
- [ ] Add plotting script `plot_figure5_sfig5.py` generating Figure 5 (trajectories, absolute risk reduction, dose-response) and Figure S5 (interaction contrasts).
- [ ] Add Makefile target `exp05-fig5`.
- [ ] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make exp05-fig5` smoke test). Fix issues if any.
- [ ] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

### Phase 7: Tissue-Scale Module Refactoring & Code Beautification (Figure 6, Figure S6, Figure S7)
*(Recommended model: Gemini 3.8 Thinking Middle)*
**Description:** Import `tissue-scale` into `tissue_scale/`, completely refactor and beautify `tissue_scale_sim.cpp` for high readability, and package Figures 6, S6, and S7.
- [ ] Create directory structure `tissue_scale/` (`src/`, `scripts/`, `data/`, `results/`).
- [ ] Refactor and beautify `tissue_scale/src/tissue_scale_sim.cpp`: expand compressed variable names into explicit domain names, format with clean C++20 conventions, and add formula documentation referencing Methods.
- [ ] Add dedicated `tissue_scale/Makefile` for compiling `tissue_scale_sim` with OpenMP optimization.
- [ ] Import and beautify python scripts: `make_figures.py` (Figure 6 & S6), `make_sfig5.py` (Figure S7), and `run_validation.py`.
- [ ] Add root Makefile targets `exp07-fig6-tissue` and `tissue-scale-test`.
- [ ] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make exp07-fig6-tissue` validation run). Fix issues if any.
- [ ] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

### Phase 8: End-to-End Validation, Backup Purge & Git History Recipe
*(Recommended model: Gemini 3.8 Thinking Middle for validation, Flash Low for cleanup)*
**Description:** Validate all figures against `/home/luis/Escritorio/abms/v4/`, permanently purge `archive_backup/` upon user approval, and prepare the clean Git history for publication.
- [ ] Run full test suite and validation scripts for both `cell_scale` and `tissue_scale`.
- [ ] Verify that all generated figures match the manuscript review PDFs in `MainFigs_review_102126` and `SupplFig-Table-review_102126`.
- [ ] Request explicit user confirmation to purge `archive_backup/`.
- [ ] Remove `archive_backup/` upon confirmation.
- [ ] Write clear documentation on how to run experiments and provide the Git commit/rebase history recipe for publishing to GitHub.
- [ ] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`git status` and `make test`). Fix issues if any.
- [ ] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

---

# ⏭️ Next Step

The next step is to execute **Phase 0: Critical Plan Review & Mathematical Cross-Check** (ideally using Gemini 3.8 Middle or High) to audit parameter constraints, formulas, and runner signatures against `Palomero_BRCA1_102126.txt`.

Plan created by 🐢 💨 (Turbotuga™, [Codely](https://codely.com)’s mascot).
