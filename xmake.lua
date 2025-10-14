-- QtForge Library XMake Configuration
-- Modern C++ Plugin System for Qt Applications with Modular Architecture
-- Version: 3.2.0
-- Updated to match CMake build system structure with all features enabled

-- Set minimum xmake version
set_xmakever("3.0.1")

-- Project configuration
set_project("QtForge")
set_version("3.2.0", {build = "%Y%m%d%H%M"})

-- Add build modes
add_rules("mode.debug", "mode.release", "mode.releasedbg", "mode.minsizerel")

-- Build options (matching CMake QtForgeOptions.cmake)
option("shared")
    set_default(true)
    set_description("Build shared libraries (QTFORGE_BUILD_SHARED)")
    set_showmenu(true)
option_end()

option("static")
    set_default(false)
    set_description("Build static libraries (QTFORGE_BUILD_STATIC)")
    set_showmenu(true)
option_end()

option("examples")
    set_default(true)
    set_description("Build example plugins (QTFORGE_BUILD_EXAMPLES)")
    set_showmenu(true)
option_end()

option("tests")
    set_default(true)
    set_description("Build unit tests (QTFORGE_BUILD_TESTS)")
    set_showmenu(true)
option_end()

option("benchmarks")
    set_default(true)
    set_description("Build performance benchmarks (QTFORGE_BUILD_BENCHMARKS)")
    set_showmenu(true)
option_end()

option("docs")
    set_default(true)
    set_description("Build documentation (QTFORGE_BUILD_DOCS)")
    set_showmenu(true)
option_end()

option("python_bindings")
    set_default(true)
    set_description("Build Python bindings using pybind11 (QTFORGE_BUILD_PYTHON_BINDINGS)")
    set_showmenu(true)
option_end()

option("lua_bindings")
    set_default(true)
    set_description("Build Lua bindings using sol2 (QTFORGE_BUILD_LUA_BINDINGS)")
    set_showmenu(true)
option_end()

option("network")
    set_default(true)
    set_description("Build network plugin support (QTFORGE_BUILD_NETWORK)")
    set_showmenu(true)
option_end()

option("ui")
    set_default(true)
    set_description("Build UI plugin support (QTFORGE_BUILD_UI)")
    set_showmenu(true)
option_end()

option("sql")
    set_default(true)
    set_description("Build SQL plugin support (QTFORGE_BUILD_SQL)")
    set_showmenu(true)
option_end()

option("concurrent")
    set_default(true)
    set_description("Build concurrent plugin support (QTFORGE_BUILD_CONCURRENT)")
    set_showmenu(true)
option_end()

-- Advanced options
option("warnings")
    set_default(true)
    set_description("Enable comprehensive compiler warnings (QTFORGE_ENABLE_WARNINGS)")
    set_showmenu(true)
option_end()

option("werror")
    set_default(false)
    set_description("Treat warnings as errors (QTFORGE_ENABLE_WERROR)")
    set_showmenu(true)
option_end()

option("sanitizers")
    set_default(false)
    set_description("Enable sanitizers in debug builds (QTFORGE_ENABLE_SANITIZERS)")
    set_showmenu(true)
option_end()

option("lto")
    set_default(false)
    set_description("Enable Link Time Optimization (QTFORGE_ENABLE_LTO)")
    set_showmenu(true)
option_end()

-- Qt6 dependency configuration (matching CMake QtForgeDependencies.cmake)
-- Core Qt6 packages with intelligent detection and configuration

-- Qt6 Core (always required)
add_requires("qt6core", {
    configs = {
        shared = has_config("shared"),
        runtimes = is_plat("windows") and "MD" or nil
    }
})

-- Qt6 Additional packages (conditional based on build options)
if has_config("network") then
    add_requires("qt6network", {
        optional = true,
        configs = {
            shared = has_config("shared"),
            runtimes = is_plat("windows") and "MD" or nil
        }
    })
end

if has_config("ui") then
    add_requires("qt6gui", {
        optional = true,
        configs = {
            shared = has_config("shared"),
            runtimes = is_plat("windows") and "MD" or nil
        }
    })

    add_requires("qt6widgets", {
        optional = true,
        configs = {
            shared = has_config("shared"),
            runtimes = is_plat("windows") and "MD" or nil
        }
    })
end

if has_config("sql") then
    add_requires("qt6sql", {
        optional = true,
        configs = {
            shared = has_config("shared"),
            runtimes = is_plat("windows") and "MD" or nil
        }
    })
