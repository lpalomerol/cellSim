# Cell Lifecycle Templates - cellSim 6-Phase Cycle

## Overview

The AgenticCell lifecycle consists of 6 phases executed sequentially. Each phase has a specific purpose, inputs, outputs, and decisions.

---

## Phase 0: Baseline Assessment

### Purpose
Snapshot and describe current cell state for logging/visualization.

### Type
**Read-only observation phase**

### Inputs
- Current state: `id`, `age`, `alive`, `neoplastic`
- Genome state: Gene statuses
- Parameters: Current thresholds

### Operations
```cpp
void phase0_BaselineAssessment() const {
  // Log current state
  logger_->logCell("Cell " + id + ": age=" + age + 
                   ", alive=" + alive + 
                   ", neoplastic=" + neoplastic);
  
  // Optionally call details() for verbose output
  details();
}
```

### State Changes
- ✅ NONE (read-only, logging only)

### Decision Points
- None

### Exception Handling
- ✅ Safe to call anytime
- ✅ Cannot throw exceptions

### When to Use
- Tracing/debugging
- Visualization
- Statistics collection

---

## Phase 1: G1 Integrity Checkpoint

### Purpose
Validate that cell is alive. If dead, stop lifecycle immediately.

### Type
**Validation phase**

### Inputs
- `alive()` status

### Operations
```cpp
void phase1_G1IntegrityCheckpoint() const {
  if (!alive()) {
    throw CellDeathException("dead@phase1");
  }
  // If we reach here, cell is alive; continue to phase 2
}
```

### State Changes
- ✅ NONE

### Decision Points

**Decision: Is cell alive?**
- If TRUE: Continue to phase 2
- If FALSE: Throw CellDeathException → stop cycle

### Constraint
**SPECIAL: Neoplastic cells BYPASS this check**
- Neoplastic cells are NOT killed at this checkpoint
- Rationale: Allow them to receive apoptosis signals in phase 2

### Exception Handling
- ✅ Throws `CellDeathException` if dead
- ✅ Caught by `live()` caller (Tissue)

### When to Use
- Every normal cell cycle
- NOT called for neoplastic cells (they go directly to phase 2)

---

## Phase 2: Endocytosis (Signal Processing)

### Purpose
Process all incoming messages (signals) from other cells. Respond to external stimuli.

### Type
**Responsive phase**

### Inputs
- `incoming_messages_` queue: `ApoptosisSignal`, others
- Current state: `alive`, `genomic_instability`
- Parameters: `apoptosis_instability_threshold`

### Operations
```cpp
void phase2_Endocytosis() {
  // Process all incoming messages
  while (!incoming_messages_.empty()) {
    auto signal = std::move(incoming_messages_.front());
    incoming_messages_.pop();
    
    // Handle based on signal type
    if (auto* apoptosis = dynamic_cast<ApoptosisSignal*>(signal.get())) {
      attemptApoptosis();  // May throw CellDeathException
    }
    // Handle other signal types as needed
  }
}
```

### State Changes
- ✅ `age++` (if cell survives)
- ✅ `alive = false` (if apoptosis succeeds)

### Decision Points

**Decision 1: Should I undergo apoptosis?**

**Trigger**: ApoptosisSignal received in queue

**Precondition**:
```
Cell must be alive
AND genomic_instability <= apoptosis_instability_threshold
```

**Logic**:
```cpp
void attemptApoptosis() {
  if (genomic_instability > apoptosis_instability_threshold_) {
    // Apoptosis BLOCKED - cell is too unstable to be killed
    logger_->logCell("Apoptosis blocked: instability too high");
    return;  // Cell survives
  }
  // Apoptosis ALLOWED - cell dies
  alive_ = false;
  throw CellDeathException("apoptosis@phase2");
}
```

**Valid Outcomes**:
- ✅ `undergo_apoptosis` → Cell dies → Exception thrown
- ✅ `survive_apoptosis` → Cell continues (genomic_instability too high)

**Biological Meaning**:
- Low instability cells: Apoptosis works (immune system can kill them)
- High instability cells: Apoptosis fails (tumors escape immune response)

### Lifecycle Increment
- ✅ `increaseAge()`: Age increments if cell survives phase 2

### Exception Handling
- ✅ Throws `CellDeathException` if apoptosis succeeds
- ✅ Caught by `live()` caller (Tissue)
- ✅ No exception if apoptosis fails (cell survives)

### When to Use
- Every normal cell cycle (phase 2 always executes)
- Neoplastic cells also execute phase 2 (to receive apoptosis)

### Example Scenario

