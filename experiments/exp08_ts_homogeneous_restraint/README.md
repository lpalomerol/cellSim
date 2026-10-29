# Experiment 08 (TS): Homogeneous Restraint Fitting and Trade-Off

## Overview
This experiment analyzes the limitations of a homogeneous tissue-level restraint model, reproducing **Supplementary Figure S6** of *Palomero et al. (2026)* (matching review manuscript `Palomero_SFig6_100126.pdf`).

## Panels
- **Panel a**: Cumulative tumor-positive tissues (%) across ages 30–80 comparing the homogeneous tissue-level restraint against clinical observed penetrance and 95% CI.
- **Panel b**: Pareto discrepancy trade-off between early/mid-life discrepancy (Ages 30–50 SSE) and late-life discrepancy (Ages 60–80 SSE) colored by overall weighted SSE ($\text{wSSE}$), highlighting that homogeneous restraint cannot simultaneously fit both early onset and late plateau without tissue barrier heterogeneity.

## Reproduction
```bash
# Render Supplementary Figure S6
make exp08-ts-sfig6
```