end

if has_config("concurrent") then
    add_requires("qt6concurrent", {
        optional = true,
        configs = {
            shared = has_config("shared"),
            runtimes = is_plat("windows") and "MD" or nil
        }
    })
end

-- Qt6 StateMachine (for advanced plugin orchestration)
add_requires("qt6statemachine", {
    optional = true,
    configs = {
        shared = has_config("shared"),
        runtimes = is_plat("windows") and "MD" or nil
    }
})

-- Python bindings dependencies (matching CMake configuration)
if has_config("python_bindings") then
    add_requires("python3", {
        optional = true,
        configs = {
            version = ">=3.8"  -- Minimum version as per CMake
        }
    })
    add_requires("pybind11", {
        optional = true,
        configs = {
            version = ">=2.6.0"
        }
    })
end

-- Lua bindings dependencies (matching CMake configuration)
if has_config("lua_bindings") then
    add_requires("lua", {
        optional = true,
        configs = {
            version = "5.4"  -- Specific version as per CMake
        }
    })
    add_requires("sol2", {
        optional = true,
        configs = {
            version = ">=3.3.0"  -- Minimum version as per CMake
        }
    })
end

-- Development dependencies
if has_config("tests") then
    add_requires("gtest", {optional = true})
end

if has_config("benchmarks") then
    add_requires("benchmark", {optional = true})
end

if has_config("docs") then
    add_requires("doxygen", {optional = true, system = true})
end

-- Global compiler configuration (matching CMake QtForgeCompiler.cmake)
set_languages("c++20")

-- Compiler warnings configuration
if has_config("warnings") then
    set_warnings("all", "extra")
    if is_plat("windows") then
        add_cxflags("/W4")
        if has_config("werror") then
            add_cxflags("/WX")
        end
    else
        add_cxflags("-Wall", "-Wextra", "-Wpedantic")
        if has_config("werror") then
            add_cxflags("-Werror")
        end
    end
end

-- Platform-specific configurations (matching CMake QtForgePlatform.cmake)
if is_plat("windows") then
    add_cxflags("/utf-8")
    add_defines("WIN32_LEAN_AND_MEAN", "NOMINMAX")
    add_defines("QTFORGE_PLATFORM_WINDOWS")
    add_cxflags("/permissive-")  -- Strict conformance for MSVC
elseif is_plat("linux") then
    add_cxflags("-fPIC")
    add_syslinks("pthread", "dl")
    add_defines("QTFORGE_PLATFORM_LINUX")
elseif is_plat("macosx") then
    add_cxflags("-fPIC")
    add_frameworks("Foundation")
    add_defines("QTFORGE_PLATFORM_MACOS")
elseif is_plat("android") then
    add_defines("QTFORGE_PLATFORM_ANDROID")
elseif is_plat("iphoneos") then
    add_defines("QTFORGE_PLATFORM_IOS")
end

-- Build mode configuration (matching CMake build types)
if is_mode("debug") then
    add_defines("QTFORGE_DEBUG=1")
    set_symbols("debug")
    set_optimize("none")
    if has_config("sanitizers") then
        add_cxflags("-fsanitize=address", "-fsanitize=undefined")
        add_ldflags("-fsanitize=address", "-fsanitize=undefined")
    end
elseif is_mode("release") then
    add_defines("QTFORGE_RELEASE=1", "NDEBUG=1")
    set_symbols("hidden")
    set_optimize("fastest")
    set_strip("all")
    if has_config("lto") then
        set_policy("build.optimization.lto", true)
    end
elseif is_mode("releasedbg") then
    add_defines("QTFORGE_RELEASE=1", "NDEBUG=1")
    set_symbols("debug")
    set_optimize("fastest")
    set_strip("all")
elseif is_mode("minsizerel") then
    add_defines("QTFORGE_RELEASE=1", "NDEBUG=1")
    set_symbols("hidden")
    set_optimize("smallest")
    set_strip("all")
end

-- Include directories
add_includedirs("include", {public = true})
add_includedirs("src", {private = true})

-- Qt feature detection and configuration (matching CMake QtForgeDependencies.cmake)
local qt_features = {}

