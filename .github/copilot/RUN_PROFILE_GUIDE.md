# 🔧 Guía: `run_profile.sh` - Generador Automático de Prompts

## ¿Qué es?

`run_profile.sh` es un **generador de prompts automatizado** que toma una plantilla (`.github/copilot/automation_profile.tpl`) y la rellena con parámetros específicos.

**Ubicación:** `.github/scripts/run_profile.sh`  
**Plantilla:** `.github/copilot/automation_profile.tpl`  
**Salida:** `/tmp/copilot_prompt.txt`

---

## 🎯 ¿Para qué sirve?

Cuando quieres que **Copilot** genere código para tu proyecto, en lugar de escribir manualmente un prompt largo cada vez, este script:

1. ✅ Carga la plantilla con las **reglas y restricciones** del proyecto
2. ✅ Rellena placeholders (`{{LANG}}`, `{{TASK}}`, etc.)
3. ✅ **Inyecta automáticamente** referencias a documentación de contexto
4. ✅ Genera un prompt **bien estructurado y consistente**
5. ✅ Guarda el resultado en `/tmp/copilot_prompt.txt` para que lo copies

---

## 📝 Cómo Usarlo

### Sintaxis básica

```bash
bash .github/scripts/run_profile.sh [LENGUAJE] [TAREA] [REQUISITOS] [INPUTS]
```

### Parámetros

| Parámetro | Valor por defecto | Ejemplo |
|-----------|-------------------|---------|
| **LENGUAJE** | `cpp` | `cpp`, `python`, `js`, etc. |
| **TAREA** | "Implementar la funcionalidad solicitada" | "Implementar Gene::live() con mutación" |
| **REQUISITOS** | "Seguir reglas del repo..." | "Validar mutaciones unidireccionales" |
| **INPUTS** | "ninguno" | "estado_actual: TP53 +/-, D1=2.5" |

---

## 🚀 Ejemplos de Uso

### Ejemplo 1: Generar prompt para implementar mutación genética

```bash
bash .github/scripts/run_profile.sh cpp \
  "Implementar Gene::live() para mutación genética" \
  "Respetar constraint: +/+ → +/- → -/-, nunca retroceder" \
  "Gene con TP53 actual +/+, noise source, threshold"
```

**Resultado:** Genera un prompt que incluye:
- ✅ Reglas del repositorio
- ✅ Referencias a `agent_roles.md` y `agent_context_structure.md`
- ✅ Ejemplos de contexto correcto vs incorrecto
- ✅ Tu tarea y requisitos específicos

### Ejemplo 2: Generar prompt para lógica de apoptosis

```bash
bash .github/scripts/run_profile.sh cpp \
  "Implementar AgenticCell::attemptApoptosis()" \
  "Solo funciona si instability ≤ apoptosis_threshold, respetar TP53 constraints" \
  "Cell actual: instability=0.8, TP53=-/-, age=100"
```

### Ejemplo 3: Solo usar valores por defecto

```bash
bash .github/scripts/run_profile.sh
```

---

## 📋 Flujo Completo: De la Tarea al Código

```
┌─────────────────────────────────────────────┐
│ 1. Tienes una tarea de desarrollo           │
│    "Implementar ciclo de división celular"  │
└────────────┬────────────────────────────────┘
             │
             ▼
┌─────────────────────────────────────────────┐
│ 2. Ejecutas run_profile.sh con parámetros   │
│    bash .github/scripts/run_profile.sh cpp  │
│      "Implementar CellDivision signal"      │
│      "Validar con tests"                    │
│      "Tissue context, division rate"        │
└────────────┬────────────────────────────────┘
             │
             ▼
┌─────────────────────────────────────────────┐
│ 3. El script genera prompt completo         │
│    - Inyecta reglas del proyecto            │
│    - Inyecta referencias a documentación    │
│    - Combina con tu tarea específica        │
│    - Guarda en /tmp/copilot_prompt.txt      │
└────────────┬────────────────────────────────┘
             │
             ▼
┌─────────────────────────────────────────────┐
│ 4. El script imprime el output (cat)        │
│    Copias el contenido que ves en terminal  │
└────────────┬────────────────────────────────┘
             │
             ▼
┌─────────────────────────────────────────────┐
│ 5. Pegas el prompt en Copilot Chat (CLion)  │
│    - Copilot lo procesa con todo el contexto│
│    - Genera código coherente con proyecto   │
└────────────┬────────────────────────────────┘
             │
             ▼
┌─────────────────────────────────────────────┐
│ 6. Copilot entrega código listo para usar   │
│    - Respeta convenciones                   │
│    - Respeta constraints biológicos         │
│    - Incluye tests si es apropiado          │
└─────────────────────────────────────────────┘
```

---

