# Section 3: Methodology — Mathematical Model and Simulation Design
*Draft — cellSim paper*

---

## 3.1 Model Overview

> **Model scope reminder.** cellSim is an exploratory proof-of-concept simulator. The biological mechanisms described below — TP53/BRCA1 gene states and D1/D2 instability accumulators — represent an initial reference implementation chosen for their tractability and biological familiarity. They are intended as examples of the class of mechanisms the platform can express, not as a complete or calibrated representation of oncogenesis. Future iterations may replace or extend these components without modifying the underlying architecture.

The cellSim simulator represents a tissue as a dynamic population of *N* autonomous cells, each executing an independent, synchronised life cycle at every discrete time step (one simulation year). Population dynamics emerge from the aggregate behaviour of individual cells: births via cell division, deaths via apoptosis, and phenotypic transitions via mutation and neoplastic transformation.

The model is **stochastic**: mutation events and division decisions are sampled from Bernoulli distributions parameterised by user-supplied rates. Reproducibility is guaranteed by a global pseudorandom number generator (Mersenne Twister, seeded by an integer parameter `seed`). Setting `seed = -1` enables non-deterministic mode (system clock seed).

---

## 3.2 Cell State Representation

Each cell is characterised by four **internal variables** and a derived **life stage**:

### 3.2.1 Internal Variables

| Variable | Type | Domain | Biological meaning |
|----------|------|--------|--------------------|
| `TP53` | discrete | {`+/+`, `+/-`, `-/-`} | Allele state of the tumour suppressor gene |
| `BRCA1` | discrete | {`+/-`, `-/-`} | Allele state of the DNA repair gene |
| D1 | continuous | [1.0, 999.0] | Genomic instability (DNA damage accumulation) |
| D2 | continuous | [1.0, 999.0] | Apoptotic evasion capacity (phenomenological accumulator; net outcome of immune evasion processes) |

> **Population assumption**: BRCA1 is initialised at `+/-` for all cells, reflecting a germline heterozygous carrier population (BRCA1 mutation carriers; HBOC context). **The model is therefore scoped to hereditary BRCA1-associated cancer predisposition, not general sporadic oncogenesis.** TP53 is initialised at `+/+` (wild-type). All cells begin with D1 = D2 = 1.0.

### 3.2.2 Life Stages

The cell life stage (*CellLifeStage*) is derived deterministically from the internal variables at each evaluation point. It is not stored as a separate field, eliminating the possibility of state synchronisation errors:

| Stage | Condition | Description |
|-------|-----------|-------------|
| **BASELINE** | TP53 = `+/+`, alive | Healthy cell; full tumour-suppressor protection |
| **UNSTABLE** | TP53 = `+/-`, alive | First mutation; partial protection; D1, D2 begin to grow |
| **UNPROTECTED** | TP53 = `-/-`, alive, D1 ≤ θ_D1 | No TP53 function; rapid instability accumulation |
| **PRIMER** | TP53 = `-/-`, D1 > θ_D1, alive | Pre-neoplastic; detectable by immune surveillance |
| **TUMORAL** | `neoplastic = true`, alive | Transformed; immortal; may divide at elevated rate |
| **DEAD** | `alive = false` | Removed from tissue at end of cycle |

where θ_D1 = 2.0 (D1 primer threshold, configurable).

The state transition graph is therefore:

```
BASELINE ──(TP53 +/+ → +/-)──▶ UNSTABLE ──(TP53 +/- → -/-)──▶ UNPROTECTED
                                                                      │
                                                               D1 > θ_D1
                                                                      │
                                                                      ▼
                                                   DEAD ◀──── PRIMER ──── D2 > θ_D2 ──▶ TUMORAL
                                                 (apoptosis)         (evades apoptosis)
```

---

## 3.3 Instability Dynamics: D1 and D2 Evolution

At each simulation cycle, D1 and D2 are updated according to the following recurrence relations:

### Genomic Instability (D1)

$$D_1(t+1) = \min\!\bigl(D_1(t) + \delta_{D_1},\; 999\bigr)$$

$$\delta_{D_1} = \delta_{\mathrm{TP53}} + \epsilon \cdot \mathrm{age}$$

where:

