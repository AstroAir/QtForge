-- QtForge Library XMake Configuration
-- Modern C++ Plugin System for Qt Applications with Modular Architecture
-- Version: 3.0.0

-- Set minimum xmake version
set_xmakever("3.0.1")

-- Project configuration
set_project("QtForge")
set_version("3.0.0", {build = "%Y%m%d%H%M"})

-- Add build modes
add_rules("mode.debug", "mode.release", "mode.releasedbg", "mode.minsizerel")

-- Build options
option("shared")
    set_default(true)
    set_description("Build shared libraries")
    set_showmenu(true)
option_end()

option("static")
    set_default(false)
    set_description("Build static libraries")
    set_showmenu(true)
option_end()

option("examples")
    set_default(true)
    set_description("Build examples")
    set_showmenu(true)
option_end()

option("tests")
    set_default(false)
    set_description("Build tests")
    set_showmenu(true)
option_end()

option("python_bindings")
    set_default(false)
    set_description("Build Python bindings")
    set_showmenu(true)
option_end()

-- Modern Qt6 configuration using xmake-repo packages
-- This approach automatically handles Qt installation and integration
-- Supports multiple Qt installation methods as documented in xmake.io

-- Qt6 Core packages (always required)
add_requires("qt6core", {
    optional = true,
    configs = {
        shared = has_config("shared"),
        runtimes = "MD"
    }
})

-- Qt6 Additional packages (optional based on availability)
add_requires("qt6gui", {
    optional = true,
    configs = {
        shared = has_config("shared"),
        runtimes = "MD"
    }
})

add_requires("qt6widgets", {
    optional = true,
    configs = {
        shared = has_config("shared"),
        runtimes = "MD"
    }
})

add_requires("qt6network", {
    optional = true,
    configs = {
        shared = has_config("shared"),
        runtimes = "MD"
    }
})

add_requires("qt6sql", {
    optional = true,
    configs = {
        shared = has_config("shared"),
        runtimes = "MD"
    }
})

add_requires("qt6concurrent", {
    optional = true,
    configs = {
        shared = has_config("shared"),
        runtimes = "MD"
    }
})

-- Python bindings dependencies
if has_config("python_bindings") then
    add_requires("python3", {optional = true})
    add_requires("pybind11", {optional = true})
end

-- Lua bindings dependencies
if has_config("lua_bindings") then
    add_requires("lua", {optional = true, configs = {version = "5.4"}})
    add_requires("sol2", {optional = true})
end

-- Global compiler configuration
set_languages("c++20")
set_warnings("all")

-- Platform-specific configurations
if is_plat("windows") then
    add_cxflags("/utf-8")
    add_defines("WIN32_LEAN_AND_MEAN", "NOMINMAX")
elseif is_plat("linux") then
    add_cxflags("-fPIC")
    add_syslinks("pthread", "dl")
elseif is_plat("macosx") then
    add_cxflags("-fPIC")
    add_frameworks("Foundation")
end

-- Debug configuration
if is_mode("debug") then
    add_defines("QTFORGE_DEBUG=1")
    set_symbols("debug")
    set_optimize("none")
elseif is_mode("release") then
    add_defines("QTFORGE_RELEASE=1", "NDEBUG=1")
    set_symbols("hidden")
    set_optimize("fastest")
    set_strip("all")
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

-- Modern Qt feature detection using package availability
-- This approach uses xmake's package system to detect Qt components
local qt_features = {}

-- Check for Qt packages using xmake's has_package function
-- This is more reliable than manual library detection
local function check_qt_package(package_name)
    -- For now, disable optional Qt packages to avoid linking issues
    -- Only enable qt6core which is required
    if package_name == "qt6core" then
        return has_package(package_name)
    end
    return false
end

-- Detect available Qt components using package detection
-- Only enable features if packages are actually linkable
if check_qt_package("qt6network") then
    qt_features.network = true
    add_defines("QTFORGE_HAS_NETWORK")
    print("Qt6Network: Available")
else
    qt_features.network = false
    print("Qt6Network: Not available")
end

