# ⚙️ Configuración de CLion - Build & Test

## 🔴 Problema Identificado

El acceso directo **"Build all and run tests"** no funciona porque:
1. CLion no tenía un target explícito `run_tests` en CMake
2. El Makefile no tenía un target `all` para compilar + tests

## ✅ Solución Implementada

He realizado los siguientes cambios:

### 1. **CMakeLists.txt** - Agregar Custom Target
```cmake
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
    
    # Target auxiliar para CLion: ejecutar todos los tests
    add_custom_target(run_tests
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            DEPENDS unit_tests
            COMMENT "Ejecutando todos los tests...")
endif()
```

### 2. **Makefile** - Agregar Targets
```makefile
.PHONY: build test all all-test

build: rebuild
test: ...
all: rebuild test
all-test: all
```

---

## 🎯 Cómo Usar en CLion

### Opción A: Usar CMake Targets directamente en CLion
1. **Abre CLion**
2. Ve a **Run → Edit Configurations**
3. Crea una nueva configuración o edita la existente:
   - **Name**: "Build All & Test"
   - **Type**: CMake
   - **Target**: `unit_tests` + `run_tests`
   - **Configuration**: Debug
   - **Build options**: `-DBUILD_TESTS=ON`

### Opción B: Usar el atajo de teclado (Recomendado)
1. **Ctrl + F9** (o tu atajo configurado) para compilar
2. **Ctrl + Shift + F10** para ejecutar tests

### Opción C: Línea de Comando en Terminal
```bash
# Opción 1: Usar Makefile
make all

# Opción 2: Usar CMake directamente
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build
cmake --build build --target run_tests
```

---

## 🔍 Verificación

Para verificar que todo está configurado correctamente:

```bash
# 1. Reconstruir desde cero
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build

# 2. Ejecutar tests
cmake --build build --target run_tests

# O alternativamente:
cd build && ctest --output-on-failure
```

---

## 📋 Checklist

- [ ] Reabrir CLion (File → Reload CMake Project)
- [ ] Verificar que aparece el target `run_tests` en CMake Targets
- [ ] Intentar compilar: `Ctrl + F9`
- [ ] Intentar ejecutar tests: `Ctrl + Shift + F10` o desde el menu
- [ ] Verificar que los 87/87 tests pasen

---

## 🐛 Si Sigue Sin Funcionar

### Paso 1: Limpiar y Reconstruir
```bash
rm -rf build cmake-build-debug
# En CLion: File → Reload CMake Project
```

### Paso 2: Verificar CMakeLists.txt
```bash
cd /home/luis/CLionProjects/cellSim
cmake --version  # Debe ser >= 3.28.3
cmake -S . -B build -DBUILD_TESTS=ON -DCMAKE_VERBOSE_MAKEFILE=ON
```

### Paso 3: Verificar que googletest se descarga
```bash
# Debería crear _deps/googletest-src
ls -la build/_deps/
```

### Paso 4: Ejecutar desde terminal
```bash
cd build
ctest --output-on-failure -V
```

Si funciona desde terminal pero no en CLion, el problema es de configuración de CLion, no del proyecto.

---

## 💡 Notas Técnicas

- **`BUILD_TESTS=ON`**: Esta flag es REQUERIDA para compilar los tests
- **`run_tests`**: Custom target que ejecuta ctest con salida detallada
- **`unit_tests`**: Ejecutable generado por googletest con todos los test cases
- **CMake 3.28.3**: Versión requerida según proyecto

---

## 📞 Contacto

Si el problema persiste, proporciona la siguiente info:
1. CLion version
2. CMake version (`cmake --version`)
3. Salida completa del error

---

**Última actualización**: 2025-12-17  
**Estado**: ✅ Configurado