$$\delta_{\mathrm{TP53}} = \begin{cases} 0 & \text{if TP53} = +/+ \\ \delta_{\mathrm{low}} & \text{if TP53} = +/- \\ \delta_{\mathrm{high}} & \text{if TP53} = -/- \end{cases}$$

### Apoptotic Evasion Capacity (D2)

$$D_2(t+1) = \min\!\bigl(D_2(t) + \delta_{D_2},\; 999\bigr)$$

$$\delta_{D_2} = \delta_{\mathrm{TP53}} + \delta_{\mathrm{BRCA1}} + \epsilon \cdot \mathrm{age}$$

where:

$$\delta_{\mathrm{BRCA1}} = \begin{cases} \delta_{\mathrm{low}} & \text{if BRCA1} = +/- \\ 2 \cdot \delta_{\mathrm{high}} & \text{if BRCA1} = -/-\ \text{(and TP53} = -/-) \end{cases}$$

The age factor ε = 1×10⁻⁵ introduces a slow basal accumulation that models natural ageing. Both accumulators saturate at 999, representing a biologically bounded instability ceiling.

### Default Parameter Values

| Parameter | Symbol | Default value |
|-----------|--------|--------------|
| Low instability delta | δ_low | 0.001 |
| High instability delta | δ_high | 0.003 |
| Age factor | ε | 1×10⁻⁵ |
| D1 primer threshold | θ_D1 | 2.0 |
| D2 apoptosis threshold | θ_D2 | 5.0 |

---

## 3.4 Mutation Model

Gene mutations are modelled as Bernoulli trials at each cycle. The mutation probability for a gene *g* in state *s* is:

$$P(\text{mutation}_g) = \mu_g \cdot D_1$$

where μ_g is the per-gene baseline mutation rate (configurable). Mutations are **unidirectional** (allele degradation only):

- TP53: `+/+` → `+/-` → `-/-`
- BRCA1: `+/-` → `-/-`

This captures the biologically observed irreversibility of loss-of-function mutations in tumour suppressor genes. The D1 multiplicative factor implements **mutational acceleration under genomic instability**: cells already accumulating DNA damage are more prone to further mutations, creating a positive feedback loop.

---

## 3.5 The Six-Phase Cell Cycle

Each simulation year, every cell executes the following deterministic, ordered sequence of phases. Any phase that triggers cell death terminates the cycle early via exception propagation:

### Phase 0 — Viability Check
The cell verifies it is alive. Dead cells skip all subsequent phases and are removed from the tissue at the end of the tick.

### Phase 1 — G1 Integrity Checkpoint (Intrinsic Apoptosis)
If BRCA1 = `-/-` **and** TP53 is functional (`+/+` or `+/-`), TP53 detects the severe genomic damage and triggers intrinsic apoptosis (`CellDeathException`). This models the canonical TP53–BRCA1 interaction in which BRCA1 deficiency is lethal in cells retaining TP53 function.

> **Exception**: If TP53 = `-/-`, the checkpoint is non-functional and the cell survives with BRCA1 `-/-`, albeit at extreme instability (δ_BRCA1 = 2·δ_high).

### Phase 2 — Endocytosis (Extrinsic Apoptosis Checkpoint)
If the cell is in PRIMER stage (TP53 `-/-`, D1 > θ_D1), the tissue sends an apoptotic signal. The cell's response depends on D2:

$$\text{outcome} = \begin{cases} \text{apoptosis} & \text{if } D_2 < \theta_{D_2} \\ \text{evasion (immortalisation)} & \text{if } D_2 \geq \theta_{D_2} \end{cases}$$

This models immune surveillance: the tissue recognises the aberrant cell and attempts to eliminate it, but cells with sufficient apoptotic evasion capacity (D2) survive and proceed to transformation.

### Phase 3 — Nuclear Dynamics (Mutation Application)
Stochastic mutation events are applied to BRCA1 and TP53 alleles using the mutation model described in Section 3.4. If BRCA1 mutates to `-/-` and TP53 remains functional, Phase 1 is re-evaluated immediately, triggering intrinsic apoptosis.

### Phase 4 — Cytoplasmic Remodelling (Transformation and Division)
Two events may occur in this phase:

