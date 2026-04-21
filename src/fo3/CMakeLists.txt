cmake_minimum_required(VERSION 3.21)
project(xedit_linker_fo3 VERSION 1.0.0 LANGUAGES CXX)

# Download : https://fose.silverlock.org/
# Extract the archive and set FOSE_SDK_PATH to the directory that contains
# the 'fose/' header tree.
set(FOSE_SDK_PATH "" CACHE PATH
    "Path to the FOSE SDK directory (https://fose.silverlock.org/)")

add_library(xedit_linker_fo3 SHARED
    main.cpp
)

set_target_properties(xedit_linker_fo3 PROPERTIES
    CXX_STANDARD          17
    CXX_STANDARD_REQUIRED ON
    MSVC_RUNTIME_LIBRARY  "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    OUTPUT_NAME           "xedit_linker_fo3"
)

target_link_libraries(xedit_linker_fo3
    PRIVATE xedit_linker_common
)

if(FOSE_SDK_PATH)
    target_include_directories(xedit_linker_fo3
        PRIVATE "${FOSE_SDK_PATH}")
    target_compile_definitions(xedit_linker_fo3
        PRIVATE FOSE_AVAILABLE)
else()
    message(WARNING
        "[fo3] FOSE_SDK_PATH is not set - plugin entry points will not be "
        "compiled.  Download FOSE from https://fose.silverlock.org/ and pass "
        "-DFOSE_SDK_PATH=<path> to CMake.")
endif()
