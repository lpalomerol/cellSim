# 🗺️ Visual Context Engineering Guide - cellSim

## Agent Hierarchy Diagram

```
┌────────────────────────────────────────────────────────────────┐
│                     SIMULATION RUNNER                          │
│                  (tick_simulation())                           │
└────────────────────┬─────────────────────────────────────────┘
                     │
         ┌───────────▼─────────────┐
         │      TISSUE (L1)        │
         │  Population Container   │
         │  Phase: 3 (Desc→Int→Ex) │
         │  Role: Orchestrator     │
         └───────────┬─────────────┘
                     │
        ┌────────────┴────────────┬───────────┐
        │                         │           │
    ┌───▼────┐            ┌───┐ ┌┴──┐   ┌───▼────┐
    │ Cell 1 │            │...│ │C n│   │ Cell m │
    │ (L2)   │            └───┘ └───┘   │ (L2)   │
    └───┬────┘                          └───┬────┘
        │                                    │
    ┌───▼──────────┐                    ┌───▼──────────┐
    │ AGENTIC CELL │                    │ AGENTIC CELL │
    │ Phase: 6 (0→5)                    │ Phase: 6 (0→5)
    │ Role: Decision Maker              │ Role: Decision Maker
    └───┬──────────┘                    └───┬──────────┘
        │                                    │
        ├─ Genome (L3) ◄─────────────────┬──┤
        │  Phase: Mutations               │  │
        │  Role: Gene Container           │  │
        │                                 │  │
        │  ├─ Gene TP53 (L4)              │  │
        │  │  State: +/+, +/-, or -/-     │  │
        │  │  Role: Single locus          │  │
        │  │                              │  │
        │  └─ Gene BRCA1 (L4)             │  │
        │     State: +/+, +/-, or -/-     │  │
        │     Role: Single locus          │  │
        │                                 │  │
        └─ INoiseSource ◄─────────────────┴──┘
           Random noise
           Role: Variability injection
```

---

## Context Structure: 4 Layers Visual

```
┌─────────────────────────────────────────────────────────────────┐
│                     CONTEXT PACKET                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  LAYER 1: IDENTITY (¿Qué es?)                                  │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ agent_type: "AgenticCell"                              │   │
│  │ role: "Central biological decision maker"              │   │
│  │ responsibilities: [lifecycle, mutation, neoplasia]     │   │
│  └─────────────────────────────────────────────────────────┘   │
│                           │                                     │
│  LAYER 2: CONSTRAINT (¿Qué restricciones?)                     │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ biological:                                             │   │
│  │  - TP53 +/+, +/- → NO neoplasia (protected)           │   │
│  │  - Apoptosis works if instability ≤ threshold          │   │
│  │ architectural:                                          │   │
│  │  - Immutable after construction                        │   │
│  │  - Signal-based communication                          │   │
│  └─────────────────────────────────────────────────────────┘   │
│                           │                                     │
│  LAYER 3: STATE (¿Cuál es su estado?)                          │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ cellular:                                               │   │
│  │   id: 1001, age: 50, alive: true, neoplastic: false   │   │
│  │   genomic_instability: 1.2                            │   │
│  │ genome:                                                │   │
│  │   TP53: "+/-", BRCA1: "+/+", is_unstable: true       │   │
│  │ parameters:                                            │   │
│  │   neoplasm_k: 0.002, division_rate: 0.001            │   │
│  │ signals:                                               │   │
│  │   incoming: [ApoptosisSignal], can_emit: true        │   │
│  └─────────────────────────────────────────────────────────┘   │
│                           │                                     │
│  LAYER 4: DECISION (¿Qué debe decidir?)                        │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ current_phase: "phase2_Endocytosis"                    │   │
│  │                                                         │   │
│  │ decisions:                                              │   │
│  │  1. Apoptosis?                                         │   │
│  │     precondition: alive AND instability ≤ threshold    │   │
│  │     outcomes: [undergo_apoptosis, survive]             │   │
│  │                                                         │   │
│  │  2. Genome mutate? (if phase3)                         │   │
│  │     factors: [noise, threshold, instability]           │   │
│  │     outcomes: [mutate, no_mutate]                      │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Cell Lifecycle Flow

```
START (each cycle)
   │
   ├─→ PHASE 0: Baseline Assessment
   │   │ Log state
   │   └─→ Continue
   │
   ├─→ PHASE 1: G1 Checkpoint
   │   │ if normal: check if alive?
   │   │ if neoplastic: skip (go to phase2)
   │   │ if dead: EXCEPTION → END
   │   └─→ Continue if alive
   │
   ├─→ PHASE 2: Endocytosis
   │   │ Process signals
   │   │ if ApoptosisSignal:
   │   │   if genomic_instability ≤ threshold:
   │   │     → EXCEPTION (Cell dies)
   │   │   else: survive (apoptosis blocked)
   │   │ age++
   │   └─→ Continue
   │
   ├─→ PHASE 3: Nuclear Dynamics
   │   │ Mutate genes (normal cells only)
   │   │ for each gene:
   │   │   threshold = base + instability_penalty * genomic_instability
   │   │   if noise < threshold: gene.mutate()
   │   │ neoplastic cells SKIP this
   │   └─→ Continue
   │
   ├─→ PHASE 4: Cytoplasmic Remodeling
   │   │ Decision 1: Become neoplastic?
   │   │   if TP53 -/-:
   │   │     if noise < (neoplasm_k * instability):
   │   │       → EXCEPTION (Become neoplastic)
   │   │
   │   │ Decision 2: Divide?
   │   │   if noise < division_rate:
   │   │     → Emit CellDivisionSignal
   │   │
   │   │ Decision 3: Update instability
   │   │   if unstable genome: instability += high_delta
   │   │   else: instability += low_delta
   │   │
   │   │ neoplastic + Big Bang mode: allow division
   │   │ neoplastic + NO Big Bang: skip to phase5
   │   └─→ Continue
   │
   ├─→ PHASE 5: Exocytosis
   │   │ Emit signals (already done via callbacks)
   │   └─→ END CYCLE
   │
   END or EXCEPTION
