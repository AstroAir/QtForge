#!/usr/bin/env lua
--[[
QtForge Lua Bindings Example 3: Comprehensive Features

This example demonstrates advanced QtForge features including:
- Security and validation systems
- Configuration and resource management
- Orchestration and workflow management
- Monitoring and hot reload capabilities
- Integration patterns and best practices
--]]

-- Helper functions for output formatting
local function print_header(title)
    print("\n" .. string.rep("=", 50))
    print("🔧 " .. title)
    print(string.rep("=", 50))
end

local function print_success(message)
    print("✅ " .. message)
end

local function print_warning(message)
    print("⚠️  " .. message)
end

local function print_error(message)
    print("❌ " .. message)
end

local function print_info(message)
    print("📋 " .. message)
end

local function print_status(message)
    print("📊 " .. message)
end

-- Check if QtForge bindings are available
local function check_qtforge_availability()
    if not qtforge then
        print_error("QtForge Lua bindings not available")
        return false
    end
    
    print_success("QtForge Lua bindings loaded successfully")
    return true
end

-- Demonstrate security and validation features
local function demonstrate_security_features()
    print_header("Security and Validation Features")
    
    -- Test security manager
    if qtforge.security and qtforge.security.create_security_manager then
        local success, manager = pcall(qtforge.security.create_security_manager)
        
        if success then
            print_success("Security manager created successfully")
            
            -- Test plugin validation
            if manager.validate_plugin then
                print("\n🔍 Testing plugin validation...")
                
                local test_plugins = {"/path/to/plugin.so", "invalid_plugin.dll", ""}
                
                for _, plugin_path in ipairs(test_plugins) do
                    local validate_success, result = pcall(function()
                        return manager:validate_plugin(plugin_path)
                    end)
                    
                    if validate_success then
                        print_status("Validation result for '" .. plugin_path .. "': " .. tostring(result))
                    else
                        print_warning("Validation failed for '" .. plugin_path .. "': " .. tostring(result))
                    end
                end
            end
            
            -- Test permission checking
            if manager.check_permissions and PluginPermission then
                print("\n🔐 Testing permission checking...")
                
                local permissions = {"FileSystemRead", "NetworkAccess", "ProcessCreation"}
                
                for _, perm_name in ipairs(permissions) do
                    if PluginPermission[perm_name] then
                        local permission = PluginPermission[perm_name]
                        
                        local perm_success, has_permission = pcall(function()
                            return manager:check_permissions("test_plugin", permission)
                        end)
                        
                        if perm_success then
                            print_status("Permission " .. perm_name .. ": " .. tostring(has_permission))
                        else
                            print_warning("Permission check failed for " .. perm_name .. ": " .. tostring(has_permission))
                        end
                    end
                end
            end
        else
            print_warning("Security manager not available: " .. tostring(manager))
        end
    else
        print_warning("Security module not available")
    end
    
    -- Test security enums
    if SecurityLevel then
        print("\n🔒 Security Levels:")
        local levels = {"None", "Low", "Medium", "High", "Maximum"}
        
        for _, level in ipairs(levels) do
            if SecurityLevel[level] then
                print("  • " .. level .. ": " .. tostring(SecurityLevel[level]))
            end
        end
    end
    
    if TrustLevel then
        print("\n🛡️  Trust Levels:")
        local levels = {"Untrusted", "Limited", "Trusted", "FullyTrusted"}
        
        for _, level in ipairs(levels) do
            if TrustLevel[level] then
                print("  • " .. level .. ": " .. tostring(TrustLevel[level]))
            end
        end
    end
end

