cmake_minimum_required(VERSION 3.21)
project(xedit_linker_fnv VERSION 1.0.0 LANGUAGES CXX)

# Repository : https://github.com/xNVSE/NVSE
# Clone / download the repo and set NVSE_SDK_PATH to the directory that
# contains the 'nvse/' header tree.
set(NVSE_SDK_PATH "" CACHE PATH
    "Path to the NVSE SDK directory (https://github.com/xNVSE/NVSE)")

add_library(xedit_linker_fnv SHARED
    main.cpp
)

set_target_properties(xedit_linker_fnv PROPERTIES
    CXX_STANDARD          17
    CXX_STANDARD_REQUIRED ON
    MSVC_RUNTIME_LIBRARY  "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    OUTPUT_NAME           "xedit_linker_fnv"
)

target_link_libraries(xedit_linker_fnv
    PRIVATE xedit_linker_common
)

if(NVSE_SDK_PATH)
    target_include_directories(xedit_linker_fnv
        PRIVATE "${NVSE_SDK_PATH}")
    target_compile_definitions(xedit_linker_fnv
        PRIVATE NVSE_AVAILABLE)
else()
    message(WARNING
        "[fnv] NVSE_SDK_PATH is not set - plugin entry points will not be "
        "compiled.  Clone NVSE from https://github.com/xNVSE/NVSE and pass "
        "-DNVSE_SDK_PATH=<path> to CMake.")
endif()
