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
        src/domain/cell/AgenticCell.cpp
        src/domain/cell/AgenticCell.h
        src/domain/cell/strategies/GenomicInstabilityDeltaStrategy.cpp
        src/domain/cell/strategies/GenomicInstabilityDeltaStrategy.h
        src/domain/cell/strategies/IInstabilityDeltaStrategy.h
        src/domain/cell/strategies/GenomicViabilityStrategy.cpp
        src/domain/cell/strategies/GenomicViabilityStrategy.h
        src/domain/cell/strategies/IViabilityStrategy.h
        src/domain/gene/Gene.cpp
        src/domain/gene/Gene.h
        src/domain/gene/Genome.cpp
        src/domain/gene/GenomeFactory.cpp
        src/domain/gene/Genome.h
        src/application/config/SimulationConfig.cpp
        src/application/config/SimulationConfig.h
        src/application/presenter/GeneticSummaryPresenter.cpp
        src/application/presenter/GeneticSummaryPresenter.h)

add_executable(cellSim app/main.cpp
        src/application/simulation/Simulation.cpp
        src/application/simulation/Simulation.h
        src/application/simulation/TissueV2Adapter.h
        src/application/simulation/Simulations.cpp
        src/application/simulation/Simulations.h
        ${COMMON_SOURCES}
        src/domain/tissue/Tissue.cpp
        src/domain/tissue/Tissue.h
        src/domain/cell/AgenticCell.cpp
        src/domain/cell/AgenticCell.h)

# Ejecutable interactivo: reutiliza las mismas fuentes para enlazar correctamente
add_executable(interactive app/main_interactive.cpp
        src/application/simulation/Simulation.cpp
        src/application/simulation/Simulation.h
        src/application/simulation/TissueV2Adapter.h
        src/application/simulation/InteractiveSimulation.cpp
        src/application/simulation/InteractiveSimulation.h
        ${COMMON_SOURCES}
        src/domain/tissue/Tissue.cpp
        src/domain/tissue/Tissue.h
        src/domain/cell/AgenticCell.cpp
        src/domain/cell/AgenticCell.h)


# Ejecutable para evaluar evolución de una única célula
add_executable(single_cell_evolution app/single_cell_evolution.cpp
        src/application/simulation/Simulation.cpp
        src/application/simulation/Simulation.h
        src/application/simulation/TissueV2Adapter.h
        src/application/simulation/InteractiveSimulation.cpp
        src/application/simulation/InteractiveSimulation.h
        ${COMMON_SOURCES}
        src/domain/tissue/Tissue.cpp
        src/domain/tissue/Tissue.h
        src/domain/cell/CellFactory.cpp
        src/domain/cell/CellFactory.h
        src/domain/cell/AgenticCell.cpp
        src/domain/cell/AgenticCell.h)


# Ejecutable: Captura completa de 6 escenarios (3 runs cada uno)
add_executable(run_all_scenarios app/run_all_scenarios.cpp
        src/application/simulation/PopulationTracker.cpp
        src/application/simulation/PopulationTracker.h
        src/application/config/SimulationConfig.cpp
        src/application/config/SimulationConfig.h
        ${COMMON_SOURCES}
        src/domain/tissue/Tissue.cpp
        src/domain/tissue/Tissue.h
        src/domain/cell/CellFactory.cpp
        src/domain/cell/CellFactory.h
        src/domain/cell/AgenticCell.cpp
        src/domain/cell/AgenticCell.h)


# Añadir tests solo si BUILD_TESTS=ON (tests pueden descargar googletest)
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)

    # Target auxiliar para CLion: ejecutar todos los tests
    add_custom_target(run_tests
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            DEPENDS unit_tests
            COMMENT "Ejecutando todos los tests...")
endif()