## 🔍 ¿Qué hay dentro de la plantilla?

El archivo `.github/copilot/automation_profile.tpl` contiene:

```yaml
Reglas del repositorio:
  ✅ Orientación a Agentes
  ✅ Contexto biológico complejo
  ✅ Pruebas unitarias
  ✅ Convenciones de código
  ✅ Manejo de errores
  ✅ Sin credenciales
  ✅ Cambios mínimos

Recursos disponibles (inyectados automáticamente):
  1. agent_context_structure.md    (4 capas: Identity, Constraint, State, Decision)
  2. agent_roles.md                (descripciones de agentes)
  3. cell_lifecycle_templates.md    (6 fases del ciclo celular)
  4. context_examples/             (ejemplos reales JSON)

Ejemplos de prompts correctos vs incorrectos
```

---

## 💡 Consejos de Uso

### ✅ Haz esto

```bash
# Tarea específica con contexto claro
bash .github/scripts/run_profile.sh cpp \
  "Implementar Tissue::detectNeoplasticCells()" \
  "Iterar cells, verificar neoplastic flag, retornar vector" \
  "Tissue con 1000 células, expected ~50 neoplásticas"
```

### ❌ No hagas esto

```bash
# Demasiado vago
bash .github/scripts/run_profile.sh cpp "hacer código"

# Información faltante
bash .github/scripts/run_profile.sh cpp "Arreglar bug"

# Parámetros mal formateados
bash .github/scripts/run_profile.sh "c++ language" "implement stuff"
```

---

## 🔧 Personalización

### Quieres cambiar las reglas base?

Edita `.github/copilot/automation_profile.tpl` directamente:

```bash
vim .github/copilot/automation_profile.tpl
```

Los placeholders que puedes usar son:
- `{{LANG}}` - Lenguaje de programación
- `{{TASK}}` - Tu tarea específica
- `{{REQUIREMENTS}}` - Requisitos/restricciones
- `{{INPUTS}}` - Contexto/estado de entrada

---

## 📊 Variables de Salida

| Variable | Contenido |
|----------|-----------|
| `$TEMPLATE` | Ruta a `.github/copilot/automation_profile.tpl` |
| `$OUT` | `/tmp/copilot_prompt.txt` (archivo de salida) |
| `$LANG` | Lenguaje especificado (cpp, python, etc.) |
| `$TASK` | Tu tarea |
| `$REQUIREMENTS` | Requisitos especificados |
| `$INPUTS` | Inputs de contexto |

---

## 🐛 Troubleshooting

### Error: "Plantilla no encontrada"

```
Plantilla no encontrada en .github/copilot/automation_profile.tpl
```

**Solución:** Asegúrate de ejecutar el comando desde la raíz del proyecto (`/home/luis/CLionProjects/cellSim/`)

### Error: "Permiso denegado"

```bash
chmod +x .github/scripts/run_profile.sh
bash .github/scripts/run_profile.sh
```

### El output no se ve

```bash
# Ver el contenido generado:
bash .github/scripts/run_profile.sh cpp "tarea" | less

# O abre directamente:
cat /tmp/copilot_prompt.txt
```

---

## 📚 Relación con la documentación

Este script es la **herramienta de integración** entre:

```
┌──────────────────────────┐
│  Tu tarea de desarrollo  │
└────────────┬─────────────┘
             │
    ┌────────▼────────┐
    │  run_profile.sh │ ◄── (Este script)
    │  (generador)    │
    └────────┬────────┘
             │
    ┌────────▼──────────────────────────────┐
    │  Documentación contexto celSim        │
    ├──────────────────────────────────────┤
    │ • CELL_STATE_MACHINE.md              │
    │ • agent_context_structure.md         │
    │ • agent_roles.md                     │
    │ • cell_lifecycle_templates.md        │
    │ • context_examples/                  │
    └────────┬──────────────────────────────┘
             │
             ▼
    ┌──────────────────────┐
    │  Copilot Chat (CLion)│
    └────────┬─────────────┘
             │
             ▼
    ┌──────────────────────┐
    │   Código generado    │
    │   consistente con    │
    │   el proyecto        │
    └──────────────────────┘
```

---

## ✨ Resumen

| Aspecto | Descripción |
|---------|------------|
| **¿Qué es?** | Generador automático de prompts estructurados |
| **¿Dónde?** | `.github/scripts/run_profile.sh` |
| **¿Cómo se usa?** | `bash .github/scripts/run_profile.sh [lang] [tarea] [requisitos] [inputs]` |
| **Salida** | Prompt completo en `/tmp/copilot_prompt.txt` |
| **Ventaja** | Consistencia, automatización, menos errores |
| **Customización** | Edita `.github/copilot/automation_profile.tpl` |

