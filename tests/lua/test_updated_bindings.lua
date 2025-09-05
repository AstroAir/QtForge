#!/usr/bin/env lua
--[[
Test script for updated QtForge Lua bindings
Tests all new functionality added to the bindings.
--]]

-- Test helper functions
local function assert_not_nil(value, message)
    if value == nil then
        error(message or "Value should not be nil")
    end
end

local function assert_type(value, expected_type, message)
    if type(value) ~= expected_type then
        error(message or string.format("Expected %s, got %s", expected_type, type(value)))
    end
end

local function test_core_bindings()
    print("Testing core bindings...")

    local success, err = pcall(function()
        -- Test that qtforge module is available
        assert_not_nil(qtforge, "qtforge module should be available")
        assert_not_nil(qtforge.core, "qtforge.core module should be available")

        -- Test enums
        print("  Testing enums...")
        assert_not_nil(PluginState, "PluginState enum should be available")
        assert_not_nil(PluginCapability, "PluginCapability enum should be available")
        assert_not_nil(PluginPriority, "PluginPriority enum should be available")
        assert_not_nil(PluginType, "PluginType enum should be available")
        assert_not_nil(ServiceExecutionMode, "ServiceExecutionMode enum should be available")
        assert_not_nil(ServiceState, "ServiceState enum should be available")
        assert_not_nil(ServicePriority, "ServicePriority enum should be available")
        assert_not_nil(ServiceHealth, "ServiceHealth enum should be available")

        -- Test enum values
        assert_not_nil(PluginState.Unloaded, "PluginState.Unloaded should be available")
        assert_not_nil(PluginState.Loaded, "PluginState.Loaded should be available")
        assert_not_nil(PluginState.Running, "PluginState.Running should be available")

        assert_not_nil(PluginCapability.Service, "PluginCapability.Service should be available")
        assert_not_nil(PluginCapability.Network, "PluginCapability.Network should be available")

        assert_not_nil(PluginPriority.Normal, "PluginPriority.Normal should be available")
        assert_not_nil(PluginPriority.High, "PluginPriority.High should be available")

        -- Test factory functions
        print("  Testing factory functions...")
        local manager = qtforge.core.create_plugin_manager()
        assert_not_nil(manager, "create_plugin_manager should return a manager")

        local loader = qtforge.core.create_plugin_loader()
        assert_not_nil(loader, "create_plugin_loader should return a loader")

        local registry = qtforge.core.create_plugin_registry()
        assert_not_nil(registry, "create_plugin_registry should return a registry")

        local resolver = qtforge.core.create_plugin_dependency_resolver()
        assert_not_nil(resolver, "create_plugin_dependency_resolver should return a resolver")

        -- Test utility functions
        print("  Testing utility functions...")
        local version = qtforge.core.version(1, 2, 3)
        assert_not_nil(version, "version constructor should work")

        local error_obj = qtforge.core.error(PluginErrorCode.LoadFailed, "Test error")
        assert_not_nil(error_obj, "error constructor should work")

        local metadata = qtforge.core.metadata()
        assert_not_nil(metadata, "metadata constructor should work")
    end)

    if success then
        print("  Core bindings test passed!")
        return true
    else
        print("  Core bindings test failed: " .. tostring(err))
        return false
    end
end

