-- QtForge MSYS2 MinGW64 Toolchain Detection and Configuration Module
-- Comprehensive support for MSYS2 MinGW64 development environment
-- Version: 3.2.0

-- Module table
local msys2_mingw64 = {}

-- MSYS2 environment detection
msys2_mingw64.paths = {
    -- Common MSYS2 installation paths
    msys2_roots = {
        "C:/msys64",
        "D:/msys64",
        "C:/tools/msys64",
        "D:/tools/msys64",
        os.getenv("MSYS2_ROOT")
    },

    -- MinGW64 prefixes within MSYS2
    mingw64_prefixes = {
        "/mingw64",
        "/clang64",
        "/ucrt64"
    }
}

-- Detected environment information
msys2_mingw64.detected = {
    available = false,
    msys2_root = nil,
    mingw64_prefix = nil,
    toolchain_prefix = nil,
    qt_installation = nil,
    pacman_available = false
}

-- Detect MSYS2 installation
function msys2_mingw64.detect_msys2()
    for _, root in ipairs(msys2_mingw64.paths.msys2_roots) do
        if root and os.isdir(root) then
            -- Check if this is a valid MSYS2 installation
            local msys2_exe = path.join(root, "msys2.exe")
            local usr_bin = path.join(root, "usr", "bin")

            if os.isfile(msys2_exe) and os.isdir(usr_bin) then
                msys2_mingw64.detected.msys2_root = root
                msys2_mingw64.detected.available = true

                -- Check for pacman
                local pacman_exe = path.join(usr_bin, "pacman.exe")
                if os.isfile(pacman_exe) then
                    msys2_mingw64.detected.pacman_available = true
                end

                return true
            end
        end
    end
    return false
end

-- Detect MinGW64 toolchain within MSYS2
function msys2_mingw64.detect_mingw64_toolchain()
    if not msys2_mingw64.detected.msys2_root then
        return false
    end

    for _, prefix in ipairs(msys2_mingw64.paths.mingw64_prefixes) do
        local mingw64_path = path.join(msys2_mingw64.detected.msys2_root, prefix:sub(2)) -- Remove leading /
        local bin_path = path.join(mingw64_path, "bin")

        if os.isdir(bin_path) then
            -- Check for essential compiler tools
            local gcc_exe = path.join(bin_path, "gcc.exe")
            local gxx_exe = path.join(bin_path, "g++.exe")

            if os.isfile(gcc_exe) and os.isfile(gxx_exe) then
                msys2_mingw64.detected.mingw64_prefix = mingw64_path
                msys2_mingw64.detected.toolchain_prefix = prefix:sub(2) -- Remove leading /
                return true
            end
        end
    end
    return false
end

-- Detect Qt6 installation in MSYS2
function msys2_mingw64.detect_qt6_installation()
    if not msys2_mingw64.detected.mingw64_prefix then
        return false
    end

    local qt_paths = {
        path.join(msys2_mingw64.detected.mingw64_prefix, "lib", "qt6"),
        path.join(msys2_mingw64.detected.mingw64_prefix, "share", "qt6"),
        path.join(msys2_mingw64.detected.mingw64_prefix, "lib", "cmake", "Qt6")
    }

    for _, qt_path in ipairs(qt_paths) do
        if os.isdir(qt_path) then
            -- Check for Qt6 tools
            local qmake_exe = path.join(msys2_mingw64.detected.mingw64_prefix, "bin", "qmake6.exe")
            local moc_exe = path.join(msys2_mingw64.detected.mingw64_prefix, "bin", "moc.exe")

            if os.isfile(qmake_exe) or os.isfile(moc_exe) then
                msys2_mingw64.detected.qt_installation = msys2_mingw64.detected.mingw64_prefix
                return true
            end
        end
    end
    return false
end

-- Get Qt6 version from MSYS2 installation
function msys2_mingw64.get_qt6_version()
    if not msys2_mingw64.detected.qt_installation then
        return nil
    end

    local qmake_exe = path.join(msys2_mingw64.detected.qt_installation, "bin", "qmake6.exe")
    if os.isfile(qmake_exe) then
        local result = os.iorunv(qmake_exe, {"-query", "QT_VERSION"})
        if result then
            return result:trim()
        end
    end

    return nil
end

