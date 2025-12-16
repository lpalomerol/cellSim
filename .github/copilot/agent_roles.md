# Agent Roles - cellSim Biological Agents

## Matriz de Agentes

```
┌──────────────┬──────────────────┬──────────────┬─────────────────┐
│ Agent        │ Level            │ Scope        │ Lifecycle       │
├──────────────┼──────────────────┼──────────────┼─────────────────┤
│ Tissue       │ Population       │ All cells    │ 3 phases        │
│ AgenticCell  │ Organism         │ Single cell  │ 6 phases        │
│ Genome       │ Cellular         │ Gene set     │ Mutation events │
│ Gene         │ Molecular        │ Single locus │ 3-state chain   │
└──────────────┴──────────────────┴──────────────┴─────────────────┘
```

---

## 1. TISSUE (Population Level)

### Identity
```
Type: Tissue
Level: Population Orchestrator
Role: Container and lifecycle manager for a cell population
```

### Responsibilities
- ✅ Hold collection of AgenticCell instances
- ✅ Execute synchronized lifecycle across all cells
- ✅ Route signals between cells (apoptosis, division)
- ✅ Track emerging neoplasms
- ✅ Provide population statistics (genetic tracking)

### Inputs
| Input | Type | Source |
|-------|------|--------|
| Cell collection | `vector<ICell*>` | Added via `addCell()` |
| Emitted signals | `ISignal` | Each cell's `signal_emitter_` |
| Queries | Various | External code (simulation runner) |

### Outputs
| Output | Type | Use |
|--------|------|-----|
| Population state | `GeneticTrackingData` | Statistics queries |
| Routed signals | `ISignal` | Directed to target cells |
| Lifecycle state | Enum (phase) | Implicit in execution |

### Lifecycle (3 Phases)

```
Phase 0: Description
  └─ Log current population state
  └─ Compute genetic tracking (neoplasm counts)
  └─ Report statistics

Phase 1: Signal Integration
  └─ Collect signals emitted by cells in previous tick
  └─ Route based on target IDs:
     - Neoplasm signals → track + send apoptosis response
     - Division signals → register daughter cell
     - Other signals → handle as needed

Phase 2: Execute Cell Cycles
  └─ For each cell:
     └─ Call cell.live()
     └─ Catch exceptions (CellDeathException, NeoplasticException)
     └─ Let signal_emitter collect resulting signals for next tick
```

### Key Decisions

**Decision 1: When to signal apoptosis?**
- Trigger: Neoplasm signal received
- Rule: Always send apoptosis response to the neoplastic cell
- Justification: Tissue acts as immune response

**Decision 2: When to add daughter cell?**
- Trigger: CellDivisionSignal with valid daughter
- Rule: Assign new unique ID, add to population
- Justification: Maintain cell population consistency

### Constraints

```
CONSTRAINT 1: Cell Autonomy
├─ Tissue does NOT directly mutate cells
├─ Cells emit signals; Tissue routes them
└─ Rationale: Separation of concerns, modularity

CONSTRAINT 2: Signal Causality
├─ Signals can only affect cells in NEXT phase
├─ No retroactive signal handling
└─ Rationale: Deterministic, repeatable execution

CONSTRAINT 3: Population Consistency
├─ Cell IDs must be unique within Tissue
├─ Removed cells cannot be re-added with same ID
└─ Rationale: Traceability, genealogy tracking

CONSTRAINT 4: Phase Atomicity
├─ Phase 0: Read-only operations only
├─ Phase 1: Update signal queue (cannot modify cells)
├─ Phase 2: Execute cell.live() (cannot call live() in phase 0/1)
└─ Rationale: Predictable ordering
```

### State Structure

```cpp
struct TissueState {
  uint64_t tissue_id;
  vector<unique_ptr<ICell>> cells_;
  set<uint64_t> identified_neoplasms_;
  vector<unique_ptr<ISignal>> signals_new_;  // Current cycle
  ILoggerPtr logger_;
};
```

### When to Use Tissue Context
- ✅ Implementing population-level simulation
- ✅ Signal routing logic
- ✅ Population statistics
- ✅ Population-level visualization

