# BRCA1-ONSET Tissue-Scale Model (10^7 Susceptible Cells)

Standalone, fully reproducible package for the tissue-scale extension of **BRCA1-ONSET**.

This module scales the virtual population from the cellular-scale 500-cell representation to an intact virtual tissue with **10^7 susceptible epithelial cells**, retains the clinical tumor-positive endpoint of 5% mass (**5×10^5 transformed cells**), models finite post-transformation clonal expansion via effective doubling time $\tau_T$, and incorporates inter-individual variation in the $D_2$-dependent tissue restraint threshold $\theta_{D2}$ via a log-normal distribution.

---

## 🔬 Scientific Highlights & Calibrated Behavior

From the frozen independent 10,000-tissue validation (`data/final_multiobjective_validation_10000.csv`):
- **Clinical penetrance agreement**: cumulative penetrance at ages 30, 40, 50, 60, 70, 80 is **5.78%, 24.65%, 42.86%, 56.50%, 65.98%, 72.57%**.
- **Minimal weighted discrepancy**: $wSSE = 4.15$ against Kuchenbaecker et al. (2017) clinical cohort data.
- **Somatic BRCA1 loss majority**: **92.50%** of emerging tumors are dominated by BRCA1-null cells (>50% null mass).
- **Subclonal retention of heterozygosity**: **21.63%** of BRCA1-loss-majority tumors retain $\ge 25\%$ BRCA1$^{+/-}$ cells, reproducing clinical intratumor heterogeneity.
- **Emergent polyclonal initiation**: **28.10%** of tumors possess $\ge 2$ substantial founder lineages ($\ge 5\%$ each), demonstrating age-dependent founder dynamics without founder number being fitted directly.

---

## 📊 Generated Figures

### 1. Main Text Figure 6 (`Figure6.pdf`, `Figure6.png`)
- **Panel a**: Cumulative penetrance comparison (Observed 95% CI vs Homogeneous restraint vs Variable $D_2$ restraint).
- **Panel b**: Somatic BRCA1 loss composition in emerging tumors (BRCA1$^{+/-}$ predominant, Mixed, BRCA1$^{-/-}$ predominant).
- **Panel c**: Distribution of substantial founder lineages across age at first tumor detection ($\chi^2 = 121.96, p = 1.79 \times 10^{-17}$).

### 2. Supplementary Figure S7 (`FigureS7.pdf`, `FigureS7.png`)
- **Panel a**: Cumulative penetrance fit under the homogeneous tissue restraint model ($wSSE = 48.96$).
- **Panel b**: Systematic trade-off between early/mid-life discrepancy (ages 30–50) vs late-life discrepancy (ages 60–80) across grid configurations.

---

## ⚡ Quickstart & Reproduction

### Regenerate Figures Instantly
```bash
# Generate Figure 6 (Panels a, b, c) in PDF (vector) and PNG (300 DPI)
make exp06-fig6

# Generate Supplementary Figure S7 (Panels a, b) in PDF (vector) and PNG (300 DPI)
make exp06-sfig7
```

### Run Fast Verification Smoke Test (< 3 seconds)
```bash
make exp06-smoke
```

### Validate Metrics from Frozen 10,000-Tissue Dataset
```bash
make exp06-validate
```

### Compile C++20 Simulator Directly
```bash
make -C tissue_scale
```

---

## 🧬 Parameters of Representative Multi-Objective Calibration

| Parameter | Symbol | Value | Interpretation |
|---|---|---:|---|
| `mu_TP53` | $\mu$ | $2.0076 \times 10^{-5}$ | Somatic TP53 mutation rate per cell per cycle |
| `beta_BRCA1` | $\beta$ | $0.00275$ | Somatic BRCA1 loss-of-heterozygosity (LOH) rate |
| `delta_low` | $\delta_{\text{low}}$ | $0.2267$ | Damage increment for moderate/heterozygous instability |
| `delta_high` | $\delta_{\text{high}}$ | $1.4928$ | Damage increment for severe/homozygous instability |
| `theta_D1` | $\theta_{D1}$ | $13.64$ | Intrinsic transformation barrier threshold ($D_1$) |
| `theta_D2_mean` | $\bar{\theta}_{D2}$ | $31.79$ | Baseline tissue-restraint barrier threshold ($D_2$) |
| `tau_expansion` | $\tau_T$ | $0.65$ yr | Effective post-transformation clonal doubling time |
| `sigma_theta_d2` | $\sigma_{\theta_{D2}}$ | $0.38$ | Inter-individual log-normal standard deviation in $D_2$ restraint |
