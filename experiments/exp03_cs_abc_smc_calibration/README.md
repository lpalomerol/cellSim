# Experiment 03 (Cell Scale): Bayesian ABC-SMC Calibration & Posterior Predictive (Figure 3 & Figure S4)

## 🎯 Overview
This vertical slice replicates **Figure 3** and **Supplementary Figure S4** of the BRCA1-ONSET manuscript (*Palomero et al., 2026*).

It implements the canonical Approximate Bayesian Computation Sequential Monte Carlo (ABC-SMC) calibration algorithm that infers the 4 unconstrained mechanistic parameters of the discrete agent-based model against the Kuchenbaecker et al. (2017) clinical cohort penetrance.

---

## 🧬 Scientific & Mathematical Context

### 1. Inferred Mechanistic Parameters
The 4 unconstrained cellular parameters estimated by ABC-SMC are:
1. $\beta$ (`brca1_rate`): Somatic inactivation rate of the second functional *BRCA1* allele $[0.025, 0.080]\,\text{year}^{-1}$.
2. $\delta_{\text{low}}$ (`low_delta`): Latent DNA damage accumulation probability in *BRCA1*-heterozygous cells $[0.060, 0.220]$.
3. $\delta_{\text{high}}$ (`high_delta`): Biallelic DNA damage accumulation probability in homozygous-null cells $[0.120, 0.600]$.
4. $\nu$ (`neoplastic_div_rate`): Division rate of transformed neoplastic cells ($D_2 \ge \theta_{D2}$) $[0.100, 0.240]\,\text{cycle}^{-1}$.

**Biological Priors and Constraints**:
- Uniform marginal priors within specified bounds.
- **Structural Constraint**: $\delta_{\text{high}} > \delta_{\text{low}}$ (biallelic loss cannot yield lower damage accumulation than heterozygous state).
- All remaining background parameters are fixed at their biological reference values validated by global sensitivity invariance:
  - $\mu_{\text{TP53}} = 0.003$
  - $\theta_{D1} = 2.0$, $\theta_{D2} = 5.0$
  - Basal division $d = 0.001\,\text{cycle}^{-1}$
  - $\kappa_{\text{BRCA1}} = 2.0$
  - Age drift $A_{\text{it}} = 10^{-5}$
  - Neoplastic diagnostic threshold = $5\%$ ($25/500$ cells).

### 2. Bayesian Calibration Pipeline (ABC-SMC)
- **Generations & Population**: $T = 20$ successive generations, maintaining $N = 250$ accepted particles per generation.
- **Weighted Distance Metric**:
  $$\text{SSE}_w = \sum_{a \in \{30, 40, 50, 60, 70, 80\}} \left(\frac{R_{\text{sim}}(a) - R_{\text{obs}}(a)}{\sigma_a}\right)^2$$
  where $\sigma_a = (\text{CI}_{\text{high}}(a) - \text{CI}_{\text{low}}(a)) / (2 \times 1.96)$ is derived from the Kuchenbaecker 2017 95% confidence intervals.
- **Adaptive Tolerance Schedule**:
  $$\epsilon_t = Q_{0.75}(\mathcal{D}_{t-1})$$
  starting from Generation 0 with unconstrained prior acceptance.
- **Perturbation Kernel**:
  $$\Sigma_t = 2 \times \operatorname{Cov}_w(\theta_{t-1}) + \text{diag}(10^{-7})$$
  using the Beaumont et al. (2009) / Toni et al. (2009) optimal empirical covariance scaling.
- **Sequential Importance Sampling (SIS)**:
  Particles are weighted by prior density over perturbed proposal mixture, with log-sum-exp numerical stabilization and normalization.

### 3. Figure 3: Calibration Performance & Posterior Predictive
- **Panel a (Left)**: Convergence of ABC-SMC tolerance threshold ($\epsilon$), median weighted $\text{SSE}_w$, and interquartile range (IQR) across the 20 generations.
- **Panel a (Right)**: Acceptance rate decay and Effective Sample Size (ESS) preservation (maintaining $>80\%$ of particle count).
- **Panel b**: Posterior predictive cumulative penetrance curve across ages 30–80 years evaluated across 1,000 simulated virtual tissues per particle, showing posterior median and 95% credible intervals against Kuchenbaecker et al. (2017).