local function test_managers_bindings()
    print("Testing managers bindings...")

    local success, err = pcall(function()
        -- Test that managers module is available
        assert_not_nil(qtforge.managers, "qtforge.managers module should be available")

        -- Test configuration scope enum
        print("  Testing configuration enums...")
        assert_not_nil(ConfigurationScope, "ConfigurationScope enum should be available")
        assert_not_nil(ConfigurationScope.Global, "ConfigurationScope.Global should be available")
        assert_not_nil(ConfigurationScope.Plugin, "ConfigurationScope.Plugin should be available")
        assert_not_nil(ConfigurationScope.User, "ConfigurationScope.User should be available")

        -- Test logging level enum
        assert_not_nil(LogLevel, "LogLevel enum should be available")
        assert_not_nil(LogLevel.Debug, "LogLevel.Debug should be available")
        assert_not_nil(LogLevel.Info, "LogLevel.Info should be available")
        assert_not_nil(LogLevel.Warning, "LogLevel.Warning should be available")
        assert_not_nil(LogLevel.Error, "LogLevel.Error should be available")

        -- Test factory functions (if available)
        print("  Testing manager factory functions...")
        if qtforge.managers.create_configuration_manager then
            local config_manager = qtforge.managers.create_configuration_manager()
            assert_not_nil(config_manager, "create_configuration_manager should return a manager")
        end

        if qtforge.managers.create_logging_manager then
            local logging_manager = qtforge.managers.create_logging_manager()
            assert_not_nil(logging_manager, "create_logging_manager should return a manager")
        end

        if qtforge.managers.create_resource_manager then
            local resource_manager = qtforge.managers.create_resource_manager()
            assert_not_nil(resource_manager, "create_resource_manager should return a manager")
        end
    end)

    if success then
        print("  Managers bindings test passed!")
        return true
    else
        print("  Managers bindings test failed: " .. tostring(err))
        return false
    end
end

local function test_security_bindings()
    print("Testing security bindings...")

    local success, err = pcall(function()
        -- Test that security module is available
        assert_not_nil(qtforge.security, "qtforge.security module should be available")

        -- Test security level enum
        print("  Testing security enums...")
        assert_not_nil(SecurityLevel, "SecurityLevel enum should be available")
        assert_not_nil(SecurityLevel.None, "SecurityLevel.None should be available")
        assert_not_nil(SecurityLevel.Low, "SecurityLevel.Low should be available")
        assert_not_nil(SecurityLevel.Medium, "SecurityLevel.Medium should be available")
        assert_not_nil(SecurityLevel.High, "SecurityLevel.High should be available")
        assert_not_nil(SecurityLevel.Maximum, "SecurityLevel.Maximum should be available")

        -- Test trust level enum
        assert_not_nil(TrustLevel, "TrustLevel enum should be available")
        assert_not_nil(TrustLevel.Untrusted, "TrustLevel.Untrusted should be available")
        assert_not_nil(TrustLevel.Limited, "TrustLevel.Limited should be available")
        assert_not_nil(TrustLevel.Trusted, "TrustLevel.Trusted should be available")
        assert_not_nil(TrustLevel.FullyTrusted, "TrustLevel.FullyTrusted should be available")

        -- Test factory functions (if available)
        print("  Testing security factory functions...")
        if qtforge.security.create_security_manager then
            local security_manager = qtforge.security.create_security_manager()
            assert_not_nil(security_manager, "create_security_manager should return a manager")
        end

        if qtforge.security.create_security_validator then
            local validator = qtforge.security.create_security_validator()
            assert_not_nil(validator, "create_security_validator should return a validator")
        end
    end)

    if success then
        print("  Security bindings test passed!")
        return true
    else
        print("  Security bindings test failed: " .. tostring(err))
        return false
    end
end

local function test_orchestration_bindings()
    print("Testing orchestration bindings...")

    local success, err = pcall(function()
        -- Test that orchestration module is available
        assert_not_nil(qtforge.orchestration, "qtforge.orchestration module should be available")

        -- Test step status enum
        print("  Testing orchestration enums...")
        if StepStatus then
            assert_not_nil(StepStatus.Pending, "StepStatus.Pending should be available")
            assert_not_nil(StepStatus.Running, "StepStatus.Running should be available")
            assert_not_nil(StepStatus.Completed, "StepStatus.Completed should be available")
            assert_not_nil(StepStatus.Failed, "StepStatus.Failed should be available")
        end

        -- Test execution mode enum
        if ExecutionMode then
            assert_not_nil(ExecutionMode.Sequential, "ExecutionMode.Sequential should be available")
            assert_not_nil(ExecutionMode.Parallel, "ExecutionMode.Parallel should be available")
            assert_not_nil(ExecutionMode.Pipeline, "ExecutionMode.Pipeline should be available")
        end

        -- Test factory functions
        print("  Testing orchestration factory functions...")
        if qtforge.orchestration.create_workflow_step then
            local step = qtforge.orchestration.create_workflow_step("test_id", "Test Step", "test_plugin")
            assert_not_nil(step, "create_workflow_step should return a step")
        end
    end)

    if success then
        print("  Orchestration bindings test passed!")
        return true
    else
        print("  Orchestration bindings test failed: " .. tostring(err))
        return false
    end