if check_qt_package("qt6widgets") then
    qt_features.widgets = true
    add_defines("QTFORGE_HAS_WIDGETS")
    print("Qt6Widgets: Available")
else
    qt_features.widgets = false
    print("Qt6Widgets: Not available")
end

if check_qt_package("qt6sql") then
    qt_features.sql = true
    add_defines("QTFORGE_HAS_SQL")
    print("Qt6SQL: Available")
else
    qt_features.sql = false
    print("Qt6SQL: Not available")
end

if check_qt_package("qt6concurrent") then
    qt_features.concurrent = true
    add_defines("QTFORGE_HAS_CONCURRENT")
    print("Qt6Concurrent: Available")
else
    qt_features.concurrent = false
    print("Qt6Concurrent: Not available")
end

-- Version definitions
add_defines("QTFORGE_VERSION_MAJOR=3")
add_defines("QTFORGE_VERSION_MINOR=0")
add_defines("QTFORGE_VERSION_PATCH=0")
add_defines("QTFORGE_VERSION_STRING=\"3.0.0\"")

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

-- Core library sources (ultra-minimal working version for initial xmake support)
local qtforge_core_sources = {
    "src/qtplugin.cpp",
    "src/core/plugin_interface.cpp",
    "src/core/plugin_loader.cpp",
    "src/utils/version.cpp",
    "src/utils/error_handling.cpp",
    -- Basic communication components (no Qt dependencies)
    "src/communication/plugin_service_contracts.cpp",
    -- Dynamic plugin system sources (v3.2.0)
    "src/core/dynamic_plugin_interface.cpp"
}

-- Sources with complex dependencies (temporarily disabled)
local qtforge_complex_sources = {
    -- These have dependencies on MOC classes or other complex components
    "src/security/security_manager.cpp",           -- depends on PermissionManager, SecurityPolicyEngine
    "src/core/advanced_plugin_interface.cpp",     -- depends on RequestResponseSystem
    "src/bridges/python_plugin_bridge.cpp"        -- complex inheritance
}

-- MOC-dependent sources (temporarily disabled until MOC integration is complete)
local qtforge_moc_sources = {
    -- These sources contain Q_OBJECT and require Qt MOC processing
    "src/core/plugin_manager.cpp",
    "src/core/plugin_registry.cpp",
    "src/core/plugin_dependency_resolver.cpp",
    "src/core/plugin_lifecycle_manager.cpp",
    "src/monitoring/plugin_hot_reload_manager.cpp",
    "src/monitoring/plugin_metrics_collector.cpp",
    "src/communication/message_bus.cpp",
    "src/communication/request_response_system.cpp",
    "src/security/components/permission_manager.cpp",
    "src/security/components/security_policy_engine.cpp",
    "src/managers/configuration_manager.cpp",
    "src/managers/components/configuration_storage.cpp",
    "src/managers/components/configuration_merger.cpp",
    "src/managers/components/configuration_watcher.cpp",
    "src/managers/logging_manager.cpp",
    "src/managers/resource_manager.cpp",
    "src/managers/resource_lifecycle.cpp",
    "src/managers/components/resource_pool.cpp",
    "src/managers/components/resource_allocator.cpp",
    "src/managers/resource_monitor.cpp",
    "src/orchestration/plugin_orchestrator.cpp",
    "src/composition/plugin_composition.cpp",
    "src/transactions/plugin_transaction_manager.cpp"
}

-- Optional sources that require Qt Network (temporarily disabled due to MOC dependencies)
local qtforge_network_sources = {
    -- Plugin marketplace integration (v3.2.0) - requires QNetworkAccessManager and MOC
    -- "src/marketplace/plugin_marketplace.cpp"  -- contains Q_OBJECT, needs MOC
}

-- Optional sources that require Qt Widgets (temporarily disabled due to MOC dependencies)
local qtforge_widgets_sources = {
    -- Advanced plugin composition (v3.2.0) - requires QGraphicsScene and MOC
    -- "src/orchestration/advanced/plugin_orchestrator_v2.cpp"  -- contains Q_OBJECT, needs MOC
}

