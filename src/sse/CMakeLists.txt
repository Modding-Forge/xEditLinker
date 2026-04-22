cmake_minimum_required(VERSION 3.21)
project(xedit_linker_sse VERSION 1.0.1 LANGUAGES CXX)

set(ENABLE_SKYRIM_SE ON  CACHE BOOL "Support Skyrim SE (pre-1.6)")
set(ENABLE_SKYRIM_AE ON  CACHE BOOL "Support Skyrim AE (1.6+)")
set(ENABLE_SKYRIM_VR OFF CACHE BOOL "Support Skyrim VR")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(SKSEPlugin)
