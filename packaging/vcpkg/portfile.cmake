# QtPlugin vcpkg portfile
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO QtForge/QtPlugin
    REF v3.0.0
    SHA512 0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
    HEAD_REF main
)

# Check for required dependencies
vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        network     QTPLUGIN_BUILD_NETWORK
        ui          QTPLUGIN_BUILD_UI
        examples    QTPLUGIN_BUILD_EXAMPLES
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DQTPLUGIN_BUILD_TESTS=OFF
        ${FEATURE_OPTIONS}
    OPTIONS_DEBUG
        -DQTPLUGIN_ENABLE_COMPONENT_LOGGING=ON
)

vcpkg_cmake_build()

# Run tests in release mode only
if(NOT VCPKG_TARGET_IS_WINDOWS OR NOT VCPKG_TARGET_ARCHITECTURE STREQUAL "x86")
    vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/QtPlugin)
endif()

vcpkg_cmake_install()

# Remove debug includes
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

# Handle copyright
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

# Copy usage file
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")

# Fix pkg-config files
vcpkg_fixup_pkgconfig()
