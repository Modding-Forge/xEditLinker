vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO alandtse/CommonLibVR
    REF 42c65076f49b482f8febf67eb1901337e2f760b6
    SHA512 4ab41826290b69d86ccefeb8aba674bcbcc62784ee5e99c102434bb69049f4f908a56de7480106a8608f835d111937c7e14a5bc043574cbe7db7830ad15567a2
    HEAD_REF ng
)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH2
    REPO ValveSoftware/openvr
    REF ebdea152f8aac77e9a6db29682b81d762159df7e
    SHA512 4fb668d933ac5b73eb4e97eb29816176e500a4eaebe2480cd0411c95edfb713d58312036f15db50884a2ef5f4ca44859e108dec2b982af9163cefcfc02531f63
    HEAD_REF master
)

file(GLOB OPENVR_FILES "${SOURCE_PATH2}/*")
file(COPY ${OPENVR_FILES} DESTINATION "${SOURCE_PATH}/extern/openvr")

vcpkg_configure_cmake(
    SOURCE_PATH "${SOURCE_PATH}"
    PREFER_NINJA
    OPTIONS
        -DBUILD_TESTS=off
        -DSKSE_SUPPORT_XBYAK=on
)

vcpkg_install_cmake()
vcpkg_cmake_config_fixup(PACKAGE_NAME CommonLibSSE CONFIG_PATH lib/cmake)
vcpkg_copy_pdbs()

file(INSTALL "${SOURCE_PATH2}/headers/openvr.h" DESTINATION ${CURRENT_PACKAGES_DIR}/include)

# Install openvr_api.lib into the packages so the cmake targets can reference it
# via ${_IMPORT_PREFIX}/lib/ - a path guaranteed to have no special characters -
# instead of the vcpkg buildtrees absolute path which may contain apostrophes
# (e.g. "Lorkhan's Vision") that break MSBuild XML property function evaluation.
file(INSTALL "${SOURCE_PATH}/extern/openvr/lib/win64/openvr_api.lib"
    DESTINATION "${CURRENT_PACKAGES_DIR}/lib")
file(INSTALL "${SOURCE_PATH}/extern/openvr/lib/win64/openvr_api.lib"
    DESTINATION "${CURRENT_PACKAGES_DIR}/debug/lib")

file(GLOB CMAKE_CONFIGS "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE/CommonLibSSE/*.cmake")
file(INSTALL ${CMAKE_CONFIGS} DESTINATION "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE")
file(INSTALL "${SOURCE_PATH}/cmake/CommonLibSSE.cmake"
    DESTINATION "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE")

# Patch the generated cmake targets: replace every absolute Windows path that
# ends in openvr_api.lib with an ${_IMPORT_PREFIX}-relative reference.
# ${_IMPORT_PREFIX} is defined at the top of the targets file and resolves to
# the vcpkg_installed triplet root (no apostrophes in its path).
file(GLOB _target_files "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE/*.cmake")
foreach(_cmake_file IN LISTS _target_files)
    file(READ "${_cmake_file}" _contents)
    string(REGEX REPLACE
        "[A-Za-z]:/[^;\"<>]*openvr_api[.]lib"
        "\${_IMPORT_PREFIX}/lib/openvr_api.lib"
        _contents "${_contents}")
    file(WRITE "${_cmake_file}" "${_contents}")
endforeach()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE/CommonLibSSE")

file(
    INSTALL "${SOURCE_PATH}/LICENSE"
    DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
    RENAME copyright)

# Suppress absolute-path warning - the remaining absolute paths are in
# CommonLibSSE's own installed cmake files which we cannot control upstream.
set(VCPKG_POLICY_SKIP_ABSOLUTE_PATHS_CHECK enabled)