-- Additional MOC sources (special cases with Q_OBJECT in .cpp files)
local qtforge_additional_moc_sources = {
    -- Version management sources (v3.1.0) - requires MOC for Qt meta-object system
    -- Temporarily disabled until proper MOC support for .cpp files is implemented
    -- "src/managers/plugin_version_manager.cpp"
}

-- Core library headers
local qtforge_core_headers = {
    "include/qtplugin/core/plugin_interface.hpp",
    "include/qtplugin/core/plugin_manager.hpp",
    "include/qtplugin/core/plugin_loader.hpp",
    "include/qtplugin/core/plugin_registry.hpp",
    "include/qtplugin/core/plugin_dependency_resolver.hpp",
    "include/qtplugin/core/plugin_lifecycle_manager.hpp",
    "include/qtplugin/monitoring/plugin_hot_reload_manager.hpp",
    "include/qtplugin/monitoring/plugin_metrics_collector.hpp",
    "include/qtplugin/core/service_plugin_interface.hpp",
    "include/qtplugin/communication/message_bus.hpp",
    "include/qtplugin/communication/message_types.hpp",
    "include/qtplugin/communication/request_response_system.hpp",
    "include/qtplugin/utils/version.hpp",
    "include/qtplugin/utils/error_handling.hpp",
    "include/qtplugin/security/security_manager.hpp",
    "include/qtplugin/security/components/security_validator.hpp",
    "include/qtplugin/security/components/signature_verifier.hpp",
    "include/qtplugin/security/components/permission_manager.hpp",
    "include/qtplugin/security/components/security_policy_engine.hpp",
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
    "include/qtplugin/core/advanced_plugin_interface.hpp",
    "include/qtplugin/orchestration/plugin_orchestrator.hpp",
    "include/qtplugin/composition/plugin_composition.hpp",
    "include/qtplugin/transactions/plugin_transaction_manager.hpp",
    -- Version management headers (v3.1.0)
    "include/qtplugin/managers/plugin_version_manager.hpp",
    -- Dynamic plugin system headers (v3.2.0)
    "include/qtplugin/core/dynamic_plugin_interface.hpp",
    -- Multi-language plugin bridge headers (v3.2.0)
    "include/qtplugin/bridges/python_plugin_bridge.hpp",
    -- Enhanced plugin composition headers (v3.2.0)
    "include/qtplugin/orchestration/advanced/plugin_orchestrator_v2.hpp",
    -- Plugin marketplace integration headers (v3.2.0)
    "include/qtplugin/marketplace/plugin_marketplace.hpp"
}

-- QtForgeCore library target
target("QtForgeCore")
    set_kind(has_config("shared") and "shared" or "static")
    set_basename("qtforge-core")

    -- Use modern Qt rules based on library type
    if has_config("shared") then
        add_rules("qt.shared")
    else
        add_rules("qt.static")
    end

    -- Add source files
    add_files(qtforge_core_sources)

    -- Add network-dependent sources if Qt Network is available
    if qt_features.network then
        add_files(qtforge_network_sources)
    end

    -- Add widgets-dependent sources if Qt Widgets is available
    if qt_features.widgets then
        add_files(qtforge_widgets_sources)
    end

    -- MOC-dependent sources temporarily disabled until MOC integration is complete
    -- add_files(qtforge_moc_sources)
    -- add_files(qtforge_additional_moc_sources)

    -- Add header files that need MOC processing
    add_headerfiles(qtforge_core_headers)

    -- Modern Qt package integration
    add_packages("qt6core")
    if qt_features.network then
        add_packages("qt6network")
    end
    if qt_features.widgets then
        add_packages("qt6widgets", "qt6gui")
    end
    if qt_features.sql then
        add_packages("qt6sql")
    end
    if qt_features.concurrent then
        add_packages("qt6concurrent")
    end
    if qt_features.sql then
        add_frameworks("QtSql")
    end
    if qt_features.concurrent then
        add_frameworks("QtConcurrent")
    end

    -- Set version
    set_version("3.0.0")

    -- Export symbols for shared library
    if has_config("shared") then
        add_defines("QTFORGE_CORE_EXPORTS")
    end

    -- Install headers
    add_installfiles("include/(qtplugin/**.hpp)", {prefixdir = "include"})