1. **Neoplastic transformation**: If the cell is in PRIMER stage and has evaded apoptosis (Phase 2), it transforms: `neoplastic = true`, `immortal = true`. A `NeoplasmSignal` is emitted to the tissue tracking service.

2. **Cell division** (if Big Bang mode is active): Neoplastic cells may divide with probability `neoplastic_division_rate` per cycle. Division produces a daughter cell that inherits the parent's full genome state and D1/D2 values. A `CellDivisionSignal` carrying the daughter's genome is emitted to the tissue.

### Phase 5 — Exocytosis (Instability Update)
D1 and D2 are incremented according to the recurrence relations in Section 3.3. The cell's age counter is incremented. This phase always executes last, ensuring that the instability state reflects the post-mutation, post-transformation status.

---

## 3.6 Tissue Management

The `Tissue` class manages a dynamic population of cells stored as `std::vector<std::unique_ptr<ICell>>`. At each simulation step:

1. Every cell executes its six-phase cycle.
2. `CellDeathException` signals are caught: dead cells are marked and removed at end of step.
3. `CellDivisionSignal` signals are collected: daughter cells are appended to the population vector after the full population has been iterated (preventing mid-iteration modification).
4. `NeoplasmSignal` signals update the `GeneticTrackingService`, which maintains yearly population statistics (alive, neoplastic, immortal, TP53 state distribution).

This signal-based protocol ensures that cell logic remains independent of tissue structure, enabling independent unit testing of both layers.

---

## 3.7 Validation Scenarios

To validate the model's biological plausibility, we defined 14 parametric scenarios across four categories. All scenarios use an initial population of N = 1,000 cells unless otherwise stated.

### 3.7.1 Baseline Controls (Scenarios 01–03)

These scenarios test the model's boundary behaviour.

| ID | Name | Key parameters | Expected outcome | Validated result |
|----|------|---------------|-----------------|-----------------|
| **01** | Stable baseline | μ_BRCA1 = 0, μ_TP53 = 0, div = 0 | Population stable at 1,000; 0% neoplastic | ✅ 1,000 alive, 0% neo |
| **02** | Exponential growth | μ_BRCA1 = 0, μ_TP53 = 0, div = 15% | Population ~5× growth; 0% neoplastic | ✅ 4,955 alive, 0% neo |
| **03** | BRCA1 collapse | μ_BRCA1 = 0.2, div = 0 | Total extinction by year 50 | ✅ 0 alive at year 50 |

Scenario 01 confirms that the model is stable in the absence of biological pressure. Scenario 02 isolates the division mechanism. Scenario 03 demonstrates that BRCA1 `+/- → -/-` mutation is lethal under intact TP53, reproducing the known synthetic lethality.

### 3.7.2 Neoplastic Phenotypes (Scenarios 04–05)

| ID | Name | Key parameters | Expected outcome | Validated result |
|----|------|---------------|-----------------|-----------------|
| **04** | TP53 invasion | μ_TP53 = 0.1, div = 0 | >90% neoplastic; 100% immortal | ✅ 462 alive, 98% neo, 100% immortal |
| **05** | High instability | δ_low = 0.1, δ_high = 0.5 | More neoplastic cells than S04 | ✅ 573 alive vs 462 (higher survival via D2) |

Scenario 04 demonstrates that elevated TP53 mutation rate drives widespread neoplastic transformation. Scenario 05 shows that increasing the instability delta produces a model-level survival increase: higher D2 accumulation allows more cells to cross the apoptotic evasion threshold (D2 > θ_D2) before dying, inflating the immortal population — a direct consequence of the shared delta design of D1 and D2 (see Section 5.1.3).

### 3.7.3 Biologically Realistic Scenarios (Scenarios 06–09)

These use mutation rates calibrated to approximate real somatic mutation frequencies.

| ID | Name | Key parameters | Year | Alive | % Neoplastic | % Immortal |
|----|------|---------------|------|-------|-------------|-----------|
| **06** | Realistic basal | μ_BRCA1 = 0.05, μ_TP53 = 0.01, div = 0 | 50 | 59 | 20% | 100% |
| **07** | Low TP53, controlled div | μ_TP53 = 0.005, div = 5% | 80 | 484 | 2.5% | 100% |
| **08** | High instability + div | δ_high = 1.5, div = 5% | 80 | 153 | 46% | 100% |
| **09** | Balanced *(reference)* | μ_BRCA1 = 0.05, μ_TP53 = 0.01, div = 5% | 80 | 362 | 4.7% | 94% |

