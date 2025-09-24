-- QtForge Enhanced Qt6 Detection Module
-- Cross-toolchain Qt6 detection and configuration
-- Version: 3.2.0

-- Module table
local qt6_detector = {}

-- Qt6 detection results
qt6_detector.detection_results = {
    msvc = {
        available = false,
        version = nil,
        path = nil,
        tools = {}
    },
    mingw64 = {
        available = false,
        version = nil,
        path = nil,
        tools = {}
    }
}

-- Qt6 components to detect
qt6_detector.components = {
    "Core", "Gui", "Widgets", "Network", "Sql", "Concurrent", "StateMachine"
}

-- Qt6 tools to detect
qt6_detector.tools = {
    "moc", "uic", "rcc", "qmake6"
}

-- Detect Qt6 for MSVC toolchain
function qt6_detector.detect_msvc_qt6()
    -- Use xmake's built-in Qt detection for MSVC
    local qt_paths = {
        os.getenv("QTDIR"),
        os.getenv("QT_DIR"),
        "C:/Qt/6.*/msvc*",
        "C:/Qt/*/6.*/msvc*"
    }

    for _, qt_path in ipairs(qt_paths) do
        if qt_path and os.isdir(qt_path) then
            local bin_path = path.join(qt_path, "bin")
            local qmake_exe = path.join(bin_path, "qmake.exe")

            if os.isfile(qmake_exe) then
                -- Get Qt version
                local result = os.iorunv(qmake_exe, {"-query", "QT_VERSION"})
                if result then
                    qt6_detector.detection_results.msvc.available = true
                    qt6_detector.detection_results.msvc.version = result:trim()
                    qt6_detector.detection_results.msvc.path = qt_path

                    -- Detect tools
                    for _, tool in ipairs(qt6_detector.tools) do
                        local tool_exe = path.join(bin_path, tool .. ".exe")
                        if os.isfile(tool_exe) then
                            qt6_detector.detection_results.msvc.tools[tool] = tool_exe
                        end
                    end

                    return true
                end
            end
        end
    end

    return false
end

-- Detect Qt6 for MinGW64 toolchain using MSYS2
function qt6_detector.detect_mingw64_qt6()
    local msys2_mingw64 = import("msys2_mingw64")

    if not msys2_mingw64.detected.available then
        return false
    end

    if msys2_mingw64.detect_qt6_installation() then
        local qt_path = msys2_mingw64.detected.qt_installation
        local version = msys2_mingw64.get_qt6_version()

        qt6_detector.detection_results.mingw64.available = true
        qt6_detector.detection_results.mingw64.version = version
        qt6_detector.detection_results.mingw64.path = qt_path

        -- Detect tools
        local bin_path = path.join(qt_path, "bin")
        for _, tool in ipairs(qt6_detector.tools) do
            local tool_exe = path.join(bin_path, tool .. ".exe")
            if os.isfile(tool_exe) then
                qt6_detector.detection_results.mingw64.tools[tool] = tool_exe
            end
        end

        return true
    end

    return false
end

-- Detect Qt6 components for a specific toolchain
function qt6_detector.detect_components(toolchain)
    local result = qt6_detector.detection_results[toolchain]
    if not result or not result.available then
        return {}
    end

    local available_components = {}
    local lib_path = path.join(result.path, "lib")

    for _, component in ipairs(qt6_detector.components) do
        -- Check for library files
        local lib_patterns = {
            path.join(lib_path, "Qt6" .. component .. ".lib"),  -- MSVC
            path.join(lib_path, "libQt6" .. component .. ".a"), -- MinGW64 static
            path.join(lib_path, "libQt6" .. component .. ".dll.a") -- MinGW64 shared
        }

        for _, pattern in ipairs(lib_patterns) do
            if os.isfile(pattern) then
                table.insert(available_components, component)
                break
            end
        end
    end

    return available_components
end

-- Get Qt6 package requirements for xmake
function qt6_detector.get_package_requirements(toolchain)
    local components = qt6_detector.detect_components(toolchain)
    local requirements = {}

    for _, component in ipairs(components) do
        local package_name = "qt6" .. component:lower()
        table.insert(requirements, package_name)
    end

    return requirements
