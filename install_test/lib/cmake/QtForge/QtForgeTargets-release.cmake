#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "QtForge::Core" for configuration "Release"
set_property(TARGET QtForge::Core APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(QtForge::Core PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/libqtforge-core.dll.a"
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "Qt6::Core;Qt6::Network;Qt6::Widgets"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/libqtforge-core.dll"
  )

list(APPEND _cmake_import_check_targets QtForge::Core )
list(APPEND _cmake_import_check_files_for_QtForge::Core "${_IMPORT_PREFIX}/lib/libqtforge-core.dll.a" "${_IMPORT_PREFIX}/bin/libqtforge-core.dll" )

# Import target "QtForge::Security" for configuration "Release"
set_property(TARGET QtForge::Security APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(QtForge::Security PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/libqtforge-security.dll.a"
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "Qt6::Core;QtForge::Core"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/libqtforge-security.dll"
  )

list(APPEND _cmake_import_check_targets QtForge::Security )
list(APPEND _cmake_import_check_files_for_QtForge::Security "${_IMPORT_PREFIX}/lib/libqtforge-security.dll.a" "${_IMPORT_PREFIX}/bin/libqtforge-security.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
