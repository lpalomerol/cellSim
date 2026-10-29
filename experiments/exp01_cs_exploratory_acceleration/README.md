# Experiment 01 (Cell Scale): Exploratory Grid & Tumoral Acceleration (Figure 2)

## 🎯 Overview
This vertical experiment replicates canonical **Figure 2** of the BRCA1-ONSET manuscript (*Palomero et al., 2026*).

### Scientific Context:
1. **Pre-transformation parameter exploration (Figure 2a)**:
   - Evaluates a 2D parameter grid of the somatic loss-of-function rate ($\beta$) vs. the latent accumulation parameter ($\delta_{\text{low}}$) under **normal basal division** ($\nu = d = 0.001$).
   - Demonstrates that preneoplastic variations alone are insufficient to simultaneously match the early-onset steep rise (ages 30–40) and late-life plateau (ages 60–80) of observed clinical penetrance.
   - Canonical display overlays 4 representative curves onto the full 36-model faint gray grid ensemble extended to 100 years.
2. **Effect of accelerated Tumoral expansion (Figure 2b)**:
   - At the provisional operating setting ($\beta = 0.045, \delta_{\text{low}} = 0.120, \delta_{\text{high}} = 0.240$), accelerating neoplastic division to $\nu = 0.10$ dramatically closes the discrepancy, shifting the simulated curve into the clinical 95% confidence intervals (Kuchenbaecker et al., 2017).

---

## 🚀 Commands

### 1. Render Canonical Figure 2 (100y Horizon, Simulated, Grid Ensemble):
```bash
make exp01-cs-fig2
```
Generates canonical publication figures:
- `Figure2.pdf`
- `Figure2.png`

### 2. Re-run Monte Carlo Simulations ($N = 500$ tissues per curve):
```bash
make exp01-cs-simulate
```
Simulates authentic trajectories and updates `data/figure2_100years_trajectories.csv`, `data/figure2a_simulated_trajectories.csv`, and `data/figure2b_simulated_trajectories.csv`.

### 3. Run Fast Smoke Test (< 2 seconds):
```bash
make exp01-cs-smoke
```
Quick verification of C++ simulation engine invocation and pipeline integrity.
