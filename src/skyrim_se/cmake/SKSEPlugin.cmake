if(NOT WIN32)
    message(FATAL_ERROR "This plugin targets Windows only.")
endif()

# -- IPO / LTO -----------------------------------------------------------------
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION       ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

# -- Dependencies (via vcpkg) --------------------------------------------------
# DirectXTK must be found before CommonLibSSE because it is part of
# CommonLibSSE's link interface and CMake requires the target to already exist.
find_package(directxtk CONFIG REQUIRED)
find_package(CommonLibSSE CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# -- Shared library target -----------------------------------------------------
add_library("${PROJECT_NAME}" SHARED)

target_compile_features("${PROJECT_NAME}" PRIVATE cxx_std_23)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# -- Source files (auto-discovered from include/ and src/) ---------------------
include(AddCXXFiles)
add_cxx_files("${PROJECT_NAME}")

# -- Generated headers / resources ---------------------------------------------
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Plugin.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/Plugin.h"
    @ONLY)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/version.rc"
    @ONLY)

target_sources("${PROJECT_NAME}" PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/Plugin.h"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/version.rc")

# -- Precompiled header --------------------------------------------------------
target_precompile_headers("${PROJECT_NAME}" PRIVATE include/PCH.h)

# -- Include directories -------------------------------------------------------
target_include_directories("${PROJECT_NAME}"
    PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/cmake"
            "${CMAKE_CURRENT_SOURCE_DIR}/src")

# -- Link libraries ------------------------------------------------------------
target_link_libraries("${PROJECT_NAME}"
    PRIVATE
        xedit_linker_common
        CommonLibSSE::CommonLibSSE
        imgui::imgui)

# -- Target properties ---------------------------------------------------------
set_target_properties("${PROJECT_NAME}" PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    OUTPUT_NAME          "SSEEditLinker")

# -- MSVC compiler / linker flags ----------------------------------------------
if(CMAKE_GENERATOR MATCHES "Visual Studio")
    add_compile_definitions(_UNICODE)
    add_compile_options(/MP)

    target_compile_definitions("${PROJECT_NAME}" PRIVATE
        "$<$<CONFIG:DEBUG>:DEBUG>")

    target_compile_options("${PROJECT_NAME}" PRIVATE
        /sdl             # Additional security checks
        /utf-8           # UTF-8 source and execution charset
        /Zi              # Debug info (even in Release via linker flags)
        /permissive-     # Standards conformance
        /Zc:preprocessor # Conforming preprocessor
        /wd4200          # nonstandard extension: zero-sized array in struct/union
        "$<$<CONFIG:DEBUG>:>"
        "$<$<CONFIG:RELEASE>:/Zc:inline;/JMC-;/Ob3>")

    target_link_options("${PROJECT_NAME}" PRIVATE
        "$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
        "$<$<CONFIG:RELEASE>:/LTCG;/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>")
endif()

# -- Post-build: copy DLL + stub INI to Skyrim mod folder ----------------------
set(MOD_ROOT      "E:/Skyrim Modding/Portable Instances/1.6.1170 Test Instance/mods/xedit linker")
set(SKSE_PLUGINS_DIR "${MOD_ROOT}/SKSE/Plugins")
set(XEDIT_DIR        "${MOD_ROOT}/xEdit")
add_custom_command(TARGET "${PROJECT_NAME}" POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${SKSE_PLUGINS_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "$<TARGET_FILE:${PROJECT_NAME}>"
        "${SKSE_PLUGINS_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${XEDIT_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/res/xEditLink.ini"
        "${XEDIT_DIR}/xEditLink.ini"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${SKSE_PLUGINS_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/res/SSEEditLinker.ini"
        "${SKSE_PLUGINS_DIR}/SSEEditLinker.ini"
    COMMENT "Copying ${PROJECT_NAME}.dll + stub INI + config INI -> mod folder")
