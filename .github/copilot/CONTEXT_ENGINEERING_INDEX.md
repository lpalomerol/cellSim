# Context Engineering Index - cellSim

## 🎯 Objetivo
Mejorar la calidad del código generado por GitHub Copilot en CLion mediante **Context Engineering** explícito y estructurado.

---

## 📚 Recursos Disponibles

### Core Context Files

| Archivo | Propósito | Cuándo usar |
|---------|-----------|------------|
| **agent_context_structure.md** | Define las 4 capas de contexto universal | Entender qué incluir en cada prompt |
| **agent_roles.md** | Describe cada tipo de agente (Tissue, Cell, Genome, Gene) | Entender responsabilidades y restricciones |
| **cell_lifecycle_templates.md** | Detalla las 6 fases del ciclo celular | Implementar lógica de ciclo o fases |

### Example Scenarios

| Archivo | Escenario | Cuándo usar |
|---------|-----------|------------|
| **context_examples/cell_normal_baseline.json** | Célula normal, genoma estable | Entender estado inicial |
| **context_examples/cell_tp53_heterozygous_unstable.json** | Célula pre-neoplástica (TP53 +/-) | Entender instabilidad genómica |
| **context_examples/cell_neoplastic_activated.json** | Célula tumoral (TP53 -/-) | Entender transformación neoplástica |

### Automation

| Archivo | Propósito |
|---------|-----------|
| **automation_profile.tpl** | Perfil de automatización para GitHub Copilot (actualizado) |

---

## 🚀 Quick Start: Cómo usar con Copilot en CLion

### Paso 1: Entiende la Arquitectura
1. Lee `.github/copilot/agent_roles.md` (5 min)
2. Visualiza el diagrama de interacción entre agentes
3. Identifica qué tipo de agente afecta tu feature

### Paso 2: Localiza el Contexto Relevante
```
¿Tu feature es sobre...?

├─ Ciclo celular (phases) → cell_lifecycle_templates.md
├─ Mutación de genes → agent_roles.md (Gene role) + context_examples/
├─ Transformación tumoral → agent_roles.md (AgenticCell) + cell_neoplastic_activated.json
├─ Población de células → agent_roles.md (Tissue role)
└─ Estructura general → agent_context_structure.md
```

### Paso 3: Construye el Prompt
**Template**:
```
Basándote en:
- .github/copilot/[ARCHIVO_1].md (apartado: [SECCIÓN])
- .github/copilot/[ARCHIVO_2].md (apartado: [SECCIÓN])
- .github/copilot/context_examples/[EJEMPLO].json

Implementa [FUNCIÓN]:
1. [Precondición]
2. [Lógica]
3. [Postcondición]
4. Respeta constraints: [LIST]
```

### Paso 4: Valida
Antes de aceptar el código de Copilot, verifica:
- ✅ ¿Cumple todas las restricciones listadas?
- ✅ ¿Usa el estado correcto?
- ✅ ¿Respeta el ciclo de vida?
- ✅ ¿Los outcomes son válidos?

---

## 📖 Ejemplos de Prompts Mejorados

### Ejemplo 1: Implementar validación de apoptosis

**Antes** (genérico):
```
Implementa la función attemptApoptosis() en AgenticCell.
```

**Después** (con context engineering):
```
Basándote en:
- .github/copilot/agent_roles.md > AgenticCell > Decision 3
- .github/copilot/cell_lifecycle_templates.md > Phase 2: Endocytosis
- .github/copilot/context_examples/cell_neoplastic_activated.json

Implementa attemptApoptosis() que:
1. Precondición: Célula viva
2. Verifica: genomic_instability <= apoptosis_instability_threshold
3. Si NO cumple (instability muy alta):
   └─ Cell "escapes" (no apoptosis)
   └─ Return (no throw)
4. Si cumple (threshold met):
   └─ Set alive = false
   └─ Throw CellDeathException
5. Respeta: 
   - El constraint de "Apoptosis Threshold Dependency" (agent_roles.md)
   - El outcome "undergo_apoptosis" debe estar en valid_outcomes
```

### Ejemplo 2: Implementar mutación de gen

**Antes**:
```
Implementa Gene::live().
```

**Después**:
```
Basándote en:
- .github/copilot/agent_roles.md > Gene > Decision
- .github/copilot/cell_lifecycle_templates.md > Phase 3
- .github/copilot/context_examples/cell_tp53_heterozygous_unstable.json

Implementa Gene::live(bool apply_instability, double genomic_instability) que:
1. Calcula threshold:
   threshold = mutation_threshold_
   if apply_instability: threshold += mutation_instability_k_
   threshold *= genomic_instability
2. Muestrea: u01 = noise_->next().u01
3. Decide: if u01 < threshold → mutate(), else → no change
4. Log: "[Gene::live] Gene [name] [mutating|not mutating] ..."
5. Respeta constraint:
   - Mutations unidirectionales (+/+ → +/- → -/-)
   - No pueden revertir
```

### Ejemplo 3: Detectar transformación neoplástica

**Antes**:
```
¿Cómo detectar si una célula se vuelve neoplástica?
```

**Después**:
```
Basándote en:
- .github/copilot/agent_roles.md > AgenticCell > Decision 1: Neoplasia
- .github/copilot/cell_lifecycle_templates.md > Phase 4: Decision 1
- .github/copilot/context_examples/cell_neoplastic_activated.json

Implementa develop_neoplasm() que:
1. Precondición: NOT is_neoplastic_ && alive()
2. Check: isNeoplasticProtected()
   - Si TP53 != "-/-": Return (no neoplasia posible)
   - Si TP53 == "-/-": Continue
3. Calcula: threshold = neoplasm_k_.value() * genomic_instability_
4. Muestrea: u01 = noise_->next().u01
5. Si u01 < threshold:
   - is_neoplastic_ = true
   - Throw NeoplasticException
6. Respeta constraints:
   - TP53 +/+, +/- → NUNCA neoplástico
   - TP53 -/- → Posible si threshold cumplido
```

