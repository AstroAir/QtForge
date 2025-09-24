-- QtForge Build Optimization and Caching Module
-- Performance improvements and build acceleration
-- Version: 3.2.0

-- Module table
local optimization = {}

-- Optimization configuration
optimization.config = {
    -- Compiler cache settings
    ccache = {
        enabled = true,
        directory = nil, -- Auto-detect
        max_size = "5G",
        compression = true
    },

    -- Link-time optimization
    lto = {
        enabled = false, -- User configurable
        thin_lto = true, -- Use thin LTO when available
        jobs = nil -- Auto-detect CPU cores
    },

    -- Parallel compilation
    parallel = {
        enabled = true,
        jobs = nil, -- Auto-detect CPU cores
        memory_limit = "80%" -- Percentage of available RAM
    },

    -- Precompiled headers
    pch = {
        enabled = true,
        common_headers = {
            "QtCore/QtCore",
            "QtGui/QtGui",
            "QtWidgets/QtWidgets",
            "memory",
            "string",
            "vector",
            "map"
        }
    },

    -- Build caching
    cache = {
        enabled = true,
        directory = "build/.cache",
        max_age_days = 30
    }
}

-- Detect number of CPU cores
function optimization.detect_cpu_cores()
    local cores = os.cpuinfo().ncpu or 4
    -- Leave one core free for system
    return math.max(1, cores - 1)
end

-- Detect available memory
function optimization.detect_memory_gb()
    local meminfo = os.meminfo()
    if meminfo and meminfo.totalram then
        return math.floor(meminfo.totalram / (1024 * 1024 * 1024))
    end
    return 8 -- Default assumption
end

-- Setup compiler cache (ccache/sccache)
function optimization.setup_compiler_cache()
    if not optimization.config.ccache.enabled then
        return false
    end

    -- Try to find ccache or sccache
    local cache_tool = nil
    if os.isfile("/usr/bin/ccache") or os.isfile("/usr/local/bin/ccache") then
        cache_tool = "ccache"
    elseif os.isfile("C:/tools/sccache/sccache.exe") then
        cache_tool = "sccache"
    end

    if cache_tool then
        print("Enabling compiler cache: " .. cache_tool)
        set_policy("build.ccache", true)

        -- Configure cache directory
        if optimization.config.ccache.directory then
            set_config("ccachedir", optimization.config.ccache.directory)
        end

        return true
    else
        print("Compiler cache not found, skipping")
        return false
    end
end

-- Setup link-time optimization
function optimization.setup_lto(toolchain)
    if not optimization.config.lto.enabled then
        return false
    end

    print("Enabling Link-Time Optimization for " .. toolchain)

    if toolchain == "msvc" then
        -- MSVC LTO
        add_cxxflags("/GL") -- Whole program optimization
        add_ldflags("/LTCG") -- Link-time code generation

    elseif toolchain == "mingw64" or toolchain == "gcc" then
        -- GCC/MinGW LTO
        if optimization.config.lto.thin_lto then
            add_cxxflags("-flto=thin")
            add_ldflags("-flto=thin")
        else
            add_cxxflags("-flto")
            add_ldflags("-flto")
        end

        -- Set number of LTO jobs
        local jobs = optimization.config.lto.jobs or optimization.detect_cpu_cores()
        add_ldflags("-flto-jobs=" .. jobs)

    elseif toolchain == "clang" then
        -- Clang LTO
        add_cxxflags("-flto=thin")
        add_ldflags("-flto=thin")
    end

    return true
end

-- Setup parallel compilation
function optimization.setup_parallel_build()
    if not optimization.config.parallel.enabled then
        return false
    end

    local jobs = optimization.config.parallel.jobs or optimization.detect_cpu_cores()
    print("Setting parallel build jobs: " .. jobs)

    -- Set xmake parallel jobs
    set_config("jobs", jobs)

    -- Memory-aware job limiting
    local memory_gb = optimization.detect_memory_gb()
    local memory_per_job = 2 -- GB per compilation job
    local max_memory_jobs = math.floor(memory_gb * 0.8 / memory_per_job)

    if max_memory_jobs < jobs then
        print("Limiting jobs due to memory constraints: " .. max_memory_jobs)
        set_config("jobs", max_memory_jobs)
    end

    return true
end