```
Scenario: Cell receives apoptosis signal
├─ Cell state: alive=true, genomic_instability=1.2
├─ Threshold: apoptosis_instability_threshold=10.0
├─ Precondition met: 1.2 <= 10.0? YES
├─ Action: Apoptosis allowed → alive=false
└─ Result: CellDeathException thrown, cell removed

Alternative: High instability cell
├─ Cell state: alive=true, genomic_instability=15.0
├─ Threshold: apoptosis_instability_threshold=10.0
├─ Precondition NOT met: 15.0 <= 10.0? NO
├─ Action: Apoptosis BLOCKED
└─ Result: Cell survives, continues to phase 3
```

---

## Phase 3: Nuclear Dynamics (Genome Mutation)

### Purpose
Execute genome-level mutations. Each gene has chance to mutate based on noise + threshold.

### Type
**Mutation phase**

### Inputs
- Genome: Gene collection with state
- Noise: Random number generator (INoiseSource)
- Genomic instability: Multiplier for mutation probability
- TP53 status: Determines if genome is unstable

### Operations
```cpp
void phase3_NuclearDynamics() {
  // Advance all genes with genomic instability factor
  genome_.liveAllGenes(genomic_instability_);
}

// Inside Genome::liveAllGenes()
void Genome::liveAllGenes(double genomic_instability) {
  bool unstable = isUnstable();  // Check TP53 status
  for (auto& kv : genes_) {
    // Each gene samples noise and decides to mutate
    kv.second.live(unstable, genomic_instability);
  }
}

// Inside Gene::live()
void Gene::live(bool apply_instability, double genomic_instability) {
  double threshold = calculateMutationThreshold(apply_instability, genomic_instability);
  double sample = noise_->next().u01;
  
  if (sample < threshold) {
    mutate();  // Transition state: +/+ → +/- or +/- → -/-
  }
}
```

### State Changes
- ✅ Gene states may transition (mutations)
- ✅ Genome.isUnstable() may change (if TP53 mutates)

### Decision Points

**Decision: Should each gene mutate?**

**Calculation**:
```
For each gene:
1. Base threshold = gene.mutation_threshold (e.g., 0.01)
2. If genome.isUnstable() (TP53 is +/- or -/-):
   └─ threshold += gene.mutation_instability_k (e.g., +0.0001)
3. Multiply by genomic instability factor:
   └─ threshold *= genomic_instability (e.g., ×1.0 or ×2.5)
4. Sample u01 from noise [0, 1]
5. If u01 < threshold:
   └─ Gene mutates
   Else:
   └─ Gene remains unchanged
```

**Example 1: Normal cell (TP53 +/+, instability 1.0)**
```
Gene TP53:
├─ base threshold = 0.01
├─ apply_instability? NO (TP53 is +/+)
├─ effective threshold = 0.01 × 1.0 = 0.01 (1%)
├─ u01 sample = 0.015
└─ Result: 0.015 >= 0.01 → NO mutation

Gene BRCA1:
├─ base threshold = 0.005
├─ apply_instability? NO (genome stable)
├─ effective threshold = 0.005 × 1.0 = 0.005 (0.5%)
├─ u01 sample = 0.003
└─ Result: 0.003 < 0.005 → MUTATES +/+ → +/-
```

**Example 2: Unstable cell (TP53 +/-, instability 2.0)**
```
Gene TP53:
├─ base threshold = 0.01
├─ apply_instability? YES (TP53 is +/-)
├─ threshold += 0.0001 = 0.0101
├─ effective threshold = 0.0101 × 2.0 = 0.0202 (2%)
├─ u01 sample = 0.015
└─ Result: 0.015 < 0.0202 → MUTATES +/- → -/-

BRCA1:
├─ base threshold = 0.005
├─ apply_instability? YES
├─ threshold += 0.0001 = 0.0051
├─ effective threshold = 0.0051 × 2.0 = 0.0102 (1%)
├─ u01 sample = 0.008
└─ Result: 0.008 < 0.0102 → MUTATES
```

### Biological Meaning
- **Stable genome** (TP53 +/+): Low, baseline mutation rates
- **Unstable genome** (TP53 +/-, -/-): High mutation rates (genomic instability)
- **Cascade effect**: If TP53 mutates during this phase, next phase sees higher instability

### Exception Handling
- ✅ Cannot throw exceptions from this phase

### When to Use
- Every normal cell cycle (always phase 3)
- Neoplastic cells SKIP this phase (lifecycle is phase2 → phase4 → phase5)

### When NOT to Use
- Neoplastic cells (they skip growth phases)

---

## Phase 4: Cytoplasmic Remodeling (S/G2/M Decisions)

### Purpose
Make major cell fate decisions: Neoplasia? Division? Update instability.

### Type
**Major decision phase**

### Inputs
- TP53 status: Determines if neoplasia possible
- Noise: Random samples for neoplasia and division
- Division rate(s): Normal vs neoplastic
- Genomic instability: Used in calculations