end

local function test_monitoring_bindings()
    print("Testing monitoring bindings...")

    local success, err = pcall(function()
        -- Test that monitoring module is available
        assert_not_nil(qtforge.monitoring, "qtforge.monitoring module should be available")

        -- Test factory functions (if available)
        print("  Testing monitoring factory functions...")
        if qtforge.monitoring.create_hot_reload_manager then
            local hot_reload_manager = qtforge.monitoring.create_hot_reload_manager()
            assert_not_nil(hot_reload_manager, "create_hot_reload_manager should return a manager")
        end

        if qtforge.monitoring.create_metrics_collector then
            local metrics_collector = qtforge.monitoring.create_metrics_collector()
            assert_not_nil(metrics_collector, "create_metrics_collector should return a collector")
        end
    end)

    if success then
        print("  Monitoring bindings test passed!")
        return true
    else
        print("  Monitoring bindings test failed: " .. tostring(err))
        return false
    end
end

local function test_transactions_bindings()
    print("Testing transactions bindings...")

    local success, err = pcall(function()
        -- Test that transactions module is available
        assert_not_nil(qtforge.transactions, "qtforge.transactions module should be available")

        -- Test transaction enums
        print("  Testing transaction enums...")
        assert_not_nil(TransactionState, "TransactionState enum should be available")
        assert_not_nil(IsolationLevel, "IsolationLevel enum should be available")

        -- Test transaction manager
        print("  Testing transaction manager...")
        local manager = qtforge.transactions.get_transaction_manager()
        assert_not_nil(manager, "get_transaction_manager should return a manager")

        print("  Transactions bindings test passed!")
        return true
    end)

    if not success then
        print("  Transactions bindings test failed: " .. tostring(err))
        return false
    end

    return success
end

local function test_composition_bindings()
    print("Testing composition bindings...")

    local success, err = pcall(function()
        -- Test that composition module is available
        assert_not_nil(qtforge.composition, "qtforge.composition module should be available")

        -- Test composition enums
        print("  Testing composition enums...")
        assert_not_nil(CompositionStrategy, "CompositionStrategy enum should be available")
        assert_not_nil(PluginRole, "PluginRole enum should be available")

        -- Test composition manager
        print("  Testing composition manager...")
        local manager = qtforge.composition.get_composition_manager()
        assert_not_nil(manager, "get_composition_manager should return a manager")

        print("  Composition bindings test passed!")
        return true
    end)

    if not success then
        print("  Composition bindings test failed: " .. tostring(err))
        return false
    end

    return success
end

local function test_marketplace_bindings()
    print("Testing marketplace bindings...")

    local success, err = pcall(function()
        -- Test that marketplace module is available
        assert_not_nil(qtforge.marketplace, "qtforge.marketplace module should be available")

        -- Test marketplace classes
        print("  Testing marketplace classes...")
        assert_not_nil(PluginMarketplace, "PluginMarketplace class should be available")

        print("  Marketplace bindings test passed!")
        return true
    end)

    if not success then
        print("  Marketplace bindings test failed: " .. tostring(err))
        return false
    end

    return success
end

-- Main test function
local function main()
    print("QtForge Lua Bindings Test Suite")
    print(string.rep("=", 40))

    local tests = {
        test_core_bindings,
        test_managers_bindings,
        test_security_bindings,
        test_orchestration_bindings,
        test_monitoring_bindings,
        test_transactions_bindings,
        test_composition_bindings,
        test_marketplace_bindings,
    }

    local passed = 0
    local failed = 0

    for _, test in ipairs(tests) do
        local success = test()
        if success then
            passed = passed + 1
        else
            failed = failed + 1
        end
        print()
    end

    print(string.rep("=", 40))
    print(string.format("Test Results: %d passed, %d failed", passed, failed))

    if failed == 0 then
        print("All tests passed!")
        return 0
    else
        print("Some tests failed!")
        return 1
    end
end

-- Run the tests
return main()
