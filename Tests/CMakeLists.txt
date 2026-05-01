cmake_minimum_required(VERSION 4.3)
project(Tests)

file(GLOB TESTS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

# Organize source files into filters based on directory structure
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${TESTS_SOURCES})

add_executable(Tests WIN32 ${TESTS_SOURCES})

target_include_directories(Tests
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${FANCY_EXTERNAL_DIR}"
)

target_link_libraries(Tests
    PRIVATE
        fancy_core
        fancy_imgui
)

target_precompile_headers(Tests REUSE_FROM fancy_core)
fancy_apply_compile_options(Tests)

set_target_properties(Tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${FANCY_ROOT}/bin/win64/$<CONFIG>/Tests"
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    VS_DEBUGGER_WORKING_DIRECTORY "${FANCY_ROOT}/bin/win64/$<CONFIG>/Tests"
)

fancy_copy_runtime_dlls(Tests)
