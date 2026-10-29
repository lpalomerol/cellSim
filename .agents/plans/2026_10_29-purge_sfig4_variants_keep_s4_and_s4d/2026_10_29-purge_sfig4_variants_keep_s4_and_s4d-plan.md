---
name: "purge_sfig4_variants_keep_s4_and_s4b"
description: "Consolidate Supplementary Figure S4 into canonical S4 (4x4 corner matrix) and S4b (pairwise parameter ratios 1D KDEs), removing obsolete variants, updating scripts, Makefile, centralized export and documentation"
created_at: "2026-10-29T17:03:00+01:00"

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

last_implementation_at: "2026-10-29T17:11:00+01:00"
has_completed_all_phases: true
---

# 🎯 Goal

Consolidate Supplementary Figure S4 in `experiments/exp03_cs_abc_smc_calibration` down to strictly two canonical figures:
1. **Figure S4**: Canonical $4 \times 4$ joint posterior parameter corner matrix (marginal KDEs + bivariate contour maps).
2. **Figure S4b** (formerly S4d): 1D posterior density KDEs and 95% CrIs of all 6 pairwise parameter ratios ($R = \theta_i / \theta_j$).

Purge obsolete variants (the former S4b cross-grid, former S4c self-strip, and former S4d artifacts), clean up `plot_sfig4.py`, update `Makefile` targets, update `results/paper_figures/export_all_figures.py`, and synchronize all project documentation (`experiments/exp03_cs_abc_smc_calibration/README.md`, `results/paper_figures/README.md`, and root `README.md`).

---

# 👀 Context

- **Experiment 03 Script**: [`experiments/exp03_cs_abc_smc_calibration/plot_sfig4.py`](file:///home/luis/CLionProjects/cellSim/experiments/exp03_cs_abc_smc_calibration/plot_sfig4.py)
- **Experiment 03 Documentation**: [`experiments/exp03_cs_abc_smc_calibration/README.md`](file:///home/luis/CLionProjects/cellSim/experiments/exp03_cs_abc_smc_calibration/README.md)
- **Root Makefile**: [`Makefile`](file:///home/luis/CLionProjects/cellSim/Makefile)
- **Centralized Figures Export**: [`results/paper_figures/export_all_figures.py`](file:///home/luis/CLionProjects/cellSim/results/paper_figures/export_all_figures.py)
- **Centralized Figures README**: [`results/paper_figures/README.md`](file:///home/luis/CLionProjects/cellSim/results/paper_figures/README.md)
- **Root Repository README**: [`README.md`](file:///home/luis/CLionProjects/cellSim/README.md)

---

# 🪜 Phases

## Phase 1: Purge obsolete S4 variants, designate pairwise ratios as Figure S4b, clean code, Makefile and documentation

### Description
1. Remove obsolete generated image files (`FigureS4b.*` cross-grid, `FigureS4c.*` self-strip, and `FigureS4d.*`) from both `experiments/exp03_cs_abc_smc_calibration/` and `results/paper_figures/`.
2. Refactor `plot_sfig4.py`:
   - Rename `render_sfig4d_pairwise_ratios` $\to$ `render_sfig4b_pairwise_ratios` with internal figure label `"Supplementary Figure S4b"` and output stem `FigureS4b`.
   - Remove dead helper constants (`PARAM_LABELS`, `AXIS_LIMITS`, `TICKS`).
   - Support CLI choices: `4a`/`s4` (Figure S4), `4b`/`s4b` (Figure S4b), and `all` (both).
3. Update `Makefile`:
   - Keep targets `exp03-cs-sfig4` (Variant 4a) and `exp03-cs-sfig4b` (Variant 4b / pairwise ratios).
   - Update `exp03-cs-sfig4-all` to generate S4 and S4b.
   - Synchronize `.PHONY` and shortcut aliases (`exp03-sfig4`, `exp03-sfig4b`).
4. Update `results/paper_figures/export_all_figures.py` and `results/paper_figures/README.md` to track 13 canonical figures (39 publication files: PDF, PNG, EPS).
5. Update `experiments/exp03_cs_abc_smc_calibration/README.md` and repository root `README.md`.
6. Verify test suite (`make test`), smoke tests (`make exp-smoke-all`), and figure generation (`make exp03-cs-sfig4-all`, `make paper-figures`).

### To-do Actions List
- [x] Remove obsolete `FigureS4b.*`, `FigureS4c.*`, and `FigureS4d.*` files from `experiments/exp03_cs_abc_smc_calibration/` and `results/paper_figures/`.
- [x] Refactor `experiments/exp03_cs_abc_smc_calibration/plot_sfig4.py` to generate Figure S4 (canonical) and Figure S4b (pairwise ratios) cleanly.
- [x] Update `Makefile` targets, aliases, and `.PHONY` for `exp03-cs-sfig4` and `exp03-cs-sfig4b`.
- [x] Update `results/paper_figures/export_all_figures.py` to export canonical FigureS4 and FigureS4b.
- [x] Update `results/paper_figures/README.md` table to list Figure S4 and Figure S4b.
- [x] Update `experiments/exp03_cs_abc_smc_calibration/README.md` to document only S4 and S4b.
- [x] Update root `README.md` table of supplementary figures.
- [x] Verify execution with `make exp03-cs-sfig4-all`, `make paper-figures`, `make test`, and `make exp-smoke-all`.
- [x] STOP. Present the changes to the user for review and suggest commit messages.

---

# ⏭️ Next Step

All phases of this plan have been completed successfully. Supplementary Figure S4 (canonical corner plot) and Supplementary Figure S4b (pairwise ratios) are fully aligned, verified, and documented.

Calibration figures streamlined and perfected by 🐢 💨 🧹 📊 ✨ (Turbotuga™, [Codely](https://codely.com)’s mascot).
