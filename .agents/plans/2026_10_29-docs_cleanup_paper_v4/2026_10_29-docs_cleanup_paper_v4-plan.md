---
name: "docs_cleanup_paper_v4"
description: "Identification and cleanup of obsolete legacy documents, discarded hypotheses, and historical paper drafts in docs/ based on manuscript v4"
created_at: "2026-10-29T17:22:00+01:00"

created_by:
  tool: "Antigravity CLI"
  model:
    name: "Gemini"
    version: "3.8 Flash (Medium)"
    reasoning_effort: "medium"

implemented_by:
  tool: "Antigravity CLI"
  model:
    name: "Gemini"
    version: "3.8 Flash (Low)"
    reasoning_effort: "low"

last_implementation_at: "2026-10-29T17:34:00Z"
has_completed_all_phases: true
---

# 🎯 Goal

Purge over 104 MB of obsolete legacy paper drafts, discarded hypotheses (such as Big Bang mode), superseded calibration traces, and outdated documentation in `docs/`, aligning the repository's documentation strictly with the published scientific manuscript **BRCA1-ONSET v4** (`/home/luis/Escritorio/abms/v4/Palomero_BRCA1_102126.txt`).

---

# 👀 Context

- **Manuscript v4 Location**: [`/home/luis/Escritorio/abms/v4`](file:///home/luis/Escritorio/abms/v4) (`Palomero_BRCA1_102126.txt`, `MainFigs_review_102126`, `SupplFig-Table-review_102126`)
- **Active Publication Pipelines**: [`experiments/`](file:///home/luis/CLionProjects/cellSim/experiments/) (`exp01_cs_...` to `exp09_ts_...`)
- **Official Deliverables**: [`results/paper_figures/`](file:///home/luis/CLionProjects/cellSim/results/paper_figures/)
- **Documentation Target**: [`docs/`](file:///home/luis/CLionProjects/cellSim/docs/)
- **Audit Document**: [`docs/DOCS_AUDIT_PAPER_V4.md`](file:///home/luis/CLionProjects/cellSim/docs/DOCS_AUDIT_PAPER_V4.md)

---

# 📜 Public Contracts

### 1. Folders Purged (`rm -rf`)
- `docs/paper/` (1.8 MB): V1 draft with legacy scenario 09 and Big Bang figures.
- `docs/paper2/` (55 MB): V2 software reproducibility paper attempt and 50+ MB of external PDFs in `refs/`.
- `docs/paper3/` (980 KB): Exploratory Big Bang calibration draft.
- `docs/paper4/` (1.7 MB): Grid search and SSE landscape draft.
- `docs/paper5/` (1.4 MB): Intermediate ABC posterior plots.
- `docs/paper6/` (6.9 MB): Intermediate collaboration notes and plots with mathematicians.
- `docs/paper7/` (2.2 MB): Tolerance level variance contraction draft.
- `docs/papers/` (1.5 MB): Duplicate 2008 PDF and static HTML report.
- `docs/paper20260828_1_validation_secondary/` (29 MB): Secondary validation draft and 29 MB of raw CSV traces.
- `docs/paper20260907_transition_states/` (3.8 MB): Complex 2D/3D transition state exploratory scripts and figures.
- `docs/diagrams_luis/` (300 KB): Handwritten/informal meeting sketches.
- `docs/adr/` (64 KB): 8 obsolete ADRs from Dec 2025 referencing Copilot prompts, obsolete paths (`src/application`), and discarded biology.

### 2. Loose Files Purged (`rm`)
- `docs/memoria-interna-calibracion.docx` (1.4 MB): June 2026 memo synthesizing legacy papers 3 and 6.
- `docs/BIG_BANG_MODE.md`: Discarded Big Bang mode hypothesis.
- `docs/CONTEXT_ENGINEERING_SUMMARY.md`: Copilot CLion context setup summary.
- `docs/PLAN_CONTEXT_ENGINEERING.md`: Copilot CLion context plan.
- `docs/RESEARCH_CONTEXT_ENGINEERING.md`: Copilot CLion research.
- `docs/QA_TESTING_GUIDE.md`: Obsolete QA guide referencing deleted tests and targets.
- `docs/AGENTIC_CELL_DOCUMENTATION.md`: V2.0 docs containing obsolete biological rules (BRCA1 -/- dying immediately, quadratic instability formulas).
- `docs/model_transitions_spec.md`: Obsolete 3D state transition spec.
- `docs/diagrams.md`: Obsolete ASCII diagrams with quadratic formulas.
- `docs/openmp_optimization.md`: Obsolete OpenMP memo referencing deleted `scripts/` path.
- `docs/CODEBASE_MANUSCRIPT_AUDIT.md`: Superseded September audit referencing deleted monolith C++ paths.

### 3. Preserved and Modernized Assets (All in English)
- `docs/DOCS_AUDIT_PAPER_V4.md`: Full audit and traceability matrix vs. paper v4.
- `docs/README.md`: Modernized to reflect the official **BRCA1-ONSET** architecture and 9 experiment pipelines.

---

# 🪜 Phases

### Phase 1: Audit docs/ Against Manuscript v4 and Mark Obsolete Assets for Deletion
*(Recommended model: Gemini 3.8 Flash Medium)*
**Description:** Analyze every directory and document in `docs/` against the paper v4 text and figures at `/home/luis/Escritorio/abms/v4`. Classify each item by obsolescence criteria (discarded hypotheses, dead simulation runs, old Copilot configs, contradictory biology) and create a comprehensive audit report marking all obsolete assets for deletion without destroying files yet.
- [x] Ask and confirm with the user that the recommended model is active.
- [x] Deeply analyze `/home/luis/Escritorio/abms/v4` manuscript (`Palomero_BRCA1_102126.txt`) and review figures (`MainFigs_review_102126`, `SupplFig-Table-review_102126`).
- [x] Inspect and catalog all 23 items and subdirectories in `docs/` with disk usage and content comparison.
- [x] Generate comprehensive audit report [`docs/DOCS_AUDIT_PAPER_V4.md`](file:///home/luis/CLionProjects/cellSim/docs/DOCS_AUDIT_PAPER_V4.md) detailing exact obsolescence reasons and marking 12 folders and 11 files (~104.9 MB) for deletion.
- [x] Verify that existing tests and experiment smoke tests continue to pass (`make test`).
- [x] STOP. Present the changes to the user for review and suggest commit messages. Do NOT proceed to the next phase until the user explicitly asks.

---

### Phase 2: Execute Purge of Marked Assets & Align docs/README.md with BRCA1-ONSET v4
*(Recommended model: Gemini 3.8 Flash Low)*
**Description:** Permanently remove the marked obsolete folders and files from `docs/` (>104 MB freed), purge `docs/adr/`, rewrite `docs/README.md` and `docs/DOCS_AUDIT_PAPER_V4.md` in English to document the canonical BRCA1-ONSET model, states, parameters, and experiments as described in manuscript v4.
- [x] Ask and confirm with the user that the recommended model is active.
- [x] Remove the 12 marked obsolete directories in `docs/`: `paper/`, `paper2/`, `paper3/`, `paper4/`, `paper5/`, `paper6/`, `paper7/`, `papers/`, `paper20260828_1_validation_secondary/`, `paper20260907_transition_states/`, `diagrams_luis/`, `adr/`.
- [x] Remove the 11 marked loose obsolete files in `docs/`: `memoria-interna-calibracion.docx`, `BIG_BANG_MODE.md`, `CONTEXT_ENGINEERING_SUMMARY.md`, `PLAN_CONTEXT_ENGINEERING.md`, `RESEARCH_CONTEXT_ENGINEERING.md`, `QA_TESTING_GUIDE.md`, `AGENTIC_CELL_DOCUMENTATION.md`, `model_transitions_spec.md`, `diagrams.md`, `openmp_optimization.md`, `CODEBASE_MANUSCRIPT_AUDIT.md`.
- [x] Modernize and rewrite [`docs/README.md`](file:///home/luis/CLionProjects/cellSim/docs/README.md) in English to accurately summarize the BRCA1-ONSET v4 architecture, cell states (Normal, Primed, Tumoral), parameters ($\omega_1, \omega_2$), and references to the 9 vertical slices in `experiments/`.
- [x] Translate [`docs/DOCS_AUDIT_PAPER_V4.md`](file:///home/luis/CLionProjects/cellSim/docs/DOCS_AUDIT_PAPER_V4.md) into English.
- [x] Verify repository integrity and test suites with `make test && make exp-smoke-all`.
- [x] STOP. Present the changes to the user for review and suggest commit messages.

---

# ⏭️ Next Step

All phases of the plan are complete!
The `docs/` folder has been thoroughly cleaned of 104.9 MB of legacy residue and now solely contains clean, publication-ready documentation in English aligned with the published manuscript v4.

Legacy clutter swept clean thanks to [Codely](https://codely.com) AI tooling. 🐢 💨 📦 🧹 📖 ✨ 🚀