Scenario 09 is the **reference configuration**: it reproduces a biologically plausible phenotype where a minority of cells undergoes transformation, the majority retains apoptotic sensitivity, and the population maintains long-term stability. The 4.7% neoplastic rate is consistent with empirical estimates of field cancerisation in epithelial tissues.

Scenario 08 highlights the non-linear coupling between instability and population dynamics: higher δ_high accelerates both D1 and D2 accumulation, increasing both the fraction of PRIMER cells that die (elevated D1) and the fraction that escape (elevated D2), resulting in a net 46% neoplastic fraction.

### 3.7.4 Big Bang Mode — Explosive Neoplastic Transformation (Scenarios 10 and 14)

Big Bang mode activates an additional division mechanism for neoplastic cells (`neoplastic_division_rate` > 0), modelling the clonal expansion phase of oncogenesis.

| ID | Name | μ_BRCA1 | μ_TP53 | k | div_n | Year 80: Alive | % Neo | % Immortal |
|----|------|---------|--------|---|-------|----------------|-------|-----------|
| **10** | Big Bang low | 0.050 | 0.010 | 0.020 | 0.050 | 9,580 | 92.1% | 99.4% |
| **14** | Big Bang control | 0.050 | 0.010 | 0.020 | 0.050 | 9,580 | 92.1% | 99.4% |

*k = neoplasm_k; div_n = neoplastic_division_rate.*

Scenarios 10 and 14 use identical parameters and produce identical results (9,580 cells, 92.1% neoplastic), confirming **model reproducibility** across independent simulation runs.

#### Temporal Dynamics of Scenario 10 (Big Bang Low)

The progression exhibits four qualitatively distinct phases, analogous to the clinical stages of tumour development:

| Phase | Years | Alive | % Neoplastic | Mechanism |
|-------|-------|-------|-------------|-----------|
| **Latency** | 0–30 | ~700–1,000 | 0–10% | Slow TP53 mutation; apoptosis eliminates most TP53 −/− cells |
| **Acceleration** | 30–50 | ~600–850 | 10–49% | Instability crosses threshold; apoptotic evasion increases |
| **Big Bang (critical)** | 50–70 | 840–3,375 | 49–89% | Exponential clonal expansion; D2 saturated; 100% immortal |
| **Neoplastic dominance** | 70–80 | 3,375–9,580 | 89–96% | Continuous transformation of remaining healthy cells |

The transition between latency and acceleration is driven by the quadratic growth of D1 once TP53 = `-/-` becomes prevalent. The subsequent Big Bang phase (years 50–70) corresponds to a 7.3× increase in neoplastic population within 20 years, producing explosive clonal expansion kinetics qualitatively analogous to the dynamics described by Tomasetti et al. (2017).

Compared to the reference scenario (09), Big Bang scenario 10 produces 26× more cells at year 80 (9,580 vs 362) and 95 percentage points higher neoplastic fraction (92.1% vs 4.7%), highlighting the decisive role of neoplastic division rate in long-term population dynamics.

---

## 3.8 Programming Language Selection

Choosing an appropriate programming language for a stochastic, agent-based cell simulator involves trade-offs across performance, usability, ecosystem support, and architectural expressiveness. Four languages are commonly considered for this class of scientific software:

### 3.8.1 Comparative Analysis

