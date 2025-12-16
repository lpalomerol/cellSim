# Agent Context Structure - cellSim

## Definición Universal de Contexto de Agente

Todos los agentes en cellSim comunican su estado mediante un contexto estructurado en **4 capas**.

---

## Las 4 Capas del Contexto

### 1️⃣ IDENTITY LAYER
**Qué es el agente y para qué existe**

```yaml
identity:
  agent_type: "AgenticCell | Genome | Gene | Tissue"
  role: "Descripción textual del rol"
  version: "Información de compatibilidad"
  responsibilities:
    - Responsabilidad 1
    - Responsabilidad 2
    - ...
```

**Ejemplo - AgenticCell**:
```yaml
identity:
  agent_type: AgenticCell
  role: "Central decision-making biological agent"
  responsibilities:
    - Execute 6-phase cell lifecycle
    - Detect neoplastic transformation
    - Respond to external signals (apoptosis, nutrition)
    - Emit signals (division, neoplasm detection)
    - Manage internal genome state
```

---

### 2️⃣ CONSTRAINT LAYER
**Restricciones inviolables que Copilot DEBE respetar**

```yaml
constraints:
  biological:
    - name: "TP53 Protection Rule"
      description: "TP53 +/+ protects from neoplasia; +/-, -/- allow it"
      enforced_in: ["develop_neoplasm()"]
    - name: "Apoptosis Threshold"
      description: "Apoptosis only works if genomic_instability ≤ apoptosis_instability_threshold"
      enforced_in: ["attemptApoptosis()"]
    - name: "Gene Mutation Unidirectional"
      description: "Gene mutations: +/+ → +/- → -/-, never reverse"
      enforced_in: ["Gene::mutate()"]
  
  architectural:
    - name: "Immutable After Construction"
      description: "Cells cannot change fundamental parameters after creation"
    - name: "Signal-Based Communication"
      description: "Cells don't directly modify tissue; they emit signals"
    - name: "Unique Cell IDs"
      description: "Each cell must have a unique, stable ID"
```

---

### 3️⃣ STATE LAYER
**Estado actual cuantificable del agente**

```yaml
state:
  # For AgenticCell
  cellular:
    id: "uint64_t"
    age: "uint64_t (in ticks)"
    alive: "bool"
    neoplastic: "bool"
    genomic_instability: "double (1.0 = baseline)"
    evaded_apoptosis: "bool"
  
  # Genome nested state
  genome:
    genes:
      TP53: "+/+ | +/- | -/-"
      BRCA1: "+/+ | +/- | -/-"
    is_unstable: "bool (derived from TP53 status)"
  
  # Thresholds
  parameters:
    neoplasm_k: "Threshold for neoplastic transformation"
    division_rate: "Probability of cell division"
    mutation_instability_k: "Additive factor when TP53 unstable"
    apoptosis_instability_threshold: "Max instability for apoptosis to work"
  
  # Message queue
  signals:
    incoming: "Queue<ISignal>"
    can_emit: "bool (via signal_emitter callback)"
```

---

### 4️⃣ DECISION CONTEXT LAYER
**Qué debe decidir el agente AHORA**

```yaml
decision_context:
  lifecycle:
    current_phase: "phase0|phase1|phase2|phase3|phase4|phase5"
    phase_purpose: "Texto descriptivo de qué hacer en esta fase"
  
  decision_points:
    - name: "Genome Mutation"
      trigger: "Each phase when live() is called"
      factors:
        - noise_sample: "random u01 [0,1]"
        - mutation_threshold: "depends on instability"
        - genomic_instability: "TP53 status multiplier"
      valid_outcomes: ["mutate | do_not_mutate"]
    
    - name: "Neoplastic Transformation"
      trigger: "phase4_CytoplasmicRemodeling"
      preconditions:
        - "TP53 must be -/- (constraint!)"
        - "Cell must be alive"
        - "Cell not already neoplastic"
      factors:
        - neoplasm_k: "base threshold"
        - genomic_instability: "adjustment factor"
      valid_outcomes: ["become_neoplastic | remain_normal"]
    
    - name: "Cell Division"
      trigger: "phase4_CytoplasmicRemodeling"
      preconditions:
        - "Cell must be alive"
      factors:
        - division_rate: "base probability"
        - neoplastic_division_rate: "if neoplastic + big_bang_mode"
      valid_outcomes: ["divide_and_emit_signal | do_not_divide"]
    
    - name: "Apoptosis Response"
      trigger: "When ApoptosisSignal is received"
      preconditions:
        - "Cell must be alive"
        - "genomic_instability ≤ apoptosis_instability_threshold"
      factors:
        - signal_type: "intrinsic | extrinsic"
        - TP53_status: "affects vulnerability"
      valid_outcomes: ["undergo_apoptosis | survive"]
```

---

## Validación de Contexto

Un contexto es **válido** si cumple:

```
✅ IDENTITY: Es claro QUÉ es el agente
✅ CONSTRAINT: Todas las restricciones están listadas
✅ STATE: Todo campo de estado tiene tipo y rango definido
✅ DECISION: Cada decision point tiene precondiciones, factores y outcomes
✅ COHERENCIA: No hay conflictos entre capas
✅ COMPLETITUD: No hay decisiones sin justificación en capas anteriores
```

---

## Patrón de Uso en Prompts a Copilot

### Estructura de Prompt Mejorada

```
CONTEXTO:
[Copia la sección relevante de IDENTITY + CONSTRAINT + STATE + DECISION]

TAREA:
Implementa [función] que:
1. Cumple restricciones: [list from CONSTRAINT LAYER]
2. Accede a estado: [list from STATE LAYER]
3. Implementa decision: [from DECISION CONTEXT LAYER]
4. Emite outcomes válidos: [from valid_outcomes]

VALIDACIÓN:
El código debe pasar estos tests:
- [Test 1: precondition A]
- [Test 2: outcome B in valid_outcomes]
- [Test 3: constraint C respected]
```

---

## Archivos Relacionados

- 📄 `agent_roles.md` - Describe cada tipo de agente
- 📄 `cell_lifecycle_templates.md` - Templates por fase del ciclo
- 📁 `context_examples/` - Ejemplos serializados (JSON)
- 📋 `.github/copilot/automation_profile.tpl` - Perfil de automatización general

---

## Evolución del Contexto

**Versión 1.0** (actual):
- 4 capas definidas
- Soporta AgenticCell, Genome, Gene, Tissue
- Compatible con C++17

**Futuro**:
- Versión 2.0: Agregar capas de "Performance" (optimization constraints)
- Serialización automática de contextos a JSON desde C++
- Validación en tiempo de compilación


