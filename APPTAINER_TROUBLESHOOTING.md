# 🐛 Troubleshooting Apptainer - Solución de Problemas Comunes

## Problema 1: "apptainer: command not found"
**Solución:**
```bash
# Instalar Apptainer
sudo apt install apptainer  # Ubuntu/Debian
sudo dnf install apptainer  # Fedora/RHEL
```

## Problema 2: "This command must be run as root"
**Solución:**
```bash
# Opción A: Usar sudo
sudo apptainer build cellSim.sif cellSim.def

# Opción B: Configurar permisos (más avanzado)
# Contacta al administrador del HPC
```

## Problema 3: "Downloaded file is corrupted"
**Solución:**
```bash
# Limpiar y reintentedefiniciones
rm -f cellSim.sif
rm -rf cellSim/  # sandbox si existe
rm ~/.apptainer/cache/*  # limpiar caché

# Recompilar
sudo apptainer build cellSim.sif cellSim.def
```

## Problema 4: Error compilando: "FATAL Cannot cache DOCKER manifest"
**Causa:** No hay conexión a internet o Docker Hub está caído
**Soluciones:**
```bash
# Opción 1: Esperar e intentar de nuevo
sleep 60
sudo apptainer build cellSim.sif cellSim.def

# Opción 2: Compilar desde imagen local
# (requiere tener imagen docker local)

# Opción 3: Crear imagen en sandbox
sudo apptainer build --sandbox cellSim/ cellSim.def
```

## Problema 5: La imagen es muy grande (~1 GB)
**Soluciones:**
```bash
# Comprimir la imagen
tar -czf cellSim.sif.tar.gz cellSim.sif
# Tamaño resultante: ~200-300 MB

# Copiar solo el tar.gz al HPC
scp cellSim.sif.tar.gz usuario@hpc.com:/home/usuario/

# En el HPC, descomprimir
tar -xzf cellSim.sif.tar.gz
```

## Problema 6: "Permission denied" al ejecutar
**Soluciones:**
```bash
# Hacer ejecutable
chmod +x cellSim.sif

# O ejecutar explícitamente
apptainer run cellSim.sif command
```

## Problema 7: Error en HPC: "Could not find NVIDIA GPU"
**Información:**
```bash
# Es NORMAL si el nodo no tiene GPU
# cellSim no necesita GPU para funcionar

# Si NECESITAS GPU:
apptainer run --nv cellSim.sif cellSim_cli --config config.json
# (--nv activa soporte NVIDIA)
```

## Problema 8: Error: "Cannot find config file" en HPC
**Causas comunes:**
- El archivo config no está en la imagen
- No se vinculó correctamente con -B

**Soluciones:**
```bash
# Opción 1: Usar config que está en la imagen
apptainer run cellSim.sif cellSim_cli --config /app/configs/default.json

# Opción 2: Vincular directorios
apptainer run -B /home/usuario/myconfigs:/configs cellSim.sif \
    cellSim_cli --config /configs/my.json

# Opción 3: Copiar config a /tmp en el contenedor
apptainer run -B /home/usuario/myconfigs:/tmp cellSim.sif \
    cellSim_cli --config /tmp/my.json
```

## Problema 9: Error: "No space left on device"
**Causa:** El disco está lleno
**Soluciones:**
```bash
# Ver espacio disponible
df -h

# Limpiar temporales
sudo apptainer cache clean

# Ver qué ocupa espacio
du -sh cellSim.sif
du -sh ~/.apptainer/

# Pasar a otro disco (si tienes)
export TMPDIR=/path/with/space
sudo apptainer build cellSim.sif cellSim.def
```

## Problema 10: "Environment: ERROR: Unable to acquire memory lock"
**Causa:** Limitaciones de memoria del sistema
**Solución:**
```bash
# En HPC, solicita más memoria:
#SBATCH --mem=32G  # en lugar de 16G

# O ejecutar con menos threads:
apptainer run cellSim.sif cellSim_cli --threads 2
```

## Problema 11: Los resultados no se guardan
**Causa:** Los cambios dentro del contenedor no persisten por defecto
**Soluciones:**
```bash
# Opción 1: Vincular directorio de salida
apptainer run -B /home/usuario/results:/output cellSim.sif \
    cellSim_cli --config config.json --output /output

# Opción 2: Usar writable-tmpfs para sandbox
apptainer run --writable-tmpfs cellSim.sif command
```

## Problema 12: "Fatal user lookup error: unknown uid"
**Causa:** Problemas con usuarios en el contenedor
**Solución:**
```bash
# Ejecutar con opción --userns
apptainer run --userns cellSim.sif cellSim_cli --config config.json

# O contactar al administrador del HPC
```

---

## 📋 Checklist de Diagnóstico

Cuando algo falle, comprueba:

- [ ] ¿Apptainer está instalado? `apptainer --version`
- [ ] ¿La imagen existe? `ls -lh cellSim.sif`
- [ ] ¿La imagen está completa? `file cellSim.sif`
- [ ] ¿Hay permisos? `ls -l cellSim.sif | grep rwx`
- [ ] ¿Hay espacio en disco? `df -h`
- [ ] ¿El comando es correcto? `apptainer run cellSim.sif --help`
- [ ] ¿Funciona en tu máquina local?
- [ ] ¿Funciona en el HPC con -B?

## 🆘 Obtener Ayuda

```bash
# Ver documentación de Apptainer
apptainer run --help
apptainer build --help

# Ver logs de compilación
sudo apptainer build -v cellSim.sif cellSim.def

# En el HPC, contactar a:
# admin@hpc.com o
# help@hpc.com
```

## 📚 Referencias

- [Documentación oficial Apptainer](https://apptainer.org/docs/)
- [Guía de definiciones](https://apptainer.org/docs/user/main/definition_files.html)
- [Best practices para HPC](https://apptainer.org/docs/user/main/appendix.html)