### When NOT to Use Tissue Context
- ❌ Single cell behavior (use AgenticCell)
- ❌ Gene-level mutations (use Gene/Genome)
- ❌ Detailed cell decision logic (use AgenticCell phase logic)

---

## 2. AGENTICCELL (Organism Level)

### Identity
```
Type: AgenticCell
Level: Central Biological Decision Maker
Role: Simulates a living eukaryotic cell with full lifecycle
```

### Responsibilities
- ✅ Execute 6-phase cell cycle
- ✅ Manage internal genome state
- ✅ Process incoming signals (apoptosis, stimuli)
- ✅ Detect neoplastic transformation
- ✅ Emit division and neoplasm signals
- ✅ Age and lifecycle management

### Inputs
| Input | Type | Source | Frequency |
|-------|------|--------|-----------|
| Noise (randomness) | `INoiseSource` | Injected at construction | Every `live()` call |
| Apoptosis signals | `ApoptosisSignal` | Tissue (from other cells) | Async, in message queue |
| Genome state | `Genome` | Internal, mutates via `live()` | Every `live()` call |
| Parameters | Various | Construction + initial state | Immutable |

### Outputs
| Output | Type | Triggered | Effect |
|--------|------|-----------|--------|
| Neoplasm signal | `NeoplasmSignal` | When neoplastic | Tissue detects tumor |
| Division signal | `CellDivisionSignal` | When dividing | Tissue registers daughter |
| Cell death | Exception | When apoptotic | Cell removed from simulation |
| Age increment | Internal state | Each `live()` | Historical tracking |

### Lifecycle (6 Phases)

```
Phase 0: Baseline Assessment
├─ Purpose: Snapshot state (logging)
├─ Operations: details() [read-only]
├─ State changes: None
└─ Copilot guideline: "What needs to be logged?"

Phase 1: G1 Integrity Checkpoint
├─ Purpose: Validate cell is alive
├─ Precondition: Cell must be alive (else → CellDeathException)
├─ Special case: Neoplastic cells SKIP this check
└─ Copilot guideline: "When should death be detected?"

Phase 2: Endocytosis (Signal Processing)
├─ Purpose: Process all incoming messages
├─ Operations: Drain message queue, handle ApoptosisSignal
├─ Decision: Should I undergo apoptosis?
│  └─ Precondition: genomic_instability ≤ apoptosis_instability_threshold
│  └─ If yes: throw CellDeathException
├─ State changes: age++, possibly alive=false
└─ Copilot guideline: "How to respond to external signals?"

Phase 3: Nuclear Dynamics
├─ Purpose: Genome mutation (interphase)
├─ Operations: Call genome_.liveAllGenes(genomic_instability_)
├─ State changes: Genes may mutate
├─ Constraint: TP53 status determines mutation rate
└─ Copilot guideline: "Calculate effective mutation probabilities"

Phase 4: Cytoplasmic Remodeling (S/G2/M Decision)
├─ Purpose: Major decisions (neoplasia, division, instability)
├─ Decisions:
│  ├─ 1. Should I become neoplastic? (if TP53 -/-)
│  │    └─ Throw NeoplasticException if YES
│  ├─ 2. Should I divide? (if division_rate met)
│  │    └─ Emit CellDivisionSignal if YES
│  └─ 3. Update genomic_instability (based on genome status)
├─ Constraint: Neoplastic cells CAN divide in Big Bang mode
└─ Copilot guideline: "What are the 3 major cell fate decisions?"

Phase 5: Exocytosis (Signal Emission)
├─ Purpose: Emit accumulated signals
├─ Operations: signal_emitter_(signal) for each signal generated
├─ State changes: None (signals held by tissue)
└─ Copilot guideline: "Which signals does this cell emit?"

Special Rule: Neoplastic Cell Cycle
├─ IF big_bang_mode = true:
│  └─ Execute: Phase 2 → Phase 4 (division allowed) → Phase 5
├─ ELSE:
│  └─ Execute: Phase 2 → Phase 5 (only signal processing)
└─ Rationale: Tumor cells either rapid proliferate (BB) or are just signaling
```

### Key Decisions

