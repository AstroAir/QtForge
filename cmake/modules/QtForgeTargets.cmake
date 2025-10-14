# QtForgeTargets.cmake Target creation and configuration utilities for QtForge
# Provides helper functions for creating and configuring targets

include_guard(GLOBAL)

# Include dependencies
include(${CMAKE_CURRENT_LIST_DIR}/QtForgePlatform.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/QtForgeDependencies.cmake)

#[=======================================================================[.rst:
qtforge_add_library
-------------------

Creates a QtForge library target with proper configuration.

Usage:
  qtforge_add_library(
    NAME target_name
    SOURCES source1.cpp source2.cpp
    HEADERS header1.hpp header2.hpp
    [PUBLIC_HEADERS header1.hpp header2.hpp]
    [QT_COMPONENTS Core Network Widgets]
    [DEPENDENCIES target1 target2]
    [DEFINITIONS DEFINE1 DEFINE2]
    [INCLUDE_DIRECTORIES dir1 dir2]
    [EXPORT_NAME export_name]
    [OUTPUT_NAME output_name]
    [VERSION version_string]
  )
#]=======================================================================]
function(qtforge_add_library)
  cmake_parse_arguments(
    LIB
    ""
    "NAME;EXPORT_NAME;OUTPUT_NAME;VERSION"
    "SOURCES;HEADERS;PUBLIC_HEADERS;QT_COMPONENTS;DEPENDENCIES;DEFINITIONS;INCLUDE_DIRECTORIES"
    ${ARGN})

  if(NOT LIB_NAME)
    message(FATAL_ERROR "qtforge_add_library: NAME is required")
  endif()

  if(NOT LIB_SOURCES)
    message(FATAL_ERROR "qtforge_add_library: SOURCES is required")
  endif()

  # Set defaults
  if(NOT LIB_EXPORT_NAME)
    set(LIB_EXPORT_NAME ${LIB_NAME})
  endif()

  if(NOT LIB_OUTPUT_NAME)
    string(TOLOWER ${LIB_NAME} LIB_OUTPUT_NAME)
    string(REPLACE "_" "-" LIB_OUTPUT_NAME ${LIB_OUTPUT_NAME})
  endif()

  if(NOT LIB_VERSION)
    set(LIB_VERSION ${PROJECT_VERSION})
  endif()

  # Create shared library if requested
  if(QTFORGE_BUILD_SHARED)
    set(SHARED_TARGET_NAME ${LIB_NAME})
    add_library(${SHARED_TARGET_NAME} SHARED ${LIB_SOURCES} ${LIB_HEADERS})

    # Set shared library properties
    set_target_properties(
      ${SHARED_TARGET_NAME}
      PROPERTIES VERSION ${LIB_VERSION}
                 SOVERSION ${PROJECT_VERSION_MAJOR}
                 EXPORT_NAME ${LIB_EXPORT_NAME}
                 OUTPUT_NAME ${LIB_OUTPUT_NAME}
                 AUTOMOC ON)

    # Configure the shared target
    qtforge_configure_library_target(
      ${SHARED_TARGET_NAME}
      PUBLIC_HEADERS
      ${LIB_PUBLIC_HEADERS}
      QT_COMPONENTS
      ${LIB_QT_COMPONENTS}
      DEPENDENCIES
      ${LIB_DEPENDENCIES}
      DEFINITIONS
      ${LIB_DEFINITIONS}
      INCLUDE_DIRECTORIES
      ${LIB_INCLUDE_DIRECTORIES})
  endif()

  # Create static library if requested
  if(QTFORGE_BUILD_STATIC)
    set(STATIC_TARGET_NAME ${LIB_NAME}Static)
    add_library(${STATIC_TARGET_NAME} STATIC ${LIB_SOURCES} ${LIB_HEADERS})

    # Set static library properties
    set_target_properties(
      ${STATIC_TARGET_NAME}
      PROPERTIES EXPORT_NAME ${LIB_EXPORT_NAME}Static
                 OUTPUT_NAME ${LIB_OUTPUT_NAME}-static
                 AUTOMOC ON)

    # Configure the static target
    qtforge_configure_library_target(
      ${STATIC_TARGET_NAME}
      PUBLIC_HEADERS
      ${LIB_PUBLIC_HEADERS}
      QT_COMPONENTS
      ${LIB_QT_COMPONENTS}
      DEPENDENCIES
      ${LIB_DEPENDENCIES}
      DEFINITIONS
      ${LIB_DEFINITIONS}
      INCLUDE_DIRECTORIES
      ${LIB_INCLUDE_DIRECTORIES})
  endif()

  # Create aliases
  if(TARGET ${SHARED_TARGET_NAME})
    add_library(QtForge::${LIB_EXPORT_NAME} ALIAS ${SHARED_TARGET_NAME})
  endif()

  if(TARGET ${STATIC_TARGET_NAME})
    add_library(QtForge::${LIB_EXPORT_NAME}Static ALIAS ${STATIC_TARGET_NAME})
  endif()
