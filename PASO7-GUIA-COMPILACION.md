# 🚀 PASO 7: Guía de Compilación y Ejecución

**Estado**: ✅ COMPLETADO  
**Fecha**: 2025-12-17

---

## 📝 Nota Importante

**NO compiles manualmente con `g++`** - eso causa linking errors porque faltan las librerías y archivos .cpp.

**SIEMPRE usa CMake + Make** para compilar el proyecto.

---

## ✅ Compilación Correcta

### Opción 1: Compilar todo (Recomendado)

```bash
cd /home/luis/CLionProjects/cellSim/build
cmake .. -DBUILD_TESTS=ON
make -j4
```

### Opción 2: Compilar solo un target

```bash
cd /home/luis/CLionProjects/cellSim/build
make single_cell_evolution
# o
make run_all_scenarios
# o
make cellSim
```

### Opción 3: Rebuild limpio (si hay issues)

```bash
cd /home/luis/CLionProjects/cellSim
rm -rf build
mkdir build
cd build
cmake .. -DBUILD_TESTS=ON
make -j4
```

---

## ✅ Ejecución

### Ejecutar tests
```bash
cd /home/luis/CLionProjects/cellSim/build
./tests/unit_tests                                # Todos los tests
./tests/unit_tests --gtest_filter=TissueV2Test   # Tests específicos
```

### Ejecutar aplicaciones
```bash
cd /home/luis/CLionProjects/cellSim/build
./cellSim                    # Ejecutable principal
./run_all_scenarios          # Ejecutar todos los escenarios
./single_cell_evolution      # Simular célula única
./interactive                # Modo interactivo
```

---

## ⚠️ ¿Por qué CMake?

**CMake**:
- ✅ Compila TODOS los .cpp necesarios
- ✅ Linkea TODAS las librerías
- ✅ Maneja dependencias automáticamente
- ✅ Genera Makefiles optimizados

**g++ manual**:
- ❌ Tienes que escribir TODOS los .cpp a mano
- ❌ Tienes que especificar TODAS las librerías
- ❌ Fácil olvidar archivos → linking errors
- ❌ NO soporta generación automática

---

## 📊 Estado PASO 7

| Item | Status |
|------|--------|
| Tissue.h deprecated | ✅ |
| run_all_scenarios V2 | ✅ |
| single_cell_evolution V1 | ✅ |
| Todos los targets compilan | ✅ |
| 87/87 tests pasan | ✅ |
| Warnings de deprecated | ✅ Visible |

---

## 🎯 PASO 7 COMPLETADO

Tissue V1 está deprecado y el proyecto usa V2 en Application layer.

**Próximos pasos**: Commit y proceder con siguiente fase.


