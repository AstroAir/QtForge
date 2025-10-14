# QtForgePackaging.cmake Packaging configuration module for QtForge Provides
# comprehensive packaging support for multiple platforms and formats

include_guard(GLOBAL)

# Include dependencies
include(${CMAKE_CURRENT_LIST_DIR}/QtForgePlatform.cmake)

#[=======================================================================[.rst:
qtforge_configure_packaging
---------------------------

Configures CPack packaging for QtForge with platform-specific settings.
#]=======================================================================]
function(qtforge_configure_packaging)
  # Only configure packaging if requested
  if(NOT QTFORGE_CREATE_PACKAGES)
    return()
  endif()

  # Include CPack
  include(CPack)

  # Basic package information
  set(CPACK_PACKAGE_NAME
      "QtForge"
      PARENT_SCOPE)
  set(CPACK_PACKAGE_VENDOR
      "QtForge Project"
      PARENT_SCOPE)
  set(CPACK_PACKAGE_VERSION
      ${PROJECT_VERSION}
      PARENT_SCOPE)
  set(CPACK_PACKAGE_VERSION_MAJOR
      ${PROJECT_VERSION_MAJOR}
      PARENT_SCOPE)
  set(CPACK_PACKAGE_VERSION_MINOR
      ${PROJECT_VERSION_MINOR}
      PARENT_SCOPE)
  set(CPACK_PACKAGE_VERSION_PATCH
      ${PROJECT_VERSION_PATCH}
      PARENT_SCOPE)
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
      "Modern C++ Plugin System for Qt Applications"
      PARENT_SCOPE)
  set(CPACK_PACKAGE_DESCRIPTION
      "QtForge is a modern, type-safe plugin system for Qt applications with modular component architecture, comprehensive security features, and cross-platform support."
      PARENT_SCOPE)
  set(CPACK_PACKAGE_HOMEPAGE_URL
      "https://github.com/qtforge/qtforge"
      PARENT_SCOPE)
  set(CPACK_PACKAGE_CONTACT
      "QtForge Team <team@qtforge.org>"
      PARENT_SCOPE)

  # Package files and directories
  set(CPACK_PACKAGE_DIRECTORY
      "${CMAKE_BINARY_DIR}/packages"
      PARENT_SCOPE)
  set(CPACK_PACKAGE_FILE_NAME
      "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}"
      PARENT_SCOPE)

  # Resource files
  if(EXISTS "${CMAKE_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_LICENSE
        "${CMAKE_SOURCE_DIR}/LICENSE"
        PARENT_SCOPE)
  endif()
  if(EXISTS "${CMAKE_SOURCE_DIR}/README.md")
    set(CPACK_RESOURCE_FILE_README
        "${CMAKE_SOURCE_DIR}/README.md"
        PARENT_SCOPE)
    set(CPACK_RESOURCE_FILE_WELCOME
        "${CMAKE_SOURCE_DIR}/README.md"
        PARENT_SCOPE)
  endif()

  # Component definitions for selective installation
  if(QTFORGE_PACKAGE_COMPONENTS)
    qtforge_configure_package_components()
  endif()

  # Platform-specific packaging configuration
  if(QTFORGE_IS_WINDOWS)
    qtforge_configure_windows_packaging()
  elseif(QTFORGE_IS_MACOS)
    qtforge_configure_macos_packaging()
  elseif(QTFORGE_IS_LINUX)
    qtforge_configure_linux_packaging()
  elseif(QTFORGE_IS_ANDROID)
    qtforge_configure_android_packaging()
  endif()

endfunction()

#[=======================================================================[.rst:
qtforge_configure_package_components
-----------------------------------

