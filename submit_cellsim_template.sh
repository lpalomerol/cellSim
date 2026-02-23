#!/bin/bash
# ============================================================
# Template de job para ejecutar cellSim en HPC con Apptainer
# ============================================================
# Uso:
#   sbatch submit_cellsim_slurm.sh
# o
#   qsub submit_cellsim_pbs.sh
# ============================================================

# ============================================================
# CONFIGURACIÓN SLURM (para clusters con Slurm)
# ============================================================
# Descomenta estas líneas si tu cluster usa SLURM
#SBATCH --job-name=cellSim
#SBATCH --output=cellSim-%j.log
#SBATCH --error=cellSim-%j.err
#SBATCH --time=24:00:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=8
#SBATCH --mem=16G
#SBATCH --partition=cpu

# ============================================================
# CONFIGURACIÓN PBS (para clusters con PBS/Torque)
# ============================================================
# Descomenta estas líneas si tu cluster usa PBS
#PBS -N cellSim
#PBS -l walltime=24:00:00
#PBS -l nodes=1:ppn=8
#PBS -l mem=16gb
#PBS -o cellSim.log
#PBS -e cellSim.err

# ============================================================
# CONFIGURACIÓN DEL JOB
# ============================================================

set -e

# Variables
IMAGE="/path/to/cellSim.sif"          # ← CAMBIA ESTO: ruta a tu imagen
CONFIG="./configs/default.json"       # ← CAMBIA ESTO: ruta a tu config
OUTPUT_DIR="./results_$(date +%Y%m%d_%H%M%S)"

# Crear directorio de salida
mkdir -p "$OUTPUT_DIR"

echo "=========================================="
echo "  🚀 cellSim Job Launcher"
echo "=========================================="
echo ""
echo "📋 Información del Job:"
echo "   Imagen: $IMAGE"
echo "   Configuración: $CONFIG"
echo "   Salida: $OUTPUT_DIR"
echo "   Fecha/Hora: $(date)"
echo ""

# Verificar que la imagen existe
if [ ! -f "$IMAGE" ]; then
    echo "❌ Error: No se encontró la imagen $IMAGE"
    exit 1
fi

echo "✅ Verificaciones pasadas"
echo ""

# ============================================================
# OPCIÓN 1: Ejecución simple
# ============================================================
echo "🔨 Compilando simulación..."
echo ""

apptainer run "$IMAGE" cellSim_cli \
    --config "$CONFIG" \
    --verbose

echo ""
echo "✅ Simulación completada"

# ============================================================
# OPCIÓN 2: Ejecutar múltiples escenarios
# ============================================================
# Descomenta esto para ejecutar todos los escenarios

# echo "🔨 Ejecutando todos los escenarios..."
# apptainer run "$IMAGE" run_all_scenarios
# echo "✅ Todos los escenarios completados"

# ============================================================
# OPCIÓN 3: Con bindings de directorios
# ============================================================
# Descomenta esto si necesitas acceso a directorios fuera de tu $HOME

# apptainer run -B /scratch:/scratch "$IMAGE" cellSim_cli \
#     --config /scratch/configs/config.json \
#     --verbose

# ============================================================
# FINALES
# ============================================================
echo ""
echo "=========================================="
echo "  ✅ Job finalizado"
echo "=========================================="
echo "Fecha/Hora: $(date)"
echo ""

