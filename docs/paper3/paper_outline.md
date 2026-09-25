# CellSim Paper — Section Outlines (draft)

---

## Abstract

CellSim is a stochastic agent-based model of cellular evolution developed from a
computer science perspective to simulate the emergence of tumour phenotypes in BRCA1
germline carrier tissue. The model frames cancer development as a computational
process: cells are autonomous agents carrying mutable genomes, accumulating damage
across discrete time steps, and transitioning through biologically defined life stages
governed by explicit rules. This interdisciplinary approach — grounding software
engineering abstractions in molecular oncology — allows the model to be both
formally analysed as a system and empirically validated against clinical data.

We present a bootstrap calibration framework that fits CellSim's stochastic parameters
to the BRCA1 cumulative breast cancer risk curve from Kuchenbaecker et al. (JAMA 2017).
Through a two-stage grid search over mutation and damage-accumulation rates (N=1000
bootstrap runs), we identify a parameter set (brca1_rate=0.045, δ₊/₋=0.12/0.24) that
reproduces the clinical curve with SSE=48.5, with simulated cumulative risks falling
within the published 95% confidence intervals for ages 40–80. The calibrated parameters
carry direct biological interpretations, demonstrating that a computationally motivated
model can generate quantitatively meaningful predictions about population-level cancer risk.

---

## Introduction

### P1 — Motivación clínica
BRCA1 germline mutations confer a lifetime breast cancer risk of approximately 70% by
age 80, as established by large prospective cohort studies. Understanding the stochastic
dynamics by which individual cells progress from heterozygous carrier state to frank
malignancy is a fundamental question in cancer biology. Computational models offer a
complementary approach to epidemiological studies, allowing exploration of the mechanistic
parameters that shape population-level risk curves in ways that are not accessible
through clinical observation alone.

### P2 — Perspectiva informática y objetivo multidisciplinar
CellSim approaches this problem from a computer science standpoint: cancer development
is modelled as an emergent property of a multi-agent system, where the tumour is not
programmed explicitly but arises from the collective behaviour of cells following local
rules — mutation, damage accumulation, apoptosis, and division. This framing is
deliberately multidisciplinary: the architecture borrows from software engineering
(hexagonal design, domain-driven modelling, reproducible stochastic simulation), while
the biological parameters are grounded in molecular oncology. Our objective is twofold —
to demonstrate that a computationally rigorous model can reproduce clinically observed
cancer risk curves, and to provide a platform that biologists and informaticians can
extend, parametrise, and interrogate collaboratively.

### P3 — Gap y contribución concreta
Existing agent-based models of tumour initiation either operate at the tissue scale
without single-cell resolution, or lack the genetic fidelity to represent the multi-step
somatic mutation process in BRCA1 carriers. We address this gap with CellSim, a
single-cell resolution model incorporating BRCA1/TP53 gene states, DNA damage
accumulation, and apoptosis signalling. Our principal contribution is a calibration
methodology that bridges the model's computational abstraction to a clinically validated
epidemiological dataset, establishing CellSim as a quantitative tool for breast cancer
risk modelling while opening the door to further multidisciplinary refinement.

---

## Methods

### P1 — Model description
CellSim represents each tissue as a population of 500 autonomous cells, each carrying
a genome with mutable BRCA1 (+/-, initialised as germline heterozygous) and TP53 (+/+)
loci. At each annual tick, cells accumulate genomic damage (D1, D2) at rates determined
by their BRCA1 genotype (δ₊/₋ for heterozygous, δ₋/₋ = 2δ₊/₋ for homozygous mutant),
stochastically mutate towards homozygous loss, and transition through six life stages
(BASELINE → UNSTABLE → UNPROTECTED → PRIMER → TUMORAL). Tumour onset is declared
when the neoplastic fraction of the tissue exceeds 5%, a threshold corresponding to
approximately 5×10⁸–10¹⁰ cells given the model's abstraction level — consistent with
the lower limit of mammographic detectability.

