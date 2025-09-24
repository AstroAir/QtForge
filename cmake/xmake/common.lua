-- QtForge Common XMake Configuration Module
-- Shared build settings, compiler configurations, and common patterns
-- Version: 3.2.0

-- Module table
local common = {}

-- Common project settings
common.XMAKE_VERSION = "3.0.1"
common.CPP_STANDARD = "c++20"
common.QTFORGE_VERSION = "3.2.0"

-- Common build modes
common.build_modes = {"mode.debug", "mode.release", "mode.releasedbg", "mode.minsizerel"}

-- Common Qt package configurations
common.qt_package_config = {
    optional = true,
    configs = {
        shared = true,
        runtimes = is_plat("windows") and "MD" or nil
    }
}

-- Apply common project settings
function common.setup_project(name, version)
    set_xmakever(common.XMAKE_VERSION)
    set_project(name or "QtForgeExample")
    set_version(version or "1.0.0")
    set_languages(common.CPP_STANDARD)

    -- Add build modes
    for _, mode in ipairs(common.build_modes) do
        add_rules(mode)
    end
end

-- Apply common compiler settings with toolchain awareness
function common.setup_compiler(toolchain)
    toolchain = toolchain or "auto"

    -- Platform-specific configurations
    if is_plat("windows") then
        add_defines("WIN32_LEAN_AND_MEAN", "NOMINMAX")
        add_defines("QTFORGE_PLATFORM_WINDOWS")

        -- Toolchain-specific compiler settings
        if toolchain == "msvc" then
            add_cxflags("/utf-8", "/permissive-")
            add_defines("QTFORGE_TOOLCHAIN_MSVC")
        elseif toolchain == "mingw64" then
            add_cxflags("-std=c++20", "-Wall", "-Wextra")
            add_defines("QTFORGE_TOOLCHAIN_MINGW64")
            -- MinGW64 specific settings
            add_cxflags("-fPIC")
            add_syslinks("pthread")
        end
    elseif is_plat("linux") then
        add_cxflags("-fPIC")
        add_syslinks("pthread", "dl")
        add_defines("QTFORGE_PLATFORM_LINUX")
    elseif is_plat("macosx") then
        add_cxflags("-fPIC")
        add_frameworks("Foundation")
        add_defines("QTFORGE_PLATFORM_MACOS")
    end

    -- Build mode specific settings
    if is_mode("debug") then
        add_defines("QTFORGE_DEBUG=1")
        set_symbols("debug")
        set_optimize("none")
        if has_config("sanitizers") then
            if toolchain == "mingw64" or not is_plat("windows") then
                add_cxflags("-fsanitize=address", "-fsanitize=undefined")
                add_ldflags("-fsanitize=address", "-fsanitize=undefined")
            end
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
end

-- Apply common warning settings
function common.setup_warnings()
    if is_plat("windows") then
        if is_config("toolchain", "msvc") then
            add_cxxflags("/W4", "/permissive-")
        else
            add_cxxflags("-Wall", "-Wextra", "-Wpedantic")
        end
    else
        add_cxxflags("-Wall", "-Wextra", "-Wpedantic")
    end
end

-- Common Qt framework setup
function common.add_qt_frameworks(frameworks)
    frameworks = frameworks or {"QtCore"}
    for _, framework in ipairs(frameworks) do
        add_frameworks(framework)
    end
end

-- Common QtForge include directories
function common.add_qtforge_includes()
    add_includedirs("../../../include", {public = false})
end

-- Common QtForge dependencies
function common.add_qtforge_deps(deps)
    deps = deps or {"QtForgeCore"}
    for _, dep in ipairs(deps) do
        add_deps(dep)
    end
end

-- Common output directory setup
function common.set_output_dirs(plugin_dir, bin_dir)
    plugin_dir = plugin_dir or "$(buildir)/plugins"
    bin_dir = bin_dir or "$(buildir)/bin"

    -- Return both for use in target configuration
    return plugin_dir, bin_dir
end

-- Common install file patterns
function common.install_plugin_metadata(json_file, prefix)
    prefix = prefix or "plugins"
    add_installfiles(json_file, {prefixdir = prefix})
end

-- Common after_build actions for plugins
function common.copy_plugin_metadata(json_file)
    after_build(function (target)
        os.cp(json_file, target:targetdir())
    end)