-- Setup precompiled headers
function optimization.setup_precompiled_headers(target_name)
    if not optimization.config.pch.enabled then
        return false
    end

    print("Setting up precompiled headers for " .. target_name)

    -- Create PCH header content
    local pch_content = ""
    for _, header in ipairs(optimization.config.pch.common_headers) do
        pch_content = pch_content .. "#include <" .. header .. ">\n"
    end

    -- Write PCH file
    local pch_file = "src/" .. target_name .. "_pch.h"
    io.writefile(pch_file, pch_content)

    -- Configure PCH
    set_pcheader(pch_file)

    return true
end

-- Setup build caching
function optimization.setup_build_cache()
    if not optimization.config.cache.enabled then
        return false
    end

    local cache_dir = optimization.config.cache.directory
    if not os.isdir(cache_dir) then
        os.mkdir(cache_dir)
    end

    print("Build cache directory: " .. cache_dir)

    -- Clean old cache files
    optimization.clean_old_cache()

    return true
end

-- Clean old cache files
function optimization.clean_old_cache()
    local cache_dir = optimization.config.cache.directory
    local max_age = optimization.config.cache.max_age_days * 24 * 3600 -- Convert to seconds

    if os.isdir(cache_dir) then
        local files = os.files(path.join(cache_dir, "*"))
        local current_time = os.time()

        for _, file in ipairs(files) do
            local stat = os.stat(file)
            if stat and (current_time - stat.mtime) > max_age then
                os.rm(file)
                print("Removed old cache file: " .. file)
            end
        end
    end
end

-- Get toolchain-specific optimization flags
function optimization.get_optimization_flags(toolchain, mode)
    local flags = {
        cxxflags = {},
        ldflags = {},
        defines = {}
    }

    if mode == "release" or mode == "releasedbg" then
        if toolchain == "msvc" then
            -- MSVC optimizations
            table.insert(flags.cxxflags, "/O2") -- Maximize speed
            table.insert(flags.cxxflags, "/Ob2") -- Inline expansion
            table.insert(flags.cxxflags, "/Ot") -- Favor fast code
            table.insert(flags.cxxflags, "/Oy") -- Frame pointer omission
            table.insert(flags.ldflags, "/OPT:REF") -- Remove unreferenced functions
            table.insert(flags.ldflags, "/OPT:ICF") -- Identical COMDAT folding

        elseif toolchain == "mingw64" or toolchain == "gcc" then
            -- GCC/MinGW optimizations
            table.insert(flags.cxxflags, "-O3") -- Aggressive optimization
            table.insert(flags.cxxflags, "-march=native") -- CPU-specific optimizations
            table.insert(flags.cxxflags, "-mtune=native")
            table.insert(flags.cxxflags, "-ffast-math") -- Fast math operations
            table.insert(flags.cxxflags, "-funroll-loops") -- Loop unrolling
            table.insert(flags.ldflags, "-Wl,--gc-sections") -- Remove unused sections
            table.insert(flags.ldflags, "-Wl,--strip-all") -- Strip symbols

        elseif toolchain == "clang" then
            -- Clang optimizations
            table.insert(flags.cxxflags, "-O3")
            table.insert(flags.cxxflags, "-march=native")
            table.insert(flags.cxxflags, "-mtune=native")
            table.insert(flags.cxxflags, "-ffast-math")
        end

        -- Common release optimizations
        table.insert(flags.defines, "NDEBUG")
        table.insert(flags.defines, "QT_NO_DEBUG")
        table.insert(flags.defines, "QT_NO_DEBUG_OUTPUT")

    elseif mode == "minsizerel" then
        if toolchain == "msvc" then
            table.insert(flags.cxxflags, "/Os") -- Minimize size
            table.insert(flags.ldflags, "/OPT:REF")
            table.insert(flags.ldflags, "/OPT:ICF")

        elseif toolchain == "mingw64" or toolchain == "gcc" then
            table.insert(flags.cxxflags, "-Os") -- Optimize for size
            table.insert(flags.cxxflags, "-ffunction-sections")
            table.insert(flags.cxxflags, "-fdata-sections")
            table.insert(flags.ldflags, "-Wl,--gc-sections")
            table.insert(flags.ldflags, "-Wl,--strip-all")
        end
    end

    return flags
end

