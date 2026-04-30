# Section 5: Discussion and Conclusions
*Draft — cellSim paper*

---

## 5.1 Biological Interpretation of Results

### 5.1.1 TP53 as the Primary Oncogenic Switch

The simulation results consistently identify TP53 allele state as the dominant determinant of neoplastic fate. Across all scenarios, neoplastic transformation was strictly gated by TP53 `−/−` status: no cell underwent transformation while retaining even a single functional TP53 allele. This is not merely a design constraint, but an emergent property of the threshold-based instability system — TP53 `+/+` cells accumulate D1 and D2 so slowly (δ_TP53 = 0) that they never reach the PRIMER stage within biologically plausible time horizons.

This finding is in strong agreement with the biology of TP53 in human cancer. TP53 is altered in approximately 50% of all human malignancies, and loss-of-function mutations in TP53 are consistently observed as late-stage events that mark the transition from benign lesion to invasive carcinoma (Hollstein et al., 1991). The model captures this gating function naturally through the threshold-based architecture of the PRIMER stage.

### 5.1.2 BRCA1–TP53 Synthetic Lethality

Scenario 03 reproduces the well-established synthetic lethal relationship between BRCA1 deficiency and functional TP53. In the model, BRCA1 `−/−` mutations are universally lethal under intact TP53, leading to population extinction when BRCA1 mutation rate is high. Clinically, this interaction is the basis of PARP inhibitor therapy, which selectively kills BRCA1/2-deficient tumour cells while sparing normal tissue that retains TP53 surveillance (Farmer et al., 2005).

The model's prediction that BRCA1 `−/−` becomes survivable — and contributes to extreme instability — only when TP53 is already lost (`−/−`) reflects the observed two-hit sequence in BRCA1-related breast and ovarian cancers: BRCA1 loss typically precedes or accompanies TP53 loss in the progression to invasive disease.

### 5.1.3 D2 Coupling Effect: Instability and Evasion as a Shared Parameter

Scenario 05 produced a counter-intuitive model result: increasing instability deltas produced *more* surviving cells than Scenario 04, despite imposing greater genomic stress. This arises as a direct consequence of the model's architecture — D1 and D2 use the same delta parameters — so any increase in genomic instability rate simultaneously accelerates the accumulation of apoptotic evasion capacity. Faster D2 growth means more cells reach the immune evasion threshold (θ_D2 = 5.0) before being eliminated, converting cells that would have died (PRIMER → apoptosis) into immortal tumour cells (PRIMER → TUMORAL).

This is an explicit model design property, not an emergent biological phenomenon. It is worth noting that in real cancer biology there is a partial analogy: high tumour mutational burden (TMB) can both drive immune recognition (via neoantigen load) and paradoxically promote immune evasion (via immune exhaustion or selection for immune escape variants). However, the mechanisms are distinct and context-dependent, and the D1/D2 coupling in this model is a deliberate simplification. The D2 accumulator should be interpreted as a phenomenological surrogate for net evasion capacity, not as a direct readout of a specific biological process.

### 5.1.4 The Big Bang Phenomenon

Scenario 10 produced explosive clonal expansion kinetics qualitatively analogous to the dynamics described in the oncological Big Bang hypothesis. The model captures three features of this phenomenon that are consistent with the clinical description:

1. **Latent phase**: Years 0–5 show a gradually declining protected population with near-zero neoplastic fraction, consistent with the clinically silent accumulation phase.
2. **Accelerated conversion**: Once TP53 `−/−` cells begin to dominate (years 15–25), the protected population contracts monotonically while neoplastic cells expand exponentially, mirroring the rapid phenotypic switch described in colorectal carcinogenesis (Tomasetti et al., 2017).
3. **Neoplastic clonal dominance**: By year 30, 93.9% of surviving cells are neoplastic — a degree of tumour dominance that corresponds clinically to an overtly malignant tissue with minimal residual normal compartment.

The model's Big Bang emerges from the interaction of three components: (a) neoplastic division at an elevated rate, (b) continued conversion of healthy cells via TP53 mutation, and (c) apoptotic evasion by all transformed cells (100% immortality). None of these components alone is sufficient; the phenomenon requires their co-occurrence, which explains its late onset relative to initial mutation events.

---

## 5.2 Software Engineering Contribution

### 5.2.1 Clean Architecture Enables Scientific Reproducibility

A key software engineering finding of this work is that Clean Architecture imposes structural benefits that directly serve scientific goals:

