#!/bin/bash
# ============================================================
# Script para construir imagen Apptainer con binarios LOCAL
# Perfecto para repositorios privados
# ============================================================

set -e

# Colores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Funciones
print_banner() {
    echo ""
    echo -e "${BLUE}╔════════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║${NC}  🐳 Constructor Apptainer (BINARIOS)     ${BLUE}║${NC}"
    echo -e "${BLUE}║${NC}  cellSim para HPC                         ${BLUE}║${NC}"
    echo -e "${BLUE}╚════════════════════════════════════════════╝${NC}"
    echo ""
}

print_step() {
    echo -e "${BLUE}➜${NC} $1"
}

print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

print_info() {
    echo -e "${YELLOW}ℹ${NC} $1"
}

# ==================================================
# INICIO
# ==================================================
print_banner

print_step "Paso 1: Verificando requisitos..."
echo ""

# Verificar Apptainer
if ! command -v apptainer &> /dev/null; then
    print_error "Apptainer no está instalado"
    echo "Instala con: sudo apt install apptainer"
    exit 1
fi
print_success "Apptainer: $(apptainer --version)"

# Verificar que estamos en el directorio correcto
if [ ! -f "cellSim.def" ]; then
    print_error "No se encontró cellSim.def"
    print_info "Ejecuta desde: ~/CLionProjects/cellSim"
    exit 1
fi
print_success "Directorio correcto"

# Verificar binarios precompilados
BINARIES=("build/cellSim" "build/cellSim_cli" "build/interactive" "build/run_all_scenarios" "build/single_cell_evolution")
for bin in "${BINARIES[@]}"; do
    if [ ! -f "$bin" ]; then
        print_error "No se encontró $bin"
        exit 1
    fi
done
print_success "Binarios encontrados"

# Verificar configs
if [ ! -d "configs" ]; then
    print_error "No se encontró directorio configs/"
    exit 1
fi
print_success "Configuraciones encontradas"

# Verificar permisos root
if [ "$EUID" -ne 0 ]; then
    print_error "Este script necesita sudo"
    echo ""
    print_info "Ejecuta: sudo $0"
    exit 1
fi
print_success "Permisos de root"

echo ""

# ==================================================
# PASO 2: COMPILAR IMAGEN
# ==================================================
print_step "Paso 2: Compilando imagen Apptainer..."
print_info "Esto puede tomar 3-5 minutos"
echo ""

IMAGE_FILE="cellSim.sif"

# Respaldo si existe
if [ -f "$IMAGE_FILE" ]; then
    print_info "Respaldando imagen anterior..."
    cp "$IMAGE_FILE" "${IMAGE_FILE}.bak"
    print_success "Backup: ${IMAGE_FILE}.bak"
fi

# Compilar
if apptainer build "$IMAGE_FILE" cellSim.def; then
    print_success "Imagen compilada"
else
    print_error "Error en compilación"
    print_info "Revisa los logs arriba"
    exit 1
fi

echo ""

# ==================================================
# PASO 3: VERIFICACIÓN
# ==================================================
print_step "Paso 3: Verificando imagen..."
echo ""

if [ -f "$IMAGE_FILE" ]; then
    SIZE=$(du -h "$IMAGE_FILE" | cut -f1)
    print_success "Archivo: $IMAGE_FILE ($SIZE)"
else
    print_error "La imagen no se creó"
    exit 1
fi

# Prueba rápida
print_info "Ejecutando prueba..."
if apptainer run "$IMAGE_FILE" cellSim_cli --help &>/dev/null; then
    print_success "✅ cellSim_cli funciona"
else
    print_error "❌ cellSim_cli falló"
    print_info "Verifica que los binarios sean compatibles"
fi

echo ""

# ==================================================
# RESULTADO FINAL
# ==================================================
echo -e "${GREEN}╔════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║${NC}  ✅ IMAGEN CREADA EXITOSAMENTE            ${GREEN}║${NC}"
echo -e "${GREEN}╚════════════════════════════════════════════╝${NC}"
echo ""
echo -e "${BLUE}Próximos pasos:${NC}"
echo ""
echo "1️⃣  Probar localmente:"
echo "   apptainer run $IMAGE_FILE cellSim_cli --help"
echo ""
echo "2️⃣  Shell interactivo:"
echo "   apptainer shell $IMAGE_FILE"
echo ""
echo "3️⃣  Copiar al HPC:"
echo "   scp $IMAGE_FILE usuario@hpc.com:/home/usuario/"
echo ""
echo "4️⃣  Comprimir para transporte:"
echo "   tar -czf $IMAGE_FILE.tar.gz $IMAGE_FILE"
echo ""
echo -e "${YELLOW}Notas:${NC}"
echo "  • Tamaño: $(du -h "$IMAGE_FILE" | cut -f1)"
echo "  • Binarios: /app/bin/ dentro del contenedor"
echo "  • Configs: /app/configs/ dentro del contenedor"
echo ""

