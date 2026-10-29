# Experiment 06 (Cell Scale): Structural & Fixed-Parameter Sensitivity (Figures S2 & S3)

## 🎯 Overview
This vertical slice reproduces **Supplementary Figure S2** and **Supplementary Figure S3** of the BRCA1-ONSET manuscript (*Palomero et al., 2026*).

It evaluates the structural robustness of the discrete agent-based model and validates the invariance of calibrated parameters under perturbations to fixed background biological rates and structural model assumptions.

---

## 🧬 Scientific Context

### 1. Fixed-Parameter Sensitivity (Supplementary Figure S2)
Evaluates one-at-a-time (OAT) perturbations of $\pm 10\%$ and $\pm 25\%$ on all 6 fixed background biological parameters:
1. $\mu_{\text{TP53}}$ (`tp53_rate`, default $0.003\,\text{cycle}^{-1}$): Baseline somatic mutation probability of *TP53*.
2. $\theta_{D2}$ (`d2_threshold`, default $5.0$): Neoplastic transformation threshold triggering immune evasion.
3. $\theta_{D1}$ (`d1_threshold`, default $2.0$): Premalignant primed threshold.
4. $d$ (`division_rate`, default $0.001\,\text{cycle}^{-1}$): Basal cell turnover division rate.
5. $K_{\text{tum}}$ (`tumor_threshold`, default $0.05$): Tissue-level diagnostic detection cutoff ($5\%$).
6. $A_{\text{it}}$ (`age_drift`, default $10^{-5}$): Age-dependent linear drift in latent damage accumulation.

**Key Findings**:
- **Panel a**: Maximum absolute change in cumulative penetrance (percentage points) remains modest across $\pm 10\%$ shifts ($\le 12\,\text{pp}$).
- **Panel b**: Agreement with Kuchenbaecker et al. (2017) clinical data (observed-CI-weighted SSE) remains robust around the baseline reference ($\text{SSE} \approx 14.0$).
- **Panel c**: Trajectory overlays under $\pm 25\%$ perturbations track within or near the empirical 95% confidence intervals.

### 2. Structural Sensitivity (Supplementary Figure S3)
Tests fundamental model assumptions regarding driver interactions and rate-limiting bottlenecks:
1. **BRCA1 D2 Multiplier ($\kappa_{\text{BRCA1}}$)** (Panel a):
   - Compares $\kappa_{\text{BRCA1}} = 1.0$ (equal contribution), $\kappa_{\text{BRCA1}} = 2.0$ (baseline calibrated reference), and $\kappa_{\text{BRCA1}} = 3.0$ (amplified contribution).
   - Calibrated penetrance curves track closely together ($\text{SSE} \le 19.8$), demonstrating that the exact weighting coefficient does not alter the fundamental sigmoidal onset kinetics.
2. **TP53 Mutation Rate Bottleneck ($\mu_{\text{TP53}}$)** (Panel b):
   - Compares $\mu_{\text{TP53}} = 0.003$ (baseline) against 3-fold ($\mu = 0.001$) and 10-fold ($\mu = 0.0003$) reductions.
   - Shows that penetrance collapses precipitously ($9.7\%$ and $1.7\%$ cumulative risk at 80 years), proving that somatic *TP53* transition is an indispensable, rate-limiting bottleneck in the evolutionary trajectory.

---

## 🚀 Commands

### 1. Render Figures (Instantaneous via authentic simulation data):
```bash
make exp06-cs-sfig2   # Generates Supplementary_Figure_S2.pdf / .png and FigureS2.pdf / .png
make exp06-cs-sfig3   # Generates Supplementary_Figure_S3.pdf / .png and FigureS3.pdf / .png
```

### 2. Fast Pipeline Smoke Test (< 2 seconds):
```bash
make exp06-cs-smoke
```
Runs a mini-pilot through the C++ `run_bootstrapping` binary to verify parameter passing and metric generation.

### 3. Re-simulate Full Sensitivity Data ($N = 300$ tissues per condition):
```bash
make exp06-cs-simulate
```
Recomputes all OAT perturbations and structural conditions, saving to `data/S2_*.csv` and `data/S3_*.csv`.
