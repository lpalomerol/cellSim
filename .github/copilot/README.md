# GitHub Copilot Context Engineering - cellSim

## 📚 Documentación Disponible

### Contexto Biológico
- **[CELL_STATE_MACHINE.md](./CELL_STATE_MACHINE.md)** ⭐ **COMIENZA AQUÍ**
  - 🔄 Máquina de estados celular completa
  - 🧬 Atributos genéticos internos (BRCA1, TP53, D1, D2)
  - 🎭 5 estadios: Baseline → Intermedio → Primer → Tumoral
  - ⚡ Matriz de decisión por estado mutacional
  - ✅ Checklist para validar implementaciones

### Arquitectura del Agente
- **[agent_context_structure.md](./agent_context_structure.md)**
  - Las 4 capas de contexto: Identity, Constraint, State, Decision
  - Estructura YAML de cada agente
  - Restricciones inviolables

### Referencias Visuales
- **[VISUAL_REFERENCE.md](./VISUAL_REFERENCE.md)**
  - Jerarquía de agentes (Simulation → Tissue → Cell → Genome → Gene)
  - Diagrama visual de 4 capas de contexto

- **[agent_roles.md](./agent_roles.md)**
  - Responsabilidades de cada tipo de agente

- **[cell_lifecycle_templates.md](./cell_lifecycle_templates.md)**
  - Templates de ciclo de vida celular

---

## 🚀 Uso Rápido

### Opción 1: Automatizado con run_profile.sh ⭐ RECOMENDADO
```bash
bash .github/scripts/run_profile.sh cpp "tarea específica" "requisitos" "inputs"
```
👉 **Lee:** [RUN_PROFILE_GUIDE.md](./RUN_PROFILE_GUIDE.md) para entender cómo funciona y ejemplos

### Opción 2: Consultar contexto manualmente
1. Lee **CELL_STATE_MACHINE.md** para entender el modelo biológico
2. Lee **agent_context_structure.md** para entender restricciones
3. Consulta **VISUAL_REFERENCE.md** para jerarquía de agentes
4. Copia el contexto relevante a tu prompt de Copilot

---

## 🛠️ Herramientas

- **[RUN_PROFILE_GUIDE.md](./RUN_PROFILE_GUIDE.md)** - Guía completa del generador de prompts
  - ¿Qué es `run_profile.sh`?
  - Cómo usarlo con ejemplos
  - Flujo completo de tarea → código
  - Troubleshooting

- **[automation_profile.tpl](./automation_profile.tpl)** - Plantilla de reglas Copilot
  - Edita aquí para ajustar reglas comunes del proyecto

---

## 📌 Notas

- Versiona esta carpeta para compartir contexto entre el equipo
- Mantén reglas concisas y estables
- Documenta cambios importantes en este README
- Consulta **docs/diagrams_luis/** para ver diagramas originales