```

---

## Decision Point: Should Gene Mutate?

```
PRECONDITION: Gene.live() called

1. GET BASE THRESHOLD
   ┌─────────────────────────────┐
   │ threshold = 0.01 (example)  │
   │ mutation_threshold_         │
   └─────────────────────────────┘
           │
2. CHECK INSTABILITY PENALTY
   ┌──────────────────────────────────────────────┐
   │ if apply_instability (TP53 unstable?):      │
   │   threshold += 0.0001                        │
   │   (mutation_instability_k)                   │
   │ else:                                        │
   │   (no change)                                │
   └──────────────────────────────────────────────┘
           │
           ├─ Normal cell (TP53 +/+): threshold = 0.01
           └─ Unstable cell (TP53 +/-): threshold = 0.0101
           │
3. APPLY GENOMIC INSTABILITY MULTIPLIER
   ┌──────────────────────────────────────────────┐
   │ threshold *= genomic_instability             │
   │ (typically 1.0 to 2.5)                       │
   └──────────────────────────────────────────────┘
           │
           ├─ Normal (instability=1.0): threshold = 0.01
           └─ Unstable (instability=2.0): threshold = 0.0202
           │
4. SAMPLE NOISE
   ┌──────────────────────────────────────────────┐
   │ u01 = random [0, 1]                         │
   └──────────────────────────────────────────────┘
           │
5. COMPARE & DECIDE
   ┌──────────────────────────────────────────────┐
   │ if u01 < threshold:                         │
   │   ✅ MUTATE                                  │
   │   state: +/+ → +/- OR +/- → -/-             │
   │ else:                                        │
   │   ⏭️ NO MUTATE                               │
   │   state unchanged                            │
   └──────────────────────────────────────────────┘

EXAMPLE:
- threshold = 0.0101 (unstable gene)
- genomic_instability = 2.0
- effective = 0.0202 (2.02%)
- u01 sample = 0.015
- 0.015 < 0.0202? YES → MUTATE ✅
```

---

## Decision Point: Should Cell Divide?

```
PRECONDITION: Cell.live() called, phase 4

1. CHECK NEOPLASTIC STATUS
   ┌──────────────────────────────────────┐
   │ if is_neoplastic:                   │
   │   rate = neoplastic_division_rate   │
   │   (typically 0.01 = 1%)            │
   │ else:                               │
   │   rate = normal_division_rate       │
   │   (typically 0.001 = 0.1%)         │
   └──────────────────────────────────────┘
           │
2. SAMPLE NOISE
   ┌──────────────────────────────────────┐
   │ u01 = random [0, 1]                │
   └──────────────────────────────────────┘
           │
