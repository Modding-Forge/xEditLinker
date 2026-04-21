cmake_minimum_required(VERSION 3.21)
project(xedit_linker_fo4 VERSION 1.0.0 LANGUAGES CXX)

# Repository : https://github.com/Ryan-rsm-McKenzie/CommonLibF4
# vcpkg alt  : vcpkg install commonlibf4
include(FetchContent)

FetchContent_Declare(
    CommonLibF4
    GIT_REPOSITORY https://github.com/Ryan-rsm-McKenzie/CommonLibF4.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(CommonLibF4)

add_library(xedit_linker_fo4 SHARED
    main.cpp
)

set_target_properties(xedit_linker_fo4 PROPERTIES
    CXX_STANDARD          23
    CXX_STANDARD_REQUIRED ON
    MSVC_RUNTIME_LIBRARY  "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    OUTPUT_NAME           "xedit_linker_fo4"
)

target_link_libraries(xedit_linker_fo4
    PRIVATE
        xedit_linker_common
        CommonLibF4::CommonLibF4
)
