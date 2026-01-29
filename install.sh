#!/bin/bash
set -e

echo "🔧 Instalando cellSim..."
echo ""

# Verificar dependencias
echo "📋 Verificando dependencias..."
command -v cmake >/dev/null 2>&1 || { echo "❌ cmake no está instalado. Instálalo con: sudo apt install cmake"; exit 1; }
command -v g++ >/dev/null 2>&1 || { echo "❌ g++ no está instalado. Instálalo con: sudo apt install g++"; exit 1; }
command -v make >/dev/null 2>&1 || { echo "❌ make no está instalado. Instálalo con: sudo apt install make"; exit 1; }

echo "✅ Todas las dependencias están instaladas"
echo ""

# Detectar directorio de instalación
PREFIX="${PREFIX:-/usr/local}"
NEEDS_SUDO=false

if [ ! -w "$PREFIX" ]; then
    NEEDS_SUDO=true
    echo "⚠️  Se requieren permisos de administrador para instalar en $PREFIX"
    echo "   Puedes usar PREFIX=\$HOME/.local para instalación local sin sudo"
    echo ""
fi

# Compilar
echo "🔨 Compilando proyecto..."
make rebuild

if [ $? -ne 0 ]; then
    echo "❌ Error al compilar el proyecto"
    exit 1
fi

echo ""
echo "📦 Instalando binarios en $PREFIX/bin..."

# Instalar
if [ "$NEEDS_SUDO" = true ]; then
    sudo make install PREFIX="$PREFIX"
else
    make install PREFIX="$PREFIX"
fi

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Instalación completada exitosamente"
    echo ""
    echo "Binarios instalados:"
    echo "  - cellSim"
    echo "  - cellSim_cli"
    echo "  - run_all_scenarios"
    echo "  - single_cell_evolution"
    echo "  - interactive"
    echo ""

    # Verificar si está en PATH
    if echo "$PATH" | grep -q "$PREFIX/bin"; then
        echo "✅ $PREFIX/bin está en tu PATH"
        echo "   Puedes ejecutar los binarios directamente: cellSim_cli --help"
    else
        echo "⚠️  $PREFIX/bin no está en tu PATH"
        echo "   Añade esto a tu ~/.bashrc o ~/.zshrc:"
        echo "   export PATH=$PREFIX/bin:\$PATH"
    fi
else
    echo "❌ Error durante la instalación"
    exit 1
fi