**Decision 1: Should cell become neoplastic?** (Phase 4)
```
Precondition: NOT already neoplastic
Rule: Sample u01 from noise
If: u01 < (neoplasm_k * genomic_instability)
    AND TP53 status is "-/-"
Then: become_neoplastic = true
Else: remain_normal = true

Constraint: TP53 +/+ or +/- → protection even if threshold met
Constraint: TP53 -/- → NO protection, threshold is absolute
```

**Decision 2: Should cell divide?** (Phase 4)
```
Precondition: Cell alive
Rule: Sample u01 from noise
If: u01 < (normal_division_rate OR neoplastic_division_rate)
Then: emit_division_signal = true
       create_daughter_cell()
Else: do_not_divide = true

Note: neoplastic_division_rate only used in Big Bang mode
```

**Decision 3: Should cell undergo apoptosis?** (Phase 2)
```
Trigger: ApoptosisSignal received
Rule: If genomic_instability <= apoptosis_instability_threshold
      Then: apoptosis_allowed = true
      Else: apoptosis_blocked = true
         (cell survives the signal)

Constraint: Neoplastic cells with high instability are resistant
Constraint: Only "well-integrated" tumors can evade apoptosis
```

**Decision 4: Update genomic instability** (Phase 4)
```
Rule: If genome.isUnstable() (TP53 is +/- or -/-)
      Then: genomic_instability = base_value + (high_delta_instability)
      Else: genomic_instability = base_value + low_delta_instability

Rationale: TP53 loss increases genome-wide instability
```

### Constraints

```
CONSTRAINT 1: Genome Ownership
├─ Cell owns Genome exclusively
├─ No other entity can mutate cell's genome
├─ Mutations happen ONLY during phase3 (live())
└─ Rationale: Encapsulation, predictability

CONSTRAINT 2: TP53 Monopoly on Neoplasia Threshold
├─ TP53 +/+ → neoplasm IMPOSSIBLE (protected)
├─ TP53 +/- → neoplasm IMPOSSIBLE (heterozygote protected)
├─ TP53 -/- → neoplasm POSSIBLE (if threshold crossed)
├─ No other gene can compensate
└─ Rationale: Biological fidelity (TP53 is "guardian of genome")

CONSTRAINT 3: Apoptosis Threshold Dependency
├─ Apoptosis only works if genomic_instability <= apoptosis_instability_threshold
├─ High instability = apoptosis resistance
├─ This is how tumors "escape" immune response
└─ Rationale: Integrate instability → survival trade-off

CONSTRAINT 4: Signal Causality
├─ Signals received in phase2 are NOT processed until they're in message queue
├─ Cannot process signal AND modify genome in same tick
└─ Rationale: Deterministic, repeatable

CONSTRAINT 5: One-Way Aging
├─ Age ONLY increments (never decrements)
├─ Age increments only if cell is alive
├─ Used for historical traceability
└─ Rationale: Temporal ordering

CONSTRAINT 6: Lifecycle Phase Execution Order
├─ MUST execute phases in 0→1→2→3→4→5 order
├─ Cannot skip phases
├─ Exceptions (Death, Neoplasia) stop further phases
└─ Rationale: Biological fidelity
```

### State Structure

```cpp
struct AgenticCellState {
  uint64_t cell_id;
  uint64_t age;
  bool alive;
  bool is_neoplastic;
  bool has_evaded_apoptosis;
  double genomic_instability;
  
  Genome genome;
  
  double neoplasm_k;
  double division_rate;
  double neoplastic_division_rate;
  double apoptosis_instability_threshold;
  
  queue<unique_ptr<ISignal>> incoming_messages_;
  function<void(unique_ptr<ISignal>)> signal_emitter_;
  
  INoiseSource* noise_;
  ILoggerPtr logger_;
};
```

### When to Use AgenticCell Context
- ✅ Cell lifecycle logic (any phase)
- ✅ Mutation probability calculations
- ✅ Neoplasia detection
- ✅ Signal response logic
- ✅ Division mechanism

### When NOT to Use AgenticCell Context
- ❌ Population-level behavior (use Tissue)
- ❌ Single gene mutation mechanics (use Gene)
- ❌ Genome-wide analysis (use Genome for queries only)