endfunction()

#[=======================================================================[.rst:
qtforge_configure_library_target
--------------------------------

Configures a library target with common QtForge settings.
#]=======================================================================]
function(qtforge_configure_library_target TARGET_NAME)
  cmake_parse_arguments(
    CONFIG "" ""
    "PUBLIC_HEADERS;QT_COMPONENTS;DEPENDENCIES;DEFINITIONS;INCLUDE_DIRECTORIES"
    ${ARGN})

  # Set include directories
  target_include_directories(
    ${TARGET_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:include>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

  # Add custom include directories
  if(CONFIG_INCLUDE_DIRECTORIES)
    target_include_directories(${TARGET_NAME}
                               PRIVATE ${CONFIG_INCLUDE_DIRECTORIES})
  endif()

  # Link Qt components
  if(CONFIG_QT_COMPONENTS)
    qtforge_link_qt_libraries(${TARGET_NAME} COMPONENTS ${CONFIG_QT_COMPONENTS})
  endif()

  # Link dependencies
  if(CONFIG_DEPENDENCIES)
    target_link_libraries(${TARGET_NAME} PRIVATE ${CONFIG_DEPENDENCIES})
  endif()

  # Add compile definitions
  if(CONFIG_DEFINITIONS)
    target_compile_definitions(${TARGET_NAME} PRIVATE ${CONFIG_DEFINITIONS})
  endif()

  # Add platform-specific configurations
  if(QTFORGE_IS_WINDOWS)
    qtforge_configure_windows_target(${TARGET_NAME})
  elseif(QTFORGE_IS_MACOS)
    qtforge_configure_macos_target(${TARGET_NAME})
  elseif(QTFORGE_IS_LINUX)
    qtforge_configure_linux_target(${TARGET_NAME})
  elseif(QTFORGE_IS_ANDROID)
    qtforge_configure_android_target(${TARGET_NAME})
  elseif(QTFORGE_IS_IOS)
    qtforge_configure_ios_target(${TARGET_NAME})
  endif()
endfunction()

#[=======================================================================[.rst:
qtforge_add_plugin
------------------

Creates a QtForge plugin target.

Usage:
  qtforge_add_plugin(
    NAME plugin_name
    SOURCES source1.cpp source2.cpp
    HEADERS header1.hpp header2.hpp
    [QT_COMPONENTS Core Network Widgets]
    [DEPENDENCIES target1 target2]
    [METADATA metadata.json]
  )
#]=======================================================================]
function(qtforge_add_plugin)
  cmake_parse_arguments(PLUGIN "" "NAME;METADATA"
                        "SOURCES;HEADERS;QT_COMPONENTS;DEPENDENCIES" ${ARGN})

  if(NOT PLUGIN_NAME)
    message(FATAL_ERROR "qtforge_add_plugin: NAME is required")
  endif()

  if(NOT PLUGIN_SOURCES)
    message(FATAL_ERROR "qtforge_add_plugin: SOURCES is required")
  endif()

  # Create plugin library
  add_library(${PLUGIN_NAME} MODULE ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

  # Set plugin properties
  set_target_properties(
    ${PLUGIN_NAME}
    PROPERTIES PREFIX ""
               SUFFIX ".qtplugin"
               AUTOMOC ON)

  # Configure plugin target
  qtforge_configure_library_target(
    ${PLUGIN_NAME} QT_COMPONENTS ${PLUGIN_QT_COMPONENTS} DEPENDENCIES
    ${PLUGIN_DEPENDENCIES})

  # Add plugin-specific definitions
  target_compile_definitions(${PLUGIN_NAME}
                             PRIVATE QTFORGE_PLUGIN_NAME="${PLUGIN_NAME}")

  # Copy metadata file if provided
  if(PLUGIN_METADATA AND EXISTS
                         "${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_METADATA}")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_METADATA}"
                   "${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.json" COPYONLY)
  endif()
endfunction()

#[=======================================================================[.rst:
Platform-specific target configuration functions
#]=======================================================================]

function(qtforge_configure_windows_target TARGET_NAME)
  if(QTFORGE_ENABLE_WINDOWS_MANIFEST)
    # Add Windows manifest support
    target_compile_definitions(${TARGET_NAME} PRIVATE QTFORGE_WINDOWS_MANIFEST)
  endif()

  if(QTFORGE_ENABLE_WINDOWS_RESOURCES)
    # Add Windows resources support
    target_compile_definitions(${TARGET_NAME} PRIVATE QTFORGE_WINDOWS_RESOURCES)
  endif()
endfunction()

function(qtforge_configure_macos_target TARGET_NAME)
  if(QTFORGE_ENABLE_MACOS_BUNDLE)
    set_target_properties(
      ${TARGET_NAME}
      PROPERTIES MACOSX_BUNDLE TRUE
                 MACOSX_BUNDLE_INFO_PLIST
                 "${CMAKE_SOURCE_DIR}/packaging/macos/Info.plist.in")
  endif()
endfunction()

function(qtforge_configure_linux_target TARGET_NAME)
  # Set RPATH for Linux
  set_target_properties(
    ${TARGET_NAME} PROPERTIES BUILD_RPATH_USE_ORIGIN ON INSTALL_RPATH
                                                        "$ORIGIN/../lib")
endfunction()

function(qtforge_configure_android_target TARGET_NAME)
  # Android-specific configuration
  target_compile_definitions(
    ${TARGET_NAME} PRIVATE QTFORGE_ANDROID
                           __ANDROID_API__=${QTFORGE_ANDROID_MIN_SDK_VERSION})
endfunction()

function(qtforge_configure_ios_target TARGET_NAME)
  # iOS-specific configuration
  set_target_properties(
    ${TARGET_NAME} PROPERTIES XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET
                              ${QTFORGE_IOS_DEPLOYMENT_TARGET})

  target_compile_definitions(${TARGET_NAME} PRIVATE QTFORGE_IOS)
endfunction()

#[=======================================================================[.rst:
qtforge_install_targets
-----------------------

Installs QtForge targets with proper component configuration.
#]=======================================================================]
function(qtforge_install_targets)
  cmake_parse_arguments(INSTALL "" "COMPONENT" "TARGETS" ${ARGN})

  if(NOT INSTALL_TARGETS)
    message(FATAL_ERROR "qtforge_install_targets: TARGETS is required")
  endif()

  if(NOT INSTALL_COMPONENT)
    set(INSTALL_COMPONENT Runtime)
  endif()

  # Install targets
  install(
    TARGETS ${INSTALL_TARGETS}
    EXPORT QtForgeTargets
    RUNTIME DESTINATION ${QTFORGE_INSTALL_RUNTIME_DIR}
            COMPONENT ${INSTALL_COMPONENT}
    LIBRARY DESTINATION ${QTFORGE_INSTALL_LIBRARY_DIR}
            COMPONENT ${INSTALL_COMPONENT}
    ARCHIVE DESTINATION ${QTFORGE_INSTALL_ARCHIVE_DIR} COMPONENT Development
    INCLUDES
    DESTINATION ${QTFORGE_INSTALL_INCLUDE_DIR})
endfunction()

#[=======================================================================[.rst:
qtforge_add_python_module
--------------------------

Creates a Python binding module using pybind11.

Usage:
  qtforge_add_python_module(
    NAME module_name
    SOURCES source1.cpp source2.cpp
    [HEADERS header1.hpp header2.hpp]
    [DEPENDENCIES target1 target2]
    [QT_COMPONENTS Core Network Widgets]
    [DEFINITIONS DEFINE1 DEFINE2]
    [INCLUDE_DIRECTORIES dir1 dir2]
    [MODULE_NAME python_module_name]
    [INSTALL_COMPONENT component_name]
  )
#]=======================================================================]
function(qtforge_add_python_module)
  if(NOT QTFORGE_BUILD_PYTHON_BINDINGS
     OR NOT QTFORGE_PYTHON_FOUND
     OR NOT QTFORGE_PYBIND11_FOUND)
    return()
  endif()

  cmake_parse_arguments(
    PY_MOD "" "NAME;MODULE_NAME;INSTALL_COMPONENT"
    "SOURCES;HEADERS;DEPENDENCIES;QT_COMPONENTS;DEFINITIONS;INCLUDE_DIRECTORIES"
    ${ARGN})

  if(NOT PY_MOD_NAME)
    message(FATAL_ERROR "qtforge_add_python_module: NAME is required")
  endif()

  if(NOT PY_MOD_SOURCES)
    message(FATAL_ERROR "qtforge_add_python_module: SOURCES is required")
  endif()

  # Set defaults
  if(NOT PY_MOD_MODULE_NAME)
    set(PY_MOD_MODULE_NAME ${PY_MOD_NAME})
  endif()

  if(NOT PY_MOD_INSTALL_COMPONENT)
    set(PY_MOD_INSTALL_COMPONENT PythonBindings)
  endif()

  # Create the pybind11 module
  pybind11_add_module(${PY_MOD_NAME} ${PY_MOD_SOURCES} ${PY_MOD_HEADERS})

  # Set module properties
  set_target_properties(
    ${PY_MOD_NAME}
    PROPERTIES OUTPUT_NAME ${PY_MOD_MODULE_NAME}
               CXX_VISIBILITY_PRESET hidden
               VISIBILITY_INLINES_HIDDEN ON)

  # Configure include directories
  target_include_directories(
    ${PY_MOD_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
            ${PY_MOD_INCLUDE_DIRECTORIES})

  # Add compile definitions
  if(PY_MOD_DEFINITIONS)
    target_compile_definitions(${PY_MOD_NAME} PRIVATE ${PY_MOD_DEFINITIONS})
  endif()

  # Link Qt components
  if(PY_MOD_QT_COMPONENTS)
    qtforge_link_qt_libraries(${PY_MOD_NAME} COMPONENTS ${PY_MOD_QT_COMPONENTS})
  endif()

  # Link dependencies
  if(PY_MOD_DEPENDENCIES)
    target_link_libraries(${PY_MOD_NAME} PRIVATE ${PY_MOD_DEPENDENCIES})
  endif()

  # Install the module
  if(QTFORGE_PYTHON_BINDINGS_INSTALL)
    if(QTFORGE_PYTHON_INSTALL_DIR)
      set(PYTHON_INSTALL_DIR ${QTFORGE_PYTHON_INSTALL_DIR})
    else()
      # Auto-detect Python site-packages directory
      execute_process(
        COMMAND ${QTFORGE_PYTHON_EXECUTABLE} -c
                "import site; print(site.getsitepackages()[0])"
        OUTPUT_VARIABLE PYTHON_INSTALL_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    endif()

    if(PYTHON_INSTALL_DIR)
      install(
        TARGETS ${PY_MOD_NAME}
        COMPONENT ${PY_MOD_INSTALL_COMPONENT}
        LIBRARY DESTINATION ${PYTHON_INSTALL_DIR}/qtforge)
    else()
      message(
        WARNING
          "QtForge: Could not determine Python installation directory for module ${PY_MOD_NAME}"
      )
    endif()
  endif()
endfunction()