end

-- Configure Qt6 for the selected toolchain
function qt6_detector.configure_qt6(toolchain)
    local result = qt6_detector.detection_results[toolchain]
    if not result or not result.available then
        return false
    end

    print("Configuring Qt6 for " .. toolchain .. " toolchain:")
    print("  Version: " .. (result.version or "unknown"))
    print("  Path: " .. result.path)

    -- Set Qt SDK directory
    set_config("qt", result.path)

    -- Configure Qt tools
    for tool, tool_path in pairs(result.tools) do
        set_config("qt." .. tool, tool_path)
        print("  " .. tool .. ": " .. tool_path)
    end

    -- Add Qt directories
    add_includedirs(path.join(result.path, "include"))
    add_linkdirs(path.join(result.path, "lib"))

    -- Add CMake modules path if available
    local cmake_path = path.join(result.path, "lib", "cmake")
    if os.isdir(cmake_path) then
        add_linkdirs(cmake_path)
    end

    return true
end

-- Check Qt6 package availability for toolchain
function qt6_detector.check_package_availability(toolchain, packages)
    local result = qt6_detector.detection_results[toolchain]
    if not result or not result.available then
        return {}
    end

    local available = {}
    local lib_path = path.join(result.path, "lib")

    for _, package in ipairs(packages) do
        -- Convert package name to component name
        local component = package:gsub("qt6", ""):gsub("^%l", string.upper)

        -- Check if component is available
        local lib_patterns = {
            path.join(lib_path, "Qt6" .. component .. ".lib"),
            path.join(lib_path, "libQt6" .. component .. ".a"),
            path.join(lib_path, "libQt6" .. component .. ".dll.a")
        }

        for _, pattern in ipairs(lib_patterns) do
            if os.isfile(pattern) then
                table.insert(available, package)
                break
            end
        end
    end

    return available
end

-- Generate Qt6 feature report
function qt6_detector.generate_feature_report(toolchain)
    local result = qt6_detector.detection_results[toolchain]
    if not result or not result.available then
        return {
            available = false,
            components = {},
            tools = {},
            version = nil
        }
    end

    return {
        available = true,
        components = qt6_detector.detect_components(toolchain),
        tools = result.tools,
        version = result.version,
        path = result.path
    }
end

-- Detect Qt6 for all toolchains
function qt6_detector.detect_all()
    print("Detecting Qt6 installations...")

    local msvc_detected = qt6_detector.detect_msvc_qt6()
    local mingw64_detected = qt6_detector.detect_mingw64_qt6()

    print("Qt6 detection results:")
    if msvc_detected then
        local report = qt6_detector.generate_feature_report("msvc")
        print("  ✓ MSVC Qt6 " .. (report.version or "unknown"))
        print("    Components: " .. table.concat(report.components, ", "))
    else
        print("  ✗ MSVC Qt6 not found")
    end

    if mingw64_detected then
        local report = qt6_detector.generate_feature_report("mingw64")
        print("  ✓ MinGW64 Qt6 " .. (report.version or "unknown"))
        print("    Components: " .. table.concat(report.components, ", "))
    else
        print("  ✗ MinGW64 Qt6 not found")
    end

    return msvc_detected or mingw64_detected
end

-- Setup Qt6 for the active toolchain
function qt6_detector.setup(active_toolchain)
    if not active_toolchain then
        print("Error: No active toolchain specified")
        return false
    end

    -- Detect Qt6 for the active toolchain
    local detected = false
    if active_toolchain == "msvc" then
        detected = qt6_detector.detect_msvc_qt6()
    elseif active_toolchain == "mingw64" then
        detected = qt6_detector.detect_mingw64_qt6()
    end

    if not detected then
        print("Warning: Qt6 not found for " .. active_toolchain .. " toolchain")
        return false
    end

    -- Configure Qt6
    return qt6_detector.configure_qt6(active_toolchain)
end

-- Get Qt6 information for active toolchain
function qt6_detector.get_info(toolchain)
    return qt6_detector.generate_feature_report(toolchain)
end

-- Export the module
return qt6_detector
