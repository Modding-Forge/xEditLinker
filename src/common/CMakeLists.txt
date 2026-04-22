cmake_minimum_required(VERSION 3.21)
project(xedit_linker_common LANGUAGES CXX)

add_library(xedit_linker_common STATIC
    xedit_linker.cpp
)

target_include_directories(xedit_linker_common
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(xedit_linker_common PROPERTIES
    CXX_STANDARD          17
    CXX_STANDARD_REQUIRED ON
    MSVC_RUNTIME_LIBRARY  "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
)
