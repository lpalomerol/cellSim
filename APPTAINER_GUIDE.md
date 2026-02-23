# 🐳 Guía Apptainer para cellSim - Para Tontos

## ¿Qué es Apptainer?

**Apptainer** (antes llamado Singularity) es un contenedor similar a Docker, pero optimizado para HPC (supercomputadoras). Es como una **máquina virtual ligera** que empaqueta tu aplicación con todo lo que necesita.

**Ventajas:**
- 📦 Tu código funciona igual en cualquier HPC
- 🔒 No necesitas permisos de admin (root)
- ⚡ Muy rápido (sin virtualización pesada)
- 🎯 Estándar en clusters científicos

## Paso 1: Instalar Apptainer en tu máquina local

### En Ubuntu/Debian:
```bash
# Instalar dependencias
sudo apt update
sudo apt install -y build-essential libssl-dev uuid-dev libgpgme-dev squashfs-tools cryptsetup

# Descargar e instalar Apptainer
export VERSION=1.3.0 && \
wget https://github.com/apptainer/apptainer/releases/download/v${VERSION}/apptainer-${VERSION}.tar.gz && \
tar -xzf apptainer-${VERSION}.tar.gz && \
cd apptainer-${VERSION} && \
./mconfig && \
make -C builddir && \
sudo make -C builddir install
```

### En Fedora/RHEL:
```bash
sudo dnf install -y apptainer
```

### En macOS:
```bash
# Nota: Apptainer NO funciona directamente en macOS
# Necesitas usar un VM Linux o usar Docker Desktop
# Pero puedes crear la imagen en Linux y llevarla
```

### Verificar instalación:
```bash
apptainer --version
```

## Paso 2: Crear la imagen Apptainer

### Opción A: FÁCIL - Usar el archivo de definición (sin compilar en tu máquina)

```bash
cd /home/luis/CLionProjects/cellSim
sudo apptainer build cellSim.sif cellSim.def
```

Esto crea un archivo llamado `cellSim.sif` (~500 MB).

### Opción B: Si quieres compilar en tu máquina primero
```bash
cd /home/luis/CLionProjects/cellSim
sudo apptainer build --sandbox cellSim/ cellSim.def
sudo apptainer build cellSim.sif cellSim/
```

**⚠️ NOTA**: La primera compilación tarda ~5-10 minutos y necesita **conexión a internet**.

## Paso 3: Usar la imagen en tu máquina local (pruebas)

### Ver lo que hay dentro:
```bash
apptainer shell cellSim.sif
# Dentro del contenedor, puedes hacer:
# $ ls /app/bin
# $ exit (para salir)
```

### Ejecutar directamente:
```bash
# Ver ayuda
apptainer run cellSim.sif cellSim_cli --help

# Ejecutar con config
apptainer run cellSim.sif cellSim_cli --config configs/default.json

# Ejecutar versión interactiva
apptainer run cellSim.sif interactive

# Ejecutar todas las simulaciones
apptainer run cellSim.sif run_all_scenarios
```

### Pasar archivos de entrada/salida:
```bash
# Los archivos en tu $HOME están accesibles en el contenedor
apptainer run -B /home/luis/datos:/datos cellSim.sif cellSim_cli --config /datos/mi_config.json
```

## Paso 4: Llevar la imagen al HPC

### Opción 1: Copiar directamente por SCP
```bash
# Desde tu máquina local:
scp cellSim.sif usuario@hpc.servidor.com:/home/usuario/
```

### Opción 2: Crear un tar comprimido
```bash
tar -czf cellSim.sif.tar.gz cellSim.sif
scp cellSim.sif.tar.gz usuario@hpc.servidor.com:/home/usuario/
# En el HPC:
tar -xzf cellSim.sif.tar.gz
```

## Paso 5: Usar en el HPC

### Conectarte al HPC:
```bash
ssh usuario@hpc.servidor.com
cd /path/to/cellSim
```

### Ejecutar pruebas rápidas:
```bash
# Shell interactivo
apptainer shell cellSim.sif

# O ejecutar directamente
apptainer run cellSim.sif cellSim_cli --help
```

### Crear un script para el scheduler (Slurm, PBS, etc.)

**Archivo: submit_cellsim.sh**
```bash
#!/bin/bash
#SBATCH --job-name=cellSim
#SBATCH --output=cellSim-%j.log
#SBATCH --error=cellSim-%j.err
#SBATCH --time=24:00:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=8

cd $SLURM_SUBMIT_DIR

# Ejecutar con la imagen
apptainer run --bind $SCRATCH:/scratch cellSim.sif \
    cellSim_cli --config configs/default.json --verbose
```

Enviar trabajo:
```bash
sbatch submit_cellsim.sh
```

## Paso 6: Troubleshooting

### Error: "Permission denied"
```bash
sudo chmod +x cellSim.sif
```

### Error: "Cannot find apptainer"
Instala Apptainer (ver Paso 1)

### Error: "Downloaded file is corrupted"
Descarga de nuevo el archivo `.def` o crea la imagen nuevamente

### La imagen es muy grande
Es normal (~500 MB). Puedes comprimirla más:
```bash
tar -czf cellSim.sif.tar.gz cellSim.sif
# Resultará en ~100-200 MB
```

## Archivos necesarios

En el directorio cellSim encontrarás:
- `cellSim.def` - Definición de la imagen (receta)
- `cellSim.sif` - Imagen final (después de compilar)
- `APPTAINER_GUIDE.md` - Esta guía

## Resumen rápido (TL;DR)

```bash
# 1. Instalar Apptainer
sudo apt install apptainer

# 2. Crear imagen (en tu máquina con internet)
cd ~/CLionProjects/cellSim
sudo apptainer build cellSim.sif cellSim.def

# 3. Probar localmente
apptainer run cellSim.sif cellSim_cli --help

# 4. Copiar al HPC
scp cellSim.sif usuario@hpc.servidor.com:/home/usuario/

# 5. Usar en HPC
ssh usuario@hpc.servidor.com
apptainer run cellSim.sif cellSim_cli --config config.json
```

¡Eso es todo! 🎉

