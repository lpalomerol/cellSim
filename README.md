# BRCA1-ONSET / CellSim

[![CI](https://github.com/lpalomerol/cellSim/actions/workflows/ci.yml/badge.svg)](https://github.com/lpalomerol/cellSim/actions)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

A multiscale, domain-driven, open-source C++20 simulation suite for stochastic modeling of breast cancer onset, clonal evolution, and early interception in germline *BRCA1* mutation carriers (*Palomero et al., 2026*).

---

## Architecture Overview

The repository features a **symmetric dual C++ simulation architecture** coupled with 9 self-contained, modular vertical experiment slices:

```text
cellSim/
├── CMakeLists.txt                         # Root CMake orchestrator (builds both C++ engines & tests)
├── Makefile                               # Master workflow orchestrator (build, test, exp01-09, paper-figures)
├── README.md                              # Repository & reproducibility master guide
│
├── cell_scale/                            # ENGINE 1: Discrete Agent-Based Engine (500 cells)
│   ├── CMakeLists.txt                     # CMake configuration for cell_scale_lib & runners
│   ├── src/                               # Hexagonal domain (Cell, Genome, Tissue, Instability)
│   ├── app/                               # C++ simulation binaries:
│   │   ├── run_bootstrapping.cpp          # Fast Monte Carlo penetrance evaluator
│   │   ├── run_paper_features.cpp         # Emergent features & clonal composition evaluator
│   │   └── run_paper_interventions.cpp    # In silico matched interception cohort runner
│   └── tests/                             # GoogleTest test suite (144 unit & integration tests)
│
├── tissue_scale/                          # ENGINE 2: Continuous Macro-Scale Engine (10^7 cells)
│   ├── CMakeLists.txt                     # CMake build for tissue_scale_sim
│   ├── src/                               # High-performance C++20 OpenMP branching simulator
│   │   ├── tissue_scale_sim.cpp           # CLI entrypoint for tissue simulations
│   │   └── TissueScaleSimulator.h         # Standalone tissue-level biological model header
│   └── configs/                           # Reference calibrated configurations (JSON)
│
├── experiments/                           # MODULAR PUBLICATION VERTICAL SLICES
│   ├── exp01_cs_exploratory_acceleration/ # Fig 2:  100-year exploratory acceleration horizon
│   ├── exp02_cs_sensitivity_global/       # Fig S1: Global sensitivity analysis (LHS & PRCC)
│   ├── exp03_cs_abc_smc_calibration/      # Fig 3 & S4: Bayesian ABC-SMC calibration & posteriors
│   ├── exp04_cs_emergent_features/        # Fig 4:  Emergent phenotypes & clonal founder lineages
│   ├── exp05_cs_cancer_interception/      # Fig 5 & S5: In silico interception & sub-additive interaction
│   ├── exp06_cs_sensitivity_structural/   # Fig S2 & S3: Fixed-parameter OAT & structural sensitivity
│   ├── exp07_ts_multiobjective_cohort/    # Fig 6:  Multiobjective virtual cohort (10^7 cells, N=10,000)
│   ├── exp08_ts_homogeneous_restraint/    # Fig S6: Homogeneous tissue restraint fitting trade-off
│   └── exp09_ts_checkpoint_relaxation/    # Fig S7: Checkpoint relaxation sensitivity heatmaps
│
└── results/
    └── paper_figures/                     # CENTRALIZED CANONICAL DELIVERABLES (PDF, EPS, PNG)
```

---

## Requirements

- **C++ Compiler**: `g++` (≥ 11) or `clang++` (≥ 14) with C++20 support
- **CMake**: ≥ 3.28
- **OpenMP**: For multicore parallel acceleration
- **Python**: ≥ 3.10 with `numpy`, `scipy`, `pandas`, `matplotlib`

```bash
# Ubuntu / Debian
sudo apt update && sudo apt install -y cmake g++ libomp-dev python3-pip
pip install numpy scipy pandas matplotlib
```

---

## Quickstart

```bash
# 1. Clean build of both engines and test suite
make rebuild

# 2. Run the complete C++ test suite (144 tests)
make test

# 3. Fast verification across all 9 experimental vertical slices (<15 seconds)
make exp-smoke-all

# 4. Export all official publication figures (PDF, EPS, PNG)
make paper-figures
```

---

## Paper Figures & Reproduction Guide

All official figures are synchronized in [`results/paper_figures/`](results/paper_figures/) in three standard formats:
- **Vector PDF** (`.pdf`)
- **Vector EPS** (`.eps`)
- **High-Resolution PNG** (`.png`, 300 DPI)

### Main Text Figures

| Figure | Experiment Target | Command | Description |
| :--- | :--- | :--- | :--- |
| **Figure 2** | `exp01_cs_exploratory_acceleration` | `make exp01-cs-fig2` | 100-year horizon, $d=0.001$, 4 representative curves overlaid on 36-model grid ensemble |
| **Figure 3** | `exp03_cs_abc_smc_calibration` | `make exp03-cs-fig3` | ABC-SMC calibration efficiency & posterior predictive cumulative risk curve vs Kuchenbaecker (2017) |
| **Figure 4** | `exp04_cs_emergent_features` | `make exp04-cs-fig4` | Emergent tumor features, somatic BRCA1 loss composition & founder lineage complexity |
| **Figure 5** | `exp05_cs_cancer_interception` | `make exp05-cs-fig5` | Precision cancer interception: intervention timing ($D_1$ vs $D_2$), ARR, and dose response |
| **Figure 6** | `exp07_ts_multiobjective_cohort` | `make exp07-ts-fig6` | Tissue-scale multiobjective validation (5 panels a–e, $10^7$ cells/tissue, $N=10,000$ cohort) |

### Supplementary Figures

| Figure | Experiment Target | Command | Description |
| :--- | :--- | :--- | :--- |
| **Figure S1** | `exp02_cs_sensitivity_global` | `make exp02-cs-sfig1` | Global sensitivity analysis: Latin Hypercube Sampling (1,000 pts) & PRCC rankings |
| **Figure S2** | `exp06_cs_sensitivity_structural` | `make exp06-cs-sfig2` | Fixed-parameter OAT sensitivity (tornado plot, SSE, $\pm 10\%$ and $\pm 25\%$ recalibrations) |
| **Figure S3** | `exp06_cs_sensitivity_structural` | `make exp06-cs-sfig3` | Structural sensitivity across genomic instability scaling $\kappa_{\text{BRCA1}}$ and somatic $\mu_{\text{TP53}}$ |
| **Figure S4** | `exp03_cs_abc_smc_calibration` | `make exp03-cs-sfig4` | Canonical $4\times 4$ posterior parameter corner matrix (Variant 4a) |
| **Figure S4b**| `exp03_cs_abc_smc_calibration` | `make exp03-cs-sfig4b` | Variant 4b: Cross-parameter normalized rate ratios ($\theta_i/\beta$) |
| **Figure S4c**| `exp03_cs_abc_smc_calibration` | `make exp03-cs-sfig4c` | Variant 4c: Self-ratio strip ($\theta_i/\beta$ vs $\theta_i$) |
| **Figure S4d**| `exp03_cs_abc_smc_calibration` | `make exp03-cs-sfig4d` | Variant 4d: 1D posterior distributions (SIS KDE) and 95% CrIs of all 6 parameter pairs |
| **Figure S5** | `exp05_cs_cancer_interception` | `make exp05-cs-sfig5` | Sub-additive interaction between genomic stabilization ($D_1$) and tissue restraint ($D_2$) |
| **Figure S6** | `exp08_ts_homogeneous_restraint` | `make exp08-ts-sfig6` | Homogeneous tissue-restraint fitting limitations & early vs late SSE trade-off |
| **Figure S7** | `exp09_ts_checkpoint_relaxation` | `make exp09-ts-sfig7` | Checkpoint relaxation sensitivity heatmaps ($\gamma \times p_{\text{bypass}}$ grid) |

---

## Simulation Campaigns

To re-run extensive simulation campaigns from scratch:

```bash
# Discrete engine (cell_scale):
make exp01-cs-simulate    # Exploratory Monte Carlo grid (N=500 per cell)
make exp02-cs-simulate    # Global LHS campaign (1,000 combinations x 200 runs)
make exp03-cs-simulate    # Full ABC-SMC calibration (250 particles x 20 generations)
make exp04-cs-simulate    # Clonal lineage evaluations (20,000 tissues)
make exp05-cs-simulate    # Interception cohorts (204,000 matched conditions)
make exp06-cs-simulate    # Structural sensitivity perturbations (N=300 per perturbation)

# Macro engine (tissue_scale):
make exp07-ts-simulate    # Stochastic tissue cohort simulation (N=10,000 tissues)
make exp09-ts-simulate    # 24-point checkpoint relaxation grid (N=3,000 per cell)
```

---

## Citation & Authorship

Palomero, L. et al. (2026). *BRCA1-ONSET: A Multiscale Stochastic Framework for Tumor Evolution and Cancer Interception in BRCA1 Mutation Carriers*.