3. COMPARE & DECIDE
   ┌──────────────────────────────────────┐
   │ if u01 < rate:                      │
   │   ✅ DIVIDE                          │
   │   - Create daughter cell (clone)    │
   │   - Emit CellDivisionSignal         │
   │ else:                               │
   │   ⏭️ NO DIVIDE                      │
   │   - Remain single cell              │
   └──────────────────────────────────────┘

EXAMPLE:
Normal cell:
- u01 = 0.0005
- rate = 0.001
- 0.0005 < 0.001? YES → DIVIDE ✅

Neoplastic cell (Big Bang):
- u01 = 0.007
- rate = 0.01
- 0.007 < 0.01? YES → DIVIDE ✅
- (10× more frequent than normal)
```

---

## Decision Point: Should Cell Become Neoplastic?

```
PRECONDITION: Cell.live() called, phase 4, NOT already neoplastic

1. CHECK TP53 PROTECTION
   ┌─────────────────────────────────────┐
   │ TP53 status = getTP53()             │
   │                                     │
   │ if TP53 == "+/+":                   │
   │   🛡️ PROTECTED                      │
   │   → Skip neoplasia check            │
   │   → Return (no neoplasia)           │
   │                                     │
   │ if TP53 == "+/-":                   │
   │   🛡️ PROTECTED                      │
   │   → Skip neoplasia check            │
   │   → Return (no neoplasia)           │
   │                                     │
   │ if TP53 == "-/-":                   │
   │   ⚠️ VULNERABLE                     │
   │   → Continue to threshold check     │
   └─────────────────────────────────────┘
           │
2. IF VULNERABLE (TP53 -/-), CALCULATE THRESHOLD
   ┌─────────────────────────────────────┐
   │ threshold = neoplasm_k              │
   │           * genomic_instability     │
   │                                     │
   │ Example:                            │
   │ 0.002 * 2.5 = 0.005 (0.5%)         │
   └─────────────────────────────────────┘
           │
3. SAMPLE NOISE
   ┌─────────────────────────────────────┐
   │ u01 = random [0, 1]                │
   └─────────────────────────────────────┘
           │
4. COMPARE & DECIDE
   ┌─────────────────────────────────────┐
   │ if u01 < threshold:                │
   │   🚨 BECOME NEOPLASTIC              │
   │   - is_neoplastic = true            │
   │   - Throw NeoplasticException       │
   │   - Lifecycle changes (skip phases) │
   │ else:                               │
   │   ⏭️ REMAIN NORMAL                  │
   │   - Continue normally               │
   └─────────────────────────────────────┘

CONSTRAINT VISUALIZATION:
┌──────────┬──────┬────────────────────┐
│ TP53     │ Role │ Neoplasia Possible?│
├──────────┼──────┼────────────────────┤
│ +/+      │ 🛡️  │ ❌ NO              │
│ +/-      │ 🛡️  │ ❌ NO              │
│ -/-      │ ⚠️  │ ✅ YES (if thresh) │
└──────────┴──────┴────────────────────┘
```

---

## Decision Point: Should Cell Undergo Apoptosis?

```
PRECONDITION: ApoptosisSignal received, phase 2

1. CHECK PRECONDITION
   ┌───────────────────────────────────────┐
   │ if NOT alive():                       │
   │   → Skip (already dead)               │
   │                                       │
   │ if genomic_instability > threshold:   │
   │   🛡️ APOPTOSIS BLOCKED               │
   │   → Cell "escapes" (too resistant)    │
   │   → Return (cell survives)            │
   └───────────────────────────────────────┘
           │
2. IF PRECONDITION MET (alive AND low instability)
   ┌───────────────────────────────────────┐
   │ ✅ APOPTOSIS ALLOWED                 │
   │ - alive = false                       │
   │ - Throw CellDeathException            │
   │ - Cell removed from simulation        │
   └───────────────────────────────────────┘

THRESHOLD CHART:
┌────────────────┬──────────────────────┐
│ Instability    │ Apoptosis Works?     │
├────────────────┼──────────────────────┤
│ 1.0 (low)      │ ✅ YES (works)       │
│ 2.0 (moderate) │ ✅ YES (works)       │
│ 5.0 (high)     │ ✅ YES (works)       │
│ 8.0 (very high)│ ✅ YES (if <10)      │
│ 10.0 (extreme) │ 🛑 THRESHOLD        │
│ 15.0 (super)   │ ❌ NO (blocked)      │
└────────────────┴──────────────────────┘