Configures CPack components for selective installation.
#]=======================================================================]
function(qtforge_configure_package_components)
  set(CPACK_COMPONENTS_ALL
      Core Security Network UI Examples Documentation
      PARENT_SCOPE)

  # Core component (required)
  set(CPACK_COMPONENT_CORE_DISPLAY_NAME
      "QtForge Core Library"
      PARENT_SCOPE)
  set(CPACK_COMPONENT_CORE_DESCRIPTION
      "Core plugin system with essential functionality"
      PARENT_SCOPE)
  set(CPACK_COMPONENT_CORE_REQUIRED
      TRUE
      PARENT_SCOPE)

  # Security component
  set(CPACK_COMPONENT_SECURITY_DISPLAY_NAME
      "Security Features"
      PARENT_SCOPE)
  set(CPACK_COMPONENT_SECURITY_DESCRIPTION
      "Advanced security and validation features"
      PARENT_SCOPE)
  set(CPACK_COMPONENT_SECURITY_DEPENDS
      Core
      PARENT_SCOPE)

  # Network component
  if(QTFORGE_BUILD_NETWORK)
    set(CPACK_COMPONENT_NETWORK_DISPLAY_NAME
        "Network Support"
        PARENT_SCOPE)
    set(CPACK_COMPONENT_NETWORK_DESCRIPTION
        "Network-enabled plugin functionality"
        PARENT_SCOPE)
    set(CPACK_COMPONENT_NETWORK_DEPENDS
        Core
        PARENT_SCOPE)
  endif()

  # UI component
  if(QTFORGE_BUILD_UI)
    set(CPACK_COMPONENT_UI_DISPLAY_NAME
        "UI Support"
        PARENT_SCOPE)
    set(CPACK_COMPONENT_UI_DESCRIPTION
        "User interface plugin functionality"
        PARENT_SCOPE)
    set(CPACK_COMPONENT_UI_DEPENDS
        Core
        PARENT_SCOPE)
  endif()

  # Examples component
  if(QTFORGE_BUILD_EXAMPLES)
    set(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME
        "Example Plugins"
        PARENT_SCOPE)
    set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION
        "Example plugins and demonstrations"
        PARENT_SCOPE)
    set(CPACK_COMPONENT_EXAMPLES_DEPENDS
        Core
        PARENT_SCOPE)
  endif()

  # Documentation component
  if(QTFORGE_BUILD_DOCS)
    set(CPACK_COMPONENT_DOCUMENTATION_DISPLAY_NAME
        "Documentation"
        PARENT_SCOPE)
    set(CPACK_COMPONENT_DOCUMENTATION_DESCRIPTION
        "API documentation and guides"
        PARENT_SCOPE)
  endif()
endfunction()

#[=======================================================================[.rst:
Platform-specific packaging configuration functions
#]=======================================================================]

function(qtforge_configure_windows_packaging)
  # Windows-specific packaging
  set(CPACK_GENERATOR
      "NSIS;ZIP;WIX"
      PARENT_SCOPE)

  # NSIS (Nullsoft Scriptable Install System) configuration
  set(CPACK_NSIS_DISPLAY_NAME
      "QtForge Library"
      PARENT_SCOPE)
  set(CPACK_NSIS_PACKAGE_NAME
      "QtForge"
      PARENT_SCOPE)
  set(CPACK_NSIS_CONTACT
      "${CPACK_PACKAGE_CONTACT}"
      PARENT_SCOPE)
  set(CPACK_NSIS_URL_INFO_ABOUT
      "${CPACK_PACKAGE_HOMEPAGE_URL}"
      PARENT_SCOPE)
  set(CPACK_NSIS_HELP_LINK
      "${CPACK_PACKAGE_HOMEPAGE_URL}"
      PARENT_SCOPE)
  set(CPACK_NSIS_MODIFY_PATH
      ON
      PARENT_SCOPE)
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL
      ON
      PARENT_SCOPE)

  # Create desktop shortcut for documentation
  if(QTFORGE_BUILD_DOCS)
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$DESKTOP\\\\QtForge Documentation.lnk' '$INSTDIR\\\\share\\\\doc\\\\qtforge\\\\README.md'"
        PARENT_SCOPE)
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete '$DESKTOP\\\\QtForge Documentation.lnk'"
        PARENT_SCOPE)
  endif()

  # WiX configuration
  set(CPACK_WIX_UPGRADE_GUID
      "12345678-1234-1234-1234-123456789012"
      PARENT_SCOPE)
  set(CPACK_WIX_PRODUCT_GUID
      "87654321-4321-4321-4321-210987654321"
      PARENT_SCOPE)
  set(CPACK_WIX_PRODUCT_ICON
      "${CMAKE_SOURCE_DIR}/resources/qtforge.ico"
      PARENT_SCOPE)
  set(CPACK_WIX_UI_BANNER
      "${CMAKE_SOURCE_DIR}/resources/wix_banner.bmp"
      PARENT_SCOPE)
  set(CPACK_WIX_UI_DIALOG
      "${CMAKE_SOURCE_DIR}/resources/wix_dialog.bmp"
      PARENT_SCOPE)
