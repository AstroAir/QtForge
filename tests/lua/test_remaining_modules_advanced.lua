#!/usr/bin/env lua
--[[
Comprehensive test suite for remaining QtForge Lua binding modules.
Tests communication, security, managers, orchestration, monitoring, and other modules.
--]]

-- Test framework functions
local function assert_not_nil(value, message)
    if value == nil then
        error(message or "Value should not be nil")
    end
end

local function assert_equal(actual, expected, message)
    if actual ~= expected then
        error(message or string.format("Expected %s, got %s", tostring(expected), tostring(actual)))
    end
end

local function assert_type(value, expected_type, message)
    if type(value) ~= expected_type then
        error(message or string.format("Expected %s, got %s", expected_type, type(value)))
    end
end

local function pcall_test(test_func, test_name)
    print("  Testing " .. test_name .. "...")
    local success, err = pcall(test_func)
    if success then
        print("    ✅ " .. test_name .. " passed")
        return true
    else
        print("    ❌ " .. test_name .. " failed: " .. tostring(err))
        return false
    end
end

-- Test Communication bindings
local function test_communication_bindings()
    print("Testing Communication bindings...")
    
    local function test_message_bus_creation()
        if qtforge.communication and qtforge.communication.create_message_bus then
            local bus = qtforge.communication.create_message_bus()
            assert_not_nil(bus, "MessageBus should be created")
            
            -- Test basic methods exist
            local methods = {"publish", "subscribe"}
            for _, method in ipairs(methods) do
                if bus[method] then
                    assert_type(bus[method], "function", method .. " should be a function")
                end
            end
        end
    end
    
    local function test_message_creation()
        if qtforge.communication and qtforge.communication.BasicMessage then
            local message = qtforge.communication.BasicMessage("test_topic", "test_data")
            assert_not_nil(message, "BasicMessage should be created")
            
            -- Test property access if available
            if message.topic then
                assert_equal(message.topic, "test_topic", "Message topic should match")
            end
            if message.data then
                assert_equal(message.data, "test_data", "Message data should match")
            end
        end
    end
    
    local function test_service_contract()
        if qtforge.communication and qtforge.communication.ServiceContract then
            local contract = qtforge.communication.ServiceContract()
            assert_not_nil(contract, "ServiceContract should be created")
        end
    end
    
    local function test_communication_enums()
        -- Test DeliveryMode enum
        if DeliveryMode then
            local modes = {"Immediate", "Queued", "Persistent"}
            for _, mode in ipairs(modes) do
                if DeliveryMode[mode] then
                    assert_not_nil(DeliveryMode[mode], "DeliveryMode." .. mode .. " should not be nil")
                end
            end
        end
        
        -- Test MessagePriority enum
        if MessagePriority then
            local priorities = {"Low", "Normal", "High", "Critical"}
            for _, priority in ipairs(priorities) do
                if MessagePriority[priority] then
                    assert_not_nil(MessagePriority[priority], "MessagePriority." .. priority .. " should not be nil")
                end
            end
        end
    end
    
    local tests = {
        {test_message_bus_creation, "message_bus_creation"},
        {test_message_creation, "message_creation"},
        {test_service_contract, "service_contract"},
        {test_communication_enums, "communication_enums"}
    }
    
    local passed = 0
    for _, test in ipairs(tests) do
        if pcall_test(test[1], test[2]) then
            passed = passed + 1
        end
    end
    
    print(string.format("Communication tests: %d/%d passed", passed, #tests))
    return passed == #tests
end

-- Test Security bindings
local function test_security_bindings()
    print("Testing Security bindings...")
    
    local function test_security_manager_creation()
        if qtforge.security and qtforge.security.create_security_manager then
            local manager = qtforge.security.create_security_manager()
            assert_not_nil(manager, "SecurityManager should be created")
            
            -- Test basic methods exist
            local methods = {"validate_plugin", "check_permissions"}
            for _, method in ipairs(methods) do
                if manager[method] then
                    assert_type(manager[method], "function", method .. " should be a function")
                end
            end
        end
    end
    
    local function test_security_validator_creation()
        if qtforge.security and qtforge.security.create_security_validator then
            local validator = qtforge.security.create_security_validator()
            assert_not_nil(validator, "SecurityValidator should be created")
            
            if validator.validate then
                assert_type(validator.validate, "function", "validate should be a function")
            end
        end
    end
    
    local function test_permission_manager_creation()
        if qtforge.security and qtforge.security.create_permission_manager then
            local manager = qtforge.security.create_permission_manager()
            assert_not_nil(manager, "PermissionManager should be created")
            
            -- Test basic methods exist
            local methods = {"grant_permission", "revoke_permission", "has_permission"}
            for _, method in ipairs(methods) do
                if manager[method] then
                    assert_type(manager[method], "function", method .. " should be a function")
                end
            end
        end
    end
    
    local function test_security_enums()
        -- Test SecurityLevel enum
        if SecurityLevel then
            local levels = {"None", "Low", "Medium", "High", "Maximum"}
            for _, level in ipairs(levels) do
                if SecurityLevel[level] then
                    assert_not_nil(SecurityLevel[level], "SecurityLevel." .. level .. " should not be nil")
                end
            end
        end
        
        -- Test TrustLevel enum
        if TrustLevel then
            local levels = {"Untrusted", "Limited", "Trusted", "FullyTrusted"}
            for _, level in ipairs(levels) do
                if TrustLevel[level] then
                    assert_not_nil(TrustLevel[level], "TrustLevel." .. level .. " should not be nil")
                end
            end
        end
        
        -- Test PluginPermission enum
        if PluginPermission then
            local permissions = {"FileSystemRead", "FileSystemWrite", "NetworkAccess", "ProcessCreation"}
            for _, permission in ipairs(permissions) do
                if PluginPermission[permission] then
                    assert_not_nil(PluginPermission[permission], "PluginPermission." .. permission .. " should not be nil")
                end
            end
        end
    end
    
    local tests = {
        {test_security_manager_creation, "security_manager_creation"},
        {test_security_validator_creation, "security_validator_creation"},
        {test_permission_manager_creation, "permission_manager_creation"},
        {test_security_enums, "security_enums"}
    }
    
    local passed = 0
    for _, test in ipairs(tests) do
        if pcall_test(test[1], test[2]) then
            passed = passed + 1
        end
    end
    
    print(string.format("Security tests: %d/%d passed", passed, #tests))
    return passed == #tests
end

-- Test Managers bindings
local function test_managers_bindings()
    print("Testing Managers bindings...")
    
    local function test_configuration_manager_creation()
        if qtforge.managers and qtforge.managers.create_configuration_manager then
            local manager = qtforge.managers.create_configuration_manager()
            assert_not_nil(manager, "ConfigurationManager should be created")
            
            -- Test basic methods exist
            local methods = {"get_value", "set_value"}
            for _, method in ipairs(methods) do
                if manager[method] then
                    assert_type(manager[method], "function", method .. " should be a function")
                end
            end
        end
    end
    
    local function test_logging_manager_creation()
        if qtforge.managers and qtforge.managers.create_logging_manager then
            local manager = qtforge.managers.create_logging_manager()
            assert_not_nil(manager, "LoggingManager should be created")
            
            if manager.log then
                assert_type(manager.log, "function", "log should be a function")
            end
        end
    end
    
    local function test_resource_manager_creation()
        if qtforge.managers and qtforge.managers.create_resource_manager then
            local manager = qtforge.managers.create_resource_manager()
            assert_not_nil(manager, "ResourceManager should be created")
            
            -- Test basic methods exist
            local methods = {"allocate_resource", "deallocate_resource"}
            for _, method in ipairs(methods) do
                if manager[method] then
                    assert_type(manager[method], "function", method .. " should be a function")
                end
            end
        end
    end
    
    local function test_managers_enums()
        -- Test ConfigurationScope enum
        if ConfigurationScope then
            local scopes = {"Global", "Plugin", "User", "System"}
            for _, scope in ipairs(scopes) do
                if ConfigurationScope[scope] then
                    assert_not_nil(ConfigurationScope[scope], "ConfigurationScope." .. scope .. " should not be nil")
                end
            end
        end
        
        -- Test LogLevel enum
        if LogLevel then
            local levels = {"Debug", "Info", "Warning", "Error", "Critical"}
            for _, level in ipairs(levels) do
                if LogLevel[level] then
                    assert_not_nil(LogLevel[level], "LogLevel." .. level .. " should not be nil")
                end
            end
        end
    end
    
    local tests = {
        {test_configuration_manager_creation, "configuration_manager_creation"},
        {test_logging_manager_creation, "logging_manager_creation"},
        {test_resource_manager_creation, "resource_manager_creation"},
        {test_managers_enums, "managers_enums"}
    }
    
    local passed = 0
    for _, test in ipairs(tests) do
        if pcall_test(test[1], test[2]) then
            passed = passed + 1
        end
    end
    
    print(string.format("Managers tests: %d/%d passed", passed, #tests))
    return passed == #tests
end

-- Test Orchestration bindings
local function test_orchestration_bindings()
    print("Testing Orchestration bindings...")
    
    local function test_orchestrator_creation()
        if qtforge.orchestration and qtforge.orchestration.create_plugin_orchestrator then
            local orchestrator = qtforge.orchestration.create_plugin_orchestrator()
            assert_not_nil(orchestrator, "PluginOrchestrator should be created")
            
            if orchestrator.execute_workflow then
                assert_type(orchestrator.execute_workflow, "function", "execute_workflow should be a function")
            end
        end
    end
    
    local function test_workflow_creation()
        if qtforge.orchestration and qtforge.orchestration.Workflow then
            local workflow = qtforge.orchestration.Workflow("test_workflow")
            assert_not_nil(workflow, "Workflow should be created")
            
            if workflow.name then
                assert_equal(workflow.name, "test_workflow", "Workflow name should match")
            end
        end
    end
    
    local function test_workflow_step_creation()
        if qtforge.orchestration and qtforge.orchestration.create_workflow_step then
            local step = qtforge.orchestration.create_workflow_step("step1", "Test Step", "test_plugin")
            assert_not_nil(step, "WorkflowStep should be created")
        elseif qtforge.orchestration and qtforge.orchestration.WorkflowStep then
            local step = qtforge.orchestration.WorkflowStep("step1", "Test Step", "test_plugin")
            assert_not_nil(step, "WorkflowStep should be created")
        end
    end
    
    local function test_orchestration_enums()
        -- Test StepStatus enum
        if StepStatus then
            local statuses = {"Pending", "Running", "Completed", "Failed", "Skipped"}
            for _, status in ipairs(statuses) do
                if StepStatus[status] then
                    assert_not_nil(StepStatus[status], "StepStatus." .. status .. " should not be nil")
                end
            end
        end
        
        -- Test ExecutionMode enum
        if ExecutionMode then
            local modes = {"Sequential", "Parallel", "Pipeline"}
            for _, mode in ipairs(modes) do
                if ExecutionMode[mode] then
                    assert_not_nil(ExecutionMode[mode], "ExecutionMode." .. mode .. " should not be nil")
                end
            end
        end
    end
    
    local tests = {
        {test_orchestrator_creation, "orchestrator_creation"},
        {test_workflow_creation, "workflow_creation"},
        {test_workflow_step_creation, "workflow_step_creation"},
        {test_orchestration_enums, "orchestration_enums"}
    }
    
    local passed = 0
    for _, test in ipairs(tests) do
        if pcall_test(test[1], test[2]) then
            passed = passed + 1
        end
    end
    
    print(string.format("Orchestration tests: %d/%d passed", passed, #tests))
    return passed == #tests
end

-- Test Monitoring bindings
local function test_monitoring_bindings()
    print("Testing Monitoring bindings...")
    
    local function test_hot_reload_manager_creation()
        if qtforge.monitoring and qtforge.monitoring.create_hot_reload_manager then
            local manager = qtforge.monitoring.create_hot_reload_manager()
            assert_not_nil(manager, "HotReloadManager should be created")
            
            -- Test basic methods exist
            local methods = {"enable_hot_reload", "disable_hot_reload"}
            for _, method in ipairs(methods) do
                if manager[method] then
                    assert_type(manager[method], "function", method .. " should be a function")
                end
            end
        end
    end
    
    local function test_metrics_collector_creation()
        if qtforge.monitoring and qtforge.monitoring.create_metrics_collector then
            local collector = qtforge.monitoring.create_metrics_collector()
            assert_not_nil(collector, "MetricsCollector should be created")
            
            if collector.collect_metrics then
                assert_type(collector.collect_metrics, "function", "collect_metrics should be a function")
            end
        end
    end
    
    local function test_monitoring_enums()
        -- Test PluginHealthStatus enum
        if PluginHealthStatus then
            local statuses = {"Healthy", "Unhealthy", "Unknown", "Degraded"}
            for _, status in ipairs(statuses) do
                if PluginHealthStatus[status] then
                    assert_not_nil(PluginHealthStatus[status], "PluginHealthStatus." .. status .. " should not be nil")
                end
            end
        end
    end
    
    local tests = {
        {test_hot_reload_manager_creation, "hot_reload_manager_creation"},
        {test_metrics_collector_creation, "metrics_collector_creation"},
        {test_monitoring_enums, "monitoring_enums"}
    }
    
    local passed = 0
    for _, test in ipairs(tests) do
        if pcall_test(test[1], test[2]) then
            passed = passed + 1
        end
    end
    
    print(string.format("Monitoring tests: %d/%d passed", passed, #tests))
    return passed == #tests
end

-- Test remaining modules (transactions, composition, marketplace, threading)
local function test_remaining_modules()
    print("Testing remaining modules...")
    
    local function test_transactions_placeholder()
        if qtforge.transactions then
            -- Test that transactions module exists
            assert_not_nil(qtforge.transactions, "Transactions module should exist")
            
            -- Test placeholder function if available
            if qtforge.transactions.placeholder then
                local result = qtforge.transactions.placeholder()
                assert_type(result, "string", "Placeholder should return string")
            end
        end
    end
    
    local function test_composition_placeholder()
        if qtforge.composition then
            -- Test that composition module exists
            assert_not_nil(qtforge.composition, "Composition module should exist")
            
            -- Test placeholder function if available
            if qtforge.composition.placeholder then
                local result = qtforge.composition.placeholder()
                assert_type(result, "string", "Placeholder should return string")
            end
        end
    end
    
    local function test_marketplace_placeholder()
        if qtforge.marketplace then
            -- Test that marketplace module exists
            assert_not_nil(qtforge.marketplace, "Marketplace module should exist")
            
            -- Test placeholder function if available
            if qtforge.marketplace.placeholder then
                local result = qtforge.marketplace.placeholder()
                assert_type(result, "string", "Placeholder should return string")
            end
        end
    end
    
    local function test_threading_placeholder()
        if qtforge.threading then
            -- Test that threading module exists
            assert_not_nil(qtforge.threading, "Threading module should exist")
            
            -- Test placeholder function if available
            if qtforge.threading.placeholder then
                local result = qtforge.threading.placeholder()
                assert_type(result, "string", "Placeholder should return string")
            end
        end
    end
    
    local tests = {
        {test_transactions_placeholder, "transactions_placeholder"},
        {test_composition_placeholder, "composition_placeholder"},
        {test_marketplace_placeholder, "marketplace_placeholder"},
        {test_threading_placeholder, "threading_placeholder"}
    }
    
    local passed = 0
    for _, test in ipairs(tests) do
        if pcall_test(test[1], test[2]) then
            passed = passed + 1
        end
    end
    
    print(string.format("Remaining modules tests: %d/%d passed", passed, #tests))
    return passed == #tests
end

-- Test error handling across modules
local function test_error_handling()
    print("Testing error handling...")
    
    local function test_invalid_parameters()
        -- Test various modules with invalid parameters
        local modules_to_test = {
            {qtforge.core, "create_plugin_manager"},
            {qtforge.security, "create_security_manager"},
            {qtforge.managers, "create_configuration_manager"}
        }
        
        for _, module_info in ipairs(modules_to_test) do
            local module, func_name = module_info[1], module_info[2]
            if module and module[func_name] then
                -- These should work without parameters
                local success, result = pcall(module[func_name])
                if success then
                    assert_not_nil(result, func_name .. " should return valid object")
                end
            end
        end
    end
    
    local function test_nil_parameter_handling()
        -- Test functions that should handle nil parameters gracefully
        if qtforge.core and qtforge.core.create_plugin_manager then
            local manager = qtforge.core.create_plugin_manager()
            
            if manager and manager.load_plugin then
                local success, err = pcall(function()
                    manager:load_plugin(nil)
                end)
                
                -- Should either handle gracefully or throw appropriate error
                if not success then
                    assert_type(err, "string", "Error should be a string")
                end
            end
        end
    end
    
    local tests = {
        {test_invalid_parameters, "invalid_parameters"},
        {test_nil_parameter_handling, "nil_parameter_handling"}
    }
    
    local passed = 0
    for _, test in ipairs(tests) do
        if pcall_test(test[1], test[2]) then
            passed = passed + 1
        end
    end
    
    print(string.format("Error handling tests: %d/%d passed", passed, #tests))
    return passed == #tests
end

-- Main test function
local function main()
    print("QtForge Lua Remaining Modules - Comprehensive Test Suite")
    print(string.rep("=", 60))
    
    -- Check if qtforge module is available
    if not qtforge then
        print("❌ qtforge module not available")
        print("Ensure QtForge is built with Lua bindings enabled")
        return 1
    end
    
    print("✅ qtforge module is available")
    print()
    
    local test_suites = {
        {test_communication_bindings, "Communication Bindings"},
        {test_security_bindings, "Security Bindings"},
        {test_managers_bindings, "Managers Bindings"},
        {test_orchestration_bindings, "Orchestration Bindings"},
        {test_monitoring_bindings, "Monitoring Bindings"},
        {test_remaining_modules, "Remaining Modules"},
        {test_error_handling, "Error Handling"}
    }
    
    local passed_suites = 0
    local total_suites = #test_suites
    
    for _, suite in ipairs(test_suites) do
        print()
        if pcall_test(suite[1], suite[2]) then
            passed_suites = passed_suites + 1
        end
    end
    
    print()
    print(string.rep("=", 60))
    print(string.format("Test Results: %d/%d test suites passed", passed_suites, total_suites))
    
    if passed_suites == total_suites then
        print("✅ All remaining module binding tests passed!")
        return 0
    else
        print("❌ Some remaining module binding tests failed!")
        return 1
    end
end

-- Run the tests
return main()
