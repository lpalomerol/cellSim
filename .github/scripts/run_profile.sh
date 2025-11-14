# File: `.github/scripts/run_profile.sh`
#!/usr/bin/env bash
set -eu
TEMPLATE=".github/copilot/automation_profile.tpl"
OUT="/tmp/copilot_prompt.txt"

LANG="${1:-cpp}"
TASK="${2:-Implementar la funcionalidad solicitada}"
REQUIREMENTS="${3:-Seguir reglas del repo (tests, estilo, manejo de errores)}"
INPUTS="${4:-ninguno}"

if [ ! -f "$TEMPLATE" ]; then
  echo "Plantilla no encontrada en $TEMPLATE" >&2
  exit 1
fi

# Escapar caracteres especiales para sed
escape() {
  printf '%s' "$1" | sed -e 's/[\/&]/\\&/g'
}

sed -e "s/{{LANG}}/$(escape "$LANG")/g" \
    -e "s/{{TASK}}/$(escape "$TASK")/g" \
    -e "s/{{REQUIREMENTS}}/$(escape "$REQUIREMENTS")/g" \
    -e "s/{{INPUTS}}/$(escape "$INPUTS")/g" \
    "$TEMPLATE" > "$OUT"

cat "$OUT"
echo
echo "Prompt generado en: $OUT"
# File: `.github/copilot/README.md`
#
# Uso rápido
#
# - Edita la plantilla en `.github/copilot/automation_profile.tpl` para ajustar reglas comunes.
# - Genera un prompt con:
#     bash .github/scripts/run_profile.sh cpp "tarea específica" "requisitos" "inputs"
# - Copia el contenido de `/tmp/copilot_prompt.txt` y pégalo en Copilot Chat en CLion.
#
# Notas
# - Versiona esta carpeta para compartir las reglas entre el equipo.
# - Mantén las reglas concisas y estables; documenta cambios importantes en el README.