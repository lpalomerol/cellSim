# Experiment 02: Global Sensitivity Analysis LHS & PRCC (Supplementary Figure S1)

## 🎯 Overview
This vertical slice replicates **Supplementary Figure S1** of the BRCA1-ONSET manuscript (*Palomero et al., 2026*).

### Scientific Context:
1. **Multi-Parameter Space-Filling Design (LHS)**:
   - Uses **Latin Hypercube Sampling (LHS)** across $N = 1,000$ parameter combinations spanning the physiological and mathematical bounds of the 10 core biological parameters:
     - $\beta$ (BRCA1 second-hit somatic rate): $[0.010, 0.100]$
     - $\mu_{\text{TP53}}$ (TP53 loss-of-function rate): $[0.0003, 0.006]$
     - $\delta_{\text{low}}$ (latent DNA damage accumulation in heterozygous cells): $[0.020, 0.200]$
     - $\delta_{\text{high}}/\delta_{\text{low}}$ (biallelic DNA damage ratio): $[1.200, 4.000]$
     - $\theta_{D1}$ (mitotic checkpoint trigger threshold): $[1.500, 3.000]$
     - $\theta_{D2}$ (immune clearance evasion threshold): $[3.000, 7.000]$
     - $d$ (basal epithelial division rate): $[0.0003, 0.003]$
     - $\kappa_{\text{BRCA1}}$ (BRCA1-dependent damage multiplier): $[1.000, 3.000]$
     - $A_{\text{it}}$ (background cellular age drift): $[0.0, 5\times 10^{-5}]$
     - $\nu$ (neoplastic transformed division rate): $[0.001, 0.300]$
   - Evaluates each parameter point on 200 independent simulated tissues (500 cells, max $t = 80$ years) measuring weighted distance to clinical penetrance ($SSE_w$).

2. **Panel a: Overall Parameter Importance (Random Forest Permutation Importance)**:
   - Fits a Random Forest Regressor on $SSE_w$ to quantify nonlinear parameter influence.
   - Highlights the dominant influence of $\mu_{\text{TP53}}$ (~31%), $\nu$ (~15%), $\delta_{\text{low}}$ (~14%), $\beta$ (~10%), and $\theta_{D2}$ (~9%).

3. **Panel b: Age-Specific PRCC Dynamics (Partial Rank Correlation Coefficients)**:
   - Computes PRCC across clinical milestone ages (30, 40, 50, 60, 70, 80 years) controlling for confounding correlations.
   - Reveals that early-onset penetrance (ages 30–40) is strongly driven by $\beta$ and $\nu$, while late penetrance is governed by $\mu_{\text{TP53}}$ and immune evasion ($\theta_{D2}$).

4. **Panel c: Posterior ECDFs versus Uniform Priors**:
   - Assesses parameter identifiability and prior-to-posterior information gain using Empirical Cumulative Distribution Functions (ECDFs) from the exploratory 10-generation ABC-SMC population ($N=250$ particles).
   - Contrasts the Global model with the Locked ablation model ($\nu = d$), exposing the mutation overcompensation artifact.

---

## 🚀 Commands

### 1. Render Supplementary Figure S1 (Instantaneous via precomputed posterior):
```bash
make exp02-sfig1
```
Generates `Supplementary_Figure_S1.pdf` (vector) and `Supplementary_Figure_S1.png` (300 DPI) matching the manuscript layout.

### 2. Fast Pipeline Smoke Test (< 2 seconds):
```bash
make exp02-smoke
```
Verifies Latin Hypercube Sampling, C++ binary execution, stdout parsing, and metrics pipeline integrity using a minimal 2-point pilot.

### 3. Full Simulation Campaign (Optional heavy recomputation):
```bash
make exp02-simulate
```
Recomputes all 1,000 LHS parameter combinations ($N = 200$ virtual tissues per combination, ~200,000 simulated tissues total) and updates all CSV assets in `data/`.
