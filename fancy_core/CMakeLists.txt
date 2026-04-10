cmake_minimum_required(VERSION 4.3)
project(fancy_core)

# Collect all source files, excluding the Vulkan backend (not configured in this build)
file(GLOB_RECURSE FANCY_CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Common/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Common/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Debug/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Debug/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/IO/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/IO/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Rendering/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Rendering/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/dds.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/fancy_core_precompile.h"
)
# Exclude the Vulkan backend
list(FILTER FANCY_CORE_SOURCES EXCLUDE REGEX ".*/Rendering/Vulkan/.*")

add_library(fancy_core STATIC ${FANCY_CORE_SOURCES})

target_precompile_headers(fancy_core PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/fancy_core_precompile.h"
)

target_include_directories(fancy_core
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${FANCY_EXTERNAL_DIR}"
        "${FANCY_EXTERNAL_DIR}/WinPixEventRuntime/Include"
)

target_link_libraries(fancy_core
    PUBLIC
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

fancy_apply_compile_options(fancy_core)

set_target_properties(fancy_core PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${FANCY_ROOT}/lib/win64/$<CONFIG>/fancy_core"
    COMPILE_PDB_OUTPUT_DIRECTORY "${FANCY_BUILD_TEMP}/win64/$<CONFIG>/fancy_core"
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
)
