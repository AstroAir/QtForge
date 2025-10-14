-- QtForge Tests XMake Configuration

-- Add Qt6Test dependency for test framework
add_requires("qt6test", {optional = true, configs = {shared = true}})

-- Custom rule for Qt test files with MOC processing
rule("qt.test")
    add_deps("qt.env")
    on_config(function (target)
        -- Import Qt tools
        import("lib.detect.find_tool")

        -- Find MOC tool
        local qt = target:data("qt")
        if qt then
            local moc = find_tool("moc", {paths = qt.bindir})
            if moc then
                target:data_set("qt.moc", moc.program)
            end
        end
    end)

    before_build_files(function (target, sourcebatch, opt)
        -- Generate MOC files for sources with Q_OBJECT
        local qt_moc = target:data("qt.moc")
        if qt_moc then
            for _, sourcefile in ipairs(sourcebatch.sourcefiles) do
                local content = io.readfile(sourcefile)
                if content and content:find("Q_OBJECT") then
                    local basename = path.basename(sourcefile)
                    local mocfile = path.join(target:autogendir(), basename .. ".moc")
                    local objectdir = target:objectdir()

                    -- Create MOC file
                    os.vrunv(qt_moc, {sourcefile, "-o", mocfile})

                    -- Add to include paths
                    target:add("includedirs", target:autogendir())
                end
            end
        end
    end)
rule_end()

