# ABC-SMC Parameter Estimation Summary

- **Execution Time:** 0.52 seconds
- **Generations Completed:** 2
- **Particles per Generation:** 2
- **Best Distance ($SSE_w$):** 2439.420000
- **Big Bang Mode:** `Enabled`

## Parameter Estimates (Posterior Distribution)

| Parameter | Mean | Median (MAP) | 95% Credible Interval |
|---|---|---|---|
| `brca1_rate` | 0.03531 | **0.03503** | [0.03503, 0.03737] |
| `low_delta` | 0.23734 | **0.23309** | [0.22892, 0.23788] |
| `high_delta` | 0.42527 | **0.42376** | [0.42376, 0.43623] |
| `neoplastic_div_rate` | 0.23247 | **0.17564** | [0.11981, 0.23973] |
| `tp53_rate` | 0.00426 | **0.00398** | [0.00398, 0.00630] |
| `d1_threshold` | 1.44791 | **1.35418** | [1.35418, 2.12622] |
| `d2_threshold` | 5.60418 | **5.53646** | [5.53646, 6.09421] |
| `division_rate` | 0.00068 | **0.00066** | [0.00066, 0.00086] |
| `kappa_brca1` | 3.64805 | **3.29387** | [2.94595, 3.69325] |

## Generation Convergence Diagnostics

| Gen | Target $\epsilon$ | Accepted | Attempted | Acc. Rate | ESS | Min Dist | Med Dist | Time (s) |
|---|---|---|---|---|---|---|---|---|
| 0 | 6375.3175 | 2 | 4 | 50.0% | 2.0 | 2671.6300 | 5140.7550 | 0.3s |
| 1 | 3172.5825 | 2 | 4 | 50.0% | 1.2 | 2439.4200 | 2928.1950 | 0.3s |
