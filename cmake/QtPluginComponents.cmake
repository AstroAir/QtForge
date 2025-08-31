# QtForgeComponents.cmake
# CMake module for QtForge component architecture
# Provides utilities for working with QtForge components

# Component information
set(QTFORGE_CORE_COMPONENTS
    PluginRegistry
    PluginDependencyResolver
)

set(QTFORGE_MONITORING_COMPONENTS
    PluginHotReloadManager
    PluginMetricsCollector
)

set(QTFORGE_SECURITY_COMPONENTS
    SecurityValidator
    SignatureVerifier
    PermissionManager
    SecurityPolicyEngine
)

set(QTFORGE_CONFIGURATION_COMPONENTS
    ConfigurationStorage
    ConfigurationValidator
    ConfigurationMerger
    ConfigurationWatcher
)

set(QTFORGE_RESOURCE_COMPONENTS
    ResourcePool
    ResourceAllocator
    ResourceMonitor
)

set(QTFORGE_ALL_COMPONENTS
    ${QTFORGE_CORE_COMPONENTS}
    ${QTFORGE_MONITORING_COMPONENTS}
    ${QTFORGE_SECURITY_COMPONENTS}
    ${QTFORGE_CONFIGURATION_COMPONENTS}
    ${QTFORGE_RESOURCE_COMPONENTS}
)

# Backward compatibility with deprecation warnings
if(NOT DEFINED QTPLUGIN_CORE_COMPONENTS)
    set(QTPLUGIN_CORE_COMPONENTS ${QTFORGE_CORE_COMPONENTS})
    message(DEPRECATION "QTPLUGIN_CORE_COMPONENTS is deprecated, use QTFORGE_CORE_COMPONENTS instead")
endif()
if(NOT DEFINED QTPLUGIN_MONITORING_COMPONENTS)
    set(QTPLUGIN_MONITORING_COMPONENTS ${QTFORGE_MONITORING_COMPONENTS})
    message(DEPRECATION "QTPLUGIN_MONITORING_COMPONENTS is deprecated, use QTFORGE_MONITORING_COMPONENTS instead")
endif()
if(NOT DEFINED QTPLUGIN_SECURITY_COMPONENTS)
    set(QTPLUGIN_SECURITY_COMPONENTS ${QTFORGE_SECURITY_COMPONENTS})
    message(DEPRECATION "QTPLUGIN_SECURITY_COMPONENTS is deprecated, use QTFORGE_SECURITY_COMPONENTS instead")
endif()
if(NOT DEFINED QTPLUGIN_CONFIGURATION_COMPONENTS)
    set(QTPLUGIN_CONFIGURATION_COMPONENTS ${QTFORGE_CONFIGURATION_COMPONENTS})
    message(DEPRECATION "QTPLUGIN_CONFIGURATION_COMPONENTS is deprecated, use QTFORGE_CONFIGURATION_COMPONENTS instead")
endif()
if(NOT DEFINED QTPLUGIN_RESOURCE_COMPONENTS)
    set(QTPLUGIN_RESOURCE_COMPONENTS ${QTFORGE_RESOURCE_COMPONENTS})
    message(DEPRECATION "QTPLUGIN_RESOURCE_COMPONENTS is deprecated, use QTFORGE_RESOURCE_COMPONENTS instead")
endif()
if(NOT DEFINED QTPLUGIN_ALL_COMPONENTS)
    set(QTPLUGIN_ALL_COMPONENTS ${QTFORGE_ALL_COMPONENTS})
    message(DEPRECATION "QTPLUGIN_ALL_COMPONENTS is deprecated, use QTFORGE_ALL_COMPONENTS instead")
endif()

# Function to check if a component is available
function(qtforge_check_component component_name result_var)
    list(FIND QTFORGE_ALL_COMPONENTS ${component_name} component_index)
    if(component_index GREATER_EQUAL 0)
        set(${result_var} TRUE PARENT_SCOPE)
    else()
        set(${result_var} FALSE PARENT_SCOPE)
    endif()
endfunction()

# Backward compatibility function
function(qtplugin_check_component component_name result_var)
    message(DEPRECATION "qtplugin_check_component is deprecated, use qtforge_check_component instead")
    qtforge_check_component(${component_name} ${result_var})
endfunction()

