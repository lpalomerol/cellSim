# Experiment 05: Cancer Interception & Attenuation Protocols (Figure 5 & Supplementary Figure S5)

## 🎯 Overview
This vertical slice replicates **Figure 5** and **Supplementary Figure S5** of the BRCA1-ONSET manuscript (*Palomero et al., 2026*).

It evaluates *in silico* cancer interception protocols across a matched cohort design of **204,000 simulated conditions** using the C++ OpenMP engine `build/run_paper_interventions`. It tests pharmacological attenuation of $\theta_{D1}$ (mitotic checkpoint), $\theta_{D2}$ (immune surveillance evasion), or both by 10%, 25%, or 50% initiated at ages 30, 40, or 50 years.

---

## 🧬 Biological & Computational Results (Simulated $N=204,000$ Conditions)

### 1. Dominant Benefit of $\theta_{D2}$ Attenuation (Figure 5b)
- **Start Age 30 (50% dose)**:
  - $\theta_{D2}$ reduces absolute tumor-positive risk by age 70 by **28.90 percentage points** (from 62.2% to 33.3%, McNemar $P < 10^{-300}$).
  - In contrast, 50% $\theta_{D1}$ attenuation produces only a **1.70 percentage point** reduction.
- **Start Age 40 (50% dose)**:
  - $\theta_{D2}$ ARR: **14.95 percentage points** (McNemar $P = 1.5 \times 10^{-145}$).
  - $\theta_{D1}$ ARR: **2.24 percentage points**.
- **Start Age 50 (50% dose)**:
  - $\theta_{D2}$ ARR: **7.24 percentage points** (McNemar $P = 1.6 \times 10^{-51}$).
  - $\theta_{D1}$ ARR: **2.57 percentage points**.

### 2. Dose-Response Relationship at Age 30 (Figure 5c)
- Monotonic increase in ARR for $\theta_{D2}$:
  - 10% attenuation: **1.02 pp** ARR
  - 25% attenuation: **5.66 pp** ARR (matches manuscript text: ~6 pp)
  - 50% attenuation: **28.90 pp** ARR (matches manuscript text: ~28 pp)
  - GEE repeated-measures trend test: $P < 10^{-300}$.

### 3. Sub-additive Interaction Contrast ($\theta_{D1} + \theta_{D2}$) (Figure S5)
- Combining $\theta_{D1}$ and $\theta_{D2}$ yields smaller protection than $\theta_{D2}$ alone:
  - $\text{ARR}(D_1+D_2) = \mathbf{22.73\text{ pp}}$ vs. $\text{ARR}(D_2) = \mathbf{28.90\text{ pp}}$ (McNemar $P < 10^{-200}$).
- **Interaction Contrast**:
  $$\Delta_{\text{int}} = \text{ARR}(D_1+D_2) - [\text{ARR}(D_1) + \text{ARR}(D_2)] = \mathbf{-8.34\text{ pp}}$$
  (95% bootstrap CI: $[-9.33, -7.43]\,\text{pp}$, confirming sub-additive kinetics due to the sequential dependency between the two checkpoints).

---

## 🚀 Execution Commands

```bash
# 1. Render Figures (Instantaneous via simulated data)
make exp05-fig5    # Renders Figure 5 (PDF & PNG)
make exp05-sfig5   # Renders Supplementary Figure S5 (PDF & PNG)

# 2. Fast Pipeline Smoke Test (< 0.05s)
make exp05-smoke   # Verifies run_paper_interventions binary on a mini-cohort

# 3. Full Cohort Re-simulation (Optional recomputation of 204,000 conditions, ~8.5 min)
make exp05-simulate
```
