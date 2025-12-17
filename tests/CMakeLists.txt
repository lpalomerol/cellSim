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
        ../src/domain/ports/ILogger.h
        ../src/domain/ports/CellNoise.h
        ../src/domain/adapters/RandomNoise.h
        ../src/domain/adapters/FixedNoise.h
        ../src/domain/adapters/Logger.h
        ../src/domain/adapters/LogLevel.h
        ../src/domain/adapters/NullLogger.h

        ../src/domain/cell/CellLifeStage.h
        ../src/domain/cell/InstabilityDeltas.h
        ../src/domain/cell/CellFactory.cpp
        ../src/domain/cell/CellFactory.h
        ../src/domain/cell/CellFactory_v2.cpp
        ../src/domain/cell/CellFactory_v2.h
        ../src/domain/cell/AgenticCell.cpp
        ../src/domain/cell/AgenticCell.h
        ../src/domain/cell/AgenticCell_v2.cpp
        ../src/domain/cell/AgenticCell_v2.h
        ../src/domain/gene/Gene.cpp
        ../src/domain/gene/Gene.h
        ../src/domain/gene/Genome.cpp
        ../src/domain/gene/Genome.h
        ../src/domain/gene/GenomeFactory.cpp
        ../src/domain/gene/GenomeFactory.h

        ../src/domain/signal/ISignal.h
        ../src/domain/signal/BaseSignal.h
        ../src/domain/signal/NeoplasmSignal.h
        ../src/domain/signal/ApoptosisSignal.h
        ../src/domain/signal/CellDivisionSignal.h

        ../src/domain/tissue/TissueV2.cpp
        ../src/domain/tissue/TissueV2.h
        ../src/domain/tissue/GeneticTrackingData.h
        ../src/domain/tissue/GeneticTrackingService.h

        ../src/application/config/SimulationConfig.h
        ../src/application/config/SimulationConfig.cpp
        ../src/application/simulation/Simulation.h
        ../src/application/simulation/Simulation.cpp
        ../src/application/simulation/TissueV2Adapter.h
        ../src/application/presenter/GeneticSummaryPresenter.h
        ../src/application/presenter/GeneticSummaryPresenter.cpp

        mocks/MockLogger.h
        GeneTest.cpp
        AgenticCellTest.cpp
        GenomeFactoryTest.cpp
        CellLifeStageTransitionTest.cpp
        TissueV2Test.cpp
        ThresholdTest.cpp
        CellDivisionInstabilityPropagationTest.cpp
        ApoptosisTest.cpp
        NeoplasticImmortalityTest.cpp
        SimulationTissueV2IntegrationTest.cpp
)

# Ajusta estos targets a los tuyos reales
target_link_libraries(unit_tests
        PRIVATE
        GTest::gtest_main
        GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
