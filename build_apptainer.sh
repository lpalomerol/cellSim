#!/bin/bash
# ============================================================
# Script para construir imagen Apptainer de cellSim
# "Para tontos" - Explicaciones incluidas
# ============================================================

set -e

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Banner
print_banner() {
    echo ""
    echo -e "${BLUE}╔════════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║${NC}  🐳 Constructor de Imagen Apptainer     ${BLUE}║${NC}"
    echo -e "${BLUE}║${NC}  cellSim para HPC                       ${BLUE}║${NC}"
    echo -e "${BLUE}╚════════════════════════════════════════════╝${NC}"
    echo ""
}

# Funciones auxiliares
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
# 1. VERIFICACIONES INICIALES
# ==================================================
print_banner

print_step "Paso 1: Verificando requisitos..."
echo ""

# Verificar si está en el directorio correcto
if [ ! -f "cellSim.def" ]; then
    print_error "No se encontró cellSim.def"
    print_info "Por favor ejecuta este script desde la raíz de cellSim"
    exit 1
fi
print_success "Encontrado cellSim.def"

# Verificar Apptainer
if ! command -v apptainer &> /dev/null; then
    print_error "Apptainer no está instalado"
    echo ""
    echo -e "${YELLOW}Para instalar Apptainer:${NC}"
    echo "  Ubuntu/Debian:"
    echo "    sudo apt install apptainer"
    echo ""
    echo "  Fedora/RHEL:"
    echo "    sudo dnf install apptainer"
    echo ""
    exit 1
fi
print_success "Apptainer instalado: $(apptainer --version)"

# Verificar permisos de root
if [ "$EUID" -ne 0 ]; then
    print_error "Este script necesita permisos de root (sudo)"
    echo ""
    print_info "Ejecuta:"
    echo "  sudo $0 $@"
    exit 1
fi
print_success "Permisos de root verificados"

# ==================================================
# 2. CONFIGURACIÓN
# ==================================================
print_step "Paso 2: Configuración..."
echo ""

IMAGE_NAME="cellSim"
IMAGE_FILE="${IMAGE_NAME}.sif"
DEF_FILE="${IMAGE_NAME}.def"
SANDBOX_DIR="${IMAGE_NAME}"
COMPRESSION="gzip"

# Permitir parámetros
if [ ! -z "$1" ]; then
    IMAGE_NAME="$1"
    IMAGE_FILE="${IMAGE_NAME}.sif"
    DEF_FILE="${IMAGE_NAME}.def"
    SANDBOX_DIR="${IMAGE_NAME}"
fi

print_info "Nombre imagen: $IMAGE_NAME"
print_info "Archivo salida: $IMAGE_FILE"
print_info "Archivo definición: $DEF_FILE"
echo ""

# ==================================================
# 3. LIMPIAR COMPILACIONES ANTERIORES
# ==================================================
print_step "Paso 3: Limpieza de compilaciones anteriores..."
echo ""

if [ -f "$IMAGE_FILE" ]; then
    print_info "Encontrado $IMAGE_FILE anterior, respaldando..."
    cp "$IMAGE_FILE" "${IMAGE_FILE}.bak"
    print_success "Backup: ${IMAGE_FILE}.bak"
fi

if [ -d "$SANDBOX_DIR" ]; then
    print_info "Limpiando sandbox anterior..."
    rm -rf "$SANDBOX_DIR"
    print_success "Sandbox limpiado"
fi

echo ""

# ==================================================
# 4. COMPILAR IMAGEN
# ==================================================
print_step "Paso 4: Compilando imagen Apptainer..."
print_info "Esto puede tomar 5-15 minutos dependiendo de tu internet"
print_info "La imagen tendrá ~500 MB cuando termine"
echo ""

# Detectar si se necesita SSH (repo privado)
if grep -q "git@github.com" "$DEF_FILE"; then
    print_info "Detectado repositorio privado, configurando SSH..."

    # Verificar que existen las keys SSH
    if [ ! -f "$HOME/.ssh/id_rsa" ] && [ ! -f "$HOME/.ssh/id_ed25519" ]; then
        print_error "No se encontraron keys SSH en ~/.ssh/"
        print_info "Genera keys con: ssh-keygen -t ed25519"
        exit 1
    fi
    print_success "Keys SSH encontradas"
fi

if apptainer build "$IMAGE_FILE" "$DEF_FILE"; then
    print_success "Imagen compilada exitosamente: $IMAGE_FILE"
else
    print_error "Error compilando la imagen"
    print_info "Si es error de SSH, asegúrate de:"
    print_info "  1. Tener keys SSH en ~/.ssh/"
    print_info "  2. Haber añadido la key pública a GitHub"
    print_info "  3. Que ssh-agent esté corriendo: eval \$(ssh-agent -s)"
    exit 1
fi

echo ""

# ==================================================
# 5. VERIFICACIÓN
# ==================================================
print_step "Paso 5: Verificando imagen..."
echo ""

if [ -f "$IMAGE_FILE" ]; then
    SIZE=$(du -h "$IMAGE_FILE" | cut -f1)
    print_success "Archivo creado: $IMAGE_FILE ($SIZE)"
else
    print_error "Error: archivo no creado"
    exit 1
fi

echo ""

# ==================================================
# 6. PRUEBAS
# ==================================================
print_step "Paso 6: Ejecutando pruebas básicas..."
echo ""

if apptainer run "$IMAGE_FILE" cellSim_cli --help &>/dev/null; then
    print_success "Prueba 1: cellSim_cli funciona"
else
    print_error "Prueba 1: cellSim_cli falló (probablemente necesita compilarse internamente)"
fi

echo ""

# ==================================================
# 7. INFORMACIÓN FINAL
# ==================================================
echo -e "${GREEN}╔════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║${NC}  ✅ IMAGEN CREADA EXITOSAMENTE            ${GREEN}║${NC}"
echo -e "${GREEN}╚════════════════════════════════════════════╝${NC}"
echo ""
echo -e "${BLUE}Pasos siguientes:${NC}"
echo ""
echo "1️⃣  Probar localmente:"
echo "   apptainer run $IMAGE_FILE cellSim_cli --help"
echo ""
echo "2️⃣  Usar shell interactivo:"
echo "   apptainer shell $IMAGE_FILE"
echo ""
echo "3️⃣  Copiar al HPC:"
echo "   scp $IMAGE_FILE usuario@hpc.servidor.com:/home/usuario/"
echo ""
echo "4️⃣  Usar en HPC:"
echo "   ssh usuario@hpc.servidor.com"
echo "   apptainer run $IMAGE_FILE cellSim_cli --help"
echo ""
echo -e "${YELLOW}Notas:${NC}"
echo "  • Tamaño: $(du -h "$IMAGE_FILE" | cut -f1)"
echo "  • Para comprimir: tar -czf $IMAGE_FILE.tar.gz $IMAGE_FILE"
echo "  • Para restaurar backup: cp ${IMAGE_FILE}.bak $IMAGE_FILE"
echo ""