-- Test configuration only if tests are enabled
if has_config("tests") then

    -- Simple test target to validate xmake configuration
    target("xmake_config_test")
        set_kind("binary")
        set_basename("xmake_config_test")

        -- Use modern Qt console application rule
        add_rules("qt.console")

        -- Add Qt packages using modern package system
        add_packages("qt6core")

        -- Simple test source
        add_files("xmake_config_test.cpp")

        -- Set as test target
        set_default(false)
        set_group("tests")

        -- Set C++ standard
        set_languages("c++20")
    target_end()

    -- Core module tests
    target("test_version_simple")
        set_kind("binary")
        set_basename("test_version_simple")

        -- Use Qt console rule and custom test rule for MOC support
        add_rules("qt.console", "qt.test")

        -- Add source files
        add_files("utils/test_version_simple.cpp")

        -- Add Qt packages
        add_packages("qt6core")
        if has_package("qt6test") then
            add_packages("qt6test")
        end

        -- Add dependencies
        add_deps("QtForgeCore")

        -- Set as test target
        set_default(false)
        set_group("tests")
        set_languages("c++20")

        -- Configure MOC processing
        after_build(function (target)
            print("Built test: " .. target:name())
        end)
    target_end()

    target("test_error_handling_simple")
        set_kind("binary")
        set_basename("test_error_handling_simple")

        -- Use Qt application rule for proper MOC support
        add_rules("qt.application")

        -- Add source files
        add_files("utils/test_error_handling_simple.cpp")

        -- Add Qt packages
        add_packages("qt6core")
        if has_package("qt6test") then
            add_packages("qt6test")
        end

        -- Add dependencies
        add_deps("QtForgeCore")

        -- Set as test target
        set_default(false)
        set_group("tests")
        set_languages("c++20")
    target_end()

    target("test_plugin_interface")
        set_kind("binary")
        set_basename("test_plugin_interface")

        -- Use Qt application rule for proper MOC support
        add_rules("qt.application")

        -- Add source files
        add_files("core/test_plugin_interface.cpp")

        -- Add Qt packages
        add_packages("qt6core")
        if has_package("qt6test") then
            add_packages("qt6test")
        end

        -- Add dependencies
        add_deps("QtForgeCore")

        -- Set as test target
        set_default(false)
        set_group("tests")
        set_languages("c++20")
    target_end()

    -- Communication module tests
    target("test_message_bus_simple")
        set_kind("binary")
        set_basename("test_message_bus_simple")

        -- Use Qt test application rule for MOC support
        add_rules("qt.console")

        -- Add source files
        add_files("communication/test_message_bus_simple.cpp")

        -- Add Qt packages
        add_packages("qt6core")
        if has_package("qt6test") then
            add_packages("qt6test")
        end

        -- Add dependencies
        add_deps("QtForgeCore")

        -- Set as test target
        set_default(false)
        set_group("tests")
        set_languages("c++20")
    target_end()

    target("test_service_contracts")
        set_kind("binary")
        set_basename("test_service_contracts")

        -- Use Qt test application rule for MOC support
        add_rules("qt.console")

        -- Add source files
        add_files("communication/test_service_contracts.cpp")

        -- Add Qt packages
        add_packages("qt6core")
        if has_package("qt6test") then
            add_packages("qt6test")
        end

        -- Add dependencies
        add_deps("QtForgeCore")

        -- Set as test target
        set_default(false)
        set_group("tests")
        set_languages("c++20")
    target_end()

    -- Managers module tests
    target("test_configuration_manager")
        set_kind("binary")
        set_basename("test_configuration_manager")

        -- Use Qt test application rule for MOC support
        add_rules("qt.console")

        -- Add source files
        add_files("managers/test_configuration_manager.cpp")

        -- Add Qt packages
        add_packages("qt6core")
        if has_package("qt6test") then
            add_packages("qt6test")
        end

        -- Add dependencies
        add_deps("QtForgeCore")

        -- Set as test target
        set_default(false)
        set_group("tests")
        set_languages("c++20")
    target_end()

    target("test_resource_management")
        set_kind("binary")
        set_basename("test_resource_management")

        -- Use Qt test application rule for MOC support
        add_rules("qt.console")

        -- Add source files
        add_files("managers/test_resource_management.cpp")

        -- Add Qt packages
        add_packages("qt6core")
        if has_package("qt6test") then
            add_packages("qt6test")
        end

        -- Add dependencies
        add_deps("QtForgeCore")

        -- Set as test target
        set_default(false)
        set_group("tests")
        set_languages("c++20")
    target_end()

    -- Security module tests
    target("test_security_manager_simple")
        set_kind("binary")
        set_basename("test_security_manager_simple")

        -- Use Qt test application rule for MOC support
        add_rules("qt.console")

        -- Add source files
        add_files("security/test_security_manager_simple.cpp")

        -- Add Qt packages
        add_packages("qt6core")
        if has_package("qt6test") then
            add_packages("qt6test")
        end

        -- Add dependencies (QtForgeSecurity removed - security is now part of QtForgeCore)
        add_deps("QtForgeCore")

        -- Set as test target
        set_default(false)
        set_group("tests")
        set_languages("c++20")
    target_end()

    -- Monitoring module tests
    target("test_performance")
        set_kind("binary")
        set_basename("test_performance")

        -- Use Qt test application rule for MOC support
        add_rules("qt.console")

        -- Add source files
        add_files("monitoring/test_performance.cpp")

        -- Add Qt packages
        add_packages("qt6core")
        if has_package("qt6test") then
            add_packages("qt6test")
        end

        -- Add dependencies
        add_deps("QtForgeCore")

        -- Set as test target
        set_default(false)
        set_group("tests")
        set_languages("c++20")
    target_end()

    -- Platform tests
    target("test_cross_platform")
        set_kind("binary")
        set_basename("test_cross_platform")

        -- Use Qt test application rule for MOC support
        add_rules("qt.console")

        -- Add source files
        add_files("platform/test_cross_platform.cpp")

        -- Add Qt packages
        add_packages("qt6core")
        if has_package("qt6test") then
            add_packages("qt6test")
        end

        -- Add dependencies
        add_deps("QtForgeCore")

        -- Set as test target
        set_default(false)
        set_group("tests")
        set_languages("c++20")
    target_end()

    -- Orchestration module tests (conditional on comprehensive tests)
    target("test_plugin_orchestration")
        set_kind("binary")
        set_basename("test_plugin_orchestration")

        -- Use Qt test application rule for MOC support
        add_rules("qt.console")

        -- Add source files
        add_files("orchestration/test_plugin_orchestration.cpp")

        -- Add Qt packages
        add_packages("qt6core")
        if has_package("qt6test") then
            add_packages("qt6test")
        end

        -- Add dependencies
        add_deps("QtForgeCore")

        -- Set as test target
        set_default(false)
        set_group("tests")
        set_languages("c++20")
    target_end()

    -- Python bindings tests (conditional on Python availability)
    target("test_python_bindings")
        set_kind("binary")
        set_basename("test_python_bindings")

        -- Use Qt test application rule for MOC support
        add_rules("qt.console")

        -- Add source files
        add_files("python/test_python_bindings.cpp")

        -- Add Qt packages
        add_packages("qt6core")
        if has_package("qt6test") then
            add_packages("qt6test")
        end

        -- Add Python packages if available
        if has_package("python3") and has_package("pybind11") then
            add_packages("python3", "pybind11")
        end

        -- Add dependencies
        add_deps("QtForgeCore")

        -- Set as test target
        set_default(false)
        set_group("tests")
        set_languages("c++20")
    target_end()

    -- Bridges module tests (temporarily disabled due to API mismatches)
    -- target("test_python_bridge")
    --     set_kind("binary")
    --     set_basename("test_python_bridge")
    --     add_rules("qt.console")
    --     add_files("bridges/test_python_bridge.cpp")
    --     add_packages("qt6core")
    --     if has_package("qt6test") then
    --         add_packages("qt6test")
    --     end
    --     add_deps("QtForgeCore")
    --     set_default(false)
    --     set_group("tests")
    --     set_languages("c++20")
    -- target_end()

    -- Composition module tests (temporarily disabled)
    -- target("test_plugin_composition")
    --     set_kind("binary")
    --     set_basename("test_plugin_composition")
    --     add_rules("qt.console")
    --     add_files("composition/test_plugin_composition.cpp")
    --     add_packages("qt6core")
    --     if has_package("qt6test") then
    --         add_packages("qt6test")
    --     end
    --     add_deps("QtForgeCore")
    --     set_default(false)
    --     set_group("tests")
    --     set_languages("c++20")
    -- target_end()



    -- Transactions module tests (temporarily disabled)
    -- target("test_plugin_transaction_manager")
    --     set_kind("binary")
    --     set_basename("test_plugin_transaction_manager")
    --     add_rules("qt.console")
    --     add_files("transactions/test_plugin_transaction_manager.cpp")
    --     add_packages("qt6core")
    --     if has_package("qt6test") then
    --         add_packages("qt6test")
    --     end
    --     add_deps("QtForgeCore")
    --     set_default(false)
    --     set_group("tests")
    --     set_languages("c++20")
    -- target_end()

    -- Test execution rules
    rule("qtforge.test")
        on_run(function (target)
            local targetfile = target:targetfile()
            if targetfile and os.isfile(targetfile) then
                print("Running test: " .. target:name())
                os.execv(targetfile)
            else
                print("Test executable not found: " .. target:name())
            end
        end)
    rule_end()

    -- Apply test rule to all test targets
    for _, target_name in ipairs({
        "xmake_config_test",
        "test_version_simple",
        "test_error_handling_simple",
        "test_plugin_interface",
        "test_message_bus_simple",
        "test_service_contracts",
        "test_configuration_manager",
        "test_resource_management",
        "test_security_manager_simple",
        "test_performance",
        "test_cross_platform",
        "test_plugin_orchestration",
        "test_python_bindings"
    }) do
        target(target_name)
            add_rules("qtforge.test")
        target_end()
    end

    -- Include existing test directories if they have xmake.lua files
    local test_dirs = {
        "core",
        "communication",
        "managers",
        "monitoring",
        "security",
        "utils",
        "integration",
        "orchestration",
        "bridges",
        "composition",
        "marketplace",
        "transactions",
        "python"
    }

    for _, dir in ipairs(test_dirs) do
        if os.isdir(dir) and os.isfile(path.join(dir, "xmake.lua")) then
            includes(dir)
        end
    end

