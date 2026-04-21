cmake_minimum_required(VERSION 3.21)
project(xedit_linker_tes4 VERSION 1.0.0 LANGUAGES CXX)

# Repository : https://github.com/llde/xOBSE
# Clone / download and set OBSE_SDK_PATH to the directory that contains
# the 'obse/' header tree.
set(OBSE_SDK_PATH "" CACHE PATH
    "Path to the OBSE/xOBSE SDK directory (https://github.com/llde/xOBSE)")

add_library(xedit_linker_tes4 SHARED
    main.cpp
)

set_target_properties(xedit_linker_tes4 PROPERTIES
    CXX_STANDARD          17
    CXX_STANDARD_REQUIRED ON
    MSVC_RUNTIME_LIBRARY  "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    OUTPUT_NAME           "xedit_linker_tes4"
)

target_link_libraries(xedit_linker_tes4
    PRIVATE xedit_linker_common
)

if(OBSE_SDK_PATH)
    target_include_directories(xedit_linker_tes4
        PRIVATE "${OBSE_SDK_PATH}")
    target_compile_definitions(xedit_linker_tes4
        PRIVATE OBSE_AVAILABLE)
else()
    message(WARNING
        "[tes4] OBSE_SDK_PATH is not set - plugin entry points will not be "
        "compiled.  Clone xOBSE from https://github.com/llde/xOBSE and pass "
        "-DOBSE_SDK_PATH=<path> to CMake.")
endif()