---

## 3. GENOME (Cellular Level)

### Identity
```
Type: Genome
Level: Genetic Information Container
Role: Manages collection of genes and aggregate mutation state
```

### Responsibilities
- ✅ Hold collection of genes (TP53, BRCA1, etc.)
- ✅ Determine aggregate stability (depends on TP53)
- ✅ Compute mutation probabilities for each gene
- ✅ Clone for daughter cells
- ✅ Report genetic status

### Inputs
| Input | Type | Frequency |
|-------|------|-----------|
| Mutation trigger | Via `live()` | Every cell.live() |
| Genomic instability factor | double | From AgenticCell |
| Noise source | INoiseSource | Injected once |

### Outputs
| Output | Type | Use |
|--------|------|-----|
| Instability status | bool | Determines mutation rate |
| Gene references | Gene* | Query specific genes |
| Clone | Genome | Daughter cell creation |

### Lifecycle

```
Event: Genome.liveAllGenes(genomic_instability)
├─ For each gene in genome:
│  └─ Call gene.live(apply_instability=genome.isUnstable(), genomic_instability)
├─ Each gene samples noise and decides to mutate
└─ Result: Genes transition states (+/+ → +/- → -/-)
```

### Key Decisions

**Decision 1: Is genome unstable?**
```
Rule: Query TP53 gene status
If: TP53 status == "+/-" OR "-/-"
Then: is_unstable = true
      (all mutations get penalty via mutation_instability_k)
Else: is_unstable = false
      (baseline mutation rates only)

Rationale: TP53 is "guardian of genome"
```

### Constraints

```
CONSTRAINT 1: Immutable Gene Set
├─ Cannot add/remove genes after construction
├─ Gene set is fixed at genome creation
└─ Rationale: Simplicity, no dynamic mutations in gene set

CONSTRAINT 2: TP53 Monopoly on Stability
├─ Only TP53 determines genome.isUnstable()
├─ No other gene affects this
└─ Rationale: Biological accuracy

CONSTRAINT 3: Unidirectional Mutation Propagation
├─ Mutations can only increase (not decrease)
├─ TP53 +/+ → +/- → -/- never reverses
└─ Rationale: Biological realism

CONSTRAINT 4: Clone Consistency
├─ Cloned genome is identical to parent
├─ Both share same logger reference
└─ Rationale: Daughter inherits parent's full genome
```

### State Structure

```cpp
struct GenomeState {
  unordered_map<string, Gene> genes_;  // e.g., {"TP53" → Gene, "BRCA1" → Gene}
  ILoggerPtr logger_;
};
```

### When to Use Genome Context
- ✅ Calculating mutation probabilities
- ✅ Checking stability
- ✅ Cloning for division
- ✅ Gene state queries

### When NOT to Use Genome Context
- ❌ Single gene mutation (use Gene directly)
- ❌ Cell-level decisions (use AgenticCell)

---

## 4. GENE (Molecular Level)

### Identity
```
Type: Gene
Level: Single Genetic Locus
Role: Represents a single gene with binary mutation states
```

### Responsibilities
- ✅ Track mutation state (+/+, +/-, -/-)
- ✅ Calculate mutation probability based on threshold
- ✅ Transition state when mutation occurs
- ✅ Report current status

### Inputs
| Input | Type | Frequency |
|-------|------|-----------|
| Noise sample | double u01 | Every `live()` call |
| Mutation threshold | double | Per live() call |
| Genomic instability | double | Per live() call |

### Outputs
| Output | Type |
|--------|------|
| Mutated state | State enum (PlusPlus/PlusMinus/MinusMinus) |
| Enabled status | bool (true if +/+ or +/-, false if -/-) |
| Mutation probability | double (for logging) |

### Lifecycle (3-State Chain)

```
State 0: +/+ (Wild-type, normal)
  └─ Allele 1: +, Allele 2: +
  └─ enabled() = true
  └─ Next: mutate() → +/-

State 1: +/- (Heterozygous)
  └─ Allele 1: +, Allele 2: -
  └─ enabled() = true (gene still functions, but reduced)
  └─ Next: mutate() → -/-

State 2: -/- (Homozygous knockout)
  └─ Allele 1: -, Allele 2: -
  └─ enabled() = false (gene is non-functional)
  └─ Next: mutate() → [no change, stuck]

Transition Rule:
├─ NOT reversible (no back-mutation)
├─ Sequential only (no jumping)
└─ Each mutate() call advances by one step
```