-- Function to check Qt package availability and configure features
local function configure_qt_features()
    -- Core is always required
    qt_features.core = has_package("qt6core")

    -- Network support
    if has_config("network") and has_package("qt6network") then
        qt_features.network = true
        add_defines("QTFORGE_HAS_NETWORK")
        print("QtForge: Network support enabled")
    else
        qt_features.network = false
        print("QtForge: Network support disabled")
    end

    -- UI support (requires both GUI and Widgets)
    if has_config("ui") and has_package("qt6gui") and has_package("qt6widgets") then
        qt_features.widgets = true
        add_defines("QTFORGE_HAS_WIDGETS")
        print("QtForge: Widgets support enabled")
    else
        qt_features.widgets = false
        print("QtForge: Widgets support disabled")
    end

    -- SQL support
    if has_config("sql") and has_package("qt6sql") then
        qt_features.sql = true
        add_defines("QTFORGE_HAS_SQL")
        print("QtForge: SQL support enabled")
    else
        qt_features.sql = false
        print("QtForge: SQL support disabled")
    end

    -- Concurrent support
    if has_config("concurrent") and has_package("qt6concurrent") then
        qt_features.concurrent = true
        add_defines("QTFORGE_HAS_CONCURRENT")
        print("QtForge: Concurrent support enabled")
    else
        qt_features.concurrent = false
        print("QtForge: Concurrent support disabled")
    end

    -- StateMachine support
    if has_package("qt6statemachine") then
        qt_features.statemachine = true
        add_defines("QTFORGE_HAS_STATEMACHINE")
        print("QtForge: StateMachine support enabled")
    else
        qt_features.statemachine = false
        print("QtForge: StateMachine support disabled")
    end
end

-- Configure Qt features
configure_qt_features()

-- Version definitions (matching CMake configuration)
add_defines("QTFORGE_VERSION_MAJOR=3")
add_defines("QTFORGE_VERSION_MINOR=2")
add_defines("QTFORGE_VERSION_PATCH=0")
add_defines("QTFORGE_VERSION_STRING=\"3.2.0\"")

-- Export definitions for shared libraries
if has_config("shared") then
    add_defines("QTFORGE_SHARED")
    if is_plat("windows") then
        add_defines("QTFORGE_EXPORT=__declspec(dllexport)")
    else
        add_defines("QTFORGE_EXPORT=__attribute__((visibility(\"default\")))")
    end
else
    add_defines("QTFORGE_STATIC")
    add_defines("QTFORGE_EXPORT=")
end

-- Core library sources (matching CMake QTFORGE_CORE_SOURCES)
-- Minimal sources that can build without Qt (for testing build system)
local qtforge_minimal_sources = {
    "src/utils/version.cpp",
    "src/utils/error_handling.cpp"
}

-- Full sources that require Qt (will be used when Qt is available)
local qtforge_core_sources = {
    "src/qtplugin.cpp",
    "src/core/plugin_interface.cpp",
    "src/core/plugin_manager.cpp",
    "src/core/plugin_loader.cpp",
    "src/core/plugin_registry.cpp",
    "src/core/plugin_dependency_resolver.cpp",
    "src/core/plugin_lifecycle_manager.cpp",
    "src/monitoring/plugin_hot_reload_manager.cpp",
    "src/monitoring/plugin_metrics_collector.cpp",
    "src/communication/message_bus.cpp",
    "src/communication/request_response_system.cpp",
    "src/communication/factory.cpp",
    "src/communication/message_publisher.cpp",
    "src/communication/plugin_service_discovery.cpp",
    "src/communication/typed_event_system.cpp",

    "src/managers/configuration_manager.cpp",
    "src/managers/components/configuration_storage.cpp",
    "src/managers/components/configuration_validator.cpp",
    "src/managers/components/configuration_merger.cpp",
    "src/managers/components/configuration_watcher.cpp",
    "src/managers/logging_manager.cpp",
    "src/managers/resource_manager.cpp",
    "src/managers/resource_lifecycle.cpp",
    "src/managers/components/resource_pool.cpp",
    "src/managers/components/resource_allocator.cpp",
    "src/managers/resource_monitor.cpp",
    -- Advanced plugin system sources (v3.1.0)
    "src/communication/plugin_service_contracts.cpp",
    "src/core/advanced_plugin_interface.cpp",
    -- Workflow module sources (v3.1.0+)
    "src/workflow/error_recovery.cpp",
    "src/workflow/rollback_manager.cpp",
    "src/workflow/state_persistence.cpp",
    "src/workflow/orchestration.cpp",
    "src/workflow/progress_tracking.cpp",
    "src/workflow/transactions.cpp",
    "src/workflow/composition.cpp",
    "src/workflow/integration.cpp",
    "src/workflow/workflow_manager.cpp",
    "src/workflow/progress_monitoring.cpp",
    "src/workflow/progress_message_bus.cpp",
    "src/workflow/transaction_error_handler.cpp",
    "src/workflow/workflow_validator.cpp",
    "src/transactions/plugin_transaction_manager.cpp",
    -- Version management sources (v3.1.0)
    "src/managers/plugin_version_manager.cpp",
    -- Dynamic plugin system sources (v3.2.0)
    "src/core/dynamic_plugin_interface.cpp",
    "src/core/plugin_capability_discovery.cpp",
    "src/core/plugin_property_system.cpp",
    -- Multi-language plugin bridges (v3.2.0)
    "src/bridges/python_plugin_bridge.cpp",
    "src/bridges/lua_plugin_bridge.cpp",
    -- Remote plugin system
    "src/remote/http_plugin_loader.cpp",
    "src/remote/plugin_download_manager.cpp",
    "src/remote/remote_plugin_configuration.cpp",
    "src/remote/remote_plugin_discovery.cpp",
    "src/remote/remote_plugin_loader.cpp",
    "src/remote/remote_plugin_manager.cpp",
    "src/remote/remote_plugin_manager_extension.cpp",
    "src/remote/remote_plugin_registry_extension.cpp",
    "src/remote/remote_plugin_source.cpp",
    "src/remote/remote_plugin_validator.cpp",
    "src/remote/unified_plugin_manager.cpp",
    -- Security components
    "src/security/components/permission_manager.cpp",
    "src/security/components/security_policy_engine.cpp",
    "src/security/components/security_validator.cpp",
    "src/security/components/signature_verifier.cpp"
}