-- Demonstrate configuration and resource management
local function demonstrate_management_features()
    print_header("Configuration and Resource Management")
    
    -- Test configuration manager
    if qtforge.managers and qtforge.managers.create_configuration_manager then
        local success, config_manager = pcall(qtforge.managers.create_configuration_manager)
        
        if success then
            print_success("Configuration manager created successfully")
            
            -- Test configuration operations
            if config_manager.set_value and config_manager.get_value then
                print("\n⚙️  Testing configuration operations...")
                
                local test_configs = {
                    {"app.name", "QtForge Example"},
                    {"app.version", "1.0.0"},
                    {"debug.enabled", "true"},
                    {"max.connections", "100"}
                }
                
                for _, config in ipairs(test_configs) do
                    local key, value = config[1], config[2]
                    
                    -- Set value
                    local set_success, set_result = pcall(function()
                        return config_manager:set_value(key, value)
                    end)
                    
                    if set_success then
                        print("  ✅ Set " .. key .. " = " .. value)
                        
                        -- Get value
                        local get_success, retrieved_value = pcall(function()
                            return config_manager:get_value(key)
                        end)
                        
                        if get_success then
                            print("    📊 Retrieved: " .. tostring(retrieved_value))
                        end
                    else
                        print_warning("Failed to set " .. key .. ": " .. tostring(set_result))
                    end
                end
            end
        else
            print_warning("Configuration manager not available: " .. tostring(config_manager))
        end
    end
    
    -- Test logging manager
    if qtforge.managers and qtforge.managers.create_logging_manager then
        local success, log_manager = pcall(qtforge.managers.create_logging_manager)
        
        if success then
            print_success("Logging manager created successfully")
            
            -- Test logging operations
            if log_manager.log and LogLevel then
                print("\n📝 Testing logging operations...")
                
                local log_levels = {"Debug", "Info", "Warning", "Error", "Critical"}
                
                for _, level_name in ipairs(log_levels) do
                    if LogLevel[level_name] then
                        local level = LogLevel[level_name]
                        
                        local log_success, log_result = pcall(function()
                            return log_manager:log(level, "Test " .. level_name .. " message")
                        end)
                        
                        if log_success then
                            print("  ✅ Logged " .. level_name .. " message")
                        else
                            print_warning("Failed to log " .. level_name .. ": " .. tostring(log_result))
                        end
                    end
                end
            end
        else
            print_warning("Logging manager not available: " .. tostring(log_manager))
        end
    end
    
    -- Test resource manager
    if qtforge.managers and qtforge.managers.create_resource_manager then
        local success, resource_manager = pcall(qtforge.managers.create_resource_manager)
        
        if success then
            print_success("Resource manager created successfully")
            
            -- Test resource operations
            if resource_manager.allocate_resource and resource_manager.deallocate_resource then
                print("\n💾 Testing resource management...")
                
                local alloc_success, resource_id = pcall(function()
                    return resource_manager:allocate_resource("test_resource", 1024)
                end)
                
                if alloc_success and resource_id then
                    print("  ✅ Allocated resource: " .. tostring(resource_id))
                    
                    local dealloc_success, dealloc_result = pcall(function()
                        return resource_manager:deallocate_resource(resource_id)
                    end)
                    
                    if dealloc_success then
                        print("  ✅ Deallocated resource")
                    else
                        print_warning("Failed to deallocate resource: " .. tostring(dealloc_result))
                    end
                else
                    print_warning("Failed to allocate resource: " .. tostring(resource_id))
                end
            end
        else
            print_warning("Resource manager not available: " .. tostring(resource_manager))
        end
    end
end

-- Demonstrate orchestration and workflow features
local function demonstrate_orchestration_features()
    print_header("Orchestration and Workflow Management")
    
    -- Test plugin orchestrator
    if qtforge.orchestration and qtforge.orchestration.create_plugin_orchestrator then
        local success, orchestrator = pcall(qtforge.orchestration.create_plugin_orchestrator)
        
        if success then
            print_success("Plugin orchestrator created successfully")
            
            -- Test workflow creation
            if qtforge.orchestration.Workflow then
                local workflow_success, workflow = pcall(function()
                    return qtforge.orchestration.Workflow("demo_workflow")
                end)
                
                if workflow_success then
                    print_success("Created workflow: demo_workflow")
                    
                    -- Test workflow steps
                    if qtforge.orchestration.WorkflowStep then
                        print("\n🔧 Creating workflow steps...")
                        
                        local steps = {
                            {"load_data", "Load Data", "data_loader"},
                            {"process_data", "Process Data", "data_processor"},
                            {"save_results", "Save Results", "result_saver"}
                        }
                        
                        for _, step_info in ipairs(steps) do
                            local step_id, step_name, plugin_id = step_info[1], step_info[2], step_info[3]
                            
                            local step_success, step = pcall(function()
                                return qtforge.orchestration.WorkflowStep(step_id, step_name, plugin_id)
                            end)
                            
                            if step_success then
                                print("  ✅ Created step: " .. step_name)
                                
                                -- Add step to workflow
                                if workflow.add_step then
                                    local add_success, add_result = pcall(function()
                                        return workflow:add_step(step)
                                    end)
                                    
                                    if add_success then
                                        print("    📋 Added to workflow")
                                    else
                                        print_warning("    Failed to add to workflow: " .. tostring(add_result))
                                    end
                                end
                            else
                                print_warning("  Failed to create step " .. step_name .. ": " .. tostring(step))
                            end
                        end
                    end
                    
                    -- Test workflow execution
                    if orchestrator.execute_workflow then
                        print("\n▶️  Testing workflow execution...")
                        
                        local exec_success, result = pcall(function()
                            return orchestrator:execute_workflow(workflow)
                        end)
                        
                        if exec_success then
                            print_success("Workflow execution completed: " .. tostring(result))
                        else
                            print_warning("Workflow execution failed: " .. tostring(result) .. " (expected if plugins not available)")
                        end
                    end
                else
                    print_warning("Failed to create workflow: " .. tostring(workflow))
                end
            end
        else
            print_warning("Plugin orchestrator not available: " .. tostring(orchestrator))
        end
    end
    
    -- Test orchestration enums
    if StepStatus then
        print("\n🔄 Step Statuses:")
        local statuses = {"Pending", "Running", "Completed", "Failed", "Skipped"}
        
        for _, status in ipairs(statuses) do
            if StepStatus[status] then
                print("  • " .. status .. ": " .. tostring(StepStatus[status]))
            end
        end
    end
    
    if ExecutionMode then
        print("\n⚙️  Execution Modes:")
        local modes = {"Sequential", "Parallel", "Pipeline"}
        
        for _, mode in ipairs(modes) do
            if ExecutionMode[mode] then
                print("  • " .. mode .. ": " .. tostring(ExecutionMode[mode]))
            end
        end
    end
