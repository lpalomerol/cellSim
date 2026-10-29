# ABC-SMC Parameter Estimation Summary

- **Execution Time:** 0.51 seconds
- **Generations Completed:** 2
- **Particles per Generation:** 2
- **Best Distance ($SSE_w$):** 2271.410000
- **Big Bang Mode:** `Enabled`

## Parameter Estimates (Posterior Distribution)

| Parameter | Mean | Median (MAP) | 95% Credible Interval |
|---|---|---|---|
| `brca1_rate` | 0.04976 | **0.04294** | [0.04138, 0.05572] |
| `low_delta` | 0.21235 | **0.20124** | [0.20124, 0.22495] |
| `high_delta` | 0.52702 | **0.47846** | [0.46732, 0.56951] |
| `neoplastic_div_rate` | 0.12770 | **0.09007** | [0.09007, 0.17041] |
| `tp53_rate` | 0.00457 | **0.00449** | [0.00449, 0.00467] |
| `d1_threshold` | 2.14047 | **1.91313** | [1.86100, 2.33932] |
| `d2_threshold` | 4.62108 | **4.06549** | [4.06549, 5.25164] |
| `division_rate` | 0.00089 | **0.00080** | [0.00078, 0.00097] |
| `kappa_brca1` | 2.81680 | **2.49552** | [2.49552, 3.18145] |

## Generation Convergence Diagnostics

| Gen | Target $\epsilon$ | Accepted | Attempted | Acc. Rate | ESS | Min Dist | Med Dist | Time (s) |
|---|---|---|---|---|---|---|---|---|
| 0 | 2600.8000 | 2 | 6 | 33.3% | 2.0 | 2388.3100 | 2529.9700 | 0.2s |
| 1 | 2447.8850 | 2 | 6 | 33.3% | 2.0 | 2271.4100 | 2389.0600 | 0.3s |