-- Apply all optimizations to target
function optimization.apply_optimizations(target_name, toolchain, mode)
    print("Applying optimizations for " .. target_name .. " (" .. toolchain .. ", " .. mode .. ")")

    -- Setup compiler cache
    optimization.setup_compiler_cache()

    -- Setup parallel build
    optimization.setup_parallel_build()

    -- Setup build cache
    optimization.setup_build_cache()

    -- Setup precompiled headers
    optimization.setup_precompiled_headers(target_name)

    -- Setup LTO if enabled
    if has_config("lto") then
        optimization.config.lto.enabled = true
        optimization.setup_lto(toolchain)
    end

    -- Apply optimization flags
    local flags = optimization.get_optimization_flags(toolchain, mode)

    for _, flag in ipairs(flags.cxxflags) do
        add_cxxflags(flag)
    end

    for _, flag in ipairs(flags.ldflags) do
        add_ldflags(flag)
    end

    for _, define in ipairs(flags.defines) do
        add_defines(define)
    end

    return true
end

-- Generate optimization report
function optimization.generate_report()
    local report = {
        timestamp = os.date("%Y-%m-%d %H:%M:%S"),
        system = {
            cpu_cores = optimization.detect_cpu_cores(),
            memory_gb = optimization.detect_memory_gb()
        },
        optimizations = {
            ccache = optimization.config.ccache.enabled,
            lto = optimization.config.lto.enabled,
            parallel = optimization.config.parallel.enabled,
            pch = optimization.config.pch.enabled,
            cache = optimization.config.cache.enabled
        }
    }

    print("Optimization Report:")
    print("  CPU Cores: " .. report.system.cpu_cores)
    print("  Memory: " .. report.system.memory_gb .. "GB")
    print("  Compiler Cache: " .. (report.optimizations.ccache and "enabled" or "disabled"))
    print("  Link-Time Optimization: " .. (report.optimizations.lto and "enabled" or "disabled"))
    print("  Parallel Build: " .. (report.optimizations.parallel and "enabled" or "disabled"))
    print("  Precompiled Headers: " .. (report.optimizations.pch and "enabled" or "disabled"))
    print("  Build Cache: " .. (report.optimizations.cache and "enabled" or "disabled"))

    return report
end

-- Toolchain-specific advanced optimizations

-- MSVC-specific optimizations
function optimization.apply_msvc_optimizations(mode)
    local optimizations = {}

    if mode == "release" or mode == "releasedbg" then
        -- Profile-guided optimization
        table.insert(optimizations, {
            name = "Profile-Guided Optimization",
            flags = {"/GL", "/LTCG"},
            description = "Whole program optimization with link-time code generation"
        })

        -- Advanced vectorization
        table.insert(optimizations, {
            name = "Vectorization",
            flags = {"/arch:AVX2", "/Qvec-report:2"},
            description = "Enable AVX2 vectorization with reporting"
        })

        -- Function-level linking
        table.insert(optimizations, {
            name = "Function-level Linking",
            flags = {"/Gy"},
            ldflags = {"/OPT:REF", "/OPT:ICF"},
            description = "Enable function-level linking and optimization"
        })

        -- Fast floating point
        table.insert(optimizations, {
            name = "Fast Floating Point",
            flags = {"/fp:fast"},
            description = "Fast floating-point model for better performance"
        })

    elseif mode == "minsizerel" then
        -- Size optimizations
        table.insert(optimizations, {
            name = "Size Optimization",
            flags = {"/Os", "/GF", "/Gy"},
            ldflags = {"/OPT:REF", "/OPT:ICF"},
            description = "Optimize for minimal size"
        })
    end

    return optimizations
end

-- MinGW64/GCC-specific optimizations
function optimization.apply_mingw64_optimizations(mode)
    local optimizations = {}

    if mode == "release" or mode == "releasedbg" then
        -- CPU-specific optimizations
        table.insert(optimizations, {
            name = "CPU-Specific Optimization",
            flags = {"-march=native", "-mtune=native"},
            description = "Optimize for the current CPU architecture"
        })

        -- Advanced vectorization
        table.insert(optimizations, {
            name = "Vectorization",
            flags = {"-ftree-vectorize", "-fvect-cost-model=dynamic"},
            description = "Enable advanced vectorization"
        })

        -- Loop optimizations
        table.insert(optimizations, {
            name = "Loop Optimization",
            flags = {"-funroll-loops", "-fpeel-loops", "-ftracer"},
            description = "Advanced loop optimizations"
        })

        -- Inter-procedural optimization
        table.insert(optimizations, {
            name = "Inter-procedural Optimization",
            flags = {"-fipa-pta", "-fdevirtualize-at-ltrans"},
            description = "Advanced inter-procedural analysis"
        })

        -- Fast math (use with caution)
        table.insert(optimizations, {
            name = "Fast Math",
            flags = {"-ffast-math", "-funsafe-math-optimizations"},
            description = "Fast math operations (may affect precision)"
        })

        -- Dead code elimination
        table.insert(optimizations, {
            name = "Dead Code Elimination",
            flags = {"-ffunction-sections", "-fdata-sections"},
            ldflags = {"-Wl,--gc-sections", "-Wl,--strip-all"},
            description = "Remove unused code and data"
        })

    elseif mode == "minsizerel" then
        -- Size optimizations
        table.insert(optimizations, {
            name = "Size Optimization",
            flags = {"-Os", "-ffunction-sections", "-fdata-sections"},
            ldflags = {"-Wl,--gc-sections", "-Wl,--strip-all"},
            description = "Optimize for minimal size"
        })
    end

    return optimizations
