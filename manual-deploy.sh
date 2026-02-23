#!/bin/bash
# ============================================================
# Script para distribución manual (sin ssh automático)
# ============================================================
# Uso: ./manual-deploy.sh [version] [servidor] [usuario] [ruta_destino]
#
# Ejemplo:
#   ./manual-deploy.sh 1.0.0 192.168.1.100 luis /home/luis/cellSim
#   ./manual-deploy.sh 1.0.0 servidor.com luis /opt/cellSim
# ============================================================

set -e

VERSION=${1:-1.0.0}
SERVER=${2:-server.com}
USER=${3:-user}
DEST_PATH=${4:-/home/$USER/cellSim}

echo ""
echo "=========================================="
echo "  📦 DISTRIBUCIÓN MANUAL DE cellSim"
echo "=========================================="
echo ""

# Paso 3: Instrucciones para el servidor
echo "📋 PASO 3: Instrucciones para ejecutar en el servidor"
echo ""
echo "   Conéctate al servidor:"
echo "   $ ssh $USER@$SERVER"
echo ""
echo "   Luego ejecuta:"
echo "   $ mkdir -p $DEST_PATH"
echo "   $ cd $DEST_PATH"
echo "   $ tar -xzf ~/cellSim-${VERSION}.tar.gz"
echo "   $ cd cellSim-dist && mv * .. && cd .. && rmdir cellSim-dist"
echo "   $ chmod +x bin/*"
echo ""
echo "   Para verificar:"
echo "   $ ls -la"
echo "   $ ./bin/cellSim_cli --help"
echo ""
echo "   Para ejecutar:"
echo "   $ ./bin/cellSim_cli --config configs/default.json --verbose"
echo ""
echo "=========================================="
echo "✅ Script completado"
echo "=========================================="

