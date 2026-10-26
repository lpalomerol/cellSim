---
name: "vertical_refactor_paper_reproducibility"
description: "Vertical refactoring of cellSim and tissue_scale experiments, code beautification, and publication pipeline for BRCA1-ONSET paper reproducibility"
created_at: "2026-10-25T17:49:00Z"

created_by:
  tool: "Antigravity CLI"
  model:
    name: "Gemini"
    version: "3.8 Flash (Low)"
    reasoning_effort: "low"

implemented_by:
  tool: "Antigravity CLI"
  model:
    name: "Gemini"
    version: "3.8 Flash (Medium)"
    reasoning_effort: "medium"

last_implementation_at: "2026-10-26T08:01:00Z"
has_completed_all_phases: false
---

# 🎯 Goal

Refactor the repository into a clean, publication-ready structure with two clearly decoupled modules: `cell_scale/` (the discrete agent-based CellSim engine for 500 cells and Figures 2–5 + S1–S5) and `tissue_scale/` (the macro-scale stochastic cohort engine for $10^7$ cells and Figures 6 + S6–S7). All code (C++20 and Python) will be beautified and explicitly documented according to the manuscript equations, legacy components will be quarantined in `archive_backup/` until final validation, and the Git history will be structured for release.

To avoid hours of costly simulation re-runs, the pipeline relies on existing calibrated posterior data assets for instant figure generation, using fast smoke tests (`--dry-run` or minimal particle pilots) strictly to verify pipeline integrity. Furthermore, **explicit user confirmation is required before starting each phase to ensure the recommended AI model is selected**.

---

# 👀 Context

