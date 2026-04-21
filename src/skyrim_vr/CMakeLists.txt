cmake_minimum_required(VERSION 3.21)
project(xedit_linker_skyrim_vr VERSION 1.0.0 LANGUAGES CXX)

# Repository : https://github.com/CharmedBaryon/CommonLibSSE-NG
# NOTE: skyrim_vr and skyrim_se share the FetchContent name "CommonLibSSE".
#       Build each in its own configure directory (see root CMakeLists.txt).
include(FetchContent)

FetchContent_Declare(
    CommonLibSSE
    GIT_REPOSITORY https://github.com/CharmedBaryon/CommonLibSSE-NG.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
)

set(ENABLE_SKYRIM_VR ON  CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_SE OFF CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_AE OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(CommonLibSSE)

add_library(xedit_linker_skyrim_vr SHARED
    main.cpp
)

set_target_properties(xedit_linker_skyrim_vr PROPERTIES
    CXX_STANDARD          23
    CXX_STANDARD_REQUIRED ON
    MSVC_RUNTIME_LIBRARY  "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    OUTPUT_NAME           "xedit_linker_skyrim_vr"
)

target_link_libraries(xedit_linker_skyrim_vr
    PRIVATE
        xedit_linker_common
        CommonLibSSE::CommonLibSSE
)
