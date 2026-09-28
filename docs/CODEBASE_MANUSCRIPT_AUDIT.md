# 📑 Codebase, Experiments & Manuscript Audit

**Target Manuscript Source:** [`/home/luis/Escritorio/abms/v2_BRCA1-ONSET_Ms_092426/MANUSCRIPT_SENTENCES.md`](file:///home/luis/Escritorio/abms/v2_BRCA1-ONSET_Ms_092426/MANUSCRIPT_SENTENCES.md)  
**Target Codebase:** `cellSim`  
**Date:** September 28, 2026  

---

## 1. Executive Summary

This document establishes formal traceability between the biological mechanisms, equations, and simulation procedures described in the manuscript (*BRCA1-ONSET*) and the `cellSim` repository.

It comprehensively categorizes and marks for archiving:
1. **Core C++ Simulation Engine & Active Tools**: The minimal, verified production stack used for manuscript simulations and new ABC campaigns.
2. **Historical Experiments & Traces (`results/`, `traces/`)**: Catalogued and **Marked for Archive**.
3. **Build System & Tooling (`Makefile`)**: Targets categorized into **Active/Essential** vs. **Marked for Archive / Deprecated**.
4. **Python Scripts (`scripts/`)**: Legacy pre-publication scripts catalogued and **Marked for Archive** in favor of `scripts/experiments/`.

---

## 2. Manuscript Sentences Traceability Matrix

| Manuscript Section / Concept | Mathematical Formulation / Rule | Implementing Code in `cellSim` | Status |
| :--- | :--- | :--- | :--- |
| **Germline Carrier State & Genotypes** | Initial tissue state: $BRCA1^{+/-}; TP53^{+/+}$. Unidirectional LoF: $BRCA1^{+/-} \to BRCA1^{-/-}$; $TP53^{+/+} \to TP53^{+/-} \to TP53^{-/-}$. | [`src/domain/gene/Genome.cpp`](file:///home/luis/CLionProjects/cellSim/src/domain/gene/Genome.cpp), [`src/domain/gene/Gene.cpp`](file:///home/luis/CLionProjects/cellSim/src/domain/gene/Gene.cpp), `domain::genome_factory::makeDefaultGenome` in [`app/run_bootstrapping.cpp`](file:///home/luis/CLionProjects/cellSim/app/run_bootstrapping.cpp#L94-L101) | **Active Core** |
| **Synthetic Lethality Checkpoint** | Second-hit loss of $BRCA1$ in $TP53^{+/+}$ or $TP53^{+/-}$ cells triggers intrinsic apoptosis. $BRCA1^{-/-}$ viability is strictly permitted only in $TP53^{-/-}$ cells. | [`src/domain/cell/strategies/GenomicViabilityStrategy.cpp`](file:///home/luis/CLionProjects/cellSim/src/domain/cell/strategies/GenomicViabilityStrategy.cpp#L20-L40), [`src/domain/cell/AgenticCell.cpp`](file:///home/luis/CLionProjects/cellSim/src/domain/cell/AgenticCell.cpp#L211-L215) (`phase1_G1IntegrityCheckpoint`) | **Active Core** |
| **Genomic Instability Accumulators ($D_1, D_2$)** | $\Delta D_{1,i,t} = \delta_{\text{TP53}, i, t} + A_{i,t}$<br>$\Delta D_{2,i,t} = \delta_{\text{TP53}, i, t} + \delta_{\text{BRCA1}, i, t} + A_{i,t}$<br>Aging drift $A_{i,t} = 10^{-5} \cdot a_{i,t}$. Multiplier $\kappa_{\text{BRCA1}} = 2.0$ on $\delta_{\text{high}}$ for homozygous $BRCA1$ loss. | [`src/domain/cell/strategies/GenomicInstabilityDeltaStrategy.cpp`](file:///home/luis/CLionProjects/cellSim/src/domain/cell/strategies/GenomicInstabilityDeltaStrategy.cpp), [`src/domain/cell/AgenticCell.cpp`](file:///home/luis/CLionProjects/cellSim/src/domain/cell/AgenticCell.cpp#L246-L255) (`phase4_CytoplasmicRemodeling`) | **Active Core** |
| **Dynamic Mutation Rate** | Probability of locus mutation per cycle: $P_{\text{mut}, g, i}(t) = \min(1, \mu_g \cdot D_{1, i, t})$. | [`src/domain/gene/Gene.cpp`](file:///home/luis/CLionProjects/cellSim/src/domain/gene/Gene.cpp#L58-L81) (`Gene::live`, `calculateMutationThreshold`) | **Active Core** |
| **Transformation Gateways** | Gate 1: $TP53^{-/-} \land D_1 > \theta_{D1} \implies \text{PRIMER}$.<br>Gate 2: $D_2 \le \theta_{D2} \implies \text{Clearance (extrinsic apoptosis)}$; $D_2 > \theta_{D2} \implies \text{TUMORAL (immune evasion)}$. | [`src/domain/cell/AgenticCell.cpp`](file:///home/luis/CLionProjects/cellSim/src/domain/cell/AgenticCell.cpp#L164-L238) (`getCurrentCellLifeStage`, `phase2_ExtrinsicApoptosisCheckpoint`) | **Active Core** |
| **Basal & Neoplastic Proliferation** | Basal division probability $d = 0.001$. Tumoral division probability $\nu \gg d$ (e.g., $\nu \approx 0.159$). Big Bang accelerated division mode. | [`app/run_bootstrapping.cpp`](file:///home/luis/CLionProjects/cellSim/app/run_bootstrapping.cpp#L106-L115), [`src/domain/cell/AgenticCell.cpp`](file:///home/luis/CLionProjects/cellSim/src/domain/cell/AgenticCell.cpp) (`phase5_Cytokinesis`), [`src/domain/tissue/Tissue.cpp`](file:///home/luis/CLionProjects/cellSim/src/domain/tissue/Tissue.cpp) | **Active Core** |
| **Tissue-Level Monte Carlo Execution** | 500 initial cells, annual ticks up to age 80 ($max\_t=80$). Parallel execution across random seeds using OpenMP. | [`app/run_bootstrapping.cpp`](file:///home/luis/CLionProjects/cellSim/app/run_bootstrapping.cpp#L90-L160) (`runOnce`, `runBatch`) | **Active Core** |
| **Clinical Penetrance & Weighted SSE** | Cumulative breast cancer risk calculated at ages 30, 40, 50, 60, 70, 80 vs. Kuchenbaecker et al. (2017). Distance $SSE_w = \sum_{a} \left(\frac{R_{\text{sim}}(a) - R_{\text{obs}}(a)}{\sigma_a}\right)^2$. | [`src/application/bootstrapping/BootstrappingMetrics.cpp`](file:///home/luis/CLionProjects/cellSim/src/application/bootstrapping/BootstrappingMetrics.cpp#L50-L80) (`computeWeightedSSE`, `cumulativeRisk`) | **Active Core** |
| **ABC-SMC Parameter Estimation** | Multi-generation Sequential Monte Carlo with adaptive quantile tolerance ($Q_{0.75}$), Gaussian perturbation kernel with optimal variance, SIS particle weights. | Standardized: [`scripts/experiments/01_abc_canonical_4p.py`](file:///home/luis/CLionProjects/cellSim/scripts/experiments/01_abc_canonical_4p.py). | **Active Core** |
| **Multi-Scale Extension ($10^7$ Cells)** | Macro-scale cohort simulation with stochastic Poisson/binomial dynamics, lognormal threshold dispersion ($\sigma_{\theta_{D2}}$), doubling time $\tau_T$, tumor threshold $5\%$. | `/home/luis/Escritorio/abms/v2_BRCA1-ONSET_Ms_092426/code_tissue-scale/` (`src/tissue_scale_sim.cpp`) | **External Subproject** |

---

## 3. Historical Experiments & Traces Audit (`results/` & `traces/`)

### Experiments in `results/`
All existing experimental folders in `results/` belong to previous iterations (versions 1 through 8, exploratory subgroups, or initial sensitivity tests) and are **Marked for Archive**:

| Directory | Content / Historical Purpose | Status | Target New Location / Replacement |
| :--- | :--- | :--- | :--- |
| `results/abc_final_v4/` | 4-parameter ABC-SMC generation CSVs and plots (May 2026 run). | **Marked for Archive** | Archive to `archive/results/abc_final_v4/`. Replaced by `results/20260928_draft_map/canonical_4p/`. |
| `results/abc_corrected_v1/` | Early test run correcting prior boundaries (May 2026). | **Marked for Archive** | Archive to `archive/results/abc_corrected_v1/`. |
| `results/abc_sensitivity/` | Initial parameter sensitivity runs (August 2026). | **Marked for Archive** | Archive to `archive/results/abc_sensitivity/`. Replaced by `results/20260928_draft_map/extended_all/`. |
| `results/paper20260825_subgroups/` | Stratified lineage and subgroup analysis runs (August 2026). | **Marked for Archive** | Archive to `archive/results/paper20260825_subgroups/`. |
| `results/paper20260825_validation/` | Model validation runs against Kuchenbaecker targets (August 2026). | **Marked for Archive** | Archive to `archive/results/paper20260825_validation/`. |
| `results/paper8_subgroups/` | Pre-submission iteration 8 subgroup traces. | **Marked for Archive** | Archive to `archive/results/paper8_subgroups/`. |

### Traces in `traces/`
| Directory | Content / Historical Purpose | Status | Action |
| :--- | :--- | :--- | :--- |
| `traces/01_*` to `05_*` | Control scenarios (baseline, no mutations, high division, high BRCA1/TP53). | **Marked for Archive** | Historical exploratory traces from April 2026. Archive to `archive/traces/scenarios_01_17/`. |
| `traces/06_*` to `09_*` | Realistic scenarios (balanced, low/high instability). | **Marked for Archive** | Archive to `archive/traces/scenarios_01_17/`. |
| `traces/10_*` to `12_*` | Big Bang accelerated proliferation scenarios. | **Marked for Archive** | Archive to `archive/traces/scenarios_01_17/`. |
| `traces/13_*` to `17_*` | Calibrated lognormal and constant rate scenarios ($N=1000, 2000$). | **Marked for Archive** | Archive to `archive/traces/scenarios_01_17/`. |
| `traces/cli/` | Traces from manual CLI test runs. | **Marked for Archive** | Transient test logs; clean or archive. |
| `traces/multi/` | 100+ seed directories generated by legacy `make multi` target. | **Marked for Archive** | Clean or archive to `archive/traces/multi/`. |

---

## 4. Makefile Targets Audit

The `Makefile` contains 326 lines with multiple targets created across different stages of project development. Here is the classification:

| Makefile Target(s) | Category | Description / Issue | Recommendation |
| :--- | :--- | :--- | :--- |
| `rebuild`, `build`, `test`, `all`, `all-test`, `clean`, `unit_tests` | **Active / Essential** | Core CMake compilation and GoogleTest execution targets. | **Keep & Maintain** |
| `cellSim`, `run_all_scenarios`, `single_cell_evolution`, `interactive` | **Marked for Archive** | Builds and launches exploratory binaries not used in manuscript simulations. | Deprecate in Makefile; mark as legacy. |
| `calibration_sweep` | **Marked for Archive** | Early 2D grid sweep over $(D_{\text{low}}, \theta_{D1})$; superseded by ABC-SMC. | Mark as legacy. |
| `cellSim_cli`, `cellSim_cli_silent`, `default` | **Marked for Archive** | Single-run CLI wrappers. | Mark as legacy. |
| `multi`, `multi-quick`, `multi-full`, `clean-multi` | **Marked for Archive** | Bash loops running `./cmake-build-debug/cellSim_cli` across seeds, filling `traces/multi/`. | Deprecate / remove hardcoded `./cmake-build-debug` references. |
| `install`, `uninstall` | **Marked for Archive** | Copies binaries to `/usr/local/bin`. Not suitable for research cluster or paper reproduction. | Mark as legacy. |
| `dist-package`, `dist-deploy`, `dist-run`, `clean-dist` | **Marked for Archive** | Remote server SSH packaging tools with hardcoded `server.com` defaults. | Mark as legacy. |
| `qa-quick`, `qa-full`, `qa`, `qa-report`, `qa-show`, `qa-clean` | **Marked for Archive** | Pipeline tied to "paper3" (`docs/paper3/validation_report.json`), superseded by current paper reproducibility suite. | Mark as legacy. Replace with `experiments-dry-run` and `experiments-all`. |

---

## 5. Python Scripts Audit (`scripts/`)

| Script | Historical Role | Status | Successor / Modern Equivalent |
| :--- | :--- | :--- | :--- |
| `scripts/abc_final_v4.py` | Standalone ABC-SMC implementation (v4). | **Marked for Archive** | [`scripts/experiments/01_abc_canonical_4p.py`](file:///home/luis/CLionProjects/cellSim/scripts/experiments/01_abc_canonical_4p.py) + `abc_smc_framework.py`. |
| `scripts/analyse_abc_final.py` | KDE and correlation plots for ABC v4. | **Marked for Archive** | Specialized per-outcome summary reporting in `results/20260928_draft_map/`. |
| `scripts/analyze_bimodality.py` | Bimodality detector for ABC v4. | **Marked for Archive** | Archive to `archive/scripts/`. |
| `scripts/generate_figures.py` | Paper 3 figure generator. | **Marked for Archive** | Superseded by `paper/scripts/`. |
| `scripts/generate_paper7_figures.py` | Iteration 7 figure generator. | **Marked for Archive** | Superseded by `paper/scripts/`. |
| `scripts/paper_figures_gen2.py`, `paper_figures_v4.py` | Earlier generation figure generators. | **Marked for Archive** | Superseded by `paper/scripts/`. |
| `scripts/generar_memoria.py` | Spanish documentation generator. | **Marked for Archive** | Archive to `archive/scripts/`. |
| `scripts/run_experiments.sh` | Shell orchestrator for Paper 3 sweeps. | **Marked for Archive** | Standalone Python runners in `scripts/experiments/`. |
| `scripts/qa_report.py` | Generates `docs/paper3/qa_report.html`. | **Marked for Archive** | Archive to `archive/scripts/`. |

---

## 6. Code Architecture Legacy Deprecations

The following C++ domain and application elements are preserved to avoid breaking legacy build configurations, but marked as legacy:
1. `src/domain/signal/*` (`ISignal.h`, `BaseSignal.h`, etc.): Message passing unused in direct Monte Carlo cell updates.
2. `src/domain/cell/strategies/LognormalNoiseInstabilityStrategy.h/.cpp`: Instability noise decorator unused in paper runs (`delta-cv = 0.0`).
3. `src/application/presenter/GeneticSummaryPresenter.h/.cpp`: Text formatter for interactive CLI.

---

## 7. Migration & Archiving Plan

When implementing archiving:
1. Create directory `archive/` with subfolders:
   - `archive/results/` (store legacy result runs)
   - `archive/traces/` (store legacy trace files)
   - `archive/scripts/` (store superseded Python scripts)
2. Retain all clean build and test capability in the root `CMakeLists.txt` and `Makefile`.
3. Establish the new standard experiment execution path exclusively under `scripts/experiments/` outputting to `results/20260928_draft_map/`.
