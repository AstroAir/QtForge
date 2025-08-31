# QtForgeBackwardCompatibility.cmake
# Backward compatibility module for QtForge
# Provides aliases for deprecated QTPLUGIN_* variables with deprecation warnings

include_guard(GLOBAL)

#[=======================================================================[.rst:
qtforge_setup_backward_compatibility
------------------------------------

Sets up backward compatibility aliases for all deprecated QTPLUGIN_* variables.
This function should be called after QtForge options are defined to ensure
proper variable mapping with deprecation warnings.

This maintains compatibility with existing user code while encouraging
migration to the new QTFORGE_* variable names.
#]=======================================================================]
function(qtforge_setup_backward_compatibility)
    # Build configuration variables
    if(DEFINED QTPLUGIN_BUILD_TESTS AND NOT DEFINED QTFORGE_BUILD_TESTS)
        message(DEPRECATION "QTPLUGIN_BUILD_TESTS is deprecated, use QTFORGE_BUILD_TESTS instead")
        set(QTFORGE_BUILD_TESTS ${QTPLUGIN_BUILD_TESTS} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTPLUGIN_BUILD_EXAMPLES AND NOT DEFINED QTFORGE_BUILD_EXAMPLES)
        message(DEPRECATION "QTPLUGIN_BUILD_EXAMPLES is deprecated, use QTFORGE_BUILD_EXAMPLES instead")
        set(QTFORGE_BUILD_EXAMPLES ${QTPLUGIN_BUILD_EXAMPLES} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTPLUGIN_BUILD_NETWORK AND NOT DEFINED QTFORGE_BUILD_NETWORK)
        message(DEPRECATION "QTPLUGIN_BUILD_NETWORK is deprecated, use QTFORGE_BUILD_NETWORK instead")
        set(QTFORGE_BUILD_NETWORK ${QTPLUGIN_BUILD_NETWORK} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTPLUGIN_BUILD_UI AND NOT DEFINED QTFORGE_BUILD_UI)
        message(DEPRECATION "QTPLUGIN_BUILD_UI is deprecated, use QTFORGE_BUILD_UI instead")
        set(QTFORGE_BUILD_UI ${QTPLUGIN_BUILD_UI} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTPLUGIN_BUILD_SQL AND NOT DEFINED QTFORGE_BUILD_SQL)
        message(DEPRECATION "QTPLUGIN_BUILD_SQL is deprecated, use QTFORGE_BUILD_SQL instead")
        set(QTFORGE_BUILD_SQL ${QTPLUGIN_BUILD_SQL} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTPLUGIN_BUILD_SHARED AND NOT DEFINED QTFORGE_BUILD_SHARED)
        message(DEPRECATION "QTPLUGIN_BUILD_SHARED is deprecated, use QTFORGE_BUILD_SHARED instead")
        set(QTFORGE_BUILD_SHARED ${QTPLUGIN_BUILD_SHARED} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTPLUGIN_BUILD_STATIC AND NOT DEFINED QTFORGE_BUILD_STATIC)
        message(DEPRECATION "QTPLUGIN_BUILD_STATIC is deprecated, use QTFORGE_BUILD_STATIC instead")
        set(QTFORGE_BUILD_STATIC ${QTPLUGIN_BUILD_STATIC} PARENT_SCOPE)
    endif()
    
    # Development and debugging options
    if(DEFINED QTPLUGIN_ENABLE_WARNINGS AND NOT DEFINED QTFORGE_ENABLE_WARNINGS)
        message(DEPRECATION "QTPLUGIN_ENABLE_WARNINGS is deprecated, use QTFORGE_ENABLE_WARNINGS instead")
        set(QTFORGE_ENABLE_WARNINGS ${QTPLUGIN_ENABLE_WARNINGS} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTPLUGIN_ENABLE_WERROR AND NOT DEFINED QTFORGE_ENABLE_WERROR)
        message(DEPRECATION "QTPLUGIN_ENABLE_WERROR is deprecated, use QTFORGE_ENABLE_WERROR instead")
        set(QTFORGE_ENABLE_WERROR ${QTPLUGIN_ENABLE_WERROR} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTPLUGIN_ENABLE_SANITIZERS AND NOT DEFINED QTFORGE_ENABLE_SANITIZERS)
        message(DEPRECATION "QTPLUGIN_ENABLE_SANITIZERS is deprecated, use QTFORGE_ENABLE_SANITIZERS instead")
        set(QTFORGE_ENABLE_SANITIZERS ${QTPLUGIN_ENABLE_SANITIZERS} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTPLUGIN_ENABLE_LTO AND NOT DEFINED QTFORGE_ENABLE_LTO)
        message(DEPRECATION "QTPLUGIN_ENABLE_LTO is deprecated, use QTFORGE_ENABLE_LTO instead")
        set(QTFORGE_ENABLE_LTO ${QTPLUGIN_ENABLE_LTO} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTPLUGIN_ENABLE_COMPONENT_LOGGING AND NOT DEFINED QTFORGE_ENABLE_COMPONENT_LOGGING)
        message(DEPRECATION "QTPLUGIN_ENABLE_COMPONENT_LOGGING is deprecated, use QTFORGE_ENABLE_COMPONENT_LOGGING instead")
        set(QTFORGE_ENABLE_COMPONENT_LOGGING ${QTPLUGIN_ENABLE_COMPONENT_LOGGING} PARENT_SCOPE)
    endif()
    
    # Packaging options
    if(DEFINED QTPLUGIN_CREATE_PACKAGES AND NOT DEFINED QTFORGE_CREATE_PACKAGES)
        message(DEPRECATION "QTPLUGIN_CREATE_PACKAGES is deprecated, use QTFORGE_CREATE_PACKAGES instead")
        set(QTFORGE_CREATE_PACKAGES ${QTPLUGIN_CREATE_PACKAGES} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTPLUGIN_PACKAGE_COMPONENTS AND NOT DEFINED QTFORGE_PACKAGE_COMPONENTS)
        message(DEPRECATION "QTPLUGIN_PACKAGE_COMPONENTS is deprecated, use QTFORGE_PACKAGE_COMPONENTS instead")
        set(QTFORGE_PACKAGE_COMPONENTS ${QTPLUGIN_PACKAGE_COMPONENTS} PARENT_SCOPE)
    endif()
    
    # Python support
    if(DEFINED QTPLUGIN_PYTHON_SUPPORT AND NOT DEFINED QTFORGE_PYTHON_SUPPORT)
        message(DEPRECATION "QTPLUGIN_PYTHON_SUPPORT is deprecated, use QTFORGE_PYTHON_SUPPORT instead")
        set(QTFORGE_PYTHON_SUPPORT ${QTPLUGIN_PYTHON_SUPPORT} PARENT_SCOPE)
    endif()
    
    # Documentation options
    if(DEFINED QTPLUGIN_BUILD_DOCS AND NOT DEFINED QTFORGE_BUILD_DOCS)
        message(DEPRECATION "QTPLUGIN_BUILD_DOCS is deprecated, use QTFORGE_BUILD_DOCS instead")
        set(QTFORGE_BUILD_DOCS ${QTPLUGIN_BUILD_DOCS} PARENT_SCOPE)
    endif()
    
    # Installation paths
    if(DEFINED QTPLUGIN_INSTALL_PLUGINS_DIR AND NOT DEFINED QTFORGE_INSTALL_PLUGINS_DIR)
        message(DEPRECATION "QTPLUGIN_INSTALL_PLUGINS_DIR is deprecated, use QTFORGE_INSTALL_PLUGINS_DIR instead")
        set(QTFORGE_INSTALL_PLUGINS_DIR ${QTPLUGIN_INSTALL_PLUGINS_DIR} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTPLUGIN_INSTALL_EXAMPLES_DIR AND NOT DEFINED QTFORGE_INSTALL_EXAMPLES_DIR)
        message(DEPRECATION "QTPLUGIN_INSTALL_EXAMPLES_DIR is deprecated, use QTFORGE_INSTALL_EXAMPLES_DIR instead")
        set(QTFORGE_INSTALL_EXAMPLES_DIR ${QTPLUGIN_INSTALL_EXAMPLES_DIR} PARENT_SCOPE)
    endif()
