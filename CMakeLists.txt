cmake_minimum_required(VERSION 3.28.3)
project(cellSim)

set(CMAKE_CXX_STANDARD 20)

# Opción para evitar descargar y construir tests (googletest) en entornos offline
option(BUILD_TESTS "Construir tests unitarios (puede descargar googletest)" OFF)

include(FetchContent)

# Fuentes comunes del núcleo de la aplicación
set(COMMON_SOURCES
        src/domain/ports/ICell.h
        src/domain/ports/INoiseSource.h
        src/domain/ports/ILogger.h
        src/domain/ports/CellNoise.h
        src/domain/adapters/RandomNoise.h
        src/domain/adapters/FixedNoise.h
        src/domain/adapters/LogLevel.h
        src/domain/adapters/Logger.h
        src/domain/adapters/NullLogger.h
        src/domain/cell/CellFactory.cpp
        src/domain/cell/CellFactory.h
        src/domain/cell/OncoState.h
        src/domain/cell/AgenticCell.cpp
        src/domain/cell/AgenticCell.h
        src/domain/gene/Gene.cpp
        src/domain/gene/Gene.h
        src/domain/gene/Genome.cpp
        src/domain/gene/GenomeFactory.cpp
        src/domain/gene/Genome.h
        src/domain/tissue/Tissue.cpp
        src/application/config/SimulationConfig.cpp
        src/application/config/SimulationConfig.h)

add_executable(cellSim app/main.cpp
        src/application/simulation/Simulation.cpp
        src/application/simulation/Simulation.h
        src/application/simulation/Simulations.cpp
        src/application/simulation/Simulations.h
        ${COMMON_SOURCES}
        src/domain/tissue/Tissue.cpp)

# Ejecutable interactivo: reutiliza las mismas fuentes para enlazar correctamente
add_executable(interactive app/main_interactive.cpp
        src/application/simulation/Simulation.cpp
        src/application/simulation/Simulation.h
        src/application/simulation/InteractiveSimulation.cpp
        src/application/simulation/InteractiveSimulation.h
        ${COMMON_SOURCES}
        src/domain/tissue/Tissue.cpp)


# Ejecutable para evaluar evolución de una única célula
add_executable(single_cell_evolution app/single_cell_evolution.cpp
        src/application/simulation/Simulation.cpp
        src/application/simulation/Simulation.h
        src/application/simulation/InteractiveSimulation.cpp
        src/application/simulation/InteractiveSimulation.h
        ${COMMON_SOURCES}
        src/domain/tissue/Tissue.cpp)


# Ejecutable: Captura completa de 6 escenarios (3 runs cada uno)
add_executable(run_all_scenarios app/run_all_scenarios.cpp
        src/application/simulation/PopulationTracker.cpp
        src/application/simulation/PopulationTracker.h
        src/application/config/SimulationConfig.cpp
        src/application/config/SimulationConfig.h
        ${COMMON_SOURCES}
        src/domain/tissue/Tissue.cpp)


# Añadir tests solo si BUILD_TESTS=ON (tests pueden descargar googletest)
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
