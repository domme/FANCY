cmake_minimum_required(VERSION 4.3)

# vcpkg toolchain must be set before project() when used standalone.
# When used as add_subdirectory() from a parent project, the parent handles this.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(CMAKE_TOOLCHAIN_FILE
        "${CMAKE_CURRENT_SOURCE_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "vcpkg toolchain"
    )
    set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "vcpkg triplet")
endif()

project(Fancy CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Absolute path variables shared by all sub-projects
set(FANCY_ROOT        "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "")
set(FANCY_EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external" CACHE INTERNAL "")
set(FANCY_BUILD_TEMP  "${CMAKE_CURRENT_SOURCE_DIR}/_build_temp" CACHE INTERNAL "")

# The directx12-agility vcpkg config calls find_library(d3d12) which needs the
# Windows SDK UM library directory to be on CMAKE_LIBRARY_PATH.
# The VS generator doesn't add it automatically for find_library() searches.
if(MSVC AND CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
    set(_winsdk_um_lib
        "$ENV{ProgramFiles\(x86\)}/Windows Kits/10/Lib/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/um/x64"
    )
    if(EXISTS "${_winsdk_um_lib}")
        list(PREPEND CMAKE_LIBRARY_PATH "${_winsdk_um_lib}")
    endif()
    unset(_winsdk_um_lib)
endif()

# Find all vcpkg-managed packages
find_package(assimp            CONFIG REQUIRED)
find_package(EASTL             CONFIG REQUIRED)
find_package(glm               CONFIG REQUIRED)
find_package(xxHash            CONFIG REQUIRED)
find_package(directx-dxc       CONFIG REQUIRED)
find_package(directx-headers   CONFIG REQUIRED)
find_package(directx12-agility CONFIG REQUIRED)

# Load helpers and external imported targets
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/WinPixEventRuntime.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/FancyHelpers.cmake")

add_subdirectory(fancy_core)
add_subdirectory(fancy_imgui)

# Tests is only built when Fancy is the root project (standalone solution)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_subdirectory(Tests)
endif()
