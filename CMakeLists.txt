cmake_minimum_required(VERSION 3.28.3)
project(cellSim)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        cxxopts
        GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
        GIT_TAG        v3.1.1
)
FetchContent_MakeAvailable(cxxopts)

add_executable(cellSim app/main.cpp
        src/domain/ports/ICell.h
        src/application/simulation/Simulation.cpp
        src/application/simulation/Simulation.h
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
        src/domain/cell/OncoMatrix.h)
target_link_libraries(cellSim PRIVATE cxxopts)

enable_testing()
add_subdirectory(tests)