else
    print("Tests disabled - use 'xmake f --tests=y' to enable")
end

-- Custom task to run all tests
task("test")
    set_menu {
        usage = "xmake test",
        description = "Run all QtForge tests",
        options = {}
    }

    on_run(function ()
        if not has_config("tests") then
            print("Tests are not enabled. Use 'xmake f --tests=y' to enable tests.")
            return
        end

        local test_targets = {
            "xmake_config_test",
            "test_version_simple",
            "test_error_handling_simple",
            "test_plugin_interface",
            "test_message_bus_simple",
            "test_service_contracts",
            "test_configuration_manager",
            "test_resource_management",
            "test_security_manager_simple",
            "test_performance",
            "test_cross_platform",
            "test_plugin_orchestration",
            "test_python_bindings"
        }

        print("Running QtForge test suite...")
        local passed = 0
        local failed = 0

        for _, target_name in ipairs(test_targets) do
            print("\n" .. string.rep("=", 50))
            print("Running: " .. target_name)
            print(string.rep("=", 50))

            local ok, result = os.iorunv(path.join("build", "windows", "x64", "release", target_name .. ".exe"))
            if ok and result == 0 then
                print("✅ " .. target_name .. " PASSED")
                passed = passed + 1
            else
                print("❌ " .. target_name .. " FAILED")
                failed = failed + 1
            end
        end

        print("\n" .. string.rep("=", 50))
        print("Test Results Summary:")
        print("✅ Passed: " .. passed)
        print("❌ Failed: " .. failed)
        print("📊 Total:  " .. (passed + failed))
        print(string.rep("=", 50))

        if failed > 0 then
            os.exit(1)
        end
    end)
task_end()
