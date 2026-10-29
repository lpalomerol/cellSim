# Experiment 01: Exploratory Grid & Tumoral Acceleration (Figure 2)

## 🎯 Overview
This vertical experiment replicates **Figure 2** of the BRCA1-ONSET manuscript (*Palomero et al., 2026*).

### Scientific Context:
1. **Pre-transformation parameter exploration (Figure 2a)**:
   - Evaluates a 2D grid of the somatic loss-of-function rate of the remaining BRCA1 allele ($\beta$) vs. the latent accumulation parameter ($\delta_{\text{low}}$) under **normal basal division** ($\nu = d = 0.001$).
   - Demonstrates that preneoplastic variations alone are insufficient to simultaneously match the early-onset steep rise (ages 30–40) and late-life plateau (ages 60–80) of observed penetrance.
2. **Effect of accelerated Tumoral expansion (Figure 2b)**:
   - At the provisional operating setting ($\beta = 0.045, \delta_{\text{low}} = 0.120, \delta_{\text{high}} = 0.240$), accelerating neoplastic division to $\nu = 0.10$ dramatically closes the discrepancy, shifting the simulated curve into the clinical 95% confidence intervals.

---

## 🚀 Commands

### 1. Render Simulated Figure 2 (Authentic Monte Carlo data):
```bash
make exp01-fig2
```
Generates `Figure2_simulated.pdf` and `Figure2_simulated.png` from `data/figure2*_simulated_trajectories.csv`.

### 2. Render Figure 2 Extra (With all 36 grid trajectories in faint gray):
```bash
make exp01-fig2-grid
```
Generates `Figure2_extra_grid.pdf` and `Figure2_extra_grid.png`, showing the full background distribution ensemble of the 36 unaccelerated models.

### 2b. Render Figure 2 Extra with d = 0.01 (Representative + 36 Grid Trajectories):
```bash
make exp01-fig2-grid-d001
```
Generates `Figure2_extra_grid_d0.01.pdf` and `Figure2_extra_grid_d0.01.png`, displaying the representative curves and all 36 background grid models simulated under 10× higher normal basal division ($d = 0.01$).

### 3. Render Figure 2 Extended to 100 Years:
```bash
make exp01-fig2-100years
```
Generates `Figure2_100years.pdf` and `Figure2_100years.png`, extending the simulation horizon to age 100 with clinical follow-up boundary at age 80.

### 4. Render Figure 2 Extra Extended to 100 Years (With all 36 grid trajectories):
```bash
make exp01-fig2-grid-100years
```
Generates `Figure2_extra_grid_100years.pdf` and `Figure2_extra_grid_100years.png`, displaying all 36 unaccelerated background grid models up to age 100.

### 5. Render Paper Reference Figure 2:
```bash
make exp01-fig2-paper
```
Generates `Figure2_paper.pdf` and `Figure2_paper.png` without modifying the simulated files.

### 6. Re-run Monte Carlo Simulations ($N = 500$ tissues per curve, ~7s total):
```bash
make exp01-simulate
```
Updates `figure2a_simulated_trajectories.csv` and `figure2b_simulated_trajectories.csv`.

### 7. Re-run Monte Carlo Simulations with d = 0.01 (Representative + Grid, N = 500 per curve):
```bash
make exp01-simulate-d001
```
Simulates both representative curves and all 36 grid models with $d = 0.01$, saving to `data/*_d001.csv`.

### 8. Run fast smoke test (< 5 seconds):
```bash
make exp01-smoke
```

---

## 📊 Comparative Analysis: Basal Division Rate ($d = 0.001$ vs $d = 0.01$)

| Regime | Setting / Curve | $d = 0.001$ SSE | $d = 0.01$ SSE | Risk at Age 80 ($d=0.001$) | Risk at Age 80 ($d=0.01$) |
|---|---|---|---|---|---|
| **Panel a (Pre-transformation)** | Representative Higher-$\beta$ | ~2078 (grid) | 1473.9 | 45.0% | 58.3% |
| **Panel a (Pre-transformation)** | Representative Lower-$\beta$ | ~7750 | 5454.9 | 61.4% | 79.0% |
| **Panel a (Pre-transformation)** | Representative Higher-$\delta_{low}$ | 2888.3 | 2201.1 | 75.1% | 76.5% |
| **Panel a (Pre-transformation)** | Representative Lower-$\delta_{low}$ | 15297.0 | 6414.5 | 0.0% | 52.6% |
| **Panel b (Tumoral)** | Accelerated expansion ($\nu = 0.10$) | **48.5** | 317.7 | **70.3%** | 79.5% |
| **Panel b (Basal)** | Basal expansion ($\nu = d$) | 3079.3 | 3375.0 | 69.8% | 65.1% |

### Key Scientific Insights:
1. **Unaccelerated Models Fail Regardless of $d$**: Raising $d$ by 10× ($0.001 \to 0.01$) increases cell turnover and early clonal emergence, but **all 36 unaccelerated models remain unable to reproduce the clinical sigmoidal shape** (early low penetrance at 30y, steep rise 30–50y, and late-age saturation).
2. **Accelerated Expansion Remains Necessary**: Accelerating neoplastic division ($\nu = 0.10$) is essential to close the discrepancy with Kuchenbaecker et al. (2017) clinical penetrance data. Under $d = 0.001$, calibrated agreement is optimal ($SSE = 48.5$), while $d = 0.01$ pushes late penetrance slightly above the clinical mean ($79.5\%$ vs $70.0\%$).
