-- QtForge Toolchain Selector Module
-- Intelligent toolchain selection between MSVC and MinGW64
-- Version: 3.2.0

-- Module table
local toolchain_selector = {}

-- Supported toolchains
toolchain_selector.supported_toolchains = {
    "msvc",
    "mingw64",
    "auto"
}

-- Toolchain detection results
toolchain_selector.detection_results = {
    msvc = {
        available = false,
        version = nil,
        path = nil,
        priority = 1
    },
    mingw64 = {
        available = false,
        version = nil,
        path = nil,
        priority = 2
    }
}

-- Selected toolchain information
toolchain_selector.selected = {
    name = nil,
    configured = false,
    qt_support = false
}

-- Detect MSVC toolchain
function toolchain_selector.detect_msvc()
    -- Check if we're on Windows
    if not is_plat("windows") then
        return false
    end

    -- Try to detect Visual Studio
    local vs_versions = {"2022", "2019", "2017"}

    for _, version in ipairs(vs_versions) do
        -- Check common Visual Studio installation paths
        local vs_paths = {
            "C:/Program Files/Microsoft Visual Studio/" .. version,
            "C:/Program Files (x86)/Microsoft Visual Studio/" .. version
        }

        for _, vs_path in ipairs(vs_paths) do
            if os.isdir(vs_path) then
                -- Look for cl.exe in various editions
                local editions = {"Enterprise", "Professional", "Community", "BuildTools"}

                for _, edition in ipairs(editions) do
                    local cl_path = path.join(vs_path, edition, "VC", "Tools", "MSVC")
                    if os.isdir(cl_path) then
                        -- Find the latest MSVC version
                        local msvc_dirs = os.dirs(path.join(cl_path, "*"))
                        if msvc_dirs and #msvc_dirs > 0 then
                            -- Sort to get the latest version
                            table.sort(msvc_dirs)
                            local latest_msvc = msvc_dirs[#msvc_dirs]

                            local cl_exe = path.join(latest_msvc, "bin", "Hostx64", "x64", "cl.exe")
                            if os.isfile(cl_exe) then
                                toolchain_selector.detection_results.msvc.available = true
                                toolchain_selector.detection_results.msvc.version = version
                                toolchain_selector.detection_results.msvc.path = latest_msvc
                                return true
                            end
                        end
                    end
                end
            end
        end
    end

    return false
end

-- Detect MinGW64 toolchain using MSYS2 module
function toolchain_selector.detect_mingw64()
    local msys2_mingw64 = import("msys2_mingw64")

    if msys2_mingw64.setup() then
        toolchain_selector.detection_results.mingw64.available = true
        toolchain_selector.detection_results.mingw64.path = msys2_mingw64.detected.mingw64_prefix

        -- Try to get GCC version
        if msys2_mingw64.detected.mingw64_prefix then
            local gcc_exe = path.join(msys2_mingw64.detected.mingw64_prefix, "bin", "gcc.exe")
            if os.isfile(gcc_exe) then
                local result = os.iorunv(gcc_exe, {"--version"})
                if result then
                    local version = result:match("gcc %(GCC%) ([%d%.]+)")
                    if version then
                        toolchain_selector.detection_results.mingw64.version = version
                    end
                end
            end
        end

        return true
    end

    return false
end

-- Detect all available toolchains
function toolchain_selector.detect_all()
    print("Detecting available toolchains...")

    local msvc_detected = toolchain_selector.detect_msvc()
    local mingw64_detected = toolchain_selector.detect_mingw64()

    print("Toolchain detection results:")
    if msvc_detected then
        print("  ✓ MSVC " .. (toolchain_selector.detection_results.msvc.version or "unknown"))
    else
        print("  ✗ MSVC not found")
    end

    if mingw64_detected then
        print("  ✓ MinGW64 " .. (toolchain_selector.detection_results.mingw64.version or "unknown"))
    else
        print("  ✗ MinGW64 not found")
    end

    return msvc_detected or mingw64_detected
end

-- Get user preference from configuration
function toolchain_selector.get_user_preference()
    -- Check for explicit toolchain configuration
    local preferred = get_config("toolchain")
    if preferred and table.contains(toolchain_selector.supported_toolchains, preferred) then
        return preferred
    end

    -- Check environment variable
    local env_toolchain = os.getenv("QTFORGE_TOOLCHAIN")
    if env_toolchain and table.contains(toolchain_selector.supported_toolchains, env_toolchain:lower()) then
        return env_toolchain:lower()
    end

    return "auto"
end

-- Select best available toolchain
function toolchain_selector.select_toolchain()
    local preference = toolchain_selector.get_user_preference()

    print("Toolchain selection (preference: " .. preference .. "):")

    -- Handle explicit preferences
    if preference == "msvc" then
        if toolchain_selector.detection_results.msvc.available then
            toolchain_selector.selected.name = "msvc"
            print("  Selected: MSVC (user preference)")
            return true
        else
            print("  Warning: MSVC requested but not available")
            return false
        end
    elseif preference == "mingw64" then
        if toolchain_selector.detection_results.mingw64.available then
            toolchain_selector.selected.name = "mingw64"
            print("  Selected: MinGW64 (user preference)")
            return true
        else
            print("  Warning: MinGW64 requested but not available")
            return false
        end
    end

    -- Auto selection based on priority and availability
    local candidates = {}

    if toolchain_selector.detection_results.msvc.available then
        table.insert(candidates, {
            name = "msvc",
            priority = toolchain_selector.detection_results.msvc.priority
        })
    end

    if toolchain_selector.detection_results.mingw64.available then
        table.insert(candidates, {
            name = "mingw64",
            priority = toolchain_selector.detection_results.mingw64.priority
        })
    end

    if #candidates == 0 then
        print("  Error: No toolchains available")
        return false
    end

    -- Sort by priority (lower number = higher priority)
    table.sort(candidates, function(a, b) return a.priority < b.priority end)

    toolchain_selector.selected.name = candidates[1].name
    print("  Selected: " .. toolchain_selector.selected.name .. " (auto)")

    return true
end

-- Configure the selected toolchain
function toolchain_selector.configure_selected()
    if not toolchain_selector.selected.name then
        return false
    end

    print("Configuring " .. toolchain_selector.selected.name .. " toolchain...")

    if toolchain_selector.selected.name == "msvc" then
        return toolchain_selector.configure_msvc()
    elseif toolchain_selector.selected.name == "mingw64" then
        return toolchain_selector.configure_mingw64()
    end

    return false
end

-- Configure MSVC toolchain
function toolchain_selector.configure_msvc()
    -- MSVC configuration is handled by xmake automatically
    -- Just set some QtForge-specific settings

    add_defines("QTFORGE_TOOLCHAIN_MSVC")

    -- Use MD runtime for shared builds, MT for static
    if has_config("shared") then
        set_runtimes("MD")
    else
        set_runtimes("MT")
    end

    -- MSVC-specific compiler flags
    add_cxflags("/utf-8", "/permissive-")
    add_defines("WIN32_LEAN_AND_MEAN", "NOMINMAX")

    toolchain_selector.selected.configured = true
    print("  MSVC toolchain configured")

    return true
end

-- Configure MinGW64 toolchain
function toolchain_selector.configure_mingw64()
    local msys2_mingw64 = import("msys2_mingw64")

    if not msys2_mingw64.configure_toolchain() then
        print("  Error: Failed to configure MinGW64 toolchain")
        return false
    end

    add_defines("QTFORGE_TOOLCHAIN_MINGW64")

    -- Apply MinGW64-specific settings
    msys2_mingw64.apply_to_target()

    -- Configure Qt6 if available
    if msys2_mingw64.configure_qt6() then
        toolchain_selector.selected.qt_support = true
        print("  Qt6 support configured for MinGW64")
    end

    toolchain_selector.selected.configured = true
    print("  MinGW64 toolchain configured")

    return true
end

-- Get toolchain-specific Qt package configuration
function toolchain_selector.get_qt_package_config()
    local config = {
        shared = has_config("shared"),
        optional = true
    }

    if toolchain_selector.selected.name == "msvc" then
        config.runtimes = "MD"
    elseif toolchain_selector.selected.name == "mingw64" then
        -- MinGW64 doesn't use runtimes config
        config.runtimes = nil
    end

    return config
end

-- Check if Qt6 is available for selected toolchain
function toolchain_selector.check_qt6_availability()
    if toolchain_selector.selected.name == "mingw64" then
        local msys2_mingw64 = import("msys2_mingw64")
        return msys2_mingw64.detected.qt_installation ~= nil
    elseif toolchain_selector.selected.name == "msvc" then
        -- For MSVC, rely on xmake's Qt detection
        return has_package("qt6core")
    end

    return false
end

-- Perform complete toolchain setup
function toolchain_selector.setup()
    -- Detect all toolchains
    if not toolchain_selector.detect_all() then
        print("Error: No supported toolchains found")
        return false
    end

    -- Select best toolchain
    if not toolchain_selector.select_toolchain() then
        print("Error: Failed to select toolchain")
        return false
    end

    -- Configure selected toolchain
    if not toolchain_selector.configure_selected() then
        print("Error: Failed to configure toolchain")
        return false
    end

    print("Toolchain setup completed successfully!")
    return true
end

-- Get current toolchain information
function toolchain_selector.get_info()
    return {
        selected = toolchain_selector.selected.name,
        configured = toolchain_selector.selected.configured,
        qt_support = toolchain_selector.selected.qt_support,
        detection_results = toolchain_selector.detection_results
    }
end

-- Utility function for table.contains
function table.contains(tbl, value)
    for _, v in ipairs(tbl) do
        if v == value then
            return true
        end
    end
    return false
end

-- Export the module
return toolchain_selector
