cmake_minimum_required(VERSION 3.28.3)
project(tissue_scale CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(tissue_scale_sim src/tissue_scale_sim.cpp)
target_include_directories(tissue_scale_sim PUBLIC src)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(tissue_scale_sim PRIVATE OpenMP::OpenMP_CXX)
endif()

# Output directly to build/ and make sure bin/ also has the binary for standalone legacy scripts
set_target_properties(tissue_scale_sim PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_custom_command(TARGET tissue_scale_sim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:tissue_scale_sim>" "${CMAKE_CURRENT_SOURCE_DIR}/bin/tissue_scale_sim"
    COMMENT "Copying tissue_scale_sim to tissue_scale/bin/"
)
