cmake_minimum_required(VERSION 3.21)
project(xEdit-Linker
    VERSION     1.0.0
    DESCRIPTION "SKSE plugin that bridges the Skyrim SE console to SSEEdit"
    LANGUAGES   CXX
)

# NOTE: 32-bit and 64-bit targets CANNOT be mixed in a single CMake configure.
#   64-bit:  cmake -B build64 -A x64   (enable SE / VR / FO4 / FO4VR / SF1)
#   32-bit:  cmake -B build32 -A Win32 (enable LE  / FO3 / FNV  / TES4)
#
# SE and VR variants of the same CommonLib share a FetchContent name, so they
# must also be built in separate configure directories:
#   skyrim_se  +  skyrim_vr  → separate builds
#   fo4        +  fo4_vr     → separate builds

option(BUILD_SKYRIM_SE  "Skyrim Special Edition  (64-bit, CommonLibSSE-NG)"  ON)
option(BUILD_SKYRIM_LE  "Skyrim Legendary Edition (32-bit, SKSE32)"          OFF)
option(BUILD_SKYRIM_VR  "Skyrim VR               (64-bit, CommonLibSSE-NG)"  OFF)
option(BUILD_FO4        "Fallout 4               (64-bit, CommonLibF4)"      OFF)
option(BUILD_FO4_VR     "Fallout 4 VR            (64-bit, CommonLibF4)"      OFF)
option(BUILD_FO3        "Fallout 3               (32-bit, FOSE)"             OFF)
option(BUILD_FNV        "Fallout: New Vegas       (32-bit, NVSE)"             OFF)
option(BUILD_TES4       "Oblivion                (32-bit, OBSE/xOBSE)"       OFF)
option(BUILD_SF1        "Starfield               (64-bit, CommonLibSF)"      OFF)

if(BUILD_SKYRIM_SE AND BUILD_SKYRIM_VR)
    message(FATAL_ERROR
        "BUILD_SKYRIM_SE and BUILD_SKYRIM_VR share the CommonLibSSE-NG "
        "FetchContent name and must be configured in separate build directories.")
endif()
if(BUILD_FO4 AND BUILD_FO4_VR)
    message(FATAL_ERROR
        "BUILD_FO4 and BUILD_FO4_VR share the CommonLibF4 FetchContent name "
        "and must be configured in separate build directories.")
endif()

foreach(_t IN ITEMS BUILD_SKYRIM_SE BUILD_SKYRIM_VR BUILD_FO4 BUILD_FO4_VR BUILD_SF1)
    if(${_t} AND CMAKE_SIZEOF_VOID_P EQUAL 4)
        message(FATAL_ERROR "${_t} requires a 64-bit toolchain. Configure with -A x64.")
    endif()
endforeach()

foreach(_t IN ITEMS BUILD_SKYRIM_LE BUILD_FO3 BUILD_FNV BUILD_TES4)
    if(${_t} AND CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "${_t} requires a 32-bit toolchain. Configure with -A Win32.")
    endif()
endforeach()


add_subdirectory(common)

if(BUILD_SKYRIM_SE)
    add_subdirectory(src/skyrim_se)
endif()
if(BUILD_SKYRIM_LE)
    add_subdirectory(src/skyrim_le)
endif()
if(BUILD_SKYRIM_VR)
    add_subdirectory(src/skyrim_vr)
endif()
if(BUILD_FO4)
    add_subdirectory(src/fo4)
endif()
if(BUILD_FO4_VR)
    add_subdirectory(src/fo4_vr)
endif()
if(BUILD_FO3)
    add_subdirectory(src/fo3)
endif()
if(BUILD_FNV)
    add_subdirectory(src/fnv)
endif()
if(BUILD_TES4)
    add_subdirectory(src/tes4)
endif()
if(BUILD_SF1)
    add_subdirectory(src/sf1)
endif()
