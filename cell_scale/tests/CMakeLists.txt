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
        TissueScaleSimulatorTest.cpp
)

target_include_directories(unit_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
        ${CMAKE_SOURCE_DIR}/tissue_scale/src
)

target_link_libraries(unit_tests
        PRIVATE
        cell_scale_lib
        GTest::gtest_main
        GTest::gmock_main
)

set_target_properties(unit_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

include(GoogleTest)
gtest_discover_tests(unit_tests)