end

-- Apply toolchain-specific optimizations
function optimization.apply_toolchain_optimizations(toolchain, mode, target_name)
    print("Applying " .. toolchain .. " optimizations for " .. target_name .. " (" .. mode .. ")")

    local optimizations = {}

    if toolchain == "msvc" then
        optimizations = optimization.apply_msvc_optimizations(mode)
    elseif toolchain == "mingw64" or toolchain == "gcc" then
        optimizations = optimization.apply_mingw64_optimizations(mode)
    end

    -- Apply optimizations
    for _, opt in ipairs(optimizations) do
        print("  Applying: " .. opt.name)

        if opt.flags then
            for _, flag in ipairs(opt.flags) do
                add_cxxflags(flag)
            end
        end

        if opt.ldflags then
            for _, flag in ipairs(opt.ldflags) do
                add_ldflags(flag)
            end
        end
    end

    return optimizations
end

-- Memory optimization strategies
function optimization.setup_memory_optimizations(toolchain)
    print("Setting up memory optimizations for " .. toolchain)

    if toolchain == "msvc" then
        -- MSVC memory optimizations
        add_cxxflags("/Zm200") -- Increase compiler memory limit
        add_ldflags("/LARGEADDRESSAWARE") -- Enable large address space

    elseif toolchain == "mingw64" or toolchain == "gcc" then
        -- GCC memory optimizations
        add_cxxflags("-fno-keep-inline-dllexport") -- Reduce memory usage
        add_ldflags("-Wl,--enable-auto-import") -- Automatic DLL imports
    end
end

-- Debug optimization strategies
function optimization.setup_debug_optimizations(toolchain, mode)
    if mode ~= "debug" then
        return
    end

    print("Setting up debug optimizations for " .. toolchain)

    if toolchain == "msvc" then
        -- MSVC debug optimizations
        add_cxxflags("/Zi") -- Debug information
        add_cxxflags("/Od") -- Disable optimizations
        add_cxxflags("/RTC1") -- Runtime checks
        add_ldflags("/DEBUG") -- Generate debug info

    elseif toolchain == "mingw64" or toolchain == "gcc" then
        -- GCC debug optimizations
        add_cxxflags("-g3") -- Maximum debug info
        add_cxxflags("-O0") -- No optimization
        add_cxxflags("-fno-omit-frame-pointer") -- Keep frame pointers
        add_cxxflags("-fstack-protector-strong") -- Stack protection
    end
end

-- Parallel compilation optimization
function optimization.optimize_parallel_compilation(toolchain)
    local cpu_cores = optimization.detect_cpu_cores()
    local memory_gb = optimization.detect_memory_gb()

    print("Optimizing parallel compilation: " .. cpu_cores .. " cores, " .. memory_gb .. "GB RAM")

    if toolchain == "msvc" then
        -- MSVC parallel compilation
        add_cxxflags("/MP" .. cpu_cores) -- Multi-processor compilation

        -- Memory-aware job limiting for large projects
        if memory_gb < 8 then
            local limited_cores = math.max(2, math.floor(cpu_cores / 2))
            add_cxxflags("/MP" .. limited_cores)
            print("Limited parallel jobs due to memory: " .. limited_cores)
        end

    elseif toolchain == "mingw64" or toolchain == "gcc" then
        -- GCC parallel compilation is handled by xmake's -j flag
        -- But we can optimize memory usage per job
        if memory_gb < 8 then
            local limited_cores = math.max(2, math.floor(cpu_cores / 2))
            set_config("jobs", limited_cores)
            print("Limited parallel jobs due to memory: " .. limited_cores)
        end
    end