endfunction()

function(qtforge_configure_macos_packaging)
  # macOS-specific packaging
  set(CPACK_GENERATOR
      "DragNDrop;TGZ;productbuild"
      PARENT_SCOPE)

  # DragNDrop (DMG) configuration
  set(CPACK_DMG_VOLUME_NAME
      "QtForge ${PROJECT_VERSION}"
      PARENT_SCOPE)
  set(CPACK_DMG_FORMAT
      "UDBZ"
      PARENT_SCOPE)

  if(EXISTS "${CMAKE_SOURCE_DIR}/resources/dmg_background.png")
    set(CPACK_DMG_BACKGROUND_IMAGE
        "${CMAKE_SOURCE_DIR}/resources/dmg_background.png"
        PARENT_SCOPE)
  endif()

  if(EXISTS "${CMAKE_SOURCE_DIR}/packaging/macos/setup_dmg.applescript")
    set(CPACK_DMG_DS_STORE_SETUP_SCRIPT
        "${CMAKE_SOURCE_DIR}/packaging/macos/setup_dmg.applescript"
        PARENT_SCOPE)
  endif()

  # ProductBuild (PKG) configuration
  set(CPACK_PRODUCTBUILD_IDENTITY_NAME
      "Developer ID Installer: QtForge"
      PARENT_SCOPE)

  # Code signing
  if(QTFORGE_ENABLE_MACOS_CODESIGN)
    set(CPACK_PRODUCTBUILD_KEYCHAIN_PATH
        ""
        PARENT_SCOPE)
  endif()
endfunction()

