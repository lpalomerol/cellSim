# Experiment 09 (TS): Checkpoint Relaxation Sensitivity Grid

## Overview
This experiment tests whether an alternative evolutionary order—where somatic driver mutations bypass apoptosis checkpoints prior to biallelic $TP53$ inactivation—is compatible with clinical penetrance. Reproduces **Supplementary Figure S7** of *Palomero et al. (2026)* (matching review manuscript `Palomero_SFig7_100126.pdf`).

## Biological Rationale & Grid Definition
- Under canonical BRCA1 pathogenesis, homozygous $BRCA1$ loss triggers apoptosis unless preceded by $TP53$ loss or equivalent checkpoint defect.
- This experiment assesses a 2D parameter grid:
  - $\gamma \in [0, 3\times 10^{-4}]$: Somatic driver mutation rate (cell$^{-1}$ year$^{-1}$) conferring checkpoint relaxation.
  - $p_{\text{bypass}} \in [0.25, 1.00]$: Probability that cells survive $BRCA1$ loss without prior $TP53$ loss.
- **Left Heatmap (Penetrance SSE)**: Low $\gamma \le 2\times 10^{-5}$ remains concordant with observed epidemiology ($\text{SSE} \le 25$). Higher $\gamma \ge 10^{-4}$ causes early penetrance blowup ($\text{SSE} > 150$).
- **Right Heatmap (% Tumors Enabled)**: Measures the proportion of emerging tumors that actually utilized this non-canonical bypass pathway ($0\%$ to $27\%$).

## Files
- `run_checkpoint_grid.py`: Runs full 24-point grid with `tissue_scale_sim` (or `--smoke-test`).
- `plot_sfig7.py`: Plots dual publication heatmaps matching `Palomero_SFig7_100126.pdf`.
- `data/checkpoint_grid_results.csv`: Exact grid results from the calibrated parameter space.

## Reproduction
```bash
# Smoke test (runs all 24 grid points in ~2s)
make exp09-ts-smoke

# Render Supplementary Figure S7
make exp09-ts-sfig7
```