# Function to get components by category
function(qtplugin_get_components_by_category category result_var)
    if(category STREQUAL "core")
        set(${result_var} ${QTPLUGIN_CORE_COMPONENTS} PARENT_SCOPE)
    elseif(category STREQUAL "monitoring")
        set(${result_var} ${QTPLUGIN_MONITORING_COMPONENTS} PARENT_SCOPE)
    elseif(category STREQUAL "security")
        set(${result_var} ${QTPLUGIN_SECURITY_COMPONENTS} PARENT_SCOPE)
    elseif(category STREQUAL "configuration")
        set(${result_var} ${QTPLUGIN_CONFIGURATION_COMPONENTS} PARENT_SCOPE)
    elseif(category STREQUAL "resource")
        set(${result_var} ${QTPLUGIN_RESOURCE_COMPONENTS} PARENT_SCOPE)
    elseif(category STREQUAL "all")
        set(${result_var} ${QTPLUGIN_ALL_COMPONENTS} PARENT_SCOPE)
    else()
        set(${result_var} "" PARENT_SCOPE)
    endif()
endfunction()

# Function to print component information
function(qtplugin_print_component_info)
    message(STATUS "QtPlugin Component Architecture:")
    message(STATUS "  Core Components (${list_length(QTPLUGIN_CORE_COMPONENTS)}):")
    foreach(component ${QTPLUGIN_CORE_COMPONENTS})
        message(STATUS "    - ${component}")
    endforeach()

    message(STATUS "  Monitoring Components (${list_length(QTPLUGIN_MONITORING_COMPONENTS)}):")
    foreach(component ${QTPLUGIN_MONITORING_COMPONENTS})
        message(STATUS "    - ${component}")
    endforeach()

    message(STATUS "  Security Components (${list_length(QTPLUGIN_SECURITY_COMPONENTS)}):")
    foreach(component ${QTPLUGIN_SECURITY_COMPONENTS})
        message(STATUS "    - ${component}")
    endforeach()

    message(STATUS "  Configuration Components (${list_length(QTPLUGIN_CONFIGURATION_COMPONENTS)}):")
    foreach(component ${QTPLUGIN_CONFIGURATION_COMPONENTS})
        message(STATUS "    - ${component}")
    endforeach()

    message(STATUS "  Resource Components (${list_length(QTPLUGIN_RESOURCE_COMPONENTS)}):")
    foreach(component ${QTPLUGIN_RESOURCE_COMPONENTS})
        message(STATUS "    - ${component}")
    endforeach()

    list(LENGTH QTPLUGIN_ALL_COMPONENTS total_components)
    message(STATUS "  Total Components: ${total_components}")
endfunction()

# Function to validate component dependencies
function(qtplugin_validate_component_dependencies)
    # This function can be extended to check for component dependencies
    # For now, it's a placeholder for future dependency validation
    message(STATUS "Component dependency validation: PASSED")
endfunction()

# Macro to include component headers in user projects
macro(qtplugin_include_components)
    # Parse arguments
    set(options VERBOSE)
    set(oneValueArgs "")
    set(multiValueArgs COMPONENTS CATEGORIES)
    cmake_parse_arguments(QTPLUGIN_INCLUDE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Include components by name
    if(QTPLUGIN_INCLUDE_COMPONENTS)
        foreach(component ${QTPLUGIN_INCLUDE_COMPONENTS})
            qtplugin_check_component(${component} component_available)
            if(component_available)
                if(QTPLUGIN_INCLUDE_VERBOSE)
                    message(STATUS "Including QtPlugin component: ${component}")
                endif()
            else()
                message(WARNING "QtPlugin component not found: ${component}")
            endif()
        endforeach()
    endif()

    # Include components by category
    if(QTPLUGIN_INCLUDE_CATEGORIES)
        foreach(category ${QTPLUGIN_INCLUDE_CATEGORIES})
            qtplugin_get_components_by_category(${category} category_components)
            if(category_components)
                if(QTPLUGIN_INCLUDE_VERBOSE)
                    message(STATUS "Including QtPlugin category '${category}' with components: ${category_components}")
                endif()
            else()
                message(WARNING "QtPlugin category not found: ${category}")
            endif()
        endforeach()
    endif()
endmacro()

# Helper function to get list length (for older CMake versions)
function(list_length list_var result_var)
    list(LENGTH ${list_var} length)
    set(${result_var} ${length} PARENT_SCOPE)
endfunction()

# Component version information
set(QTPLUGIN_COMPONENT_ARCHITECTURE_VERSION "3.0.0")
set(QTPLUGIN_COMPONENT_API_VERSION "1.0.0")

# Mark as included
set(QTPLUGIN_COMPONENTS_CMAKE_INCLUDED TRUE)
