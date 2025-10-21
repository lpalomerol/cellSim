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
        domain/cell/cell.cpp
        domain/cell/cell.h
        domain/ports/ICell.h
        domain/cell/SimpleCell.cpp
        domain/cell/SimpleCell.h
        application/simulation/Simulation.cpp
        application/simulation/Simulation.h
        domain/ports/INoiseSource.h
        domain/ports/CellNoise.h
        domain/adapters/RandomNoise.h
        domain/adapters/FixedNoise.h)
target_link_libraries(cellSim PRIVATE cxxopts)