| Criterion | Java | Python | MATLAB | C++ |
|-----------|------|--------|--------|-----|
| **Execution speed** | Medium (JIT-compiled, GC pauses) | Low–Medium (interpreted; NumPy offloads to C) | Medium (vectorised operations; slow loops) | **Very high** (native, ahead-of-time compiled) |
| **Memory control** | Automatic (GC) | Automatic (GC) | Automatic | **Manual** (`unique_ptr`, RAII) |
| **OOP & design patterns** | Excellent (interfaces, generics) | Good (duck-typed) | Limited | **Excellent** (templates, abstract classes) |
| **Scientific ecosystem** | Moderate | **Excellent** (NumPy, SciPy, Pandas, PyTorch) | **Excellent** (built-in numerical toolbox) | Moderate (Eigen, Boost) |
| **Usability / learning curve** | Medium | **Low** | **Low** | High |
| **Licensing** | Open (OpenJDK) | Open | **Proprietary** | Open |
| **Portability** | Excellent (JVM) | Excellent | Limited (runtime required) | Good (CMake cross-platform) |
| **Concurrency** | Good (threads, ForkJoin) | Limited (GIL) | Limited | **Excellent** (std::thread, OpenMP) |

**Java** offers a clean object-oriented model and good performance through JIT compilation, but garbage-collector pauses introduce non-deterministic latency that is problematic for tight simulation loops over large populations. Its scientific ecosystem is weaker than Python's or MATLAB's.

**Python** is the dominant language for scientific computing and data analysis, benefiting from an extensive ecosystem (NumPy, SciPy, Pandas, Matplotlib) and very low development overhead. However, its interpreted nature makes pure-Python loops orders of magnitude slower than compiled alternatives for computationally intensive kernels. Extensions such as Cython, Numba, or embedding C++ via pybind11 can recover performance but add architectural complexity.

**MATLAB** provides an integrated environment for mathematical modelling and rapid prototyping, with built-in numerical solvers and visualisation. Its proprietary licence restricts reproducibility in open-science contexts, and its loop performance is poor without vectorisation, making it less suited for agent-level simulation at scale.

**C++** delivers the highest throughput of any mainstream language once compiled, with no runtime overhead from garbage collection or interpretation. Manual memory management via RAII patterns (`std::unique_ptr`) provides deterministic resource lifetime—a critical property for simulations that create and destroy large numbers of cell objects per step. Its strong type system and support for abstract interfaces (`ICell`, `INoiseSource`) enable the layered Clean Architecture employed by cellSim without sacrificing performance.

### 3.8.2 Language Decision

**cellSim is implemented in C++17** for the following primary reasons:

1. **Computational throughput**: simulating 10,000+ cell agents over 80 years with stochastic sampling per cell per phase is inherently loop-intensive. C++ compiled with optimisations (`-O2`) completes all 14 validation scenarios in 3–7 seconds on a standard workstation; equivalent Python loops would require 100–1,000× more time without native extensions.

2. **Deterministic memory management**: RAII via `std::unique_ptr<ICell>` guarantees that cell objects are destroyed exactly when they are removed from the tissue, avoiding the unpredictable pause events of garbage-collected runtimes.

3. **Architectural expressiveness**: C++ abstract classes and interfaces support the full Clean Architecture dependency inversion pattern, enabling the biological domain layer to remain entirely independent of infrastructure concerns.

4. **HPC compatibility**: C++ executables integrate natively with HPC schedulers (SLURM) and containerisation tools (Apptainer/Singularity), which are mandatory in the cluster environments where large-scale parametric studies are run.

It is worth noting that **Python remains a viable and attractive alternative** for exploratory or prototype implementations of this model. A Python port using NumPy-vectorised state arrays (encoding all cell genomes as matrix columns) could achieve acceptable performance for populations up to ~5,000 cells, while dramatically reducing development effort and making the tool accessible to a broader scientific audience. The clean separation between model logic and infrastructure in the present architecture would also facilitate the creation of Python bindings via pybind11, allowing downstream analysis pipelines to invoke the C++ simulation core from Python scripts without sacrificing runtime performance.

---

## 3.9 Implementation Details

The simulator is implemented in **C++17** and built with CMake 3.14+. Randomness is encapsulated behind the `INoiseSource` interface, enabling substitution of `RandomNoise` (Mersenne Twister) with `FixedNoise` (deterministic sequence) in unit tests. Simulation output is written as Markdown tables and CSV files to `traces/`, enabling downstream analysis in R or Python without additional preprocessing.

Execution time for all 14 scenarios (1,000 cells × 80 years each) is approximately 3–7 seconds on a standard desktop workstation (Intel Core i7, single thread), making the tool practical for rapid exploratory parametric studies.
