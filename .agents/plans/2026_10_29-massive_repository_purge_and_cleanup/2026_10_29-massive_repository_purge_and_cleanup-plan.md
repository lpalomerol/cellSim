---
name: "massive_repository_purge_and_cleanup"
description: "Massive purge of obsolete AI assistant configs, editor metadata, root historical documents, build caches, and legacy scripts directory"
created_at: "2026-10-29T09:59:40+01:00"

created_by:
  tool: "Antigravity CLI"
  model:
    name: "Gemini"
    version: "3.8 Flash (Medium)"
    reasoning_effort: "medium"
---

# 🎯 Goal

Execute a massive repository cleanup to eliminate over 160 MB of obsolete AI assistant configurations, IDE metadata, legacy markdown summaries, build caches, and the superseded `scripts/` folder, ensuring the codebase strictly retains only the core simulation engines (`cell_scale/`, `tissue_scale/`), the 9 publication vertical slices (`experiments/`), the centralized publication deliverables (`results/paper_figures/`), and container deployment scripts.

---

# 👀 Context

- **Repository Root**: [`/home/luis/CLionProjects/cellSim`](file:///home/luis/CLionProjects/cellSim)
- **Active Core Engines**: [`cell_scale/`](file:///home/luis/CLionProjects/cellSim/cell_scale/), [`tissue_scale/`](file:///home/luis/CLionProjects/cellSim/tissue_scale/)
- **Active Publication Experiments**: [`experiments/`](file:///home/luis/CLionProjects/cellSim/experiments/) (`exp01_cs_...` to `exp09_ts_...`)
- **Centralized Deliverables**: [`results/paper_figures/`](file:///home/luis/CLionProjects/cellSim/results/paper_figures/)
- **Build & CI Orchestration**: [`Makefile`](file:///home/luis/CLionProjects/cellSim/Makefile), [`CMakeLists.txt`](file:///home/luis/CLionProjects/cellSim/CMakeLists.txt), [`.github/workflows/ci.yml`](file:///home/luis/CLionProjects/cellSim/.github/workflows/ci.yml)
- **Git Tracking Rules**: [`.gitignore`](file:///home/luis/CLionProjects/cellSim/.gitignore)
- **Container Deployment**: [`cellSim.def`](file:///home/luis/CLionProjects/cellSim/cellSim.def), [`build_apptainer.sh`](file:///home/luis/CLionProjects/cellSim/build_apptainer.sh), [`build_apptainer_local.sh`](file:///home/luis/CLionProjects/cellSim/build_apptainer_local.sh), [`APPTAINER_GUIDE.md`](file:///home/luis/CLionProjects/cellSim/APPTAINER_GUIDE.md)

---

# 📜 Public Contracts

### 1. Files and Directories to Delete
- **AI and Tool Configs**:
  - `.opencode/` (Node.js plugin dependencies, ~62 MB)
  - `.claude/` (Redundant skill symlinks)
  - `openspec/` (OpenSpec CLI configuration and change logs)
  - `.copilot-config.json` and `.copilot-instructions.md` (GitHub Copilot settings)
  - `.cursorrules` (Cursor IDE rules)
  - `.idea/` (CLion project configuration)
  - `.github/copilot/`, `.github/copilot-instructions.md`
  - `.github/prompts/`, `.github/skills/`, `.github/.archives/`
- **Root Historical Documents**:
  - `BIG_BANG_SUMMARY.md`
  - `CLION-BUILD-CONFIG.md`
  - `CONTEXT.md`
  - `INDEX.md`
  - `QA_IMPLEMENTATION_SUMMARY.md`
  - `QA_README.md`
  - `RESUMEN-EJECUCION-REFACTORIZATION-V2.md`
- **Build Caches and Scratch Files**:
  - `cmake-build-debug/` (~82 MB)
  - `Testing/`
  - `out1/`
  - `traces/`
  - `single_cell_evolution_log.txt`
  - `calibration_results.csv`
  - Temporary files in `docs/`: `.Rhistory`, backup copies (`*~`, `#*#`), and LaTeX compilation files (`*.aux`, `*.log`, `*.out`, `*.toc`, `*.bbl`, `*.blg`)
- **Legacy Script Hierarchy**:
  - `scripts/` (including `scripts/experiments/`, `scripts/__pycache__/`, `scripts/docs/`)

### 2. Files to Modify
- **`.gitignore`**: Add ignore rules for `.opencode/`, `openspec/`, `.claude/`, `.copilot-*`, `.cursorrules`, `cmake-build-debug/`, `*.Rhistory`, `out1/`, and `traces/`.
- **`Makefile`**: Update residual targets (`calibration-dry-run`, `calibration-pilot`, `calibration-paper`, `sensitivity-global-lhs`, `sensitivity-unaccelerated`) to invoke the modular scripts in `experiments/exp03_cs_abc_smc_calibration/` and `experiments/exp02_cs_sensitivity_global/`.

### 3. Untouchable Assets (Preserved)
- `.agents/` and `skills-lock.json`
- `.github/workflows/ci.yml`
- `configs/default.json` and container deployment assets (`cellSim.def`, `build_apptainer*.sh`, `APPTAINER_*.md`)

---

# 🪜 Phases

### Phase 1: Purge AI Configs, IDE Metadata, Root Historical Docs, Build Caches & Update .gitignore
*(Recommended model: Gemini 3.8 Flash Low)*
**Description:** Remove all surplus AI assistant configurations, IDE metadata, historical markdown summaries from the repository root, ephemeral build caches, and LaTeX temp files. Update `.gitignore` to prevent any of these files from being tracked in the future.
- [x] Ask and confirm with the user that the recommended model is active.
- [x] Delete AI assistant and tool configuration folders and files:
  - Remove `.opencode/`
  - Remove `.claude/`
  - Remove `openspec/`
  - Remove `.copilot-config.json` and `.copilot-instructions.md`
  - Remove `.cursorrules`
  - Remove `.idea/`
  - Remove `.github/copilot/` and `.github/copilot-instructions.md`
  - Remove `.github/prompts/`, `.github/skills/`, and `.github/.archives/`
- [x] Delete root historical markdown files:
  - Remove `BIG_BANG_SUMMARY.md`
  - Remove `CLION-BUILD-CONFIG.md`
  - Remove `CONTEXT.md`
  - Remove `INDEX.md`
  - Remove `QA_IMPLEMENTATION_SUMMARY.md`
  - Remove `QA_README.md`
  - Remove `RESUMEN-EJECUCION-REFACTORIZATION-V2.md`
- [x] Delete ephemeral build directories and scratch outputs:
  - Remove `cmake-build-debug/`
  - Remove `Testing/`
  - Remove `out1/` and `traces/`
  - Remove `single_cell_evolution_log.txt` and `calibration_results.csv`
- [x] Clean temporary files across `docs/`:
  - Remove `.Rhistory` files from `docs/paper2/` and `docs/paper3/`
  - Remove editor backup files (`*~`, `#*#`) and LaTeX compilation files (`*.aux`, `*.log`, `*.out`, `*.toc`, `*.bbl`, `*.blg`) in `docs/`
- [x] Update [`.gitignore`](file:///home/luis/CLionProjects/cellSim/.gitignore) with comprehensive rules covering:
  - `.opencode/`, `openspec/`, `.claude/`, `.copilot-*`, `.cursorrules`, `.idea/`, `.vscode/`
  - `cmake-build-debug/`, `Testing/`, `out1/`, `traces/`, `*.Rhistory`, `*.log`, `*.aux`
- [x] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make test`). Fix issues if any.
- [x] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

---

### Phase 2: Makefile Legacy Targets Realignment, Decommissioning of scripts/ & Master Validation
*(Recommended model: Gemini 3.8 Flash Low)*
**Description:** Realign the residual Makefile targets so that they reference the self-contained vertical slices in `experiments/`, permanently remove the obsolete `scripts/` directory, and perform a full validation pass to verify build, tests, smoke tests, and paper figures export.
- [x] Ask and confirm with the user that the recommended model is active.
- [x] Update legacy calibration and sensitivity targets in [`Makefile`](file:///home/luis/CLionProjects/cellSim/Makefile):
  - Update `calibration-dry-run` to invoke `experiments/exp03_cs_abc_smc_calibration/abc_smc.py --smoke-test`
  - Update `calibration-pilot` to invoke `experiments/exp03_cs_abc_smc_calibration/abc_smc.py --particles 50 --generations 4 --n-sim 100 --workers 4`
  - Update `calibration-paper` to invoke `experiments/exp03_cs_abc_smc_calibration/abc_smc.py --particles 250 --generations 20 --n-sim 300 --workers 8`
  - Update `sensitivity-global-lhs` to invoke `experiments/exp02_cs_sensitivity_global/run_lhs_sensitivity.py --n-samples 1000 --n-sim 200`
  - Update `sensitivity-unaccelerated` to invoke `experiments/exp01_cs_exploratory_acceleration/run_exploratory.py --generate-trajectories`
- [x] Permanently remove the obsolete `scripts/` directory:
  - Remove `scripts/experiments/`
  - Remove `scripts/docs/`
  - Remove `scripts/__pycache__/`
  - Remove the entire `scripts/` directory
- [x] Verify that all unit tests pass without errors (`make test`).
- [x] Verify that all 9 experiment smoke tests execute cleanly (`make exp-smoke-all`).
- [x] Verify that the centralized figure export runs with zero missing files (`make paper-figures`).
- [x] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make test && make exp-smoke-all`). Fix issues if any.
- [x] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

---

# ⏭️ Next Step

Both phases of the cleanup plan are fully completed, tested, and validated!
The repository is completely purged of obsolete configs, historical documents, and legacy scripts, leaving a clean, symmetric, dual C++ architecture and 9 self-contained experiment slices.

Plan created with precision and speed by 🐢 💨 (Turbotuga™, [Codely](https://codely.com)’s mascot) 🔍 📦 🧹 ✨.