- **Manuscript Source**: [`/home/luis/Escritorio/abms/v4/Palomero_BRCA1_102126.txt`](file:///home/luis/Escritorio/abms/v4/Palomero_BRCA1_102126.txt) and [`Palomero_BRCA1_102126.docx`](file:///home/luis/Escritorio/abms/v4/Palomero_BRCA1_102126.docx).
- **Target Manuscript Figures**: [`/home/luis/Escritorio/abms/v4/MainFigs_review_102126`](file:///home/luis/Escritorio/abms/v4/MainFigs_review_102126) and [`/home/luis/Escritorio/abms/v4/SupplFig-Table-review_102126`](file:///home/luis/Escritorio/abms/v4/SupplFig-Table-review_102126).
- **Calibrated Data Assets (Do Not Recompute)**: Existing posterior populations and validation traces in `results/20260928_draft_map/`, `BRCA1_ONSET_reproducibility/`, and `code_tissue-scale/final_multiobjective_validation_10000.csv`.
- **Cell-Scale Codebase**: [`src/domain/`](file:///home/luis/CLionProjects/cellSim/src/domain), [`src/application/`](file:///home/luis/CLionProjects/cellSim/src/application), [`tests/`](file:///home/luis/CLionProjects/cellSim/tests).
- **Audit Documentation**: [`docs/CODEBASE_MANUSCRIPT_AUDIT.md`](file:///home/luis/CLionProjects/cellSim/docs/CODEBASE_MANUSCRIPT_AUDIT.md).

---

# 🤖 Execution & Model Allocation Strategy (Cost-Complexity-Tokens)

### Model Allocation Policy
Before each phase begins, the agent **MUST request explicit confirmation** from the user to verify that the appropriate model is active in the IDE:
- **Architect Role (Deep Reasoning - Gemini 3.8 Middle / High)**:
  - Phase 0: Critical plan review & mathematical consistency audit.
  - Phase 7: Algorithm beautification & stochastic decomposition of `tissue_scale_sim.cpp`.
  - Phase 8: Final regression validation against review PDFs.
- **Worker Role (Fast & Token Efficient - Gemini 3.8 Flash Low)**:
  - Phase 1: File movement, directory scaffolding, legacy quarantining.
  - Phases 2, 3, 4, 5, 6: Mechanical python runners, boilerplate wrappers, and Makefile wiring.
- **Specialist Escalation (4.0 Argon)**:
  - Fallback strictly for complex C++ template regressions or subtle OpenMP race conditions.

### Simulation Runtime & Calibration Policy (Fast Verification vs Heavy Computation)
Full ABC-SMC calibration runs take hours on multi-core clusters (20 generations × 250 particles × 300 tissues = 1.5M tissue simulations).
1. **Preserve Existing Calibrated Data**: All scripts will accept `--data-file` or load pre-existing calibrated outputs from the paper freeze package.
2. **Fast Smoke Tests Only**: Verification commands in all phases will use `--dry-run`, `--quick`, or minimal particle limits ($N=5$ particles, 1 generation, 5 tissues) taking $< 10$ seconds to prove code wiring and CLI argument parsing work without launching full campaigns.
3. **Full Run Targets Marked Optional**: Targets for full execution (e.g. `make exp03-abc-full`) will be separated from figure rendering targets (e.g. `make exp03-fig3` using existing posteriors).

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
│   ├── data/                         # Pre-calibrated ABC posteriors & Kuchenbaecker targets
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
    ├── data/                         # Empirical penetrance & precomputed 10k validation traces
    └── results/                      # Generated publication figures
```

### 2. Code Beautification & Style Contract
- **C++ Standards**: All C++ code in `tissue_scale/` and `cell_scale/` will use readable variable names matching paper notation ($\beta$, $\mu$, $\delta_{\text{low}}$, $\delta_{\text{high}}$, $\theta_{D1}$, $\theta_{D2}$, $\tau_T$, $\sigma_{\theta_{D2}}$), explicit types, and in-code formula citations from Methods.
- **Python Scripts**: PEP8 compliant, modular functions with typed docstrings, argparse interfaces, and unified publication-quality matplotlib styling matching the journal specifications.

### 3. Execution Interface Contract (`Makefile`)
- Root targets:
  - `make build`: Compiles `cell_scale` and `tissue_scale`.
  - `make test`: Runs `cell_scale` unit tests and `tissue_scale` sanity test.
  - `make bootstrap-quick`: Fast Monte Carlo test ($N=50$ tissues, ~1s) evaluating penetrance vs Kuchenbaecker.
  - `make bootstrap-paper`: Canonical Monte Carlo evaluation ($N=1,000$ tissues) with posterior parameters.
  - `make exp01-fig2` ... `make exp06-sfig2-3`: Generates figures using pre-computed calibrated data (instant).
  - `make exp07-fig6-tissue`: Generates tissue-scale figures from precomputed validation traces (instant).
  - `make smoke-tests`: Fast execution check of all binaries and scripts ($< 30$ seconds total).
  - `make paper-figures-all`: End-to-end rendering of all 6 main and 7 supplementary figures.

---

# 🪜 Phases

### Phase 0: Critical Plan Review & Mathematical Cross-Check
*(Recommended model: Gemini 3.8 Thinking Middle or High)*
**Description:** Deep-reasoning verification pass across manuscript equations, parameter priors, confidence intervals, and C++ runner interfaces to preempt any scientific regressions before file operations begin.
- [x] Ask and confirm with the user that the recommended model (Gemini 3.8 Middle or High) is active.
- [x] Cross-check parameter bounds ($\beta, \delta_{\text{low}}, \delta_{\text{high}}, \nu, \mu$) in `run_bootstrapping.cpp` and `exp_01_abc_smc_global.py` against text lines 112–136 of `Palomero_BRCA1_102126.txt`.
- [x] Confirm alignment of $SSE_w$ formula: $\sum \left(\frac{R_{\text{sim}}(a) - R_{\text{obs}}(a)}{\sigma_a}\right)^2$ with Kuchenbaecker 2017 CI weights.
- [x] Audit `run_paper_features.cpp` and `run_paper_interventions.cpp` to verify exact signature match with `src/domain/` interfaces.
- [x] Verify that all 6 main figures and 7 supplementary figures have a dedicated, identified generation path.
- [x] Verify the presence of all pre-calibrated data files so full ABC runs are never required to generate figures.
- [x] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

### Phase 1: Quarantine Backup & Repository Restructuring
*(Recommended model: Gemini 3.8 Flash Low)*
**Description:** Isolate all legacy apps, intermediate scripts, and historical traces into `archive_backup/`. Move active cell simulation components into `cell_scale/` and verify that compilation and tests pass cleanly without breaking any dependencies.
- [x] Ask and confirm with the user that the recommended model (Gemini 3.8 Flash Low) is active.
- [x] Create `archive_backup/` directory structure (`archive_backup/app`, `archive_backup/scripts`, `archive_backup/traces`, `archive_backup/results`).
- [x] Move legacy apps (`main_interactive.cpp`, `single_cell_evolution.cpp`, `run_all_scenarios.cpp`, `calibration_sweep.cpp`, `main_cli.cpp`, `main.cpp`) into `archive_backup/app/`.
- [x] Move obsolete python scripts (`scripts/abc_final_v4.py`, `scripts/analyse_abc_final.py`, `scripts/analyze_bimodality.py`, `scripts/generate_figures.py`, `scripts/generate_paper7_figures.py`, `scripts/paper_figures_gen2.py`, `scripts/paper_figures_v4.py`, `scripts/generar_memoria.py`, `scripts/qa_report.py`) into `archive_backup/scripts/`.
- [x] Move historical traces (`traces/`) and older result runs (`results/abc_*`, `results/paper*`) into `archive_backup/`.
- [x] Clean and optimize `CMakeLists.txt` and root `Makefile` so that `make build` and `make test` pass 100% cleanly.
- [x] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make build && make test`). Fix issues if any.
- [x] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

### Phase 2: Vertical Slice 1 – Exploratory Grid & Tumoral Acceleration (Figure 2)
*(Recommended model: Gemini 3.8 Flash Low)*
**Description:** Build the vertical slice for the exploratory grid search ($\beta$ vs $\delta_{\text{low}}$) comparing basal division ($\nu = d$) vs accelerated tumoral expansion ($\nu = 0.10$).
- [x] Ask and confirm with the user that the recommended model (Gemini 3.8 Flash Low) is active.
- [x] Create directory `experiments/exp01_exploratory_acceleration/` with dedicated documentation and data subfolder.
- [x] Add clean, beautified runner script `run_exploratory.py` interfacing with `run_bootstrapping` (supporting instant plotting via precomputed CSV and `--smoke-test` for quick runs).
- [x] Add publication-ready plotting script `plot_figure2.py` matching Figure 2 against Kuchenbaecker et al. (2017) clinical targets.
- [x] Add Makefile targets `exp01-fig2`, `exp01-fig2-grid`, `exp01-fig2-100years`, `exp01-fig2-grid-100years`, `exp01-simulate` and `exp01-smoke`.
- [x] **Debate científico y resolución cinética documentada (26-Oct-2026):**
  - *Discrepancia resuelta:* En versiones preliminares (`docs/paper3/fig1_initial_vs_calibrated.png`) la curva sobrepasaba el target clínico porque tenía `big_bang = true` ($\nu=0.10$) con deltas no restringidos ($\delta_{\text{high}} = 2.67 \delta_{\text{low}}$). En la Figura 2a actual, todas las curvas quedan por debajo entre los 30 y 60 años porque representan estrictamente la división basal ($\nu = d = 0.001$, control negativo del paper).
  - *Imposibilidad del ajuste preneoplásico puro:* Con solo $\beta$ y $\delta_{\text{low}}$ (con la restricción estructural $\delta_{\text{high}} = 2 \delta_{\text{low}}$), es imposible ajustar simultáneamente el despegue temprano a los 40 años (26%) y la meseta a los 80 años (70%).
  - *Mecanismo biológico de control:* $D_1$ ($\theta_{D1}=2.0$) rige la entrada a *Primed*; $D_2$ ($\theta_{D2}=5.0$) determina la evasión del aclaramiento tisular; y $TP53$ funcional purga las células con mutación temprana de $BRCA1$ por apoptosis intrínseca, lo que evita positivos tempranos pero estanca a $\beta=0.060$ en ~48% por agotamiento del pool celular.
  - *Simulación a 100 años:* Se evaluó el horizonte ampliado hasta los 100 años ($N=1.000$ tejidos por curva y grid completo de 36 combinaciones), demostrando que las curvas verde ($\beta=0.010$) y roja ($\beta=0.040$) son fuertemente tardías y explosivas (llegan a 98.9% y 88.2% a los 100 años), mientras que la naranja permanece bloqueada en 49.9%. El modelo calibrado con aceleración ($\nu=0.10$) alcanza una meseta fisiológica perfecta en 70.9% (no-penetranza ~29%).
- [x] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make exp01-smoke && make exp01-fig2 && make exp01-fig2-100years`).
- [x] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

### Phase 3: Vertical Slice 2 – Global Sensitivity Analysis LHS & PRCC (Supplementary Figure S1)
*(Recommended model: Gemini 3.8 Flash Low)*
**Description:** Implement the Latin Hypercube Sampling (LHS) global sensitivity analysis across 1,000 parameter combinations evaluated on 200 simulated tissues.
- [x] Ask and confirm with the user that the recommended model (Gemini 3.8 Flash Low) is active.
- [x] Create directory `experiments/exp02_sensitivity_global/` with precalibrated assets (`data/`).
- [x] Add beautified LHS sampler and batch runner `run_lhs_sensitivity.py` (with fast `--smoke-test` mode completing in <0.1s).
- [x] Add publication-grade plotting script `plot_sfig1.py` generating all 3 panels of Supplementary Figure S1 matching manuscript PDF `Palomero_SFig1_101526.pdf` (Panel a: Random Forest relative nonlinear importance, Panel b: age-specific PRCC heatmap, Panel c: 3x3 grid of posterior ECDFs vs uniform priors).
- [x] Add Makefile targets `exp02-sfig1`, `exp02-smoke`, and `exp02-simulate`.
- [x] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make exp02-smoke && make exp02-sfig1 && make test`). Fix issues if any.
- [x] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

### Phase 4: Vertical Slice 3 – Bayesian ABC-SMC Calibration & Posterior Predictive (Figure 3 & Figure S4)
*(Recommended model: Gemini 3.8 Flash Low)*
**Description:** Implement the canonical 4-parameter ABC-SMC calibration pipeline (20 generations, 250 particles, adaptive $Q_{0.75}$ tolerance, weighted SSE).
- [x] Ask and confirm with the user that the recommended model (Gemini 3.8 Flash Low) is active.
- [x] Create directory `experiments/exp03_abc_smc_calibration/` with precalibrated assets (`data/`).
- [x] Consolidate and beautify the ABC-SMC algorithm into `abc_smc.py` (with fast `--smoke-test` completing in ~0.2s).
- [x] Add convergence and posterior predictive generation script `plot_figure3.py` (Figure 3: tolerance, SSE IQR, ESS, PPC penetrance curve vs Kuchenbaecker 2017).
- [x] Add joint and marginal posterior distribution plotting script `plot_sfig4.py` (Figure S4: KDEs, medians, 95% CIs, pairwise contours).
- [x] Add Makefile targets `exp03-fig3`, `exp03-sfig4`, `exp03-smoke`, `exp03-abc-pilot`, and `exp03-simulate`.
- [x] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make exp03-fig3 && make exp03-sfig4 && make exp03-smoke && make test`).
- [x] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

### Phase 5: Vertical Slice 4 – Emergent Tumor Features & Clonal Lineages (Figure 4)
*(Recommended model: Gemini 3.8 Flash Low)*
**Description:** Integrate `run_paper_features.cpp` into `cell_scale/app/` and package the evaluation of emergent tumor properties across the posterior population.
- [x] Ask and confirm with the user that the recommended model (Gemini 3.8 Flash Low) is active.
- [x] Add `app/run_paper_features.cpp` and update `CMakeLists.txt`.
- [x] Create directory `experiments/exp04_emergent_features/` with precalibrated assets (`data/`).
- [x] Add beautified runner script `run_emergent_features.py` (supporting precomputed summaries and fast `--smoke-test` in 0.05s).
- [x] Add plotting script `plot_figure4.py` generating Figure 4 (posterior parameter correlations, $BRCA1^{+/-}$ vs. $BRCA1^{-/-}$ composition, founder lineages vs age).
- [x] Add Makefile targets `exp04-fig4`, `exp04-smoke`, and `exp04-simulate`.
- [x] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make exp04-fig4 && make exp04-smoke && make test`).
- [x] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

### Phase 6: Vertical Slice 5 – Cancer Interception & Attenuation Protocols (Figure 5 & Supplementary Figure S5)
*(Recommended model: Gemini 3.8 Flash Low)*
**Description:** Integrate `run_paper_interventions.cpp` to evaluate $D_1$ and $D_2$ attenuation (10%, 25%, 50%) initiated at ages 30, 40, and 50.
- [x] Ask and confirm with the user that the recommended model (Gemini 3.8 Flash Low) is active.
- [x] Add `app/run_paper_interventions.cpp` and update `CMakeLists.txt`.
- [x] Create directory `experiments/exp05_cancer_interception/` with precalibrated assets (`data/`).
- [x] Add runner script `run_interception.py` (supporting precomputed traces and fast `--smoke-test` in 0.04s).
- [x] Add plotting script `plot_figure5_sfig5.py` generating Figure 5 (trajectories, absolute risk reduction, dose-response) and Figure S5 (interaction contrasts).
- [x] Add Makefile targets `exp05-fig5`, `exp05-sfig5`, `exp05-smoke`, and `exp05-simulate`.
- [x] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make exp05-fig5 && make exp05-sfig5 && make exp05-smoke && make test`).
- [x] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

### Phase 7: Tissue-Scale Module Refactoring & Code Beautification (Figure 6, Figure S6, Figure S7)
*(Recommended model: Gemini 3.8 Thinking Middle)*
**Description:** Import `tissue-scale` into `tissue_scale/`, completely refactor and beautify `tissue_scale_sim.cpp` for high readability, and package Figures 6, S6, and S7 using precomputed 10k validation results.
- [ ] Ask and confirm with the user that the recommended model (Gemini 3.8 Thinking Middle) is active.
- [ ] Create directory structure `tissue_scale/` (`src/`, `scripts/`, `data/`, `results/`).
- [ ] Refactor and beautify `tissue_scale/src/tissue_scale_sim.cpp`: expand compressed variable names into explicit domain names, format with clean C++20 conventions, and add formula documentation referencing Methods.
- [ ] Add dedicated `tissue_scale/Makefile` for compiling `tissue_scale_sim` with OpenMP optimization.
- [ ] Import and beautify python scripts: `make_figures.py` (Figure 6 & S6), `make_sfig5.py` (Figure S7), and `run_validation.py` (using precomputed `final_multiobjective_validation_10000.csv` for instantaneous figure generation).
- [ ] Add root Makefile targets `exp07-fig6-tissue` and `tissue-scale-test`.
- [ ] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make exp07-fig6-tissue` validation run). Fix issues if any.
- [ ] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

