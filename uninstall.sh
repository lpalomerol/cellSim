#!/bin/bash
set -e

echo "🗑️  Desinstalando cellSim..."
echo ""

# Detectar directorio de instalación
PREFIX="${PREFIX:-/usr/local}"
NEEDS_SUDO=false

if [ ! -w "$PREFIX" ]; then
    NEEDS_SUDO=true
    echo "⚠️  Se requieren permisos de administrador para desinstalar de $PREFIX"
    echo ""
fi

# Verificar si los binarios existen
BINDIR="$PREFIX/bin"
FOUND=false

for binary in cellSim cellSim_cli run_all_scenarios single_cell_evolution interactive; do
    if [ -f "$BINDIR/$binary" ]; then
        FOUND=true
        echo "📍 Encontrado: $BINDIR/$binary"
    fi
done

if [ "$FOUND" = false ]; then
    echo "ℹ️  No se encontraron binarios de cellSim en $BINDIR"
    echo "   Si instalaste en otra ubicación, usa: PREFIX=/ruta ./uninstall.sh"
    exit 0
fi

echo ""
read -p "¿Deseas continuar con la desinstalación? (s/N): " -n 1 -r
echo ""

if [[ ! $REPLY =~ ^[SsYy]$ ]]; then
    echo "❌ Desinstalación cancelada"
    exit 0
fi

# Desinstalar
if [ "$NEEDS_SUDO" = true ]; then
    sudo make uninstall PREFIX="$PREFIX"
else
    make uninstall PREFIX="$PREFIX"
fi

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Desinstalación completada exitosamente"
else
    echo "❌ Error durante la desinstalación"
    exit 1
fi