end

-- Demonstrate monitoring and hot reload features
local function demonstrate_monitoring_features()
    print_header("Monitoring and Hot Reload")
    
    -- Test hot reload manager
    if qtforge.monitoring and qtforge.monitoring.create_hot_reload_manager then
        local success, hot_reload_manager = pcall(qtforge.monitoring.create_hot_reload_manager)
        
        if success then
            print_success("Hot reload manager created successfully")
            
            -- Test hot reload operations
            if hot_reload_manager.enable_hot_reload and hot_reload_manager.disable_hot_reload then
                print("\n🔄 Testing hot reload operations...")
                
                local test_plugin = "example_plugin"
                
                -- Enable hot reload
                local enable_success, enable_result = pcall(function()
                    return hot_reload_manager:enable_hot_reload(test_plugin)
                end)
                
                if enable_success then
                    print("  ✅ Enabled hot reload for " .. test_plugin)
                    
                    -- Check if enabled
                    if hot_reload_manager.is_hot_reload_enabled then
                        local check_success, is_enabled = pcall(function()
                            return hot_reload_manager:is_hot_reload_enabled(test_plugin)
                        end)
                        
                        if check_success then
                            print("    📊 Hot reload enabled: " .. tostring(is_enabled))
                        end
                    end
                    
                    -- Disable hot reload
                    local disable_success, disable_result = pcall(function()
                        return hot_reload_manager:disable_hot_reload(test_plugin)
                    end)
                    
                    if disable_success then
                        print("  ✅ Disabled hot reload for " .. test_plugin)
                    else
                        print_warning("  Failed to disable hot reload: " .. tostring(disable_result))
                    end
                else
                    print_warning("Failed to enable hot reload: " .. tostring(enable_result))
                end
            end
        else
            print_warning("Hot reload manager not available: " .. tostring(hot_reload_manager))
        end
    end
    
    -- Test metrics collector
    if qtforge.monitoring and qtforge.monitoring.create_metrics_collector then
        local success, metrics_collector = pcall(qtforge.monitoring.create_metrics_collector)
        
        if success then
            print_success("Metrics collector created successfully")
            
            -- Test metrics collection
            if metrics_collector.collect_metrics then
                print("\n📊 Testing metrics collection...")
                
                local collect_success, metrics = pcall(function()
                    return metrics_collector:collect_metrics("test_plugin")
                end)
                
                if collect_success then
                    print("  ✅ Collected metrics: " .. tostring(metrics))
                else
                    print_warning("Failed to collect metrics: " .. tostring(metrics) .. " (expected if plugin not available)")
                end
            end
        else
            print_warning("Metrics collector not available: " .. tostring(metrics_collector))
        end
    end
    
    -- Test monitoring enums
    if PluginHealthStatus then
        print("\n🏥 Plugin Health Statuses:")
        local statuses = {"Healthy", "Unhealthy", "Unknown", "Degraded"}
        
        for _, status in ipairs(statuses) do
            if PluginHealthStatus[status] then
                print("  • " .. status .. ": " .. tostring(PluginHealthStatus[status]))
            end
        end
    end
end

