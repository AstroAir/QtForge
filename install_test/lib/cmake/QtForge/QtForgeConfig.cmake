# QtForgeConfig.cmake.in - CMake package configuration file


####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was QtForgeConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

# QtForge library configuration
set(QTFORGE_VERSION "3.0.0")
set(QTFORGE_VERSION_MAJOR "3")
set(QTFORGE_VERSION_MINOR "0")
set(QTFORGE_VERSION_PATCH "0")

# Component availability
set(QTFORGE_NETWORK_FOUND )
set(QTFORGE_UI_FOUND )
set(QTFORGE_SECURITY_FOUND TRUE)  # Always available
set(QTFORGE_COMPONENTS_FOUND TRUE)  # Component architecture always available

# Find Qt dependencies
find_dependency(Qt6 REQUIRED COMPONENTS Core)

if(QTFORGE_NETWORK_FOUND)
    find_dependency(Qt6 REQUIRED COMPONENTS Network)
endif()

if(QTFORGE_UI_FOUND)
    find_dependency(Qt6 REQUIRED COMPONENTS Widgets)
endif()

# Include target files
include("${CMAKE_CURRENT_LIST_DIR}/QtForgeTargets.cmake")

# Verify that all required targets exist
check_required_components(QtForge)
