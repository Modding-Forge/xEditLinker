cmake_minimum_required(VERSION 3.21)
project(xedit_linker_sf1 VERSION 1.0.0 LANGUAGES CXX)

# Repository : https://github.com/Starfield-Reverse-Engineering/CommonLibSF
# vcpkg alt  : vcpkg install commonlibsf
include(FetchContent)

FetchContent_Declare(
    CommonLibSF
    GIT_REPOSITORY https://github.com/Starfield-Reverse-Engineering/CommonLibSF.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(CommonLibSF)

add_library(xedit_linker_sf1 SHARED
    main.cpp
)

set_target_properties(xedit_linker_sf1 PROPERTIES
    CXX_STANDARD          23
    CXX_STANDARD_REQUIRED ON
    MSVC_RUNTIME_LIBRARY  "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    OUTPUT_NAME           "xedit_linker_sf1"
)

target_link_libraries(xedit_linker_sf1
    PRIVATE
        xedit_linker_common
        CommonLibSF::CommonLibSF
)