### Key Decision

**Decision: Should this gene mutate?** (per `live()` call)
```
Rule:
1. Calculate threshold = mutation_threshold
2. If apply_instability:
   └─ threshold += mutation_instability_k
3. Multiply by genomic_instability factor:
   └─ threshold *= genomic_instability
4. Sample u01 from noise
5. If u01 < threshold:
   └─ mutate() = true → transition state
   Else:
   └─ mutate() = false → remain in current state

Example:
  mutation_threshold = 0.01
  apply_instability = true (TP53 is +/- or -/-)
  mutation_instability_k = 0.005
  genomic_instability = 2.0
  
  Effective threshold = (0.01 + 0.005) * 2.0 = 0.03
  If u01 < 0.03 → mutate
  Else → don't mutate
```

### Constraints

```
CONSTRAINT 1: Unidirectional Mutation
├─ Mutations: +/+ → +/- → -/-, NEVER reverse
├─ Cannot go from -/- back to +/-
└─ Rationale: Biological accuracy (no back-mutation)

CONSTRAINT 2: Sequential State Progression
├─ Cannot jump states (e.g., +/+ directly to -/-)
├─ Must go through +/- intermediate
└─ Rationale: Represents allele-by-allele mutation

CONSTRAINT 3: Deterministic Given Noise
├─ Same noise sample + same threshold = same result
├─ Mutation is fully determined by noise
└─ Rationale: Reproducibility, debugging

CONSTRAINT 4: Threshold Additivity
├─ Thresholds combine: base + instability_penalty + genomic_factor
├─ All factors are multiplicative/additive as specified
└─ Rationale: Clear, predictable behavior
```

### State Structure

```cpp
struct GeneState {
  string name_;                      // "TP53", "BRCA1", etc.
  State state_;                      // +/+, +/-, or -/-
  double mutation_threshold_;        // Base mutation probability
  double mutation_instability_k_;    // Penalty added when TP53 unstable
  INoiseSource* noise_;              // Injected randomness
  ILoggerPtr logger_;
};
```

### When to Use Gene Context
- ✅ Calculating individual gene mutation probability
- ✅ Understanding state progression
- ✅ Validating mutation logic

### When NOT to Use Gene Context
- ❌ Multiple genes interaction (use Genome)
- ❌ Cell-level decisions (use AgenticCell)

---

## Agent Interaction Diagram

```
     Simulation Runner
            │
            ▼
      ┌─────────────────┐
      │    Tissue       │
      │ (Population)    │
      └────────┬────────┘
               │
        ┌──────┴──────┐
        ▼             ▼
    ┌──────────┐  ┌──────────┐
    │ Cell 1   │  │ Cell 2   │
    │ (Organism)│  │ (Organism)│
    │          │  │          │
    │ ┌──────┐ │  │ ┌──────┐ │
    │ │Genome│ │  │ │Genome│ │
    │ │      │ │  │ │      │ │
    │ │┌────┐│ │  │ │┌────┐│ │
    │ ││Gene││ │  │ ││Gene││ │
    │ │└────┘│ │  │ │└────┘│ │
    │ └──────┘ │  │ └──────┘ │
    └──────────┘  └──────────┘
         │             │
         ├─ Signal(Neoplasm)
         ├─ Signal(Division)
         └─ Signal(Apoptosis)
```

---

## Summary Table

| Agent | Level | Scope | Owns | Lifecycle | Key Decision |
|-------|-------|-------|------|-----------|--------------|
| Tissue | Population | All cells | Cell vector | 3 phases | Signal routing |
| AgenticCell | Organism | Single cell | Genome | 6 phases | Neoplasia/Division |
| Genome | Cellular | Gene set | Gene map | Mutation events | Stability check |
| Gene | Molecular | Single locus | State enum | 3-state chain | Mutate? |


