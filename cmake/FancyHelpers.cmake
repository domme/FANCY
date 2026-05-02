set(_fancy_vcpkg_installed "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
set(FANCY_VCPKG_BIN         "${_fancy_vcpkg_installed}/bin"       CACHE INTERNAL "")
set(FANCY_VCPKG_BIN_DEBUG   "${_fancy_vcpkg_installed}/debug/bin" CACHE INTERNAL "")
unset(_fancy_vcpkg_installed)

# ---------------------------------------------------------------------------
# clang-format target
# ---------------------------------------------------------------------------
find_program(CLANG_FORMAT_EXE
    NAMES clang-format
    HINTS
        "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/x64/bin"
        "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Professional/VC/Tools/Llvm/x64/bin"
        "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Enterprise/VC/Tools/Llvm/x64/bin"
)

if(CLANG_FORMAT_EXE)
    # Collect all project .h/.cpp files (skip external/vcpkg/stb)
    file(GLOB_RECURSE _ALL_FANCY_SOURCES
        "${FANCY_ROOT}/fancy_core/*.cpp"  "${FANCY_ROOT}/fancy_core/*.h"
        "${FANCY_ROOT}/fancy_imgui/*.cpp" "${FANCY_ROOT}/fancy_imgui/*.h"
        "${FANCY_ROOT}/Tests/*.cpp"       "${FANCY_ROOT}/Tests/*.h"
    )
    add_custom_target(fancy_format
        COMMAND ${CLANG_FORMAT_EXE} -i ${_ALL_FANCY_SOURCES}
        COMMENT "Running clang-format on Fancy sources"
        VERBATIM
    )
    unset(_ALL_FANCY_SOURCES)
else()
    message(STATUS "clang-format not found — fancy_format target unavailable")
endif()


# Applies the common MSVC compile options that all Fancy targets must share.
# IMPORTANT: PCH consumers (REUSE_FROM fancy_core) must have the same flags as the PCH creator.
function(fancy_apply_compile_options target)
    target_compile_options(${target} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W3 /MP /Zc:__cplusplus>
    )
endfunction()

# Copies a single DLL to the target's output directory.
# Uses generator expressions to select debug vs release bin dirs.
function(_fancy_copy_dll target dll_release dll_debug)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<IF:$<CONFIG:Debug>,${FANCY_VCPKG_BIN_DEBUG}/${dll_debug},${FANCY_VCPKG_BIN}/${dll_release}>"
            "$<TARGET_FILE_DIR:${target}>"
        COMMENT "Copying $<IF:$<CONFIG:Debug>,${dll_debug},${dll_release}> to output dir"
    )
endfunction()

# Copies a DLL that has the same name in both configs (e.g. from the debug/bin dir in debug).
function(_fancy_copy_dll_same target dll)
    _fancy_copy_dll(${target} "${dll}" "${dll}")
endfunction()

# Copies a DLL that only exists in the release bin dir regardless of config.
function(_fancy_copy_dll_release_only target dll)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${FANCY_VCPKG_BIN}/${dll}"
            "$<TARGET_FILE_DIR:${target}>"
        COMMENT "Copying ${dll} to output dir"
    )
endfunction()

# Copies all required runtime DLLs for a Fancy application target.
function(fancy_copy_runtime_dlls target)
    # Assimp (debug and release have different DLL names)
    _fancy_copy_dll(${target} "assimp-vc143-mt.dll"  "assimp-vc143-mtd.dll")
    _fancy_copy_dll_same(${target} "kubazip.dll")
    _fancy_copy_dll_same(${target} "minizip.dll")
    _fancy_copy_dll_same(${target} "poly2tri.dll")
    _fancy_copy_dll_same(${target} "pugixml.dll")
    _fancy_copy_dll(${target} "z.dll" "zd.dll")

    # XXHash
    _fancy_copy_dll_same(${target} "xxhash.dll")

    # DXC
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${FANCY_VCPKG_BIN}/dxcompiler.dll"
            "$<TARGET_FILE_DIR:${target}>"
        COMMENT "Copying dxcompiler.dll to output dir"
    )
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${FANCY_VCPKG_BIN}/dxil.dll"
            "$<TARGET_FILE_DIR:${target}>"
        COMMENT "Copying dxil.dll to output dir"
    )

    # DirectX12 Agility SDK — D3D12Core.dll lives in debug/bin for debug, bin for release
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<IF:$<CONFIG:Debug>,${FANCY_VCPKG_BIN_DEBUG},${FANCY_VCPKG_BIN}>/D3D12Core.dll"
            "$<TARGET_FILE_DIR:${target}>"
        COMMENT "Copying D3D12Core.dll to output dir"
    )

    # WinPixEventRuntime (single binary, no debug variant; downloaded separately)
    set(_winpix_dll "${FANCY_ROOT}/external/WinPixEventRuntime/bin/x64/WinPixEventRuntime.dll")
    if(EXISTS "${_winpix_dll}")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${_winpix_dll}"
                "$<TARGET_FILE_DIR:${target}>"
            COMMENT "Copying WinPixEventRuntime.dll to output dir"
        )
    endif()
    unset(_winpix_dll)
endfunction()
