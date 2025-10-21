cmake_minimum_required(VERSION 3.20)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# Para evitar instalar en el sistema
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(unit_tests
        SimpleCellTests.cpp
        ../src/domain/cell/SimpleCell.cpp
        ../src/domain/cell/SimpleCell.h
        ../src/domain/cell/cell.cpp
        ../src/domain/cell/cell.h
        ../src/domain/cell/GeneCell.cpp
        ../src/domain/cell/GeneCell.h
        ../src/domain/ports/ICell.h
        ../src/domain/ports/INoiseSource.h
        ../src/domain/ports/CellNoise.h
        ../src/domain/adapters/RandomNoise.h
        ../src/domain/adapters/FixedNoise.h
        ../src/domain/gene/Gene.cpp
        ../src/domain/gene/Gene.h

        GeneCellTest.cpp

)

# Ajusta estos targets a los tuyos reales
target_link_libraries(unit_tests
        PRIVATE
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