endfunction()

#[=======================================================================[.rst:
qtforge_create_reverse_compatibility
------------------------------------

Creates reverse compatibility by setting QTPLUGIN_* variables from QTFORGE_*
variables. This is useful for packages that depend on the old variable names.

This function should be called after all QTFORGE_* variables are set.
#]=======================================================================]
function(qtforge_create_reverse_compatibility)
    # Only set QTPLUGIN_* variables if they're not already defined
    # This prevents overriding user-defined values
    
    if(DEFINED QTFORGE_BUILD_TESTS AND NOT DEFINED QTPLUGIN_BUILD_TESTS)
        set(QTPLUGIN_BUILD_TESTS ${QTFORGE_BUILD_TESTS} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTFORGE_BUILD_EXAMPLES AND NOT DEFINED QTPLUGIN_BUILD_EXAMPLES)
        set(QTPLUGIN_BUILD_EXAMPLES ${QTFORGE_BUILD_EXAMPLES} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTFORGE_BUILD_NETWORK AND NOT DEFINED QTPLUGIN_BUILD_NETWORK)
        set(QTPLUGIN_BUILD_NETWORK ${QTFORGE_BUILD_NETWORK} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTFORGE_BUILD_UI AND NOT DEFINED QTPLUGIN_BUILD_UI)
        set(QTPLUGIN_BUILD_UI ${QTFORGE_BUILD_UI} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTFORGE_BUILD_SQL AND NOT DEFINED QTPLUGIN_BUILD_SQL)
        set(QTPLUGIN_BUILD_SQL ${QTFORGE_BUILD_SQL} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTFORGE_BUILD_SHARED AND NOT DEFINED QTPLUGIN_BUILD_SHARED)
        set(QTPLUGIN_BUILD_SHARED ${QTFORGE_BUILD_SHARED} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTFORGE_BUILD_STATIC AND NOT DEFINED QTPLUGIN_BUILD_STATIC)
        set(QTPLUGIN_BUILD_STATIC ${QTFORGE_BUILD_STATIC} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTFORGE_ENABLE_WARNINGS AND NOT DEFINED QTPLUGIN_ENABLE_WARNINGS)
        set(QTPLUGIN_ENABLE_WARNINGS ${QTFORGE_ENABLE_WARNINGS} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTFORGE_ENABLE_WERROR AND NOT DEFINED QTPLUGIN_ENABLE_WERROR)
        set(QTPLUGIN_ENABLE_WERROR ${QTFORGE_ENABLE_WERROR} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTFORGE_ENABLE_SANITIZERS AND NOT DEFINED QTPLUGIN_ENABLE_SANITIZERS)
        set(QTPLUGIN_ENABLE_SANITIZERS ${QTFORGE_ENABLE_SANITIZERS} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTFORGE_ENABLE_LTO AND NOT DEFINED QTPLUGIN_ENABLE_LTO)
        set(QTPLUGIN_ENABLE_LTO ${QTFORGE_ENABLE_LTO} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTFORGE_CREATE_PACKAGES AND NOT DEFINED QTPLUGIN_CREATE_PACKAGES)
        set(QTPLUGIN_CREATE_PACKAGES ${QTFORGE_CREATE_PACKAGES} PARENT_SCOPE)
    endif()
    
    if(DEFINED QTFORGE_PYTHON_SUPPORT AND NOT DEFINED QTPLUGIN_PYTHON_SUPPORT)
        set(QTPLUGIN_PYTHON_SUPPORT ${QTFORGE_PYTHON_SUPPORT} PARENT_SCOPE)
    endif()