end

-- Common debug postfix handling
function common.set_debug_postfix(basename)
    if is_mode("debug") then
        set_basename(basename .. "_d")
    else
        set_basename(basename)
    end
end

-- Version definitions
function common.add_version_defines()
    add_defines("QTFORGE_VERSION_MAJOR=3")
    add_defines("QTFORGE_VERSION_MINOR=2")
    add_defines("QTFORGE_VERSION_PATCH=0")
    add_defines("QTFORGE_VERSION_STRING=\"3.2.0\"")
end

-- Enhanced utility functions for better code reuse

-- File and directory utilities
function common.ensure_directory_exists(dir_path)
    if not os.isdir(dir_path) then
        os.mkdir(dir_path)
        return true
    end
    return false
end

function common.copy_files_if_exist(files, target_dir)
    local copied = {}
    for _, file in ipairs(files) do
        if os.isfile(file) then
            os.cp(file, target_dir)
            table.insert(copied, file)
        end
    end
    return copied
end

-- Build configuration utilities
function common.is_debug_mode()
    return is_mode("debug")
end

function common.is_release_mode()
    return is_mode("release") or is_mode("releasedbg") or is_mode("minsizerel")
end

function common.get_build_suffix()
    if common.is_debug_mode() then
        return "_d"
    else
        return ""
    end
end

-- Target configuration helpers
function common.configure_plugin_target(name, config)
    config = config or {}

    target(name)
        set_kind("shared")
        set_basename(config.basename or name:lower())

        -- Add debug suffix if in debug mode
        if config.debug_postfix ~= false and common.is_debug_mode() then
            set_basename((config.basename or name:lower()) .. "_d")
        end

        -- Set output directory
        set_targetdir(config.output_dir or "$(buildir)/plugins")

        -- Set version
        set_version(config.version or "1.0.0")

        -- Add common plugin defines
        add_defines("QT_PLUGIN")

        -- Set visibility for shared libraries
        if config.visibility then
            set_symbols(config.visibility)
        end

        -- Add custom configuration if provided
        if config.custom_config then
            config.custom_config()
        end
    target_end()
end

-- Validation utilities
function common.validate_config(config, required_fields)
    for _, field in ipairs(required_fields) do
        if not config[field] then
            error("Missing required configuration field: " .. field)
        end
    end
end

function common.validate_files_exist(files)
    local missing = {}
    for _, file in ipairs(files) do
        if not os.isfile(file) then
            table.insert(missing, file)
        end
    end
    return missing
end

-- Logging and output utilities
function common.log_info(message)
    print("[INFO] " .. message)
end

function common.log_warning(message)
    print("[WARNING] " .. message)
end

function common.log_error(message)
    print("[ERROR] " .. message)
end

function common.log_success(message)
    print("[SUCCESS] " .. message)
end

-- Template system utilities
function common.create_template_function(template_name, default_config, setup_func)
    return function(name, config)
        config = config or {}

        -- Merge with defaults
        for k, v in pairs(default_config) do
            if config[k] == nil then
                config[k] = v
            end
        end

        -- Validate required fields
        if config.required_fields then
            common.validate_config(config, config.required_fields)
        end

        -- Call setup function
        setup_func(name, config)
    end
end

-- Enhanced installation utilities
function common.install_with_structure(files, base_dir, preserve_structure)
    preserve_structure = preserve_structure or false

    for _, file in ipairs(files) do
        if os.isfile(file) then
            if preserve_structure then
                local rel_path = path.relative(file, ".")
                local install_path = path.join(base_dir, path.directory(rel_path))
                add_installfiles(file, {prefixdir = install_path})
            else
                add_installfiles(file, {prefixdir = base_dir})
            end
        end
    end
end

-- Enhanced Qt6 dependency management with toolchain support

-- Detect active toolchain
function common.detect_active_toolchain()
    local toolchain_selector = import("toolchain_selector")

    -- Check if toolchain is already detected
    if toolchain_selector.selected.name then
        return toolchain_selector.selected.name
    end

    -- Perform detection
    if toolchain_selector.detect_all() and toolchain_selector.select_toolchain() then
        return toolchain_selector.selected.name
    end

    return "unknown"