-- Sources that require specific Qt components
local qtforge_widgets_sources = {
    -- No widget-specific sources currently
}

local qtforge_network_sources = {
    -- No network-specific sources currently
}

-- Conditionally add Lua-related sources if Lua bindings are enabled
local qtforge_lua_sources = {
    "src/bridges/lua_plugin_bridge.cpp",
    "src/core/lua_plugin_loader.cpp"
}

-- Add conditional sources based on Qt component availability
if qt_features.network then
    for _, source in ipairs(qtforge_network_sources) do
        table.insert(qtforge_core_sources, source)
    end
    print("QtForge: Including network-dependent sources")
else
    print("QtForge: Excluding network-dependent sources")
end

if qt_features.widgets then
    for _, source in ipairs(qtforge_widgets_sources) do
        table.insert(qtforge_core_sources, source)
    end
    print("QtForge: Including widgets-dependent sources")
else
    print("QtForge: Excluding widgets-dependent sources")
end

-- Add Lua sources if enabled
if has_config("lua_bindings") and has_package("lua") and has_package("sol2") then
    for _, source in ipairs(qtforge_lua_sources) do
        table.insert(qtforge_core_sources, source)
    end
    print("QtForge: Including Lua plugin sources in core library")
else
    print("QtForge: Excluding Lua plugin sources from core library")
end

