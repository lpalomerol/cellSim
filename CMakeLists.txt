cmake_minimum_required(VERSION 3.28.3)
project(cellSim)

set(CMAKE_CXX_STANDARD 20)

# Opción para construir tests unitarios
option(BUILD_TESTS "Construir tests unitarios" ON)

include(FetchContent)

# Librerías externas
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

FetchContent_Declare(cxxopts GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git GIT_TAG v3.2.1)
FetchContent_MakeAvailable(cxxopts)

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
        src/domain/cell/strategies/LognormalNoiseInstabilityStrategy.h
        src/domain/cell/strategies/LognormalNoiseInstabilityStrategy.cpp
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

# Bootstrap calibration: cumulative risk curve vs. Kuchenbaecker 2017 BRCA1 data
add_executable(run_bootstrapping app/run_bootstrapping.cpp
        src/application/bootstrapping/BootstrappingMetrics.h
        src/application/bootstrapping/BootstrappingMetrics.cpp
        src/application/simulation/Simulation.cpp
        src/application/simulation/Simulation.h
        src/application/simulation/PopulationTracker.cpp
        src/application/simulation/PopulationTracker.h
        ${COMMON_SOURCES}
        src/domain/tissue/Tissue.cpp
        src/domain/tissue/Tissue.h
        src/domain/cell/CellFactory.cpp
        src/domain/cell/CellFactory.h
        src/domain/cell/AgenticCell.cpp
        src/domain/cell/AgenticCell.h)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(run_bootstrapping PRIVATE nlohmann_json::nlohmann_json cxxopts::cxxopts OpenMP::OpenMP_CXX)
    target_compile_definitions(run_bootstrapping PRIVATE BOOTSTRAPPING_USE_OMP)
else()
    target_link_libraries(run_bootstrapping PRIVATE nlohmann_json::nlohmann_json cxxopts::cxxopts)
    message(WARNING "OpenMP not found — run_bootstrapping will run single-threaded")
endif()

# Paper-only emergent-feature executable
add_executable(run_paper_features app/run_paper_features.cpp
        src/application/bootstrapping/BootstrappingMetrics.h
        src/application/bootstrapping/BootstrappingMetrics.cpp
        src/application/simulation/Simulation.cpp
        src/application/simulation/Simulation.h
        src/application/simulation/PopulationTracker.cpp
        src/application/simulation/PopulationTracker.h
        ${COMMON_SOURCES}
        src/domain/tissue/Tissue.cpp
        src/domain/tissue/Tissue.h
        src/domain/cell/CellFactory.cpp
        src/domain/cell/CellFactory.h
        src/domain/cell/AgenticCell.cpp
        src/domain/cell/AgenticCell.h)

if(OpenMP_CXX_FOUND)
    target_link_libraries(run_paper_features PRIVATE nlohmann_json::nlohmann_json cxxopts::cxxopts OpenMP::OpenMP_CXX)
    target_compile_definitions(run_paper_features PRIVATE BOOTSTRAPPING_USE_OMP)
else()
    target_link_libraries(run_paper_features PRIVATE nlohmann_json::nlohmann_json cxxopts::cxxopts)
endif()

# Paper-only matched interception executable
add_executable(run_paper_interventions app/run_paper_interventions.cpp
        src/application/bootstrapping/BootstrappingMetrics.h
        src/application/bootstrapping/BootstrappingMetrics.cpp
        src/application/simulation/Simulation.cpp
        src/application/simulation/Simulation.h
        src/application/simulation/PopulationTracker.cpp
        src/application/simulation/PopulationTracker.h
        ${COMMON_SOURCES}
        src/domain/tissue/Tissue.cpp
        src/domain/tissue/Tissue.h
        src/domain/cell/CellFactory.cpp
        src/domain/cell/CellFactory.h
        src/domain/cell/AgenticCell.cpp
        src/domain/cell/AgenticCell.h)

if(OpenMP_CXX_FOUND)
    target_link_libraries(run_paper_interventions PRIVATE nlohmann_json::nlohmann_json cxxopts::cxxopts OpenMP::OpenMP_CXX)
    target_compile_definitions(run_paper_interventions PRIVATE BOOTSTRAPPING_USE_OMP)
else()
    target_link_libraries(run_paper_interventions PRIVATE nlohmann_json::nlohmann_json cxxopts::cxxopts)
endif()

# Añadir tests solo si BUILD_TESTS=ON
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)

    # Target auxiliar para ejecutar todos los tests
    add_custom_target(run_tests
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            DEPENDS unit_tests
            COMMENT "Ejecutando todos los tests...")
endif()
