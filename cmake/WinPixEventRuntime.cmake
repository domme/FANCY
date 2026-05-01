set(_winpix_dir "${CMAKE_CURRENT_LIST_DIR}/../external/WinPixEventRuntime")

if(NOT EXISTS "${_winpix_dir}")
    message(FATAL_ERROR
        "WinPixEventRuntime not found at ${_winpix_dir}. "
        "Please ensure the directory exists with WinPixEventRuntime binaries and headers. "
        "You can obtain it from: https://devblogs.microsoft.com/pix/"
    )
endif()

add_library(WinPixEventRuntime SHARED IMPORTED GLOBAL)
set_target_properties(WinPixEventRuntime PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${_winpix_dir}/Include"
    IMPORTED_IMPLIB               "${_winpix_dir}/bin/x64/WinPixEventRuntime.lib"
    IMPORTED_LOCATION             "${_winpix_dir}/bin/x64/WinPixEventRuntime.dll"
)

unset(_winpix_dir)
