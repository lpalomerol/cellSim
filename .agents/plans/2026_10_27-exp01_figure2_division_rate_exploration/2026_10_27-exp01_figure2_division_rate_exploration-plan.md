---
name: "exp01_figure2_division_rate_exploration"
description: "Exploration of increased basal division rate (d=0.01) and generation of a new Figure 2 overlaying representative and 36-model grid trajectories"
created_at: "2026-10-27T17:30:16+01:00"

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

last_implementation_at: "2026-10-27T17:51:00+01:00"
has_completed_all_phases: true
---

# 🎯 Goal

Investigate the impact of increasing the basal cell division rate from $d = 0.001$ to $d = 0.01$ in the CellSim pre-transformation and accelerated expansion regimes. Parameterize the exploratory pipeline to run Monte Carlo simulations with $d = 0.01$ and generate a new publication-quality figure (`Figure2_extra_grid_d0.01`) that displays the 4 representative parameter settings alongside the 36-model background grid ensemble.

---

# 👀 Context

- **Experiment 01 Directory**: [`experiments/exp01_exploratory_acceleration/`](file:///home/luis/CLionProjects/cellSim/experiments/exp01_exploratory_acceleration)
- **Simulation Runner Script**: [`experiments/exp01_exploratory_acceleration/run_exploratory.py`](file:///home/luis/CLionProjects/cellSim/experiments/exp01_exploratory_acceleration/run_exploratory.py)
- **Figure 2 Plotting Script**: [`experiments/exp01_exploratory_acceleration/plot_figure2.py`](file:///home/luis/CLionProjects/cellSim/experiments/exp01_exploratory_acceleration/plot_figure2.py)
- **Simulation Binary**: [`build/run_bootstrapping`](file:///home/luis/CLionProjects/cellSim/build/run_bootstrapping) (compiled from C++20 engine with OpenMP)
- **C++ Cell Division Logic**: [`src/domain/cell/AgenticCell.cpp`](file:///home/luis/CLionProjects/cellSim/src/domain/cell/AgenticCell.cpp#L308-L335)
- **Project Makefile**: [`Makefile`](file:///home/luis/CLionProjects/cellSim/Makefile)
- **Experiment Documentation**: [`experiments/exp01_exploratory_acceleration/README.md`](file:///home/luis/CLionProjects/cellSim/experiments/exp01_exploratory_acceleration/README.md)

---

# 📜 Public Contracts

### CLI Contracts in `run_exploratory.py`
- `--division-rate`: Float argument (default: `0.001`, accepts `0.01`) passed as `--division-rate` to `run_bootstrapping`.
- `--file-suffix`: String suffix appended to generated CSV outputs (e.g., `_d001`), producing:
  - `data/figure2a_simulated_trajectories_d001.csv`
  - `data/figure2b_simulated_trajectories_d001.csv`
  - `data/figure2a_all_grid_trajectories_d001.csv`
  - `data/figure2_simulated_summary_d001.csv`
- `--generate-grid-trajectories`: Flag to simulate and export all 36 parameter grid curves ($\beta \times \delta_{low}$) at the specified division rate.

### CLI Contracts in `plot_figure2.py`
- `--data-suffix`: String suffix to select data files (default: empty for standard $d=0.001$, `_d001` for $d=0.01$).
- `--output-name`: Output basename for exported PDF and PNG figures (defaulting to `Figure2_extra_grid_d0.01` when rendering $d=0.01$ with grid).

### Makefile Targets
- `exp01-simulate-d001`: Run Monte Carlo simulations for representative curves and all 36 grid models with $d=0.01$.
- `exp01-fig2-grid-d001`: Render `Figure2_extra_grid_d0.01.png` and `Figure2_extra_grid_d0.01.pdf`.

---

# 🪜 Phases

## Phase 1: Parameterize basal division rate ($d$) and grid simulation pipeline with fast verification

### Description
Refactor `run_exploratory.py` and `plot_figure2.py` to support arbitrary division rates ($d$), configurable data file suffixes, and automated export of all 36 grid trajectories. Verify the integration using fast smoke tests without altering existing $d=0.001$ baseline files.

### To-do Actions List
- [x] Add `--division-rate` and `--file-suffix` arguments to [`run_exploratory.py`](file:///home/luis/CLionProjects/cellSim/experiments/exp01_exploratory_acceleration/run_exploratory.py).
- [x] Update `generate_figure2_trajectories` to pass the configured `division_rate` to `evaluate_simulation`.
- [x] Add `generate_all_grid_trajectories` function in [`run_exploratory.py`](file:///home/luis/CLionProjects/cellSim/experiments/exp01_exploratory_acceleration/run_exploratory.py) to simulate and export annual cumulative curves for all 36 combinations of `BETA_COARSE` and `DLOW_COARSE`.
- [x] Add `--data-suffix` argument to [`plot_figure2.py`](file:///home/luis/CLionProjects/cellSim/experiments/exp01_exploratory_acceleration/plot_figure2.py) and update legend/labels to reflect the active division rate ($d = 0.01$).
- [x] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make exp01-smoke` and `python3 experiments/exp01_exploratory_acceleration/run_exploratory.py --smoke-test`). Fix issues if any.
- [x] STOP. Present the changes to the user for review and suggest commit messages (or pull request titles, when the phases are implemented through pull requests). Do NOT proceed to the next phase until the user explicitly asks.

---

## Phase 2: Execute $d=0.01$ Monte Carlo simulations, generate new Figure 2 with representative & grid overlays, and wire Makefile targets

### Description
Execute the full Monte Carlo simulations with $d = 0.01$ ($N = 1000$ tissues per curve) for both representative curves (Panels a & b) and the 36 background grid combinations. Render `Figure2_extra_grid_d0.01.png` and `Figure2_extra_grid_d0.01.pdf`, wire makefile targets, and document comparative metrics against $d=0.001$.

### To-do Actions List
- [x] Run full Monte Carlo simulations with $d = 0.01$ and export `figure2a_simulated_trajectories_d001.csv`, `figure2b_simulated_trajectories_d001.csv`, `figure2a_all_grid_trajectories_d001.csv`, and `figure2_simulated_summary_d001.csv`.
- [x] Render `Figure2_extra_grid_d0.01.png` and `Figure2_extra_grid_d0.01.pdf` displaying the 4 representative curves overlaid on the 36-model faint gray grid ensemble in Panel a, and the basal vs accelerated curves in Panel b.
- [x] Add `exp01-simulate-d001` and `exp01-fig2-grid-d001` targets to [`Makefile`](file:///home/luis/CLionProjects/cellSim/Makefile).
- [x] Update [`experiments/exp01_exploratory_acceleration/README.md`](file:///home/luis/CLionProjects/cellSim/experiments/exp01_exploratory_acceleration/README.md) with instructions, comparative analysis, and SSE values ($d=0.001$ vs $d=0.01$).
- [x] Verify the changes in terms of typechecking, linting and tests using the project's verification command (`make exp01-fig2-grid-d001` and `make test`). Fix issues if any.
- [x] STOP. Present the changes to the user for review and suggest commit messages (or pull request titles, when the phases are implemented through pull requests). Do NOT proceed to the next phase until the user explicitly asks.

---

# ⏭️ Next Step

All phases of this plan have been completed successfully. The new figure with representative and 36 grid models under $d = 0.01$ is generated and documented.

All simulation goals reached full speed across the finish line! 🏁 🐢 💨 🏆 (Turbotuga™, [Codely](https://codely.com)’s mascot).
