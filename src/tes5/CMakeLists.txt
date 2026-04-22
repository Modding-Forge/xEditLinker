cmake_minimum_required(VERSION 3.21)
project(xedit_linker_tes5 VERSION 1.0.0 LANGUAGES CXX)

# Download : https://skse.silverlock.org/  (choose the "Classic" / LE build)
# Extract the archive and set SKSE32_SDK_PATH to the 'src' subdirectory,
# which contains the 'skse/' and 'common/' header trees.
set(SKSE32_SDK_PATH "" CACHE PATH
    "Path to the SKSE32 SDK 'src' directory (https://skse.silverlock.org/)")

add_library(xedit_linker_tes5 SHARED
    main.cpp
)

set_target_properties(xedit_linker_tes5 PROPERTIES
    CXX_STANDARD          17
    CXX_STANDARD_REQUIRED ON
    MSVC_RUNTIME_LIBRARY  "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    OUTPUT_NAME           "xedit_linker_tes5"
)

target_link_libraries(xedit_linker_tes5
    PRIVATE xedit_linker_common
)

if(SKSE32_SDK_PATH)
    target_include_directories(xedit_linker_tes5
        PRIVATE "${SKSE32_SDK_PATH}")
    target_compile_definitions(xedit_linker_tes5
        PRIVATE SKSE32_AVAILABLE)
else()
    message(WARNING
        "[tes5] SKSE32_SDK_PATH is not set - plugin entry points will "
        "not be compiled.  Download the SDK from https://skse.silverlock.org/ "
        "and pass -DSKSE32_SDK_PATH=<path> to CMake.")
endif()