function(qtforge_configure_linux_packaging)
  # Linux-specific packaging
  set(CPACK_GENERATOR
      "DEB;RPM;TGZ;STGZ"
      PARENT_SCOPE)

  # Common Linux settings
  set(CPACK_PACKAGE_RELOCATABLE
      FALSE
      PARENT_SCOPE)
  set(CPACK_PACKAGE_VENDOR
      "QtForge Project"
      PARENT_SCOPE)

  # Debian package configuration
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER
      "${CPACK_PACKAGE_CONTACT}"
      PARENT_SCOPE)
  set(CPACK_DEBIAN_PACKAGE_SECTION
      "libs"
      PARENT_SCOPE)
  set(CPACK_DEBIAN_PACKAGE_PRIORITY
      "optional"
      PARENT_SCOPE)
  set(CPACK_DEBIAN_PACKAGE_HOMEPAGE
      "${CPACK_PACKAGE_HOMEPAGE_URL}"
      PARENT_SCOPE)

  # Debian dependencies
  set(CPACK_DEBIAN_PACKAGE_DEPENDS
      "libqt6core6 (>= 6.0.0)"
      PARENT_SCOPE)
  if(QTFORGE_BUILD_NETWORK)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS
        "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libqt6network6"
        PARENT_SCOPE)
  endif()
  if(QTFORGE_BUILD_UI)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS
        "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libqt6widgets6"
        PARENT_SCOPE)
  endif()

  # RPM package configuration
  set(CPACK_RPM_PACKAGE_SUMMARY
      "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}"
      PARENT_SCOPE)
  set(CPACK_RPM_PACKAGE_DESCRIPTION
      "${CPACK_PACKAGE_DESCRIPTION}"
      PARENT_SCOPE)
  set(CPACK_RPM_PACKAGE_GROUP
      "System Environment/Libraries"
      PARENT_SCOPE)
  set(CPACK_RPM_PACKAGE_LICENSE
      "MIT"
      PARENT_SCOPE)
  set(CPACK_RPM_PACKAGE_URL
      "${CPACK_PACKAGE_HOMEPAGE_URL}"
      PARENT_SCOPE)

  # RPM dependencies
  set(CPACK_RPM_PACKAGE_REQUIRES
      "qt6-qtbase >= 6.0.0"
      PARENT_SCOPE)

  # Set architecture
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE
        "amd64"
        PARENT_SCOPE)
    set(CPACK_RPM_PACKAGE_ARCHITECTURE
        "x86_64"
        PARENT_SCOPE)
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i[3-6]86")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE
        "i386"
        PARENT_SCOPE)
    set(CPACK_RPM_PACKAGE_ARCHITECTURE
        "i686"
        PARENT_SCOPE)
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE
        "arm64"
        PARENT_SCOPE)
    set(CPACK_RPM_PACKAGE_ARCHITECTURE
        "aarch64"
        PARENT_SCOPE)
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE
        "armhf"
        PARENT_SCOPE)
    set(CPACK_RPM_PACKAGE_ARCHITECTURE
        "armv7hl"
        PARENT_SCOPE)
  endif()

  # Post-install scripts
  if(EXISTS "${CMAKE_SOURCE_DIR}/packaging/debian/postinst")
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
        "${CMAKE_SOURCE_DIR}/packaging/debian/postinst;${CMAKE_SOURCE_DIR}/packaging/debian/prerm"
        PARENT_SCOPE)
  endif()

  if(EXISTS "${CMAKE_SOURCE_DIR}/packaging/rpm/postinstall.sh")
    set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE
        "${CMAKE_SOURCE_DIR}/packaging/rpm/postinstall.sh"
        PARENT_SCOPE)
    set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE
        "${CMAKE_SOURCE_DIR}/packaging/rpm/preuninstall.sh"
        PARENT_SCOPE)
  endif()
endfunction()

function(qtforge_configure_android_packaging)
  # Android-specific packaging
  set(CPACK_GENERATOR
      "TGZ"
      PARENT_SCOPE)

  # Android packages are typically handled by Qt's androiddeployqt tool
endfunction()

#[=======================================================================[.rst:
qtforge_add_package_target
--------------------------

Adds a custom target for creating packages.
#]=======================================================================]
function(qtforge_add_package_target)
  if(NOT QTFORGE_CREATE_PACKAGES)
    return()
  endif()

  add_custom_target(
    package-all
    COMMAND ${CMAKE_CPACK_COMMAND} --config CPackConfig.cmake
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Creating all packages"
    VERBATIM)

  # Add platform-specific package targets
  if(QTFORGE_IS_WINDOWS)
    add_custom_target(
      package-nsis
      COMMAND ${CMAKE_CPACK_COMMAND} -G NSIS
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Creating NSIS installer"
      VERBATIM)

    add_custom_target(
      package-wix
      COMMAND ${CMAKE_CPACK_COMMAND} -G WIX
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Creating WiX installer"
      VERBATIM)
  elseif(QTFORGE_IS_LINUX)
    add_custom_target(
      package-deb
      COMMAND ${CMAKE_CPACK_COMMAND} -G DEB
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Creating Debian package"
      VERBATIM)

    add_custom_target(
      package-rpm
      COMMAND ${CMAKE_CPACK_COMMAND} -G RPM
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Creating RPM package"
      VERBATIM)
  elseif(QTFORGE_IS_MACOS)
    add_custom_target(
      package-dmg
      COMMAND ${CMAKE_CPACK_COMMAND} -G DragNDrop
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Creating DMG package"
      VERBATIM)
  endif()
endfunction()