### Phase 8: End-to-End Validation, Backup Purge & Git History Recipe
*(Recommended model: Gemini 3.8 Thinking Middle for validation, Flash Low for cleanup)*
**Description:** Validate all figures against `/home/luis/Escritorio/abms/v4/`, permanently purge `archive_backup/` upon user approval, and prepare the clean Git history for publication.
- [ ] Ask and confirm with the user that the recommended model is active.
- [ ] Run fast smoke test suite and figure generation checks for both `cell_scale` and `tissue_scale`.
- [ ] Verify that all generated figures match the manuscript review PDFs in `MainFigs_review_102126` and `SupplFig-Table-review_102126`.
- [ ] Request explicit user confirmation to purge `archive_backup/`.
- [ ] Remove `archive_backup/` upon confirmation.
- [ ] Write clear documentation on how to run experiments and provide the Git commit/rebase history recipe for publishing to GitHub.
- [ ] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`git status` and `make test`). Fix issues if any.
- [ ] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

---

# ⏭️ Next Step

The next step is to execute **Phase 4: Vertical Slice 3 – Bayesian ABC-SMC Calibration & Posterior Predictive (Figure 3 & Figure S4)** (using Gemini 3.8 Flash Low) to package `experiments/exp03_abc_smc_calibration/` and wire `make exp03-fig3`.

Plan verified and guided by 🐢 💨 (Turbotuga™, [Codely](https://codely.com)’s mascot) 🔍 📦 📊 ✨.
