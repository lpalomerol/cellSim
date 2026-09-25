# OpenMP Threading Optimization for ABC-SMC

## Problem

The `run_bootstrapping` binary uses OpenMP (`#pragma omp parallel for`) to parallelize simulations internally. By default, OpenMP uses all available CPU cores.

When running ABC-SMC with Python's `ThreadPoolExecutor` and N workers:
- **Without control**: N workers × n_cores threads → oversubscription
- Example: 8 workers × 12 cores = 96 threads competing for 12 CPUs
- Result: Excessive context switching, cache thrashing, poor throughput

## Solution

Force each C++ binary instance to use **1 OpenMP thread**:
```python
os.environ["OMP_NUM_THREADS"] = "1"
```

Now:
- Python's ThreadPoolExecutor manages parallelism at the **process level**
- Each worker runs 1 simulation with 1 thread
- No oversubscription: 12 workers → 12 threads on 12 cores (ideal)

## Performance Impact

### Expected behavior:
- **Individual simulation**: Slower (~3-4x) because it uses 1 core instead of all cores
- **Total throughput**: Faster (~2-3x) because you can run more simulations concurrently without contention

### When OMP_NUM_THREADS=1 wins:
- High parallelism workloads (ABC-SMC, parameter sweeps)
- Many independent tasks to queue
- Limited CPU cores

### When multi-threaded OpenMP wins:
- Single simulation runs
- Very fast simulations (overhead dominates)
- More cores than tasks

## Usage

### Automatic (recommended for ABC-SMC)
The script now sets `OMP_NUM_THREADS=1` automatically:
```bash
python scripts/abc_final_v4.py --workers 12 [other args]
```

### Manual control (advanced)
Override if needed:
```bash
# Force 2 threads per binary
OMP_NUM_THREADS=2 python scripts/abc_final_v4.py --workers 6 [...]
```

### Benchmark your system
```bash
bash scripts/benchmark_omp_threading.sh
```

## Recommendations

| System | Workers | OMP_NUM_THREADS |
|--------|---------|-----------------|
| 12-core CPU | 12-16 | 1 (default) |
| 8-core CPU | 8-10 | 1 (default) |
| 4-core CPU | 4-6 | 1 (default) |

**Rule of thumb**: Set `workers = n_cores` or slightly higher (1.2-1.5×) when `OMP_NUM_THREADS=1`.

## References

- OpenMP Environment Variables: https://www.openmp.org/spec-html/5.0/openmpse50.html
- Python ThreadPoolExecutor: https://docs.python.org/3/library/concurrent.futures.html
- Oversubscription in HPC: https://software.intel.com/content/www/us/en/develop/articles/openmp-thread-affinity-control.html
