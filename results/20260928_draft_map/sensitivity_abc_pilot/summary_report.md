# ABC-SMC Parameter Estimation Summary

- **Execution Time:** 90.18 seconds
- **Generations Completed:** 4
- **Particles per Generation:** 50
- **Best Distance ($SSE_w$):** 35.460000
- **Big Bang Mode:** `Enabled`

## Parameter Estimates (Posterior Distribution)

| Parameter | Mean | Median (MAP) | 95% Credible Interval |
|---|---|---|---|
| `brca1_rate` | 0.05025 | **0.05335** | [0.01627, 0.07596] |
| `low_delta` | 0.14103 | **0.12488** | [0.05896, 0.22615] |
| `high_delta` | 0.37004 | **0.36742** | [0.12703, 0.55946] |
| `neoplastic_div_rate` | 0.13415 | **0.15257** | [0.01137, 0.23038] |
| `tp53_rate` | 0.00364 | **0.00320** | [0.00124, 0.00816] |
| `d1_threshold` | 2.39306 | **2.47511** | [1.26247, 3.81025] |
| `d2_threshold` | 6.15949 | **5.93805** | [3.13181, 7.95379] |
| `division_rate` | 0.00244 | **0.00270** | [0.00065, 0.00478] |
| `kappa_brca1` | 2.32399 | **1.91562** | [1.16705, 3.80466] |

## Generation Convergence Diagnostics

| Gen | Target $\epsilon$ | Accepted | Attempted | Acc. Rate | ESS | Min Dist | Med Dist | Time (s) |
|---|---|---|---|---|---|---|---|---|
| 0 | 4700.6925 | 50 | 50 | 100.0% | 50.0 | 34.4500 | 1098.8800 | 15.4s |
| 1 | 2213.2650 | 50 | 69 | 72.5% | 35.4 | 109.6200 | 921.5250 | 19.8s |
| 2 | 976.6300 | 50 | 63 | 79.4% | 19.9 | 14.3100 | 445.9200 | 17.9s |
| 3 | 577.8250 | 50 | 87 | 57.5% | 17.3 | 35.4600 | 315.2250 | 37.1s |
