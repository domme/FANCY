cmake_minimum_required(VERSION 4.3)
project(fancy_imgui)

file(GLOB FANCY_IMGUI_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_library(fancy_imgui STATIC ${FANCY_IMGUI_SOURCES})

target_include_directories(fancy_imgui
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${FANCY_EXTERNAL_DIR}"
)

target_link_libraries(fancy_imgui
    PUBLIC
        fancy_core
        assimp::assimp
        EASTL
        glm::glm
        xxHash::xxhash
        Microsoft::DirectXShaderCompiler
        Microsoft::DirectX-Headers
        Microsoft::DirectX12-Agility
        WinPixEventRuntime
        dxgi
        d3d12
)

fancy_apply_compile_options(fancy_imgui)

set_target_properties(fancy_imgui PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${FANCY_ROOT}/lib/win64/$<CONFIG>/fancy_imgui"
    COMPILE_PDB_OUTPUT_DIRECTORY "${FANCY_BUILD_TEMP}/win64/$<CONFIG>/fancy_imgui"
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
)
