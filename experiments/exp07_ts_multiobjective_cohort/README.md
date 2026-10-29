# Experiment 07 (TS): Multiobjective Virtual Cohort Validation

## Overview
This experiment executes and visualizes the multiobjective calibrated tissue-scale model ($10^7$ cells per virtual breast organoid, $N=10,000$ virtual tissue cohort) reproducing **Figure 6** of *Palomero et al. (2026)*.

## Panels
- **Panel a**: Age-dependent cumulative penetrance comparison (variable tissue restraint vs homogeneous restraint vs Kuchenbaecker et al. 2017 clinical penetrance).
- **Panel b**: Somatic BRCA1 loss composition in emerging tumors (predominant $BRCA1^{+/-}$, mixed, predominant $BRCA1^{-/-}$).
- **Panel c**: Founder-lineage complexity distribution across diagnosis age bins ($<30$, $30\text{--}39$, $\dots$, $80$).
- **Panel d**: Impact of local tissue barrier heterogeneity ($\sigma_{\theta_{D2}} \in \{0.20, 0.38, 0.60\}$).
- **Panel e**: Somatic driver-mediated checkpoint relaxation ($\gamma \in \{10^{-4}, 2\times 10^{-4}, 3\times 10^{-4}\}$, with inset quantifying % tumors enabled by checkpoint bypass).

## Files
- `simulate_figure6_data.py`: CLI driver calling `tissue_scale_sim` to simulate condition cohorts.
- `plot_figure6.py`: Canonical 5-panel publication plotter.
- `data/`:
  - `clinical_penetrance.csv`: Reference cohort penetrance and 95% CIs.
  - `final_multiobjective_validation_10000.csv`: 10,000 calibrated virtual tissues.
  - `homogeneous_validation_3000.csv`: 3,000 homogeneous restraint tissues.
  - `figure6d_sigma_020.csv` / `figure6d_sigma_060.csv`: Heterogeneity cohorts.
  - `figure6e_gamma_1e4.csv` / `figure6e_gamma_2e4.csv` / `figure6e_gamma_3e4.csv`: Driver cohorts.

## Reproduction
```bash
# Fast smoke test
make exp07-ts-smoke

# Render canonical Figure 6
make exp07-ts-fig6
```