### P2 — Calibration methodology
We performed a two-stage grid search calibrating three parameters (brca1_rate, δ₊/₋,
δ₋/₋) against the six age-anchored cumulative risk estimates from Kuchenbaecker et al.
using sum of squared errors (SSE) as the objective. Each parameter combination was
evaluated over N=200–500 bootstrap runs (independent random seeds), with parallelisation
across 12 cores (OpenMP). A key modelling choice — big bang mode, in which neoplastic
cells divide at 10% per tick rather than 0.1% — was identified as necessary to decouple
the time-to-first-event from the time-to-detectable-clone, reducing SSE from 5295
(default parameters) to 48.5 (calibrated). Convergence analysis confirmed that N=500
produces stable point estimates (max variation <1pp vs. N=2000).

---

## Results

### P1 — Calibration progression
Default parameters produced a substantially delayed onset curve (SSE=5295), with no
simulated cancers before age 50 and overshooting at age 80 (84% vs. 70%). A coarse
grid search without big bang mode achieved a best SSE of 2078 but revealed a structural
trade-off: no parameter combination could simultaneously reproduce early onset (r40≈26%)
and the correct plateau (~70% at age 80). Activating big bang mode broke this trade-off,
reducing SSE to 243 and recovering the correct curve shape.

### P2 — Final calibrated result
Fine-grained grid search (42 combinations, N=500 each) identified the optimal parameter
set (brca1_rate=0.045, δ₊/₋=0.120, δ₋/₋=0.240), confirmed with N=1000 bootstrap runs
(SSE=48.5). Simulated cumulative risks fell within the Kuchenbaecker 95% CI for ages
40–80; the sole residual gap at age 30 (1.7% simulated vs. 4% clinical, CI [2–7%]) was
shown to be structurally irreducible without disrupting the plateau, and is interpreted
as a model limitation. The model's non-penetrance (~30% of tissues never reaching the
5% threshold) emerges naturally from TP53 protection and stochastic resistance,
consistent with the clinical observation that ~30% of BRCA1 carriers remain cancer-free
at age 80.

---

## Discussion

### P1 — Biological interpretation
The calibrated parameters carry direct biological interpretations: brca1_rate=0.045
corresponds to an annual probability of 4.5% for loss of heterozygosity at the BRCA1
locus, consistent with estimates of replication-associated somatic mutation rates in
breast epithelium. The damage accumulation ratio δ₋/₋ = 2δ₊/₋ reflects the amplified
genomic instability of biallelic BRCA1 loss. Big bang mode, while a modelling
simplification, approximates the Gompertzian clonal expansion dynamics of early breast
tumour clones and is essential for reproducing the steep risk increase between ages
40–50 observed clinically.

### P2 — Multidisciplinary value and limitations
A key strength of the CellSim approach is its interpretability: unlike black-box
statistical models, every simulated outcome can be traced to a specific sequence of
cellular events — mutation, damage accumulation, apoptosis failure — making it a
useful tool for generating mechanistic hypotheses. Limitations include the tissue-agnostic
architecture (no spatial structure, no microenvironment), a single tumour threshold, and
the absence of treatment or surveillance effects. These are not fundamental barriers but
opportunities for multidisciplinary extension: oncologists can propose biologically
motivated refinements (e.g. PIK3CA as a cooperating oncogene, lognormal noise on
damage rates) while software engineers can extend the architecture without disrupting
the calibrated core.

---

## Conclusion

### P1 — Synthesis
We have demonstrated that CellSim, with appropriate parameter calibration, can reproduce
population-level breast cancer risk curves from a single-cell mechanistic model. The
calibration framework — bootstrapped grid search against Kuchenbaecker et al. — provides
a reproducible, computationally transparent methodology for fitting stochastic agent-based
models to clinical incidence data.

### P2 — Multidisciplinary outlook
This work establishes CellSim as a proof of concept for a broader research programme:
the formal modelling of carcinogenesis as a computable, parameterisable process. The
calibrated parameters — annual LOH probability, damage accumulation rates — are
biologically interpretable quantities that can be refined as molecular data on BRCA1-
associated somatic evolution accumulates. We envision the model as a shared platform
for oncologists, epidemiologists, and software engineers to collaboratively explore
cancer risk under different genetic backgrounds, treatment strategies, and environmental
conditions.

