cmake_minimum_required(VERSION 3.28.3)
project(BRCA1_ONSET_Simulation_Suite CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Opciones de compilación
option(BUILD_TESTS "Construir suite de tests unitarios" ON)

include(FetchContent)

# Dependencias externas globales
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

FetchContent_Declare(cxxopts GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git GIT_TAG v3.2.1)
FetchContent_MakeAvailable(cxxopts)

# Habilitar soporte de tests
if(BUILD_TESTS)
    enable_testing()
endif()

# ============================================================
# Motores Simétricos de Simulación (Arquitectura Opción A)
# ============================================================

# 1. Motor Celular Discreto (Cell Scale: 500 células / tejido)
add_subdirectory(cell_scale)

# 2. Motor Continuo a Escala de Tejido (Tissue Scale: 10^7 células / tejido)
add_subdirectory(tissue_scale)

# Target auxiliar para ejecutar todos los tests
if(BUILD_TESTS)
    add_custom_target(run_tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS unit_tests
        COMMENT "Ejecutando todos los tests unitarios...")
endif()