-- Get available Qt6 packages via pacman
function msys2_mingw64.get_available_qt6_packages()
    if not msys2_mingw64.detected.pacman_available then
        return {}
    end

    local pacman_exe = path.join(msys2_mingw64.detected.msys2_root, "usr", "bin", "pacman.exe")
    local prefix = msys2_mingw64.detected.toolchain_prefix or "mingw64"

    -- Common Qt6 packages in MSYS2
    local qt6_packages = {
        prefix .. "-qt6-base",
        prefix .. "-qt6-tools",
        prefix .. "-qt6-declarative",
        prefix .. "-qt6-multimedia",
        prefix .. "-qt6-networkauth",
        prefix .. "-qt6-svg",
        prefix .. "-qt6-imageformats",
        prefix .. "-qt6-translations"
    }

    local available = {}
    for _, package in ipairs(qt6_packages) do
        -- Check if package is installed
        local result = os.iorunv(pacman_exe, {"-Q", package})
        if result and result:find(package) then
            table.insert(available, package)
        end
    end

    return available
end

-- Configure toolchain for xmake
function msys2_mingw64.configure_toolchain()
    if not msys2_mingw64.detected.mingw64_prefix then
        return false
    end

    local bin_path = path.join(msys2_mingw64.detected.mingw64_prefix, "bin")

    -- Set toolchain paths
    set_toolchains("mingw")

    -- Configure compiler paths
    local gcc_exe = path.join(bin_path, "gcc.exe")
    local gxx_exe = path.join(bin_path, "g++.exe")
    local ar_exe = path.join(bin_path, "ar.exe")
    local ranlib_exe = path.join(bin_path, "ranlib.exe")
    local ld_exe = path.join(bin_path, "ld.exe")

    if os.isfile(gcc_exe) then set_config("cc", gcc_exe) end
    if os.isfile(gxx_exe) then set_config("cxx", gxx_exe) end
    if os.isfile(ar_exe) then set_config("ar", ar_exe) end
    if os.isfile(ranlib_exe) then set_config("ranlib", ranlib_exe) end
    if os.isfile(ld_exe) then set_config("ld", ld_exe) end

    -- Add include and library directories
    add_includedirs(path.join(msys2_mingw64.detected.mingw64_prefix, "include"))
    add_linkdirs(path.join(msys2_mingw64.detected.mingw64_prefix, "lib"))

    return true
end

-- Configure Qt6 for MinGW64
function msys2_mingw64.configure_qt6()
    if not msys2_mingw64.detected.qt_installation then
        return false
    end

    local qt_root = msys2_mingw64.detected.qt_installation
    local qt_bin = path.join(qt_root, "bin")

    -- Set Qt6 SDK directory for xmake
    set_config("qt", qt_root)

    -- Configure Qt6 tools
    local moc_exe = path.join(qt_bin, "moc.exe")
    local uic_exe = path.join(qt_bin, "uic.exe")
    local rcc_exe = path.join(qt_bin, "rcc.exe")
    local qmake_exe = path.join(qt_bin, "qmake6.exe")

    if os.isfile(moc_exe) then set_config("qt.moc", moc_exe) end
    if os.isfile(uic_exe) then set_config("qt.uic", uic_exe) end
    if os.isfile(rcc_exe) then set_config("qt.rcc", rcc_exe) end
    if os.isfile(qmake_exe) then set_config("qt.qmake", qmake_exe) end

    -- Add Qt6 specific directories
    add_includedirs(path.join(qt_root, "include"))
    add_linkdirs(path.join(qt_root, "lib"))

    -- Add Qt6 CMake modules path
    local qt_cmake_path = path.join(qt_root, "lib", "cmake")
    if os.isdir(qt_cmake_path) then
        add_linkdirs(qt_cmake_path)
    end

    return true
end

-- Get MinGW64-specific compiler flags
function msys2_mingw64.get_compiler_flags()
    local flags = {
        cxxflags = {
            "-std=c++20",
            "-Wall",
            "-Wextra",
            "-Wpedantic"
        },
        defines = {
            "QTFORGE_PLATFORM_WINDOWS",
            "QTFORGE_MINGW64_BUILD"
        },
        ldflags = {}
    }

    -- Add debug/release specific flags
    if is_mode("debug") then
        table.insert(flags.cxxflags, "-g")
        table.insert(flags.cxxflags, "-O0")
        table.insert(flags.defines, "QTFORGE_DEBUG=1")
    else
        table.insert(flags.cxxflags, "-O2")
        table.insert(flags.cxxflags, "-DNDEBUG")
        table.insert(flags.defines, "QTFORGE_RELEASE=1")
    end

    return flags
end

