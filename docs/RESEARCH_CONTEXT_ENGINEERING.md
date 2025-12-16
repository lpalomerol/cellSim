# Análisis de Context Engineering - cellSim

## Fase 1: RESEARCH - Estado Actual

### Arquitectura de Agentes Biológicos

**Jerarquía de Entidades:**
```
Tissue (contenedor)
  └── AgenticCell (agente principal)
       ├── Genome (información genética)
       │    └── Gene[] (TP53, BRCA1, etc.)
       └── INoiseSource (generador de variabilidad)
```

### Estructura Actual de Información

#### 1. **AgenticCell (núcleo del agente)**
- **Estado**: Vivo/Muerto, Neoplástico, Edad, ID
- **Ciclo Celular**: 6 fases (Baseline, G1, Endocitosis, Dinámicas Nucleares, Remodelación Citoplasmática, Exocitosis)
- **Genoma**: Mapa de genes con estado mutacional
- **Señales**: Emite/recibe mensajes (Neoplasma, Apoptosis, División)
- **Parámetros**: Umbrales de instabilidad, tasas de división

#### 2. **Genome (perfil genético)**
- **Genes**: Mapa {nombre → Gene}
- **Estado**: Estable (+/+) o Inestable (+/-, -/-)
- **Mutabilidad**: Cada gen tiene umbral de mutación

#### 3. **Gene (unidad hereditaria)**
- **Estados**: +/+ (wild-type), +/- (heterozigoto), -/- (homozigoto)
- **Mutación**: Progresa secuencialmente
- **Probabilidad**: Calculada con factor de instabilidad genómica

#### 4. **Tissue (orquestador)**
- **Colección**: Vector de células
- **Ciclo**: Description → Signal Integration → Execute Cell Cycles
- **Tracking**: Identifica neoplasmas activos

---

## Problemas Identificados en el Contexto Actual

### 🔴 **Falta de Estructura de Contexto Explícita**
- No hay archivo que defina "qué es un contexto de agente"
- El `automation_profile.tpl` es demasiado genérico
- Copilot no tiene referencia clara de cómo modelar el estado celular

### 🔴 **Información Dispersa**
- El estado biológico está esparcido en múltiples clases
- No hay "snapshot" serializado del estado celular
- Copilot debe inferir la lógica desde múltiples archivos

### 🔴 **Prompts Implícitos**
- Cuando pides una feature (ej: "implementa apoptosis"), Copilot:
  - No sabe qué información considera un "contexto completo"
  - No conoce las restricciones biológicas implícitas
  - Genera código sin validar coherencia con el dominio

### 🔴 **Sin Templates Reutilizables**
- Cada nueva feature requiere repetir el contexto biológico
- No hay ejemplos de "cómo se estructura una decisión celular"
- Ineficiencia en prompting

### 🔴 **Sin Ejemplos Claros**
- Copilot no ve ejemplos de "contexto bien formado"
- Las fases del ciclo celular no tienen guía clara

---

## Propuesta de Mejora

### Crear 4 Nuevos Archivos en `.github/copilot/`

#### 1. **`agent_context_structure.md`**
Define la estructura JSON/conceptual del contexto que recibe todo agente:
```
{
  "agent_type": "cell|genome|gene|tissue",
  "biological_constraints": [...],
  "state_snapshot": {...},
  "lifecycle_phase": "...",
  "decision_context": {...}
}
```

#### 2. **`agent_roles.md`**
Describe cada tipo de agente, responsabilidades, inputs/outputs:
- **AgenticCell Role**: Decisor central del ciclo celular
- **Genome Role**: Gestor de mutaciones heredables
- **Gene Role**: Unidad de variabilidad genética
- **Tissue Role**: Orquestador de población

#### 3. **`cell_lifecycle_templates.md`**
Templates para cada fase del ciclo:
- Phase 0: Baseline Assessment
- Phase 1: G1 Checkpoint
- ... etc

#### 4. **`context_examples/`** (directorio)
- `context_normal_cell.json` - Ejemplo: célula normal
- `context_neoplastic_cell.json` - Ejemplo: célula tumoral
- `context_gene_mutation.json` - Ejemplo: gen mutando

---

## Beneficios Esperados

✅ **Copilot entiende qué incluir en un contexto**
✅ **Genera código más coherente con la biología**
✅ **Reduces tokens gastados (contexto reutilizable)**
✅ **Facilita onboarding de nuevas features**
✅ **Validación automática: "¿este código respeta las restricciones?"**

---

## Próximos Pasos

1. **PLAN** (este documento) → define estructura mejorada
2. **Crear archivos de contexto** (next iteration)
3. **Actualizar automation_profile.tpl** para referenciarlos
4. **Testear con casos reales** en CLion