end

-- Get toolchain-specific build settings
function common.get_toolchain_build_settings(toolchain)
    local settings = {
        defines = {},
        cxxflags = {},
        ldflags = {},
        syslinks = {}
    }

    if toolchain == "msvc" then
        settings.defines = {"QTFORGE_TOOLCHAIN_MSVC"}
        settings.cxxflags = {"/utf-8", "/permissive-"}
    elseif toolchain == "mingw64" then
        settings.defines = {"QTFORGE_TOOLCHAIN_MINGW64"}
        settings.cxxflags = {"-std=c++20", "-Wall", "-Wextra", "-fPIC"}
        settings.syslinks = {"pthread"}
    end

    return settings
end

-- Apply toolchain-specific settings to target
function common.apply_toolchain_settings(toolchain)
    local settings = common.get_toolchain_build_settings(toolchain)

    for _, define in ipairs(settings.defines) do
        add_defines(define)
    end

    for _, flag in ipairs(settings.cxxflags) do
        add_cxxflags(flag)
    end

    for _, flag in ipairs(settings.ldflags) do
        add_ldflags(flag)
    end

    for _, lib in ipairs(settings.syslinks) do
        add_syslinks(lib)
    end
end

-- Add QtForge dependencies with toolchain awareness
function common.add_qtforge_deps_with_toolchain(toolchain)
    toolchain = toolchain or common.detect_active_toolchain()

    -- Import required modules
    local dependencies = import("dependencies")
    local qt6_detector = import("qt6_detector")

    -- Apply toolchain-specific settings
    common.apply_toolchain_settings(toolchain)

    -- Setup Qt6 with toolchain support
    if dependencies.setup_qt6_with_toolchain(toolchain, {}) then
        print("Qt6 dependencies configured for " .. toolchain .. " toolchain")

        -- Add Qt6 packages based on build configuration
        local qt_config = dependencies.get_toolchain_qt_config(toolchain)

        -- Core Qt6 packages
        add_packages("qt6core", qt_config)

        -- Conditional packages
        if has_config("ui") then
            add_packages("qt6gui", qt_config)
            add_packages("qt6widgets", qt_config)
        end

        if has_config("network") then
            add_packages("qt6network", qt_config)
        end

        if has_config("sql") then
            add_packages("qt6sql", qt_config)
        end

        if has_config("concurrent") then
            add_packages("qt6concurrent", qt_config)
        end

        -- Add Qt6 rules
        add_rules("qt.shared")

        return true
    else
        print("Warning: Failed to configure Qt6 for " .. toolchain .. " toolchain")
        return false
    end
end

-- Configure Qt6 linking for MinGW64
function common.configure_mingw64_qt6_linking(components)
    components = components or {"Core"}

    local qt6_detector = import("qt6_detector")
    local qt_info = qt6_detector.get_info("mingw64")

    if not qt_info.available then
        return false
    end

    -- Add Qt6 library directory
    add_linkdirs(path.join(qt_info.path, "lib"))

    -- Add Qt6 libraries with proper naming for MinGW64
    for _, component in ipairs(components) do
        -- MinGW64 uses libQt6Component.dll.a for shared libraries
        local lib_name = "Qt6" .. component
        add_links(lib_name)
    end

    -- Add Windows-specific libraries for Qt6
    add_syslinks("user32", "gdi32", "shell32", "ole32", "oleaut32", "uuid", "winmm", "ws2_32")

    return true
end

-- Configure Qt6 linking for MSVC
function common.configure_msvc_qt6_linking(components)
    components = components or {"Core"}

    local qt6_detector = import("qt6_detector")
    local qt_info = qt6_detector.get_info("msvc")

    if not qt_info.available then
        return false
    end

    -- Add Qt6 library directory
    add_linkdirs(path.join(qt_info.path, "lib"))

    -- Add Qt6 libraries with MSVC naming
    for _, component in ipairs(components) do
        local lib_name = "Qt6" .. component
        add_links(lib_name)
    end

    return true
end

-- Universal Qt6 linking configuration
function common.configure_qt6_linking(toolchain, components)
    if toolchain == "mingw64" then
        return common.configure_mingw64_qt6_linking(components)
    elseif toolchain == "msvc" then
        return common.configure_msvc_qt6_linking(components)
    end

    return false
