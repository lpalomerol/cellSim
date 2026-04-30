# Abstract
*Draft — cellSim paper*

---

## Proposed Title

**cellSim: An Agent-Based Cell Evolution Simulator for Oncogenesis Modelling Using Clean Software Architecture**

---

## Structured Abstract (≤250 words)

**Background.**
Computational modelling of oncogenesis requires tools that are simultaneously biologically grounded, scientifically reproducible, and software-engineered for long-term maintainability. Existing agent-based simulators often prioritise biological complexity or computational performance at the expense of testability and architectural clarity, limiting their use in systematic parametric research.

**Objective.**
We present cellSim, an open-source, exploratory agent-based simulator conceived as a proof of concept for modelling cell evolution under progressively degrading genomic conditions. The model uses two well-characterised molecular references — TP53/BRCA1 mutation states and continuous instability accumulators (D1/D2) — as representative examples of oncogenic mechanisms, providing a minimal but extensible biological foundation for iterative development. The primary objective is to demonstrate that Clean Architecture principles and an autonomous-agent design pattern are directly applicable to biological simulation, producing a testable, reproducible, and extensible platform.

**Methods.**
The model targets a BRCA1 germline heterozygous carrier population (HBOC context). Each cell is modelled as an autonomous agent (Agentic Cell) characterised by two tumour-suppressor genes — *TP53* and *BRCA1* — and two continuous instability accumulators representing genomic damage (D1) and apoptotic evasion capacity (D2, a phenomenological parameter). Cells progress through six discrete life stages via a deterministic six-phase cycle executed each simulation year. The simulator is implemented in C++17 following Clean Architecture (domain, application, adapters, ports), with 53 unit tests and fully deterministic output via configurable random seeds. Fourteen parametric scenarios were defined to validate the model across baseline controls, neoplastic phenotypes, biologically realistic configurations, and a Big Bang explosive transformation mode.

**Results.**
All boundary conditions were correctly reproduced: zero-mutation populations remained stable, BRCA1 deficiency under intact TP53 caused complete extinction, and elevated TP53 mutation rates drove 98% neoplastic transformation. In the reference balanced scenario, the first neoplastic cell appeared at year 17 — coinciding with apoptotic evasion capacity (D2) crossing its evasion threshold — reaching 95.2% neoplastic fraction among surviving cells by year 80 (2,018 of 2,120 alive cells). Big Bang mode produced a 34× higher neoplastic cell count at year 30 compared to the balanced configuration, with 93.9% neoplastic fraction and 100% immortality, producing explosive clonal expansion kinetics consistent with Big Bang-like tumour growth dynamics.

**Conclusions.**
cellSim demonstrates that rigorous software engineering practices — component isolation, signal-based communication, and interface-driven design — are directly compatible with, and beneficial for, biological simulation. The simulator is available at https://github.com/lpalomerol/cellSim with full documentation, JSON-configurable scenarios, and a CI-validated test suite.

---

## Word count: ~248 words

---

## Keywords (suggested)

agent-based modelling; oncogenesis simulation; TP53; BRCA1; genomic instability; Clean Architecture; C++17; tumour suppressor; neoplastic transformation; Big Bang tumour growth

---

## Notes for revision

- **Journal fit (SoftwareX / JOSS)**: Both journals accept structured or unstructured abstracts. For JOSS, the abstract is typically shorter (~150 words) and more software-focused; consider a shorter variant if submitting there.
- **Passive voice**: used throughout as appropriate for scientific writing.
- **No citations**: compliant with abstract conventions.
- **No undefined acronyms**: TP53, BRCA1, D1, D2, CI are defined inline or are widely known.
- **Check word count** before submission — target journals may impose different limits (SoftwareX: 250 words; JOSS: ~150 words).