### Operations
```cpp
void phase4_CytoplasmicRemodeling() {
  // 1. Check for neoplastic transformation
  develop_neoplasm();  // May throw NeoplasticException
  
  // 2. Attempt cell division
  attemptDivision();  // May emit CellDivisionSignal
  
  // 3. Update genomic instability based on genome status
  adjust_neoplasm_k();  // Update future neoplasm probability
}
```

### State Changes
- ✅ `is_neoplastic = true` (if neoplasia triggered)
- ✅ `genomic_instability` updated (based on TP53 status)
- ✅ New daughter cell created (if division triggered)

### Decision Points

**Decision 1: Should I become neoplastic?**

**Trigger**: Phase 4 execution

**Preconditions**:
```
Cell must not already be neoplastic (is_neoplastic == false)
AND Cell must be alive (alive == true)
```

**Constraints** (from agent_roles.md):
```
TP53 +/+ (wild-type)     → Neoplasia IMPOSSIBLE (protected)
TP53 +/- (heterozygous)  → Neoplasia IMPOSSIBLE (protected)
TP53 -/- (homozygous)    → Neoplasia POSSIBLE (if threshold met)
```

**Logic**:
```cpp
void AgenticCell::develop_neoplasm() {
  if (is_neoplastic_ || !alive()) return;  // Already neoplastic or dead
  
  // Check TP53 protection
  if (!isNeoplasticProtected()) {
    // TP53 is -/-, can proceed
    double threshold = neoplasm_k_.value() * genomic_instability_;
    double sample = noise_->next().u01;
    
    if (sample < threshold) {
      is_neoplastic_ = true;
      throw NeoplasticException("neoplastic@phase4");
    }
  }
}

bool AgenticCell::isNeoplasticProtected() const {
  std::string tp53 = getTP53();
  // Protected if TP53 is +/+ or +/-
  // Only -/- allows neoplasia
  return (tp53 != "-/-");
}
```

**Calculation**:
```
threshold = neoplasm_k * genomic_instability
Sample u01 from noise
If u01 < threshold:
  └─ Cell becomes neoplastic
Else:
  └─ Cell remains normal
```

**Example 1: Normal cell (TP53 +/+)**
```
TP53 status: +/+
isNeoplasticProtected(): true
Result: Neoplasia check SKIPPED
Cell remains normal (even if threshold would be met)
```

**Example 2: Unstable but protected (TP53 +/-)**
```
TP53 status: +/-
isNeoplasticProtected(): true
Result: Neoplasia check SKIPPED
Cell remains normal (protected despite instability)
```

**Example 3: Vulnerable cell (TP53 -/-, instability 2.0)**
```
TP53 status: -/-
isNeoplasticProtected(): false
neoplasm_k = 0.002
genomic_instability = 2.0
threshold = 0.002 × 2.0 = 0.004 (0.4%)
u01 sample = 0.003
Result: 0.003 < 0.004 → Cell becomes NEOPLASTIC
```

**Valid Outcomes**:
- ✅ `become_neoplastic` (if TP53 -/- and threshold met)
- ✅ `remain_normal` (if TP53 +/+, +/-, or threshold not met)

---

**Decision 2: Should I divide?**

**Trigger**: After neoplasia check (if cell still alive)

**Preconditions**:
```
Cell must be alive
Cell must not be too early in lifecycle (optional check)
```

**Logic**:
```cpp
void AgenticCell::attemptDivision() {
  if (!alive()) return;
  
  double rate = is_neoplastic_ ? neoplastic_division_rate_ : division_rate_;
  double sample = noise_->next().u01;
  
  if (sample < rate) {
    auto daughter = clone();
    auto signal = std::make_unique<CellDivisionSignal>(
      cell_id_, "division", std::move(daughter)
    );
    signal_emitter_(std::move(signal));
  }
}
```

**Calculation**:
```
If neoplastic:
  └─ rate = neoplastic_division_rate (e.g., 0.01 = 1%)
Else:
  └─ rate = division_rate (e.g., 0.001 = 0.1%)

Sample u01 from noise
If u01 < rate:
  └─ Create daughter cell (clone)
  └─ Emit CellDivisionSignal
Else:
  └─ Do not divide
```

**Example**:
```
Normal cell (not neoplastic):
├─ division_rate = 0.001
├─ u01 sample = 0.0005
├─ Result: 0.0005 < 0.001 → DIVIDE
└─ Daughter cell created and signaled

Neoplastic cell (Big Bang mode):
├─ neoplastic_division_rate = 0.01
├─ u01 sample = 0.007
├─ Result: 0.007 < 0.01 → DIVIDE
└─ Result: Much higher division rate (10× baseline)
```

**Valid Outcomes**:
- ✅ `divide_and_emit` (if u01 < division_rate)
- ✅ `do_not_divide` (if u01 >= division_rate)