-- Apply MinGW64 configuration to target
function msys2_mingw64.apply_to_target()
    if not msys2_mingw64.detected.available then
        return false
    end

    local flags = msys2_mingw64.get_compiler_flags()

    -- Apply compiler flags
    for _, flag in ipairs(flags.cxxflags) do
        add_cxxflags(flag)
    end

    -- Apply defines
    for _, define in ipairs(flags.defines) do
        add_defines(define)
    end

    -- Apply linker flags
    for _, flag in ipairs(flags.ldflags) do
        add_ldflags(flag)
    end

    return true
end

-- Perform full detection and configuration
function msys2_mingw64.setup()
    print("Detecting MSYS2 MinGW64 environment...")

    if msys2_mingw64.detect_msys2() then
        print("  MSYS2 found at: " .. msys2_mingw64.detected.msys2_root)

        if msys2_mingw64.detect_mingw64_toolchain() then
            print("  MinGW64 toolchain found at: " .. msys2_mingw64.detected.mingw64_prefix)

            if msys2_mingw64.detect_qt6_installation() then
                local qt_version = msys2_mingw64.get_qt6_version()
                print("  Qt6 installation found" .. (qt_version and (" (version " .. qt_version .. ")") or ""))

                local qt_packages = msys2_mingw64.get_available_qt6_packages()
                if #qt_packages > 0 then
                    print("  Available Qt6 packages: " .. table.concat(qt_packages, ", "))
                end
            else
                print("  Qt6 not found in MinGW64 installation")
            end

            return true
        else
            print("  MinGW64 toolchain not found")
        end
    else
        print("  MSYS2 not found")
    end

    return false
end

-- Enhanced MSYS2 package manager integration

-- Check if a specific package is installed
function msys2_mingw64.is_package_installed(package_name)
    if not msys2_mingw64.detected.pacman_available then
        return false
    end

    local pacman_exe = path.join(msys2_mingw64.detected.msys2_root, "usr", "bin", "pacman.exe")
    local result = os.iorunv(pacman_exe, {"-Q", package_name})

    return result and result:find(package_name) ~= nil
end

-- Get list of installed Qt6 packages
function msys2_mingw64.get_installed_qt6_packages()
    if not msys2_mingw64.detected.pacman_available then
        return {}
    end

    local pacman_exe = path.join(msys2_mingw64.detected.msys2_root, "usr", "bin", "pacman.exe")
    local prefix = msys2_mingw64.detected.toolchain_prefix or "mingw64"

    local installed = {}
    local qt6_packages = {
        prefix .. "-qt6-base",
        prefix .. "-qt6-tools",
        prefix .. "-qt6-declarative",
        prefix .. "-qt6-multimedia",
        prefix .. "-qt6-networkauth",
        prefix .. "-qt6-svg",
        prefix .. "-qt6-imageformats",
        prefix .. "-qt6-translations",
        prefix .. "-qt6-charts",
        prefix .. "-qt6-datavis3d",
        prefix .. "-qt6-quick3d",
        prefix .. "-qt6-serialport",
        prefix .. "-qt6-websockets"
    }

    for _, package in ipairs(qt6_packages) do
        if msys2_mingw64.is_package_installed(package) then
            table.insert(installed, package)
        end
    end

    return installed
end

-- Suggest missing Qt6 packages based on build configuration
function msys2_mingw64.suggest_missing_qt6_packages(required_features)
    if not msys2_mingw64.detected.pacman_available then
        return {}
    end

    local prefix = msys2_mingw64.detected.toolchain_prefix or "mingw64"
    local suggestions = {}

    -- Map features to packages
    local feature_package_map = {
        ui = {prefix .. "-qt6-base"},
        network = {prefix .. "-qt6-base"},
        sql = {prefix .. "-qt6-base"},
        concurrent = {prefix .. "-qt6-base"},
        multimedia = {prefix .. "-qt6-multimedia"},
        svg = {prefix .. "-qt6-svg"},
        charts = {prefix .. "-qt6-charts"},
        websockets = {prefix .. "-qt6-websockets"},
        serialport = {prefix .. "-qt6-serialport"}
    }

    -- Always suggest base packages
    local base_packages = {
        prefix .. "-qt6-base",
        prefix .. "-qt6-tools"
    }

    for _, package in ipairs(base_packages) do
        if not msys2_mingw64.is_package_installed(package) then
            table.insert(suggestions, package)
        end
    end

    -- Add feature-specific packages
    for _, feature in ipairs(required_features or {}) do
        local packages = feature_package_map[feature]
        if packages then
            for _, package in ipairs(packages) do
                if not msys2_mingw64.is_package_installed(package) then
                    table.insert(suggestions, package)
                end
            end
        end
    end

    return suggestions
