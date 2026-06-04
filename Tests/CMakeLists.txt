cmake_minimum_required(VERSION 4.3)
project(Tests)

set(TESTS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/TestApplication.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/TestApplication.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Test.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Tests_main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Test_Profiler.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Test_Profiler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Test_GpuMemoryAllocator.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Test_GpuMemoryAllocator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Test_Synchronization.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Test_Synchronization.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Test_AsyncCompute.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Test_AsyncCompute.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Test_Raytracing.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Test_Raytracing.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Test_SharedQueueResourceUsage.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Test_SharedQueueResourceUsage.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Test_ImGui.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Test_ImGui.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Test_Mipmapping.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Test_Mipmapping.cpp"
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
