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
        ../src/domain/ports/ICell.h
        ../src/domain/ports/INoiseSource.h
        ../src/domain/ports/CellNoise.h
        ../src/domain/adapters/RandomNoise.h
        ../src/domain/adapters/FixedNoise.h

        ../src/domain/cell/CellFactory.cpp
        ../src/domain/cell/CellFactory.h
        ../src/domain/cell/AgenticCell.cpp
        ../src/domain/cell/AgenticCell.h
        ../src/domain/gene/Gene.cpp
        ../src/domain/gene/Gene.h
        ../src/domain/gene/Genome.cpp
        ../src/domain/gene/Genome.h
        ../src/domain/gene/GenomeFactory.cpp
        ../src/domain/signal/ApoptosisSignal.h
        ../src/domain/tissue/Tissue.cpp
        GeneTest.cpp
        ExperimentalTrackingTest.cpp
        AgenticCellTest.cpp
        GenomeTest.cpp
        GenomeFactoryTest.cpp
        GenomeMutateTest.cpp
        ThresholdTest.cpp
        TissueTest.cpp
        TissueIdTest.cpp
        NeoplasmSignalTest.cpp
        AgenticCellSignalTest.cpp
        TissueIntegrationSignalTest.cpp
        AgenticCellReceiveMessageTest.cpp
        CellDivisionIntegrationTest.cpp
        ApoptosisTest.cpp
)

# Ajusta estos targets a los tuyos reales
target_link_libraries(unit_tests
        PRIVATE
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
