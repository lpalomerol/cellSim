---
name: "sfig4_variant_4b_beta_normalized"
description: "Implementation of Variant 4b (cross-parameter grid) and Variant 4c (self-ratio strip) for Supplementary Figure S4 normalized by BRCA1 mutation rate (Beta) to evaluate parameter decoupling and identifiability"
created_at: "2026-10-28T07:58:30+01:00"

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
    version: "3.8 Flash (Low)"
    reasoning_effort: "low"

last_implementation_at: "2026-10-28T08:42:00+01:00"
has_completed_all_phases: true
---

# 🎯 Goal

Implement Variant 4b (cross-parameter scatterplot grid) and Variant 4c (self-ratio strip) for Supplementary Figure S4 in [`plot_sfig4.py`](file:///home/luis/CLionProjects/cellSim/experiments/exp03_abc_smc_calibration/plot_sfig4.py). In Variant 4b, the X-axis represents the dimensionless rate ratio normalized by the BRCA1 mutation rate ($\theta / \beta$), while the Y-axis displays complementary unnormalized parameters ($\theta'$). In Variant 4c, each subplot relates the dimensionless ratio ($\theta / \beta$) directly to its own numerator parameter ($\theta$). This visualization directly addresses structural identifiability and parameter decoupling by demonstrating that the sharply identified relative rates decouple from individual parameter variations.

---

# 👀 Context

- **Supplementary Figure S4 Script**: [`experiments/exp03_abc_smc_calibration/plot_sfig4.py`](file:///home/luis/CLionProjects/cellSim/experiments/exp03_abc_smc_calibration/plot_sfig4.py)
- **ABC-SMC Posterior Particles Data**: [`experiments/exp03_abc_smc_calibration/data/posterior_particles_250.csv`](file:///home/luis/CLionProjects/cellSim/experiments/exp03_abc_smc_calibration/data/posterior_particles_250.csv)
- **Experiment 03 Documentation**: [`experiments/exp03_abc_smc_calibration/README.md`](file:///home/luis/CLionProjects/cellSim/experiments/exp03_abc_smc_calibration/README.md)
- **Project Makefile**: [`Makefile`](file:///home/luis/CLionProjects/cellSim/Makefile)

---

# 📜 Public Contracts

### CLI Contracts in `plot_sfig4.py`
- `--variant`: Choice argument (`4a`, `4b`, `4c`, `all`, default: `all`) specifying whether to generate:
  - `4a`: baseline canonical parameter corner plot (Figure S4).
  - `4b`: cross-parameter ratio grid (Figure S4b).
  - `4c`: self-parameter ratio strip (Figure S4c).
  - `all`: generates all figures.
- `--output-dir`: Optional directory path argument to override default output destination (`experiments/exp03_abc_smc_calibration`).

### Generated Output Files
- `experiments/exp03_abc_smc_calibration/FigureS4.pdf`: Baseline 4x4 posterior corner plot (Variant 4a).
- `experiments/exp03_abc_smc_calibration/FigureS4.png`: High-resolution raster (300 DPI) for Variant 4a.
- `experiments/exp03_abc_smc_calibration/FigureS4b.pdf`: Cross-parameter ratio grid (Variant 4b) with X-axis as $\theta/\beta$ and Y-axis as $\theta'$.
- `experiments/exp03_abc_smc_calibration/FigureS4b.png`: High-resolution raster (300 DPI) for Variant 4b.
- `experiments/exp03_abc_smc_calibration/FigureS4c.pdf`: Self-parameter ratio strip (Variant 4c) with X-axis as $\theta/\beta$ and Y-axis as $\theta$.
- `experiments/exp03_abc_smc_calibration/FigureS4c.png`: High-resolution raster (300 DPI) for Variant 4c.

### Makefile Targets
- `exp03-sfig4b`: Target executing `python3 experiments/exp03_abc_smc_calibration/plot_sfig4.py --variant 4b`.
- `exp03-sfig4c`: Target executing `python3 experiments/exp03_abc_smc_calibration/plot_sfig4.py --variant 4c`.
- `exp03-sfig4`: Target executing `python3 experiments/exp03_abc_smc_calibration/plot_sfig4.py --variant all`.

---

# 🪜 Phases

## Phase 1: Modularize plotting pipeline, calculate normalized rate ratios, and add CLI contract

### Description
Refactor `plot_sfig4.py` so that bivariate density estimation and corner plot rendering accept modular parameter definitions. Implement dynamic calculation of rate ratios normalized by `brca1_rate` ($\beta$), add the `--variant` CLI argument, and verify backwards compatibility with baseline Variant 4a generation.

### To-do Actions List
- [x] Add CLI argument parsing with `--variant` (`4a`, `4b`, `4c`, `all`) and `--output-dir` in [`experiments/exp03_abc_smc_calibration/plot_sfig4.py`](file:///home/luis/CLionProjects/cellSim/experiments/exp03_abc_smc_calibration/plot_sfig4.py).
- [x] Compute normalized parameter columns in the loaded particle DataFrame:
  - `low_delta_norm = low_delta / brca1_rate` ($\delta_{\rm low} / \beta$)
  - `high_delta_norm = high_delta / brca1_rate` ($\delta_{\rm high} / \beta$)
  - `neoplastic_div_rate_norm = neoplastic_div_rate / brca1_rate` ($\nu / \beta$)
- [x] Refactor the corner plot generation into a reusable plotting function parameterized by dimension metadata (keys, mathematical labels, units, ranges, and formatting).
- [x] Verify that running with `--variant 4a` reproduces identical `FigureS4.pdf` and `FigureS4.png` outputs without regression.
- [x] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`python3 experiments/exp03_abc_smc_calibration/plot_sfig4.py --variant 4a`). Fix issues if any.
- [x] STOP. Present the changes to the user for review and suggest commit messages (or pull request titles, when the phases are implemented through pull requests). Do NOT proceed to the next phase until the user explicitly asks.

---

## Phase 2: Design Variant 4b cross-parameter grid, Variant 4c self-ratio strip, wire Makefile targets, and update docs

### Description
Implement the publication-grade $3 \times 2$ cross-parameter grid for Variant 4b and the $1 \times 3$ self-ratio strip for Variant 4c. Include bivariate density contours, scatter markers, and median indicators. Wire Makefile targets and update documentation.

### To-do Actions List
- [x] Implement the cross-parameter scatterplot grid in [`experiments/exp03_abc_smc_calibration/plot_sfig4.py`](file:///home/luis/CLionProjects/cellSim/experiments/exp03_abc_smc_calibration/plot_sfig4.py) for Variant 4b covering the 6 cross-parameter combinations:
  - Panel 1: X = $\delta_{\mathrm{high}} / \beta$, Y = $\delta_{\mathrm{low}}$
  - Panel 2: X = $\delta_{\mathrm{low}} / \beta$, Y = $\delta_{\mathrm{high}}$
  - Panel 3: X = $\delta_{\mathrm{low}} / \beta$, Y = $\nu$
  - Panel 4: X = $\nu / \beta$, Y = $\delta_{\mathrm{low}}$
  - Panel 5: X = $\delta_{\mathrm{high}} / \beta$, Y = $\nu$
  - Panel 6: X = $\nu / \beta$, Y = $\delta_{\mathrm{high}}$
- [x] Implement Variant 4c self-ratio strip ($1 \times 3$) comparing each normalized ratio against its own numerator parameter ($\theta / \beta$ vs $\theta$).
- [x] Overlay 2D KDE contour density maps with individual particle scatter points and dashed vertical lines at the weighted median of the X-axis ratio.
- [x] Export `FigureS4b.pdf`, `FigureS4b.png`, `FigureS4c.pdf`, and `FigureS4c.png` alongside baseline `FigureS4` outputs.
- [x] Add the `exp03-sfig4b` and `exp03-sfig4c` targets in [`Makefile`](file:///home/luis/CLionProjects/cellSim/Makefile) and update `exp03-sfig4` to generate all variants via `--variant all`.
- [x] Document the interpretation of the cross-parameter and self-ratio plots in [`experiments/exp03_abc_smc_calibration/README.md`](file:///home/luis/CLionProjects/cellSim/experiments/exp03_abc_smc_calibration/README.md).
- [x] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make exp03-sfig4`). Fix issues if any.
- [x] STOP. Present the changes to the user for review and suggest commit messages (or pull request titles, when the phases are implemented through pull requests). Do NOT proceed to the next phase until the user explicitly asks.

---

# ⏭️ Next Step

All phases of this plan have been completed successfully. Figure S4 (canonical corner plot), Figure S4b (cross-parameter ratio grid), and Figure S4c (self-ratio strip) are generated, wired into Makefile targets, and fully documented.

All calibration and plotting milestones conquered by 🐢 💨 🏆 ✨ (Turbotuga™, [Codely](https://codely.com)’s mascot).