-- Demonstrate utility functions
local function demonstrate_utility_features()
    print_header("Utility Functions")
    
    -- Test version utilities
    if qtforge.utils and qtforge.utils.Version then
        local success, version = pcall(function()
            return qtforge.utils.Version(1, 2, 3)
        end)
        
        if success then
            print_success("Created version 1.2.3")
            print("  📊 Version string: " .. tostring(version))
        else
            print_warning("Version utility not available: " .. tostring(version))
        end
    end
    
    -- Test JSON utilities
    if qtforge.utils and qtforge.utils.parse_json and qtforge.utils.stringify_json then
        print("\n📄 Testing JSON utilities...")
        
        local test_data = {name = "QtForge", version = "1.0", features = {"plugins", "security", "orchestration"}}
        
        local stringify_success, json_string = pcall(function()
            return qtforge.utils.stringify_json(test_data)
        end)
        
        if stringify_success then
            print("  ✅ JSON stringify: " .. tostring(json_string))
            
            local parse_success, parsed_data = pcall(function()
                return qtforge.utils.parse_json(json_string)
            end)
            
            if parse_success then
                print("  ✅ JSON parse successful")
                if parsed_data and parsed_data.name then
                    print("    📊 Parsed name: " .. tostring(parsed_data.name))
                end
            else
                print_warning("  JSON parse failed: " .. tostring(parsed_data))
            end
        else
            print_warning("JSON stringify failed: " .. tostring(json_string))
        end
    end
    
    -- Test string utilities
    if qtforge.utils then
        print("\n🔤 Testing string utilities...")
        
        if qtforge.utils.trim_string then
            local trim_success, trimmed = pcall(function()
                return qtforge.utils.trim_string("  hello world  ")
            end)
            
            if trim_success then
                print("  ✅ String trim: '" .. tostring(trimmed) .. "'")
            end
        end
        
        if qtforge.utils.to_upper then
            local upper_success, upper = pcall(function()
                return qtforge.utils.to_upper("hello")
            end)
            
            if upper_success then
                print("  ✅ String upper: '" .. tostring(upper) .. "'")
            end
        end
    end
    
    -- Test time utilities
    if qtforge.utils and qtforge.utils.current_timestamp then
        local time_success, timestamp = pcall(qtforge.utils.current_timestamp)
        
        if time_success then
            print("\n⏰ Current timestamp: " .. tostring(timestamp))
        end
    end
end

-- Demonstrate error handling across all modules
local function demonstrate_comprehensive_error_handling()
    print_header("Comprehensive Error Handling")
    
    print("🧪 Testing error handling across all modules...")
    
    -- Test invalid parameters across different modules
    local error_tests = {
        {
            "Core - Invalid plugin path",
            function()
                if qtforge.core and qtforge.core.create_plugin_manager then
                    local manager = qtforge.core.create_plugin_manager()
                    return manager:load_plugin(nil)
                end
            end
        },
        {
            "Communication - Invalid message",
            function()
                if qtforge.communication and qtforge.communication.create_message_bus then
                    local bus = qtforge.communication.create_message_bus()
                    return bus:publish(nil)
                end
            end
        },
        {
            "Security - Invalid plugin validation",
            function()
                if qtforge.security and qtforge.security.create_security_manager then
                    local manager = qtforge.security.create_security_manager()
                    return manager:validate_plugin("")
                end
            end
        },
        {
            "Configuration - Invalid key",
            function()
                if qtforge.managers and qtforge.managers.create_configuration_manager then
                    local manager = qtforge.managers.create_configuration_manager()
                    return manager:get_value(nil)
                end
            end
        }
    }
    
    for _, test in ipairs(error_tests) do
        local test_name, test_func = test[1], test[2]
        
        if test_func then
            local success, result = pcall(test_func)
            
            if success then
                print_warning("  " .. test_name .. ": Unexpected success (" .. tostring(result) .. ")")
            else
                print_success("  " .. test_name .. ": Correctly caught error")
            end
        else
            print("  " .. test_name .. ": Test not available")
        end
    end
end

-- Main demonstration function
local function main()
    print("QtForge Lua Bindings - Comprehensive Features Example")
    print(string.rep("=", 60))
    
    -- Check if QtForge is available
    if not check_qtforge_availability() then
        return 1
    end
    
    -- Demonstrate each category of features
    demonstrate_security_features()
    demonstrate_management_features()
    demonstrate_orchestration_features()
    demonstrate_monitoring_features()
    demonstrate_utility_features()
    demonstrate_comprehensive_error_handling()
    
    print("\n" .. string.rep("=", 60))
    print("🎉 Comprehensive Features Example Complete!")
    print(string.rep("=", 60))
    
    print("\n📚 Key Takeaways:")
    print("• QtForge provides comprehensive plugin management capabilities")
    print("• Security features ensure safe plugin execution")
    print("• Configuration and resource management enable flexible deployments")
    print("• Orchestration supports complex multi-plugin workflows")
    print("• Monitoring and hot reload improve development productivity")
    print("• Robust error handling is essential for production systems")
    
    print("\n🔗 Next Steps:")
    print("• Create custom plugins using the demonstrated patterns")
    print("• Implement production-ready error handling and logging")
    print("• Build comprehensive test suites for your plugins")
    print("• Explore advanced integration patterns and architectures")
    
    return 0
end

-- Run the main function
return main()