### 4. Supplementary Figures S4 & S4b
- **Figure S4 (Canonical Corner Plot)**:
  - **Diagonal**: 1D marginal posterior Kernel Density Estimates (KDEs) with posterior median (solid line) and 95% credible intervals (dashed lines).
  - **Lower Triangle**: Smoothed 2D bivariate posterior contour density maps capturing compensatory parameter trade-offs (e.g., negative correlation between $\nu$ and $\delta_{\text{low}}$).
  - **Upper Triangle**: Normalized posterior density scale legend.
- **Figure S4b (1D Posterior Distributions of All 6 Pairwise Ratios $2 \times 3$)**:
  - Directly addresses the relative time-scale identifiability across all $\binom{4}{2} = 6$ parameter pairs:
    - (a) $R_1 = \delta_{\rm high} / \delta_{\rm low}$ (Median: 3.65, 95% CrI: [1.15, 6.48])
    - (b) $R_2 = \delta_{\rm low} / \beta$ (Median: 2.29, 95% CrI: [1.95, 2.65])
    - (c) $R_3 = \nu / \beta$ (Median: 3.50, 95% CrI: [2.08, 5.60])
    - (d) $R_4 = \nu / \delta_{\rm low}$ (Median: 1.50, 95% CrI: [0.84, 2.72])
    - (e) $R_5 = \delta_{\rm high} / \beta$ (Median: 8.25, 95% CrI: [2.91, 13.13])
    - (f) $R_6 = \nu / \delta_{\rm high}$ (Median: 0.43, 95% CrI: [0.22, 1.10])
  - Each panel displays the weighted SIS KDE, shaded 95% credible interval, particle rug ticks, and numerical summary box.

---

## 📊 Calibrated Posterior Results (Generation 19)

| Parameter | Posterior Mean | Posterior Median | 95% Credible Interval | Manuscript Target |
| :--- | :---: | :---: | :---: | :---: |
| $\beta$ | 0.0456 | 0.0455 | 0.0409 – 0.0504 | 0.0457 (0.041–0.050) |
| $\delta_{\text{low}}$ | 0.1045 | 0.1040 | 0.0852 – 0.1268 | 0.104 (0.083–0.133) |
| $\delta_{\text{high}}$ | 0.3745 | 0.3875 | 0.1397 – 0.5853 | 0.392 (0.135–0.588) |
| $\nu$ | 0.1615 | 0.1569 | 0.1028 – 0.2368 | 0.159 (0.101–0.236) |

---

## 🚀 Execution & Reproduction Commands

### 1. Render Figures (Instantaneous via precomputed posteriors)
```bash
make exp03-cs-fig3        # Renders Figure 3 (PDF & PNG)
make exp03-cs-sfig4       # Renders Supplementary Figure S4 canonical corner matrix (PDF & PNG)
make exp03-cs-sfig4b      # Renders Supplementary Figure S4b pairwise ratios (PDF & PNG)
make exp03-cs-sfig4-all   # Renders both Supplementary Figures S4 and S4b
```

### 2. Fast Pipeline Smoke Test (< 1 second)
```bash
make exp03-cs-smoke
```
Runs a 2-particle, 2-generation mini-pilot to verify particle sampling, C++ Monte Carlo execution, metric calculation, and SIS weighting without heavy compute.

### 3. Pilot ABC-SMC (Optional - ~30 seconds)
```bash
make exp03-cs-abc-pilot
```
Runs 50 particles across 4 generations ($N=100$ virtual tissues per particle).

### 4. Full Canonical ABC-SMC (Heavy calibration recomputation)
```bash
make exp03-cs-simulate
```
Re-runs the complete 250 particles $\times$ 20 generations calibration campaign ($N=300$ simulations per particle).
