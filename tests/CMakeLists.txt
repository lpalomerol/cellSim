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
        ../src/domain/cell/CellFactory.cpp
        ../src/domain/cell/CellFactory.h
        ../src/domain/cell/AgenticCell.cpp
        ../src/domain/cell/AgenticCell.h
        ../src/domain/cell/model/InstabilityDeltas.h
        ../src/domain/cell/strategies/IInstabilityDeltaStrategy.h
        ../src/domain/cell/strategies/GenomicInstabilityDeltaStrategy.h
        ../src/domain/cell/strategies/GenomicInstabilityDeltaStrategy.cpp
        ../src/domain/cell/strategies/LognormalNoiseInstabilityStrategy.h
        ../src/domain/cell/strategies/LognormalNoiseInstabilityStrategy.cpp
        ../src/domain/cell/strategies/IViabilityStrategy.h
        ../src/domain/cell/strategies/GenomicViabilityStrategy.h
        ../src/domain/cell/strategies/GenomicViabilityStrategy.cpp
        ../src/domain/gene/Gene.cpp
        ../src/domain/gene/Gene.h
        ../src/domain/gene/GeneStatus.h
        ../src/domain/gene/GeneConstants.h
        ../src/domain/gene/Genome.cpp
        ../src/domain/gene/Genome.h
        ../src/domain/gene/GenomeFactory.cpp
        ../src/domain/gene/GenomeFactory.h

        ../src/domain/signal/ISignal.h
        ../src/domain/signal/BaseSignal.h
        ../src/domain/signal/NeoplasmSignal.h
        ../src/domain/signal/CellDivisionSignal.h

        ../src/domain/tissue/Tissue.cpp
        ../src/domain/tissue/Tissue.h
        ../src/domain/tissue/GeneticTrackingData.h
        ../src/domain/tissue/GeneticTrackingService.h

        ../src/application/config/SimulationConfig.h
        ../src/application/config/SimulationConfig.cpp
        ../src/application/simulation/Simulation.h
        ../src/application/simulation/Simulation.cpp
        ../src/application/presenter/GeneticSummaryPresenter.h
        ../src/application/presenter/GeneticSummaryPresenter.cpp
        ../src/application/bootstrapping/BootstrappingMetrics.h
        ../src/application/bootstrapping/BootstrappingMetrics.cpp

        mocks/MockLogger.h
        GeneTest.cpp
        AgenticCellTest.cpp
        GenomeFactoryTest.cpp
        CellLifeStageTransitionTest.cpp
        TissueTest.cpp
        ThresholdTest.cpp
        CellDivisionInstabilityPropagationTest.cpp
        NeoplasticImmortalityTest.cpp
        SimulationTissueV2IntegrationTest.cpp
        LognormalNoiseInstabilityStrategyTest.cpp
        RunBootstrappingTest.cpp
)

# Ajusta estos targets a los tuyos reales
target_link_libraries(unit_tests
        PRIVATE
        GTest::gtest_main
        GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