target_end()

-- QtForgeSecurity library target
target("QtForgeSecurity")
    set_kind(has_config("shared") and "shared" or "static")
    set_basename("qtforge-security")

    -- Add Qt rules
    add_rules("qt.shared")

    -- Add source files
    add_files("src/security/security_manager.cpp")
    add_files("src/security/components/security_validator.cpp")
    add_files("src/security/components/signature_verifier.cpp")
    add_files("src/security/components/permission_manager.cpp")
    add_files("src/security/components/security_policy_engine.cpp")

    -- Add header files
    add_headerfiles("include/qtplugin/security/security_manager.hpp")
    add_headerfiles("include/qtplugin/security/components/security_validator.hpp")
    add_headerfiles("include/qtplugin/security/components/signature_verifier.hpp")
    add_headerfiles("include/qtplugin/security/components/permission_manager.hpp")
    add_headerfiles("include/qtplugin/security/components/security_policy_engine.hpp")

    -- Add Qt packages for proper MOC support
    add_packages("qt6core")

    -- Add Qt frameworks
    add_frameworks("QtCore")

    -- Add dependency on QtForgeCore
    add_deps("QtForgeCore")

    -- Set version
    set_version("3.0.0")

    -- Export symbols for shared library
    if has_config("shared") then
        add_defines("QTFORGE_SECURITY_EXPORTS")
    end
target_end()

-- Optional QtForgeNetwork library
if qt_features.network then
    target("QtForgeNetwork")
        set_kind(has_config("shared") and "shared" or "static")
        set_basename("qtforge-network")

        -- Add Qt rules
        add_rules("qt.shared")

        -- Add Qt libraries
        add_links("Qt6Core", "Qt6Network")

        -- Add dependency on QtForgeCore
        add_deps("QtForgeCore")

        -- Set version
        set_version("3.0.0")

        -- Export symbols for shared library
        if has_config("shared") then
            add_defines("QTFORGE_NETWORK_EXPORTS")
        end

        -- Note: Source files will be added when implemented
        -- add_files("src/network/*.cpp")
        -- add_headerfiles("include/qtplugin/network/*.hpp")
    target_end()
end

-- Optional QtForgeUI library
if qt_features.widgets then
    target("QtForgeUI")
        set_kind(has_config("shared") and "shared" or "static")
        set_basename("qtforge-ui")

        -- Add Qt rules
        add_rules("qt.shared")

        -- Add Qt libraries
        add_links("Qt6Core", "Qt6Widgets", "Qt6Gui")

        -- Add dependency on QtForgeCore
        add_deps("QtForgeCore")

        -- Set version
        set_version("3.0.0")

        -- Export symbols for shared library
        if has_config("shared") then
            add_defines("QTFORGE_UI_EXPORTS")
        end

        -- Note: Source files will be added when implemented
        -- add_files("src/ui/*.cpp")
        -- add_headerfiles("include/qtplugin/ui/*.hpp")
    target_end()
end

-- Python bindings target
if has_config("python_bindings") and has_package("python3") and has_package("pybind11") then
    target("qtforge_python")
        set_kind("shared")
        set_basename("qtforge")

        -- Add Python module rules
        add_rules("python.library", {soabi = true})

        -- Python binding sources
        add_files("src/python/qtforge_python.cpp")
        add_files("src/python/qt_conversions.cpp")
        add_files("src/python/core/core_bindings.cpp")
        add_files("src/python/utils/utils_bindings.cpp")
        add_files("src/python/security/security_bindings.cpp")
        add_files("src/python/managers/managers_bindings.cpp")
        add_files("src/python/orchestration/orchestration_bindings.cpp")
        add_files("src/python/monitoring/monitoring_bindings.cpp")
        add_files("src/python/transactions/transaction_bindings.cpp")
        add_files("src/python/composition/composition_bindings.cpp")
        add_files("src/python/marketplace/marketplace_bindings.cpp")
        add_files("src/python/threading/threading_bindings.cpp")

        -- Communication bindings (if Qt6Network available)
        if has_package("qt6network") then
            add_files("src/python/communication/communication_bindings.cpp")
        end

        -- Add packages
        add_packages("python3", "pybind11")
        add_packages("qt6core")
        if has_package("qt6network") then
            add_packages("qt6network")
        end
        if has_package("qt6widgets") then
            add_packages("qt6widgets")
        end
        if has_package("qt6sql") then
            add_packages("qt6sql")
        end

        -- Add dependencies
        add_deps("QtForgeCore")

        -- Set properties
        set_symbols("hidden")
        add_defines("QTFORGE_PYTHON_BINDINGS")

        -- Version definitions
        add_defines("QTPLUGIN_VERSION_MAJOR=3")
        add_defines("QTPLUGIN_VERSION_MINOR=0")
        add_defines("QTPLUGIN_VERSION_PATCH=0")
    target_end()