- **Testability**: The isolation of domain logic from infrastructure allowed the development of 53 unit tests, each targeting a specific biological rule without dependency on simulation orchestration, random seeds, or file I/O. This level of component-level verification is rare in scientific simulation software.
- **Reproducibility**: Encapsulating the random number generator behind the `INoiseSource` interface enables substitution of `FixedNoise` in tests, guaranteeing deterministic outcomes and eliminating flaky tests — a common problem in stochastic simulation testing.
- **Extensibility**: Adding a new gene, a new apoptosis mechanism, or a new tissue dynamics model requires modifying or adding domain classes only, without touching the simulation orchestration or output layer. This was demonstrated by the addition of Big Bang mode, which extended `AgenticCell` and `SimulationConfig` without requiring changes to `Tissue`, `Simulation`, or any test infrastructure.

### 5.2.2 The Agentic Cell Pattern as a Reusable Paradigm

The Agentic Cell pattern — a fully autonomous agent that encapsulates its state, executes an ordered phase cycle, and communicates via typed signals — is transferable beyond cellSim. Any biological simulation where individual entities have internal state, decision-making logic, and asynchronous interaction with a shared environment could benefit from this architectural design. The pattern naturally enforces separation of individual behaviour (cell logic) from collective dynamics (tissue management), which is the fundamental challenge of all agent-based modelling.

The signal-based communication model (`CellDeathException`, `CellDivisionSignal`, `NeoplasmSignal`) proved particularly effective: it eliminates the need for cells to hold references to the tissue container, avoids concurrent modification issues during population iteration, and provides a natural audit trail for tissue-level events.

---

## 5.3 Limitations

### 5.3.0 Scope of the Model: Proof of Concept

Before detailing specific limitations, it is important to reiterate the intended scope of cellSim. This simulator is designed as an exploratory proof of concept for applying Clean Architecture to biological agent-based modelling. The biological mechanisms implemented — TP53/BRCA1 mutation states and the D1/D2 instability accumulators — were chosen as representative, well-characterised examples to provide a tractable initial foundation. They are not claimed to constitute a complete or clinically calibrated model of oncogenesis. The limitations described below are therefore expected properties of a first-iteration platform, and are explicitly identified to guide future biological enrichment of the model.

### 5.3.1 Model Limitations

**Spatial homogeneity**: The current model treats the tissue as a well-mixed population with no spatial structure. Real tumours exhibit spatial heterogeneity in oxygen tension, nutrient availability, and immune infiltration, all of which influence local cell behaviour. Incorporating a spatial lattice or off-lattice geometry would add biological realism at the cost of computational complexity.

**Two-gene simplification**: Only BRCA1 and TP53 are modelled as discrete gene states. Real oncogenesis involves dozens of driver genes operating across distinct functional axes: oncogene activation (KRAS, MYC, ERBB2), alternative tumour suppressor loss (RB1, PTEN, CDKN2A), epigenetic silencing, and pathway-level logic (PI3K/AKT/mTOR, RAS/MAPK, WNT/β-catenin). As a result, the following oncological phenomena lie outside the scope of the current model and cannot be reproduced without biological extension:

- Gain-of-function oncogenesis (e.g., KRAS-driven adenocarcinoma): the model has no activating mutations, only loss-of-function events.
- Cancer-type-specific mutational signatures: without pathway logic, the model cannot distinguish breast from colorectal from lung carcinogenesis.
- Synthetic lethal interactions beyond BRCA1–TP53 (e.g., PTEN loss + PI3K inhibition, RB1 loss + CDK4/6 dependence).
- Epigenetic silencing as an alternative mechanism to allelic mutation.

These constraints are intentional for a first-iteration proof of concept and are precisely the axes along which the model is designed to be extended.

**Binary immune response**: The D2-threshold model of immune evasion is a significant simplification of the tumour–immune microenvironment. In reality, immune evasion involves multiple cell types, ligand–receptor interactions (e.g., PD-1/PD-L1), and dynamic adaptation of the immune response. The D2 accumulator captures the net outcome of this process (evasion or not) but not its mechanistic detail.

**Absence of clonal competition**: All cells within a scenario share identical parameter sets; there is no per-cell variation in mutation rates, instability deltas, or fitness. This is a fundamental constraint with significant biological consequences that go beyond what is immediately apparent:

In real tumours, clonal selection is the principal mechanism by which aggressive subclones dominate. A subclone acquiring a second TP53 mutation, a KRAS amplification, or an immune checkpoint upregulation gains a fitness advantage over neighbouring cells and expands preferentially — even if numerically rare at first. Without this selection pressure, the current model cannot reproduce:

