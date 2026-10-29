cmake_minimum_required(VERSION 3.28.3)
project(cell_scale CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fuentes comunes del núcleo de simulación a escala celular
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
    src/domain/tissue/Tissue.cpp
    src/domain/tissue/Tissue.h
    src/application/config/SimulationConfig.cpp
    src/application/config/SimulationConfig.h
    src/application/presenter/GeneticSummaryPresenter.cpp
    src/application/presenter/GeneticSummaryPresenter.h
    src/application/bootstrapping/BootstrappingMetrics.cpp
    src/application/bootstrapping/BootstrappingMetrics.h
    src/application/simulation/Simulation.cpp
    src/application/simulation/Simulation.h
    src/application/simulation/PopulationTracker.cpp
    src/application/simulation/PopulationTracker.h
)

# Librería compartida para targets y tests
add_library(cell_scale_lib STATIC ${COMMON_SOURCES})
target_include_directories(cell_scale_lib PUBLIC src)
target_link_libraries(cell_scale_lib PUBLIC nlohmann_json::nlohmann_json cxxopts::cxxopts)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(cell_scale_lib PUBLIC OpenMP::OpenMP_CXX)
    target_compile_definitions(cell_scale_lib PUBLIC BOOTSTRAPPING_USE_OMP)
endif()

# Ejecutable 1: run_bootstrapping (Calibración y simulaciones Figure 2, 3, S1, S2, S3)
add_executable(run_bootstrapping app/run_bootstrapping.cpp)
target_link_libraries(run_bootstrapping PRIVATE cell_scale_lib)
set_target_properties(run_bootstrapping PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Ejecutable 2: run_paper_features (Características emergentes y clonalidad Figure 4)
add_executable(run_paper_features app/run_paper_features.cpp)
target_link_libraries(run_paper_features PRIVATE cell_scale_lib)
set_target_properties(run_paper_features PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Ejecutable 3: run_paper_interventions (Intercepción y sinergia terapéutica Figure 5, S5)
add_executable(run_paper_interventions app/run_paper_interventions.cpp)
target_link_libraries(run_paper_interventions PRIVATE cell_scale_lib)
set_target_properties(run_paper_interventions PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Tests unitarios
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
