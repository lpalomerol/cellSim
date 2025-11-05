cmake_minimum_required(VERSION 3.28.3)
project(cellSim)

set(CMAKE_CXX_STANDARD 20)

# Opción para evitar descargar y construir tests (googletest) en entornos offline
option(BUILD_TESTS "Construir tests unitarios (puede descargar googletest)" OFF)

include(FetchContent)

# Fuentes comunes del núcleo de la aplicación
set(COMMON_SOURCES
        src/application/simulation/Simulation.cpp
        src/application/simulation/Simulation.h
        src/application/simulation/Simulations.cpp
        src/application/simulation/Simulations.h
        src/domain/ports/ICell.h
        src/domain/ports/INoiseSource.h
        src/domain/ports/CellNoise.h
        src/domain/adapters/RandomNoise.h
        src/domain/adapters/FixedNoise.h
        src/domain/cell/SimpleCell.cpp
        src/domain/cell/SimpleCell.h
        src/domain/cell/CellState.h
        src/domain/cell/CellFactory.cpp
        src/domain/cell/CellFactory.h
        src/domain/cell/OncoState.h
        src/domain/cell/OncoMatrix.h
        src/domain/cell/AgenticCell.cpp
        src/domain/cell/AgenticCell.h
        src/domain/gene/Gene.cpp
        src/domain/gene/Gene.h
        src/domain/gene/Genome.cpp
        src/domain/gene/GenomeFactory.cpp
        src/domain/gene/Genome.h)

add_executable(cellSim app/main.cpp ${COMMON_SOURCES})

# Ejecutable interactivo: reutiliza las mismas fuentes para enlazar correctamente
add_executable(interactive app/main_interactive.cpp ${COMMON_SOURCES})

# Añadir tests solo si BUILD_TESTS=ON (tests pueden descargar googletest)
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