endfunction()

#[=======================================================================[.rst:
qtforge_print_compatibility_summary
-----------------------------------

Prints a summary of backward compatibility mappings that were applied.
This helps users understand which deprecated variables are being used.
#]=======================================================================]
function(qtforge_print_compatibility_summary)
    set(DEPRECATED_VARS_FOUND FALSE)
    
    # Check for deprecated variables and collect them
    set(DEPRECATED_VARS "")
    
    if(DEFINED QTPLUGIN_BUILD_TESTS)
        list(APPEND DEPRECATED_VARS "QTPLUGIN_BUILD_TESTS -> QTFORGE_BUILD_TESTS")
        set(DEPRECATED_VARS_FOUND TRUE)
    endif()
    
    if(DEFINED QTPLUGIN_BUILD_EXAMPLES)
        list(APPEND DEPRECATED_VARS "QTPLUGIN_BUILD_EXAMPLES -> QTFORGE_BUILD_EXAMPLES")
        set(DEPRECATED_VARS_FOUND TRUE)
    endif()
    
    if(DEFINED QTPLUGIN_BUILD_NETWORK)
        list(APPEND DEPRECATED_VARS "QTPLUGIN_BUILD_NETWORK -> QTFORGE_BUILD_NETWORK")
        set(DEPRECATED_VARS_FOUND TRUE)
    endif()
    
    if(DEFINED QTPLUGIN_BUILD_UI)
        list(APPEND DEPRECATED_VARS "QTPLUGIN_BUILD_UI -> QTFORGE_BUILD_UI")
        set(DEPRECATED_VARS_FOUND TRUE)
    endif()
    
    if(DEPRECATED_VARS_FOUND)
        message(STATUS "QtForge: Backward compatibility mappings applied:")
        foreach(VAR_MAPPING ${DEPRECATED_VARS})
            message(STATUS "  ${VAR_MAPPING}")
        endforeach()
        message(STATUS "QtForge: Please update your CMake files to use QTFORGE_* variables")
    endif()
endfunction()