-- Core library headers (matching CMake QTFORGE_CORE_HEADERS)
local qtforge_core_headers = {
    "include/qtplugin/interfaces/core/plugin_interface.hpp",
    "include/qtplugin/core/plugin_manager.hpp",
    "include/qtplugin/core/plugin_loader.hpp",
    "include/qtplugin/core/plugin_registry.hpp",
    "include/qtplugin/core/plugin_dependency_resolver.hpp",
    "include/qtplugin/core/plugin_lifecycle_manager.hpp",
    "include/qtplugin/monitoring/plugin_hot_reload_manager.hpp",
    "include/qtplugin/monitoring/plugin_metrics_collector.hpp",
    "include/qtplugin/interfaces/core/service_plugin_interface.hpp",
    "include/qtplugin/communication/message_bus.hpp",
    "include/qtplugin/communication/message_types.hpp",
    "include/qtplugin/communication/request_response_system.hpp",
    "include/qtplugin/utils/version.hpp",
    "include/qtplugin/utils/error_handling.hpp",
    -- Security headers removed
    "include/qtplugin/managers/configuration_manager.hpp",
    "include/qtplugin/managers/configuration_manager_impl.hpp",
    "include/qtplugin/managers/components/configuration_storage.hpp",
    "include/qtplugin/managers/components/configuration_validator.hpp",
    "include/qtplugin/managers/components/configuration_merger.hpp",
    "include/qtplugin/managers/components/configuration_watcher.hpp",
    "include/qtplugin/managers/logging_manager.hpp",
    "include/qtplugin/managers/logging_manager_impl.hpp",
    "include/qtplugin/managers/resource_manager.hpp",
    "include/qtplugin/managers/resource_manager_impl.hpp",
    "include/qtplugin/managers/resource_pools.hpp",
    "include/qtplugin/managers/resource_lifecycle.hpp",
    "include/qtplugin/managers/resource_lifecycle_impl.hpp",
    "include/qtplugin/managers/components/resource_pool.hpp",
    "include/qtplugin/managers/components/resource_allocator.hpp",
    "include/qtplugin/managers/resource_monitor_impl.hpp",
    "include/qtplugin/qtplugin.hpp",
    "include/qtplugin/components.hpp",
    -- Advanced plugin system headers (v3.1.0)
    "include/qtplugin/communication/plugin_service_contracts.hpp",
    "include/qtplugin/interfaces/core/advanced_plugin_interface.hpp",
    "include/qtplugin/workflow/transactions.hpp",
    "include/qtplugin/workflow/composition.hpp",
    "include/qtplugin/workflow/integration.hpp",
    "include/qtplugin/workflow/workflow.hpp",
    "include/qtplugin/workflow/error_recovery.hpp",
    "include/qtplugin/workflow/rollback_manager.hpp",
    "include/qtplugin/workflow/state_persistence.hpp",
    "include/qtplugin/workflow/orchestration.hpp",
    "include/qtplugin/workflow/progress_tracking.hpp",
    "include/qtplugin/workflow/progress_monitoring.hpp",
    "include/qtplugin/workflow/progress_message_bus.hpp",
    "include/qtplugin/workflow/transaction_error_handler.hpp",
    "include/qtplugin/workflow/workflow_validator.hpp",
    "include/qtplugin/workflow/workflow_types.hpp",
    -- Version management headers (v3.1.0)
    "include/qtplugin/managers/plugin_version_manager.hpp",
    -- Dynamic plugin system headers (v3.2.0)
    "include/qtplugin/interfaces/core/dynamic_plugin_interface.hpp",
    "include/qtplugin/core/plugin_capability_discovery.hpp",
    -- Multi-language plugin bridge headers (v3.2.0)
    "include/qtplugin/bridges/python_plugin_bridge.hpp",
    "include/qtplugin/bridges/lua_plugin_bridge.hpp",
    -- Remote plugin system headers
    "include/qtplugin/remote/http_plugin_loader.hpp",
    "include/qtplugin/remote/plugin_download_manager.hpp",
    "include/qtplugin/remote/remote_plugin_configuration.hpp",
    "include/qtplugin/remote/remote_plugin_discovery.hpp",
    "include/qtplugin/remote/remote_plugin_loader.hpp",
    "include/qtplugin/remote/remote_plugin_manager.hpp",
    "include/qtplugin/remote/remote_plugin_manager_extension.hpp",
    "include/qtplugin/remote/remote_plugin_registry_extension.hpp",
    "include/qtplugin/remote/remote_plugin_source.hpp",
    "include/qtplugin/remote/remote_plugin_validator.hpp",
    "include/qtplugin/remote/unified_plugin_manager.hpp",
    "include/qtplugin/remote/remote_security_manager.hpp",
    -- Security component headers
    "include/qtplugin/security/components/permission_manager.hpp",
    "include/qtplugin/security/components/security_policy_engine.hpp",
    "include/qtplugin/security/components/security_validator.hpp",
    "include/qtplugin/security/components/signature_verifier.hpp",
    "include/qtplugin/security/security_manager.hpp",

}

-- Conditionally add Lua headers if enabled
local qtforge_lua_headers = {
    "include/qtplugin/bridges/lua_plugin_bridge.hpp",
    "include/qtplugin/core/lua_plugin_loader.hpp"
}

if has_config("lua_bindings") and has_package("lua") and has_package("sol2") then
    for _, header in ipairs(qtforge_lua_headers) do
        table.insert(qtforge_core_headers, header)
    end
end