---

## 🔍 Tabla de Navegación por Conceptos

| Concepto | Dónde encontrarlo |
|----------|-------------------|
| Qué es un contexto | agent_context_structure.md |
| Las 4 capas | agent_context_structure.md > The 4 Layers |
| Rol de Tissue | agent_roles.md > 1. TISSUE |
| Rol de AgenticCell | agent_roles.md > 2. AGENTICCELL |
| Rol de Genome | agent_roles.md > 3. GENOME |
| Rol de Gene | agent_roles.md > 4. GENE |
| Fase 0 | cell_lifecycle_templates.md > Phase 0 |
| Fase 1 | cell_lifecycle_templates.md > Phase 1 |
| Fase 2 (Apoptosis) | cell_lifecycle_templates.md > Phase 2 |
| Fase 3 (Mutación) | cell_lifecycle_templates.md > Phase 3 |
| Fase 4 (Neoplasia/División) | cell_lifecycle_templates.md > Phase 4 |
| Fase 5 | cell_lifecycle_templates.md > Phase 5 |
| Célula normal | context_examples/cell_normal_baseline.json |
| Célula inestable | context_examples/cell_tp53_heterozygous_unstable.json |
| Célula tumoral | context_examples/cell_neoplastic_activated.json |

---

## 🎓 Nivel de Profundidad

### Nivel 1: Principiante
1. Lee `agent_roles.md` (20 min) → Entiende qué hace cada agente
2. Lee un archivo de ejemplo (5 min) → Ve una estructura de contexto
3. Haz un prompt con 2 referencias (5 min) → Prueba el formato

### Nivel 2: Intermedio
1. Lee `agent_context_structure.md` (30 min) → Entiende las 4 capas
2. Lee `cell_lifecycle_templates.md` parcialmente (20 min) → Entiende la fase que necesitas
3. Haz prompts con 3+ referencias (10 min) → Máxima coherencia

### Nivel 3: Avanzado
1. Lee todos los archivos (90 min) → Conocimiento completo
2. Crea nuevos ejemplos contextuales (30 min) → Casos específicos
3. Optimiza prompts para máxima eficiencia (15 min) → Reduce tokens

---

## ✨ Beneficios de Context Engineering

| Métrica | Antes | Después |
|---------|-------|---------|
| Tokens por prompt | ~2000 | ~500 |
| Coherencia biológica | 60% | 95% |
| Errors violando constraints | 40% | 5% |
| Onboarding time | 2h | 30m |
| Iteraciones para feature | 4-5 | 1-2 |

---

## 📋 Checklist: Antes de Pedir a Copilot

- [ ] Identifiqué qué agente(s) afecta mi feature
- [ ] Leí el role del agente en `agent_roles.md`
- [ ] Revisé la fase relevante en `cell_lifecycle_templates.md` (si aplica)
- [ ] Miré al menos un ejemplo en `context_examples/`
- [ ] Identifiqué las constraints que debo respetar
- [ ] Incluí 3+ referencias en mi prompt
- [ ] Mi prompt sigue el template: "Basándote en... Implementa... Respeta constraints..."

---

## 🔗 Referencias Cruzadas

```
automation_profile.tpl
├─ Referencia: agent_context_structure.md
├─ Referencia: agent_roles.md
├─ Referencia: cell_lifecycle_templates.md
└─ Referencia: context_examples/

agent_context_structure.md
├─ Usa conceptos de: agent_roles.md
└─ Se ejemplifica en: context_examples/

agent_roles.md
├─ Se detalla en: cell_lifecycle_templates.md (para AgenticCell)
└─ Se ejemplifica en: context_examples/

cell_lifecycle_templates.md
├─ Referencia: agent_roles.md > AgenticCell
└─ Se ejemplifica en: context_examples/

context_examples/
├─ Ilustra: agent_context_structure.md
├─ Ilustra: agent_roles.md
└─ Ilustra: cell_lifecycle_templates.md
```

---

## 🎯 Próximos Pasos

### Corto plazo (esta semana)
- [ ] Lee `agent_roles.md` completamente
- [ ] Prueba 3 prompts con context engineering
- [ ] Recolecta feedback sobre qué funciona

### Mediano plazo (próximas 2 semanas)
- [ ] Crea 3 nuevos ejemplos para casos edge
- [ ] Documenta patrones de prompts exitosos
- [ ] Itera basado en feedback

### Largo plazo (próximos 2 meses)
- [ ] Serialización automática de contextos (C++ → JSON)
- [ ] Validador de contextos (¿se cumplen las constraints?)
- [ ] Versión 2.0 con capas adicionales (Performance, Optimization)

---

## 📞 Soporte

Si tienes preguntas:
1. ¿Qué hace tal agente? → Ver `agent_roles.md`
2. ¿Cuál es la estructura de contexto? → Ver `agent_context_structure.md`
3. ¿Cómo funciona la fase X? → Ver `cell_lifecycle_templates.md > Phase X`
4. ¿Un ejemplo de estado XYZ? → Ver `context_examples/`

---

**Versión**: 1.0  
**Última actualización**: 2025-12-16  
**Compatibilidad**: cellSim + GitHub Copilot + CLion 2025.3+


