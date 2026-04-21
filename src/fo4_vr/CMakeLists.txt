cmake_minimum_required(VERSION 3.21)
project(xedit_linker_fo4_vr VERSION 1.0.0 LANGUAGES CXX)

# Repository : https://github.com/Ryan-rsm-McKenzie/CommonLibF4
# NOTE: fo4_vr and fo4 share the FetchContent name "CommonLibF4".
#       Build each in its own configure directory (see root CMakeLists.txt).
# NOTE: CommonLibF4 does not expose a discrete VR-vs-non-VR CMake flag the
#       way CommonLibSSE-NG does.  VR-specific hooks will be implemented in
#       main.cpp once the API differences are mapped.
include(FetchContent)

FetchContent_Declare(
    CommonLibF4
    GIT_REPOSITORY https://github.com/Ryan-rsm-McKenzie/CommonLibF4.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(CommonLibF4)

add_library(xedit_linker_fo4_vr SHARED
    main.cpp
)

set_target_properties(xedit_linker_fo4_vr PROPERTIES
    CXX_STANDARD          23
    CXX_STANDARD_REQUIRED ON
    MSVC_RUNTIME_LIBRARY  "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    OUTPUT_NAME           "xedit_linker_fo4_vr"
)

target_link_libraries(xedit_linker_fo4_vr
    PRIVATE
        xedit_linker_common
        CommonLibF4::CommonLibF4
)