end

-- Get Qt6 components from build configuration
function common.get_qt6_components_from_config()
    local components = {"Core"}

    if has_config("ui") then
        table.insert(components, "Gui")
        table.insert(components, "Widgets")
    end

    if has_config("network") then
        table.insert(components, "Network")
    end

    if has_config("sql") then
        table.insert(components, "Sql")
    end

    if has_config("concurrent") then
        table.insert(components, "Concurrent")
    end

    return components
end

-- MinGW64-specific utilities

-- Check if MinGW64 is available
function common.is_mingw64_available()
    local msys2_mingw64 = import("msys2_mingw64")
    return msys2_mingw64.detected.available and msys2_mingw64.detected.mingw64_prefix ~= nil
end

-- Get MinGW64 installation path
function common.get_mingw64_path()
    local msys2_mingw64 = import("msys2_mingw64")
    return msys2_mingw64.detected.mingw64_prefix
end

-- Configure MinGW64 environment variables
function common.setup_mingw64_environment()
    if not common.is_mingw64_available() then
        return false
    end

    local mingw64_path = common.get_mingw64_path()
    local bin_path = path.join(mingw64_path, "bin")

    -- Add MinGW64 bin to PATH
    add_toolchains("mingw")

    -- Set environment variables for build
    set_config("mingw64_root", mingw64_path)
    set_config("mingw64_bin", bin_path)

    return true
end

-- Get MinGW64 compiler version
function common.get_mingw64_compiler_version()
    if not common.is_mingw64_available() then
        return nil
    end

    local mingw64_path = common.get_mingw64_path()
    local gcc_exe = path.join(mingw64_path, "bin", "gcc.exe")

    if os.isfile(gcc_exe) then
        local result = os.iorunv(gcc_exe, {"--version"})
        if result then
            local version = result:match("gcc %(GCC%) ([%d%.]+)")
            return version
        end
    end

    return nil
end

-- Validate MinGW64 toolchain for QtForge
function common.validate_mingw64_toolchain()
    local validation = {
        valid = true,
        issues = {},
        info = {}
    }

    if not common.is_mingw64_available() then
        validation.valid = false
        table.insert(validation.issues, "MinGW64 not available")
        return validation
    end

    local mingw64_path = common.get_mingw64_path()
    local bin_path = path.join(mingw64_path, "bin")

    -- Check essential tools
    local essential_tools = {"gcc.exe", "g++.exe", "ar.exe", "ranlib.exe", "ld.exe"}

    for _, tool in ipairs(essential_tools) do
        local tool_path = path.join(bin_path, tool)
        if os.isfile(tool_path) then
            table.insert(validation.info, tool .. " found at " .. tool_path)
        else
            validation.valid = false
            table.insert(validation.issues, tool .. " not found")
        end
    end

    -- Check compiler version
    local compiler_version = common.get_mingw64_compiler_version()
    if compiler_version then
        table.insert(validation.info, "GCC version: " .. compiler_version)
    else
        table.insert(validation.issues, "Could not determine GCC version")
    end

    return validation
end

-- Setup complete MinGW64 development environment
function common.setup_mingw64_development_environment()
    print("Setting up MinGW64 development environment...")

    -- Validate toolchain
    local validation = common.validate_mingw64_toolchain()
    if not validation.valid then
        print("MinGW64 validation failed:")
        for _, issue in ipairs(validation.issues) do
            print("  - " .. issue)
        end
        return false
    end

    -- Setup environment
    if not common.setup_mingw64_environment() then
        print("Failed to setup MinGW64 environment")
        return false
    end

    -- Validate MSYS2 packages
    local msys2_mingw64 = import("msys2_mingw64")
    local required_features = common.get_qt6_components_from_config()
    local package_validation = msys2_mingw64.validate_environment(required_features)

    if not package_validation.valid then
        print("MSYS2 package validation issues found:")
        for _, issue in ipairs(package_validation.issues) do
            print("  - " .. issue)
        end
        print("Suggestions:")
        for _, suggestion in ipairs(package_validation.suggestions) do
            print("  - " .. suggestion)
        end
    end

    print("MinGW64 development environment setup completed")
    return true
end

-- Export the module
return common