---

**Decision 3: Update genomic instability**

**Trigger**: End of phase 4

**Logic**:
```cpp
void AgenticCell::adjust_neoplasm_k() {
  if (genome_.isUnstable()) {
    // TP53 is +/- or -/-
    genomic_instability_ = base_neoplasm_k_ + high_delta_instability_;
  } else {
    // TP53 is +/+
    genomic_instability_ = base_neoplasm_k_ + low_delta_instability_;
  }
}
```

**Calculation**:
```
If genome.isUnstable() (TP53 +/- or -/-):
  └─ genomic_instability = base + high_delta (e.g., 0 + 0.2 = 0.2)
Else:
  └─ genomic_instability = base + low_delta (e.g., 0 + 0.01 = 0.01)
```

**Biological Meaning**:
- Unstable genomes accumulate mutations faster
- This factor is used in phase 3 (next cycle) for mutation probabilities

### Lifecycle Rules

**For Normal Cells**:
1. Execute all 3 decisions (neoplasia, division, instability)
2. Any neoplasia → throw NeoplasticException → stop

**For Neoplastic Cells** (if Big Bang mode):
1. Skip neoplasia check (already neoplastic)
2. Execute division with neoplastic_division_rate
3. Continue to phase 5

**For Neoplastic Cells** (if NOT Big Bang mode):
1. Skip entire phase 4
2. Go directly from phase 2 to phase 5

### Exception Handling
- ✅ Throws `NeoplasticException` if neoplasia triggered
- ✅ Caught by `live()` caller (Tissue)
- ✅ No exception for division (signals handled in phase 5)

### When to Use
- Every normal cell (phase 4 always executes)
- Neoplastic cells in Big Bang mode (execute division)
- Neoplastic cells NOT in Big Bang mode SKIP this phase

---

## Phase 5: Exocytosis (Signal Emission)

### Purpose
Emit all accumulated signals to the tissue/external agents.

### Type
**Output/Communication phase**

### Inputs
- Pending signals (neoplasm, division, others)
- Signal emitter callback: `signal_emitter_(signal)`

### Operations
```cpp
void phase5_Exocytosis() {
  // Signals are emitted during phase4, but can be finalized here
  // In current implementation, signals are emitted immediately
  // This phase is reserved for future signal batching/validation
}
```

### State Changes
- ✅ NONE (signals handled via callback)

### Decision Points
- None (signals already decided in earlier phases)

### Exception Handling
- ✅ Cannot throw exceptions
- ✅ Signal emission is async (via callback)

### When to Use
- Every cell (executes for normal and neoplastic)
- Reserve for future signal management

---

## Lifecycle Summary Table

| Phase | Purpose | Neoplastic? | Exception? | Next |
|-------|---------|------------|-----------|------|
| 0 | Baseline | YES | NO | 1 |
| 1 | G1 Checkpoint | NO* | YES | 2 |
| 2 | Signal Processing | YES | YES | 3/2 |
| 3 | Genome Mutation | NO | NO | 4 |
| 4 | Major Decisions | YES (if BB) | YES | 5 |
| 5 | Signal Emission | YES | NO | DONE |

*Neoplastic cells skip phase 1 in normal (non-BB) mode

---

## Decision Tree Diagram

```
┌─ PHASE 0: Baseline ─────────┐
│  Log state                   │
└──────────────────┬──────────┘
                   │
┌─ PHASE 1: G1 Checkpoint ───┐
│  if NOT neoplastic:        │
│    if dead? → throw Death   │
└──────────────────┬──────────┘
                   │
┌─ PHASE 2: Endocytosis ─────┐
│  Process signals           │
│  if Apoptosis signal:      │
│    if instability OK?      │
│      → throw Death         │
│  age++                     │
└──────────────────┬──────────┘
                   │
┌─ PHASE 3: Nuclear Dynamics ┐
│  Mutate genes              │
│  (neoplastic skip this)    │
└──────────────────┬──────────┘
                   │
┌─ PHASE 4: Cytoplasmic Rem.┐
│  if NOT neoplastic:        │
│    Neoplasia check?        │
│    → if YES: throw Neo     │
│  Division check?           │
│  Update instability        │
└──────────────────┬──────────┘
                   │
┌─ PHASE 5: Exocytosis ──────┐
│  Emit signals              │
│  (already done via emitter)│
└──────────────────┬──────────┘
                   │
              DONE / EXCEPTION
```

---

## Related Files

- 📄 `.github/copilot/agent_context_structure.md` - Context layers
- 📄 `.github/copilot/agent_roles.md` - AgenticCell role details
- 📁 `.github/copilot/context_examples/` - Specific scenario examples
- 📊 `src/domain/cell/AgenticCell.h` - Header with phase signatures