-- QtForgeCore library target (matching CMake qtforge_add_library configuration)
target("QtForgeCore")
    set_kind(has_config("shared") and "shared" or "static")
    set_basename("qtforge-core")

    -- Use modern Qt rules with MOC support (temporarily disabled for testing)
    -- add_rules("qt.shared")
    -- set_values("qt.moc.flags", "-DQTFORGE_VERSION_MAJOR=3")

    -- Configure shared library to generate import library on Windows
    if has_config("shared") and is_plat("windows") then
        add_shflags("/IMPLIB:$(builddir)/qtforge-core.lib")
    end

    -- Add source files (use minimal sources for now to test build system)
    -- TODO: Switch to qtforge_core_sources when Qt integration is properly configured
    add_files(qtforge_minimal_sources)

    -- Add header files that need MOC processing
    add_headerfiles(qtforge_core_headers)

    -- Qt package integration (matching CMake QT_COMPONENTS)
    -- Temporarily disable Qt packages to test basic build structure
    -- add_packages("qt6core")

    -- if qt_features.network then
    --     add_packages("qt6network")
    -- end

    -- if qt_features.widgets then
    --     add_packages("qt6widgets", "qt6gui")
    -- end

    -- if qt_features.sql then
    --     add_packages("qt6sql")
    -- end

    -- if qt_features.concurrent then
    --     add_packages("qt6concurrent")
    -- end

    -- if qt_features.statemachine then
    --     add_packages("qt6statemachine")
    -- end

    -- Compile definitions based on available features
    local core_definitions = {}
    if qt_features.network then
        table.insert(core_definitions, "QTFORGE_HAS_NETWORK")
    end
    if qt_features.widgets then
        table.insert(core_definitions, "QTFORGE_HAS_WIDGETS")
    end
    if qt_features.sql then
        table.insert(core_definitions, "QTFORGE_HAS_SQL")
    end
    if qt_features.concurrent then
        table.insert(core_definitions, "QTFORGE_HAS_CONCURRENT")
    end
    if qt_features.statemachine then
        table.insert(core_definitions, "QTFORGE_HAS_STATEMACHINE")
    end

    if #core_definitions > 0 then
        add_defines(core_definitions)
    end

    -- Set version and properties
    set_version("3.2.0")

    -- Export symbols for shared library
    if has_config("shared") then
        add_defines("QTFORGE_CORE_EXPORTS")
    end

    -- Install headers
    add_installfiles("include/(qtplugin/**.hpp)", {prefixdir = "include"})
target_end()

-- QtForgeSecurity library target - REMOVED
-- Security components have been removed from QtForge
-- SHA256 verification is preserved in the core PluginManager

-- Optional QtForgeNetwork library (disabled in CMake, keeping for future use)
-- Note: Network functionality is integrated into QtForgeCore in current CMake setup
-- if qt_features.network then
--     target("QtForgeNetwork")
--         set_kind(has_config("shared") and "shared" or "static")
--         set_basename("qtforge-network")
--         add_rules("qt.shared")
--         add_packages("qt6core", "qt6network")
--         add_deps("QtForgeCore")
--         set_version("3.0.0")
--         if has_config("shared") then
--             add_defines("QTFORGE_NETWORK_EXPORTS")
--         end
--     target_end()
-- end

-- Optional QtForgeUI library (disabled in CMake, keeping for future use)
-- Note: UI functionality is integrated into QtForgeCore in current CMake setup
-- if qt_features.widgets then
--     target("QtForgeUI")
--         set_kind(has_config("shared") and "shared" or "static")
--         set_basename("qtforge-ui")
--         add_rules("qt.shared")
--         add_packages("qt6core", "qt6widgets", "qt6gui")
--         add_deps("QtForgeCore")
--         set_version("3.0.0")
--         if has_config("shared") then
--             add_defines("QTFORGE_UI_EXPORTS")
--         end
--     target_end()
-- end