end

-- Lua bindings target
if has_config("lua_bindings") and has_package("lua") and has_package("sol2") then
    target("qtforge_lua")
        set_kind("shared")
        set_basename("qtforge_lua")

        -- Lua binding sources (will be created in next phase)
        add_files("src/lua/qtforge_lua.cpp")
        add_files("src/lua/qt_conversions.cpp")
        add_files("src/lua/core/core_bindings.cpp")
        add_files("src/lua/utils/utils_bindings.cpp")
        add_files("src/lua/security/security_bindings.cpp")
        add_files("src/lua/managers/managers_bindings.cpp")
        add_files("src/lua/orchestration/orchestration_bindings.cpp")
        add_files("src/lua/monitoring/monitoring_bindings.cpp")
        add_files("src/lua/transactions/transaction_bindings.cpp")
        add_files("src/lua/composition/composition_bindings.cpp")
        add_files("src/lua/marketplace/marketplace_bindings.cpp")
        add_files("src/lua/threading/threading_bindings.cpp")

        -- Communication bindings (if Qt6Network available)
        if has_package("qt6network") then
            add_files("src/lua/communication/communication_bindings.cpp")
        end

        -- Add packages
        add_packages("lua", "sol2")
        add_packages("qt6core")
        if has_package("qt6network") then
            add_packages("qt6network")
        end
        if has_package("qt6widgets") then
            add_packages("qt6widgets")
        end
        if has_package("qt6sql") then
            add_packages("qt6sql")
        end

        -- Add dependencies
        add_deps("QtForgeCore")

        -- Set properties
        set_symbols("hidden")
        add_defines("QTFORGE_LUA_BINDINGS")

        -- Version definitions
        add_defines("QTPLUGIN_VERSION_MAJOR=3")
        add_defines("QTPLUGIN_VERSION_MINOR=0")
        add_defines("QTPLUGIN_VERSION_PATCH=0")
    target_end()
end

-- Examples
if has_config("examples") then
    includes("examples")
end

-- Tests
if has_config("tests") then
    includes("tests")
end

-- Installation rules
on_install(function (target)
    -- Install libraries
    if target:name() == "QtForgeCore" or target:name() == "QtForgeSecurity" or
       target:name() == "QtForgeNetwork" or target:name() == "QtForgeUI" then
        os.cp(target:targetfile(), path.join(target:installdir(), "lib"))
    end

    -- Install headers
    if target:name() == "QtForgeCore" then
        os.cp("include/qtplugin", path.join(target:installdir(), "include"))
    end

    -- Install Python module
    if target:name() == "qtforge_python" then
        local python_site_packages = path.join(target:installdir(), "lib", "python", "site-packages")
        os.mkdir(python_site_packages)
        os.cp(target:targetfile(), python_site_packages)
    end
end)

-- Package configuration
on_package(function (target)
    -- Create package metadata
    local package_info = {
        name = "QtForge",
        version = "3.0.0",
        description = "Modern C++ Plugin System for Qt Applications with Modular Architecture",
        homepage = "https://github.com/xmake-io/QtForge",
        license = "Apache-2.0"
    }

    -- Save package info
    io.writefile(path.join(target:installdir(), "package.json"), json.encode(package_info))
end)
