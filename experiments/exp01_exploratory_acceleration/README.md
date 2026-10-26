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

### 1. Render Figure 2 instantly:
```bash
make exp01-fig2
```
Generates `Figure2.pdf` and `Figure2.png` inside `experiments/exp01_exploratory_acceleration/`.

### 2. Run fast smoke test (< 5 seconds):
```bash
make exp01-smoke
```

### 3. Run full simulation sweep from scratch:
```bash
python3 experiments/exp01_exploratory_acceleration/run_exploratory.py --run --n-runs 200
```