end

-- Qt-specific optimizations
function optimization.setup_qt_optimizations(toolchain, qt_features)
    print("Setting up Qt optimizations for " .. toolchain .. " with features: " .. qt_features)

    -- Common Qt optimizations
    add_defines("QT_NO_DEBUG_OUTPUT") -- Remove debug output in release
    add_defines("QT_NO_WARNING_OUTPUT") -- Remove warning output in release

    if qt_features == "basic" then
        -- Minimal Qt optimizations
        add_defines("QT_NO_CAST_FROM_ASCII")
        add_defines("QT_NO_CAST_TO_ASCII")

    elseif qt_features == "ui" then
        -- UI-specific optimizations
        add_defines("QT_NO_ACCESSIBILITY") -- Disable accessibility if not needed

    elseif qt_features == "network" then
        -- Network-specific optimizations
        add_defines("QT_NO_SSL") -- Disable SSL if not needed
    end

    -- Toolchain-specific Qt optimizations
    if toolchain == "msvc" then
        add_cxxflags("/bigobj") -- Handle large Qt object files

    elseif toolchain == "mingw64" then
        add_cxxflags("-Wa,-mbig-obj") -- Handle large object files
    end
end

-- Profile-guided optimization setup
function optimization.setup_pgo(toolchain, target_name)
    if not has_config("pgo") then
        return false
    end

    print("Setting up Profile-Guided Optimization for " .. target_name)

    if toolchain == "msvc" then
        -- MSVC PGO
        if is_mode("release") then
            add_cxxflags("/GL") -- Whole program optimization
            add_ldflags("/LTCG") -- Link-time code generation

            -- PGO phases
            if has_config("pgo_instrument") then
                add_ldflags("/LTCG:PGInstrument")
            elseif has_config("pgo_optimize") then
                add_ldflags("/LTCG:PGOptimize")
            end
        end

    elseif toolchain == "mingw64" or toolchain == "gcc" then
        -- GCC PGO
        if is_mode("release") then
            if has_config("pgo_instrument") then
                add_cxxflags("-fprofile-generate")
                add_ldflags("-fprofile-generate")
            elseif has_config("pgo_optimize") then
                add_cxxflags("-fprofile-use")
                add_ldflags("-fprofile-use")
            end
        end
    end

    return true
end

-- Enhanced optimization application
function optimization.apply_enhanced_optimizations(target_name, toolchain, mode, qt_features)
    print("Applying enhanced optimizations for " .. target_name)

    -- Apply base optimizations
    optimization.apply_optimizations(target_name, toolchain, mode)

    -- Apply toolchain-specific optimizations
    optimization.apply_toolchain_optimizations(toolchain, mode, target_name)

    -- Apply memory optimizations
    optimization.setup_memory_optimizations(toolchain)

    -- Apply debug optimizations if needed
    optimization.setup_debug_optimizations(toolchain, mode)

    -- Optimize parallel compilation
    optimization.optimize_parallel_compilation(toolchain)

    -- Apply Qt-specific optimizations
    if qt_features then
        optimization.setup_qt_optimizations(toolchain, qt_features)
    end

    -- Setup PGO if requested
    optimization.setup_pgo(toolchain, target_name)

    return true
end

-- Generate enhanced optimization report
function optimization.generate_enhanced_report(toolchain, mode, target_name)
    local report = optimization.generate_report()

    -- Add toolchain-specific information
    report.toolchain = toolchain
    report.mode = mode
    report.target = target_name

    -- Add applied optimizations
    if toolchain == "msvc" then
        report.toolchain_optimizations = optimization.apply_msvc_optimizations(mode)
    elseif toolchain == "mingw64" or toolchain == "gcc" then
        report.toolchain_optimizations = optimization.apply_mingw64_optimizations(mode)
    end

    print("Enhanced Optimization Report for " .. target_name .. ":")
    print("  Toolchain: " .. toolchain)
    print("  Mode: " .. mode)
    print("  Applied Optimizations: " .. #(report.toolchain_optimizations or {}))

    for _, opt in ipairs(report.toolchain_optimizations or {}) do
        print("    - " .. opt.name .. ": " .. opt.description)
    end

    return report
end

-- Export the module
return optimization