-- Python bindings target (matching CMake Python bindings configuration)
if has_config("python_bindings") and has_package("python3") and has_package("pybind11") then
    target("qtforge_python")
        set_kind("shared")
        set_basename("qtforge")

        -- Add Python module rules
        add_rules("python.library", {soabi = true})

        -- Python binding sources (matching CMake QTFORGE_PYTHON_SOURCES)
        -- Core components (always included)
        add_files("src/python/qtforge_python.cpp")
        add_files("src/python/core/core_bindings.cpp")
        add_files("src/python/utils/utils_bindings.cpp")

        -- Optional binding sources (commented out in CMake for now)
        -- add_files("src/python/qt_conversions.cpp")  -- Temporarily disabled in CMake
        -- Security bindings removed
        -- add_files("src/python/managers/managers_bindings.cpp")
        -- add_files("src/python/orchestration/orchestration_bindings.cpp")
        -- add_files("src/python/monitoring/monitoring_bindings.cpp")
        -- add_files("src/python/transactions/transaction_bindings.cpp")
        -- add_files("src/python/composition/composition_bindings.cpp")

        -- add_files("src/python/threading/threading_bindings.cpp")

        -- Communication bindings (temporarily disabled in CMake)
        -- if qt_features.network then
        --     add_files("src/python/communication/communication_bindings.cpp")
        -- end

        -- Add packages (minimal configuration matching CMake)
        add_packages("python3", "pybind11")
        -- Ultra-minimal bindings - no Qt dependencies for now (matching CMake)
        -- add_packages("qt6core")
        -- if qt_features.network then
        --     add_packages("qt6network")
        -- end
        -- if qt_features.widgets then
        --     add_packages("qt6widgets")
        -- end
        -- if qt_features.sql then
        --     add_packages("qt6sql")
        -- end

        -- Add dependencies (disabled in CMake for now)
        -- add_deps("QtForgeCore")

        -- Set properties
        set_symbols("hidden")
        add_defines("QTFORGE_PYTHON_BINDINGS")

        -- Version definitions (matching CMake)
        add_defines("QTPLUGIN_VERSION_MAJOR=3")
        add_defines("QTPLUGIN_VERSION_MINOR=0")
        add_defines("QTPLUGIN_VERSION_PATCH=0")
    target_end()
end

-- Lua bindings target (matching CMake Lua bindings configuration)
if has_config("lua_bindings") and has_package("lua") and has_package("sol2") then
    target("qtforge_lua")
        set_kind("shared")
        set_basename("qtforge_lua")

        -- Lua binding sources (matching CMake QTFORGE_LUA_SOURCES - minimal components only)
        add_files("src/lua/qtforge_lua.cpp")
        add_files("src/lua/qt_conversions.cpp")  -- Required for Qt type conversions in Lua bindings
        add_files("src/lua/core/core_bindings.cpp")
        add_files("src/lua/utils/utils_bindings.cpp")

        -- Temporarily disabled sources (matching CMake configuration)
        -- add_files("src/lua/core/metadata_bindings.cpp")  -- Temporarily disabled due to Qt dependencies
        -- add_files("src/lua/utils/error_handling_bindings.cpp")  -- Temporarily disabled due to Qt dependencies
        -- Security bindings removed
        -- add_files("src/lua/managers/managers_bindings.cpp")
        -- add_files("src/lua/orchestration/orchestration_bindings.cpp")
        -- add_files("src/lua/monitoring/monitoring_bindings.cpp")
        -- add_files("src/lua/transactions/transaction_bindings.cpp")
        -- add_files("src/lua/composition/composition_bindings.cpp")

        -- add_files("src/lua/threading/threading_bindings.cpp")

        -- Communication bindings (temporarily disabled in CMake)
        -- if qt_features.network then
        --     add_files("src/lua/communication/communication_bindings.cpp")
        -- end

        -- Ultra-minimal bindings - only link Lua libraries for now (matching CMake)
        add_packages("lua", "sol2")
        -- Temporarily disabled Qt dependencies (matching CMake)
        -- add_packages("qt6core")
        -- if qt_features.network then
        --     add_packages("qt6network")
        -- end
        -- if qt_features.widgets then
        --     add_packages("qt6widgets")
        -- end
        -- if qt_features.sql then
        --     add_packages("qt6sql")
        -- end

        -- Add dependencies (temporarily disabled in CMake)
        -- add_deps("QtForgeCore")

        -- Set properties
        set_symbols("hidden")
        add_defines("QTFORGE_LUA_BINDINGS")

        -- Add sandboxing support if enabled (matching CMake)
        if has_config("lua_sandbox") then
            add_defines("QTFORGE_LUA_SANDBOX_ENABLED")
        end

        -- Version definitions (matching CMake)
        add_defines("QTPLUGIN_VERSION_MAJOR=3")
        add_defines("QTPLUGIN_VERSION_MINOR=0")
        add_defines("QTPLUGIN_VERSION_PATCH=0")
    target_end()
end

-- Examples (matching CMake configuration)
-- Temporarily disabled until Qt integration is properly configured
-- if has_config("examples") then
--     includes("examples")
-- end