Biological meaning:
- Low instability → Healthy immune response works
- High instability → Tumor has become resistant
```

---

## Mutation Chain Visualization

```
Gene State Progression (Unidirectional)

START: +/+ (Wild-type)
│ All alleles normal
│ enabled() = true
└─→ mutation occurs? → YES
        │
        └──→ MUTATE EVENT
            state = +/-
            │
        +/- (Heterozygous)
        │ One normal, one mutated
        │ enabled() = true
        └─→ mutation occurs again? → YES
                │
                └──→ MUTATE EVENT
                    state = -/-
                    │
                -/- (Homozygous knockout)
                │ Both alleles mutated
                │ enabled() = false
                │ (gene disabled/non-functional)
                │
                └─→ mutate() called again?
                    → NO CHANGE (stuck)
                    → CANNOT GO BACK

CONSTRAINT:
╔════════════════════════════════════════╗
║ Mutations are UNIDIRECTIONAL ONLY      ║
║ +/+ ──(mutate)──> +/-                  ║
║ +/- ──(mutate)──> -/-                  ║
║ -/- ──(mutate)──> -/- (NO CHANGE)      ║
║                                        ║
║ NEVER reverse:                         ║
║ -/- ──X CANNOT MUTATE TO──> +/-        ║
║                                        ║
║ This represents biological reality:    ║
║ mutations accumulate, never revert     ║
╚════════════════════════════════════════╝
```

---

## Context Usage Flow in CLion

```
┌─ YOU want to implement something
│
├─ Open .github/copilot/CONTEXT_ENGINEERING_INDEX.md
│  (5 min - understand what's available)
│
├─ Identify which file has the info:
│  ├─ Agent roles? → agent_roles.md
│  ├─ Cell cycle? → cell_lifecycle_templates.md
│  ├─ Constraints? → agent_context_structure.md
│  └─ Examples? → context_examples/
│
├─ Read the specific section (15 min)
│
├─ Construct your prompt:
│  ├─ "Basándote en..."
│  ├─ Reference 3+ files
│  ├─ Include constraints
│  └─ Specify decision points
│
├─ Paste prompt into Copilot
│
├─ Review generated code:
│  ├─ Checklist: Constraints respected?
│  ├─ Checklist: Preconditions checked?
│  ├─ Checklist: Outcomes valid?
│  └─ Checklist: Biology coherent?
│
├─ Accept or iterate
│
└─ DONE ✅ (usually 1-2 iterations vs 4-5 before)
```

---

## File Navigation Map

```
.github/copilot/
├── CONTEXT_ENGINEERING_INDEX.md ◄── START HERE
├── automation_profile.tpl
├── agent_context_structure.md ◄── For understanding 4 layers
├── agent_roles.md ◄── For agent details (most important)
├── cell_lifecycle_templates.md ◄── For phase details
└── context_examples/
    ├── cell_normal_baseline.json ◄── Example 1
    ├── cell_tp53_heterozygous_unstable.json ◄── Example 2
    └── cell_neoplastic_activated.json ◄── Example 3

docs/
├── CONTEXT_ENGINEERING_SUMMARY.md ◄── Executive summary
├── RESEARCH_CONTEXT_ENGINEERING.md ◄── Analysis
└── PLAN_CONTEXT_ENGINEERING.md ◄── Proposal
```

---

## Quick Reference Checklist

```
BEFORE ASKING COPILOT:
☐ Identified which agent is involved
☐ Read agent_roles.md for that agent
☐ Read relevant phase in cell_lifecycle_templates.md
☐ Reviewed one example in context_examples/
☐ Identified all constraints
☐ Listed valid outcomes
☐ Formulated prompt with 3+ references

REVIEWING COPILOT'S ANSWER:
☐ Does it respect ALL constraints?
☐ Are preconditions checked?
☐ Are outcomes in valid_outcomes list?
☐ Is the biology consistent?
☐ Is error handling appropriate?
☐ Are state changes clear?

ACCEPTING CODE:
☐ Run tests if available
☐ Code review for constraints
☐ Commit with reference to context files
```

---

**Version**: 1.0  
**Visual Reference**: Context Engineering Guide  
**Last Updated**: 2025-12-16


