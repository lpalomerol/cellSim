# Section 0: Related Work (State of the Art)
*Draft — cellSim paper*

---

## Related Work

Agent-based modelling of tumour growth has a rich literature, and several software tools have been developed to support it. We briefly review the most relevant platforms and situate cellSim within this landscape.

**NetLogo** (Wilensky, 1999) is a widely used ABM environment with a domain-specific language and visual interface. It has been applied to cancer modelling [REF], but its interpreted nature limits performance on large populations, and its monolithic design makes unit testing of biological rules difficult.

**PhysiCell** (Ghaffarizadeh et al., 2018) is a C++ framework for 3D off-lattice ABM of multicellular systems. It models cells with detailed biophysical properties (mechanics, oxygen diffusion, signalling) and is well-suited to spatial tumour growth. However, its complexity makes it difficult to isolate and test individual biological rules, and it does not model discrete gene-state transitions (TP53, BRCA1 allele states).

**BioDynaMo** (Breitwieser et al., 2021) is a high-performance ABM platform using C++17 and parallel execution for large-scale simulations (millions of agents). Its strength is scalability; however, it is a general-purpose framework without built-in oncogenesis semantics, requiring users to implement gene mutation logic from scratch.

**Chaste** (Mirams et al., 2013) provides a multiscale simulation environment for cardiac and tumour biology. Its cell-cycle models are well-validated, but the framework targets wet-lab replication of known data rather than exploratory parametric modelling.

**OncoSimulR** (Diaz-Uriarte, 2019) is an R package for simulating tumour evolution under fitness landscapes. It models clonal dynamics and mutation accumulation but operates at the population level rather than per-cell, and lacks architectural design for software engineering goals.

### Comparison with cellSim

| Feature | NetLogo | PhysiCell | BioDynaMo | cellSim |
|---------|---------|-----------|-----------|---------|
| Language | NetLogo DSL | C++ | C++17 | C++17 |
| Per-cell gene-state model | Limited | No | No | ✅ Yes |
| Clean Architecture | No | No | No | ✅ Yes |
| Unit-testable components | Difficult | Partial | Partial | ✅ 53 tests |
| JSON parametric config | No | No | Partial | ✅ Yes |
| Big Bang oncological mode | No | No | No | ✅ Yes |
| Open source | ✅ | ✅ | ✅ | ✅ |

cellSim occupies a distinct niche: it targets **biologically specific gene-level modelling** (TP53/BRCA1 allele states, D1/D2 instability accumulation) while simultaneously applying **software engineering best practices** that are rarely prioritised in scientific simulators. Its configurability via JSON and its deterministic seed control make it particularly suitable for systematic parametric studies and reproducible research.

---

## References (placeholder)

- Wilensky, U. (1999). NetLogo. http://ccl.northwestern.edu/netlogo/
- Ghaffarizadeh, A. et al. (2018). PhysiCell: An open source physics-based cell simulator for 3-D multicellular systems. *PLOS Computational Biology*, 14(2), e1005991.
- Breitwieser, L. et al. (2021). BioDynaMo: a modular platform for high-performance agent-based simulation. *Bioinformatics*, 38(2), 453–460.
- Mirams, G.R. et al. (2013). Chaste: An Open Source C++ Library for Computational Physiology and Biology. *PLOS Computational Biology*, 9(3), e1002970.
- Diaz-Uriarte, R. (2019). OncoSimulR: genetic simulation with arbitrary epistasis and mutator genes in asexual populations. *Bioinformatics*, 35(22), 4841–4843.