- **Clonal sweeps**: the replacement of a founding population by a fitter subclone.
- **Intra-tumour heterogeneity (ITH)**: the coexistence of genetically distinct subpopulations, which is a hallmark of most solid tumours at clinical presentation and the primary driver of therapy resistance.
- **Resistance to therapy**: drug resistance typically emerges from pre-existing rare subclones that are selectively amplified under treatment pressure. A uniform-parameter model has no substrate for this process.
- **Neutral evolution**: the accumulation of passenger mutations in the absence of selection, which Tomasetti et al. (2017) argue accounts for a substantial fraction of somatic mutations in colorectal cancer.

Introducing clonal heterogeneity — for example, sampling per-cell μ_TP53 or δ_high from a distribution at birth — would be a high-priority biological extension and is listed in Section 5.4. In the current version, the model should be interpreted as simulating an isogenic population under uniform oncogenic pressure, a scenario that is biologically uncommon but useful for isolating the effect of individual parameters.

### 5.3.2 Software Limitations

**Single-threaded execution**: The current implementation iterates cells sequentially, limiting scalability to populations beyond ~10,000 cells at interactive speeds. Parallelisation using OpenMP or task-based concurrency would be required for large-scale studies.

**No graphical output**: Simulation results are exported as Markdown tables and CSV files. Integration with a visualisation library (e.g., matplotlib via Python binding, or a native plotting library) would improve accessibility for non-technical users and facilitate figure generation for publications.

---

## 5.4 Future Work

Based on the findings and limitations identified above, we propose the following extensions:

1. **Multi-gene driver model**: Incorporate additional genes (KRAS, RB1, PTEN) with their respective interaction rules, enabling simulation of specific cancer types with known mutational signatures.
2. **Spatial tissue model**: Implement a 2D lattice or Voronoi tessellation to model neighbourhood-dependent cell interactions, local oxygen gradients, and spatially constrained immune surveillance.
3. **Clonal heterogeneity**: Allow per-cell variation in mutation rates, sampled from a distribution at birth, to model genetic diversity within a tumour population.
4. **Therapeutic intervention simulation**: Model drug effects as parameter modifications (e.g., increasing θ_D2 to simulate PARP inhibition, or reducing neoplasm_k for targeted therapy), allowing in silico testing of treatment strategies.
5. **Empirical calibration**: Compare simulation outputs against longitudinal genomic datasets (e.g., TCGA somatic mutation frequencies) to fit μ_TP53, μ_BRCA1, and instability delta parameters to specific cancer types.

---

## 5.5 Conclusions

We have presented cellSim, an open-source agent-based simulator of cell evolution and oncogenesis, designed around the Agentic Cell pattern and implemented with Clean Architecture principles in C++17. The simulator models TP53 and BRCA1 gene-state dynamics, dual-accumulator instability (D1/D2), and six-phase cell cycle logic within a tissue of autonomous agents communicating via typed signals.

Across 14 validation scenarios, cellSim qualitatively reproduces key oncological dynamics: BRCA1–TP53 synthetic lethality (Scenario 03), TP53-driven neoplastic transformation (Scenario 04), apoptotic evasion via instability accumulation (Scenario 09), and explosive clonal expansion kinetics analogous to Big Bang-like tumour growth (Scenario 10). All neoplastic cells are 100% immortal, a property that emerges from the model's apoptosis-evasion precondition for transformation rather than being explicitly programmed.

From a software engineering perspective, the application of Clean Architecture delivered measurable benefits: 53 unit tests with full component isolation, deterministic reproducibility via seed control, and a modular structure that enabled Big Bang mode to be added without modifying validated core components. The signal-based tissue communication pattern eliminated a class of concurrency and coupling bugs common in imperative ABM implementations.

cellSim is available at [https://github.com/lpalomerol/cellSim](https://github.com/lpalomerol/cellSim) under an open-source licence, with full documentation, configurable JSON parameter files, and a CI-validated test suite.

---

## References (placeholder — to be completed in Section 8)

- Hollstein M, Sidransky D, Vogelstein B, Harris CC. (1991). p53 mutations in human cancers. *Science*, 253(5015), 49–53.
- Farmer H et al. (2005). Targeting the DNA repair defect in BRCA mutant cells as a therapeutic strategy. *Nature*, 434, 917–921.
- Tomasetti C, Li L, Vogelstein B. (2017). Stem cell divisions, somatic mutations, cancer etiology, and cancer prevention. *Science*, 355(6331), 1330–1334.
- Martin RC. (2017). *Clean Architecture: A Craftsman's Guide to Software Structure and Design*. Prentice Hall.
