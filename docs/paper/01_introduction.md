# Section 1: Introduction
*Draft — cellSim paper*

---

## 1.1 Biological Background

Cancer arises from the progressive failure of cellular control mechanisms that normally maintain genomic integrity and regulate cell fate. In healthy tissue, three principal protective systems operate in concert: (i) DNA repair, carried out by genes such as *BRCA1* (Breast Cancer Susceptibility Gene 1), which corrects replication errors and double-strand breaks; (ii) tumour suppression, mediated primarily by *TP53* (Tumour Protein 53), which acts as the "guardian of the genome" by sensing DNA damage and triggering programmed cell death (apoptosis); and (iii) immune surveillance, by which the immune system identifies and eliminates aberrant cells presenting damage-associated molecular patterns.

When one or more of these systems are compromised—through germline mutations, somatic mutations, or epigenetic silencing—cells may accumulate genomic instability, evade apoptosis, and ultimately undergo neoplastic transformation. The sequence of events is rarely abrupt; rather, it follows a multi-step progression in which successive mutations confer incremental survival advantages.

A particularly striking manifestation of this process is the *oncological Big Bang* phenomenon, first described by Tomasetti et al. (2017) in the context of colorectal cancer. This model proposes that a single founder cell, after a prolonged latent phase of mutation accumulation, can generate a clonal expansion so rapid that the tumour appears to arise explosively rather than gradually. Replicating this dynamics computationally requires models that capture both stochastic mutation events and the non-linear accumulation of genomic instability.

## 1.2 Computational Modelling of Oncogenesis

Agent-based models (ABMs) have emerged as a natural paradigm for simulating tumour progression, as they can represent individual cell behaviour, local interactions, and population-level emergent phenomena simultaneously. Existing tools such as NetLogo [REF], BioDynaMo [REF], and PhysiCell [REF] provide general-purpose ABM frameworks that have been applied to cancer biology. However, these platforms are typically designed as domain-specific languages or as monolithic simulation engines, which limits their extensibility, unit-level testability, and integration into modern software development workflows.

A gap therefore exists for a simulator that is: (a) biologically grounded in well-characterised gene–mutation interactions; (b) architecturally designed for long-term maintainability and scientific reproducibility; and (c) validated through a systematic suite of parametric scenarios with quantifiable outcomes.

## 1.3 The cellSim Simulator

We present **cellSim**, an open-source, agent-based cell evolution simulator implemented in C++17, designed to model oncogenesis through the *Agentic Cell* pattern.

> **Model scope.** cellSim is an exploratory proof-of-concept simulator, not a clinically calibrated model. Its biological components — TP53/BRCA1 mutation states and the D1/D2 instability accumulators — are used as representative examples of well-characterised oncogenic mechanisms, chosen to provide a minimal and tractable starting point for iterative model development. The primary contribution of this work is architectural and methodological: to demonstrate that rigorous software engineering practices can be applied to agent-based biological simulation, producing a platform that is testable, reproducible, and ready for progressive biological enrichment.

Each cell is a fully autonomous agent governed by four internal variables:

- **BRCA1** — DNA repair gene (states: `+/-`, `-/-`)
- **TP53** — tumour suppressor gene (states: `+/+`, `+/-`, `-/-`)
- **D1** — genomic instability accumulator (range: [1, 999])
- **D2** — apoptotic evasion accumulator (range: [1, 999]; phenomenological parameter representing net immune evasion capacity)

Cells progress through six discrete life stages — BASELINE, UNSTABLE, UNPROTECTED, PRIMER, TUMORAL, and DEAD — governed by probabilistic mutation rules and threshold-based transitions. The simulator implements a *Big Bang mode* that reproduces the rapid neoplastic expansion observed clinically, validated across 14 parametric scenarios including controls, realistic configurations, and extreme oncogenic conditions.

Crucially, cellSim is built on **Clean Architecture** principles, separating domain logic (cell behaviour, genome mutation, tissue management) from application orchestration and infrastructure concerns. This design enables 53 unit tests covering all core components, deterministic reproducibility via configurable random seeds, and straightforward extension of the model without modifying validated core logic.

## 1.4 Paper Objectives

This paper has four objectives:

1. **O1 — Model description**: To describe the mathematical and biological foundations of the Agentic Cell model, including state transitions, instability dynamics, and apoptosis mechanisms.
2. **O2 — Software architecture**: To demonstrate how Clean Architecture and the autonomous-agent design pattern improve testability, reproducibility, and extensibility of biological simulators written in C++17.
3. **O3 — Experimental validation**: To show that cellSim produces behaviours qualitatively consistent with selected oncological dynamics, including TP53-dependent neoplastic transformation, apoptotic evasion, and Big Bang-like tumour growth kinetics.
4. **O4 — Open-source availability**: To make available to the scientific community a configurable, documented, and fully tested tool for parametric modelling of oncogenesis.

---

## References (placeholder — to be completed in Section 8)

- Tomasetti C, Li L, Vogelstein B. (2017). Stem cell divisions, somatic mutations, cancer etiology, and cancer prevention. *Science*, 355(6331), 1330–1334.
- [NetLogo reference]
- [BioDynaMo reference]
- [PhysiCell reference]
- [Clean Architecture — Robert C. Martin]