-- Tests (matching CMake configuration)
if has_config("tests") then
    includes("tests")
end

-- Documentation target (matching CMake configuration)
if has_config("docs") and has_package("doxygen") then
    target("docs")
        set_kind("phony")
        on_build(function (target)
            os.exec("doxygen Doxyfile")
        end)
    target_end()
end

-- Installation rules (matching CMake installation configuration)
on_install(function (target)
    -- Install libraries (matching CMake QTFORGE_INSTALL_TARGETS)
    if target:name() == "QtForgeCore" then
        os.cp(target:targetfile(), path.join(target:installdir(), "lib"))
    end

    -- Install headers (matching CMake header installation)
    if target:name() == "QtForgeCore" then
        os.cp("include/qtplugin", path.join(target:installdir(), "include"))
    end

    -- Install Python module (matching CMake Python bindings installation)
    if target:name() == "qtforge_python" then
        local python_site_packages = path.join(target:installdir(), "lib", "python", "site-packages")
        os.mkdir(python_site_packages)
        os.cp(target:targetfile(), python_site_packages)
    end

    -- Install Lua module (matching CMake Lua bindings installation)
    if target:name() == "qtforge_lua" then
        os.cp(target:targetfile(), path.join(target:installdir(), "lib"))
    end

    -- Install documentation files (matching CMake documentation installation)
    if target:name() == "QtForgeCore" then
        local doc_files = {"README.md", "LICENSE", "CHANGELOG.md", "CODE_OF_CONDUCT.md", "CONTRIBUTING.md"}
        local doc_dir = path.join(target:installdir(), "share", "doc", "qtforge")
        os.mkdir(doc_dir)
        for _, file in ipairs(doc_files) do
            if os.isfile(file) then
                os.cp(file, doc_dir)
            end
        end

        -- Install docs directory
        if os.isdir("docs") then
            os.cp("docs", doc_dir)
        end
    end
end)

-- Package configuration (matching CMake packaging)
on_package(function (target)
    -- Create package metadata (matching CMake package configuration)
    local package_info = {
        name = "QtForge",
        version = "3.2.0",
        description = "Modern C++ Plugin System for Qt Applications with Modular Architecture",
        homepage = "https://github.com/AstroAir/QtForge",  -- Updated to match actual repository
        license = "Apache-2.0"
    }

    -- Save package info
    io.writefile(path.join(target:installdir(), "package.json"), json.encode(package_info))

    -- Create pkg-config file (matching CMake qtforge.pc generation)
    local pkgconfig_content = string.format([[
prefix=%s
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include

Name: QtForge
Description: Modern C++ Plugin System for Qt Applications with Modular Architecture
Version: %s
Libs: -L${libdir} -lqtforge-core
Cflags: -I${includedir}
]], target:installdir(), "3.0.0")

    local pkgconfig_dir = path.join(target:installdir(), "lib", "pkgconfig")
    os.mkdir(pkgconfig_dir)
    io.writefile(path.join(pkgconfig_dir, "qtforge.pc"), pkgconfig_content)
end)

-- Print build configuration summary (matching CMake summary)
after_build(function (target)
    if target:name() == "QtForgeCore" then
        print("")
        print("QtForge Build System Summary:")
        print("=============================")
        print("XMake build system successfully configured!")
        print("Platform: " .. os.host())
        print("Architecture: " .. os.arch())
        print("Build Mode: " .. get_config("mode"))
        print("")
        print("Library Types:")
        print("  Shared Libraries: " .. tostring(has_config("shared")))
        print("  Static Libraries: " .. tostring(has_config("static")))
        print("")
        print("Components:")
        print("  Network Support: " .. tostring(qt_features.network))
        print("  UI Support: " .. tostring(qt_features.widgets))
        print("  SQL Support: " .. tostring(qt_features.sql))
        print("  Concurrent Support: " .. tostring(qt_features.concurrent))
        print("  StateMachine Support: " .. tostring(qt_features.statemachine))
        print("")
        print("Development:")
        print("  Examples: " .. tostring(has_config("examples")))
        print("  Tests: " .. tostring(has_config("tests")))
        print("  Documentation: " .. tostring(has_config("docs")))
        print("")
        print("Scripting Bindings:")
        print("  Python Bindings: " .. tostring(has_config("python_bindings")))
        print("  Lua Bindings: " .. tostring(has_config("lua_bindings")))
        print("")
    end
end)