end

-- Generate pacman install command for missing packages
function msys2_mingw64.generate_install_command(packages)
    if not packages or #packages == 0 then
        return nil
    end

    return "pacman -S " .. table.concat(packages, " ")
end

-- Check and report Qt6 package status
function msys2_mingw64.check_qt6_package_status(required_features)
    print("Checking MSYS2 Qt6 package status...")

    if not msys2_mingw64.detected.pacman_available then
        print("  Warning: pacman not available")
        return false
    end

    local installed = msys2_mingw64.get_installed_qt6_packages()
    local missing = msys2_mingw64.suggest_missing_qt6_packages(required_features)

    print("  Installed Qt6 packages: " .. (#installed > 0 and table.concat(installed, ", ") or "none"))

    if #missing > 0 then
        print("  Missing Qt6 packages: " .. table.concat(missing, ", "))
        local install_cmd = msys2_mingw64.generate_install_command(missing)
        if install_cmd then
            print("  To install missing packages, run:")
            print("    " .. install_cmd)
        end
        return false
    else
        print("  All required Qt6 packages are installed")
        return true
    end
end

-- Get MSYS2 environment information
function msys2_mingw64.get_environment_info()
    local info = {
        msys2_available = msys2_mingw64.detected.available,
        msys2_root = msys2_mingw64.detected.msys2_root,
        mingw64_prefix = msys2_mingw64.detected.mingw64_prefix,
        toolchain_prefix = msys2_mingw64.detected.toolchain_prefix,
        qt_installation = msys2_mingw64.detected.qt_installation,
        pacman_available = msys2_mingw64.detected.pacman_available,
        installed_qt6_packages = {},
        environment_variables = {}
    }

    if msys2_mingw64.detected.available then
        info.installed_qt6_packages = msys2_mingw64.get_installed_qt6_packages()

        -- Get relevant environment variables
        info.environment_variables = {
            MSYS2_ROOT = msys2_mingw64.detected.msys2_root,
            MINGW_PREFIX = msys2_mingw64.detected.mingw64_prefix,
            PATH_ADDITIONS = {
                path.join(msys2_mingw64.detected.mingw64_prefix, "bin"),
                path.join(msys2_mingw64.detected.msys2_root, "usr", "bin")
            }
        }
    end

    return info
end

-- Validate MSYS2 environment for QtForge development
function msys2_mingw64.validate_environment(required_features)
    print("Validating MSYS2 environment for QtForge development...")

    local validation_results = {
        valid = true,
        issues = {},
        suggestions = {}
    }

    -- Check MSYS2 availability
    if not msys2_mingw64.detected.available then
        validation_results.valid = false
        table.insert(validation_results.issues, "MSYS2 not found")
        table.insert(validation_results.suggestions, "Install MSYS2 from https://www.msys2.org/")
        return validation_results
    end

    -- Check MinGW64 toolchain
    if not msys2_mingw64.detected.mingw64_prefix then
        validation_results.valid = false
        table.insert(validation_results.issues, "MinGW64 toolchain not found")
        table.insert(validation_results.suggestions, "Install MinGW64 toolchain: pacman -S mingw-w64-x86_64-toolchain")
    end

    -- Check Qt6 installation
    if not msys2_mingw64.detected.qt_installation then
        validation_results.valid = false
        table.insert(validation_results.issues, "Qt6 not found")

        local missing_packages = msys2_mingw64.suggest_missing_qt6_packages(required_features)
        if #missing_packages > 0 then
            local install_cmd = msys2_mingw64.generate_install_command(missing_packages)
            table.insert(validation_results.suggestions, "Install Qt6: " .. install_cmd)
        end
    else
        -- Check for missing Qt6 packages
        local missing_packages = msys2_mingw64.suggest_missing_qt6_packages(required_features)
        if #missing_packages > 0 then
            table.insert(validation_results.issues, "Some Qt6 packages are missing")
            local install_cmd = msys2_mingw64.generate_install_command(missing_packages)
            table.insert(validation_results.suggestions, "Install missing packages: " .. install_cmd)
        end
    end

    -- Report results
    if validation_results.valid then
        print("  ✓ MSYS2 environment is ready for QtForge development")
    else
        print("  ✗ MSYS2 environment has issues:")
        for _, issue in ipairs(validation_results.issues) do
            print("    - " .. issue)
        end
        print("  Suggestions:")
        for _, suggestion in ipairs(validation_results.suggestions) do
            print("    - " .. suggestion)
        end
    end

    return validation_results
end

-- Export the module
return msys2_mingw64
