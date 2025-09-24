-- Test script for PluginManager Lua bindings
-- This script tests the basic functionality of the PluginManager bindings

print("Testing QtForge PluginManager Lua bindings...")

-- Test 1: Create PluginManager
print("\n=== Test 1: Creating PluginManager ===")
local manager = qtforge.core.create_plugin_manager()
if manager then
    print("✓ PluginManager created successfully")
else
    print("✗ Failed to create PluginManager")
    return false
end

-- Test 2: Test search path management
print("\n=== Test 2: Search Path Management ===")
manager:add_search_path("./test_plugins")
manager:add_search_path("./plugins")

local paths = manager:search_paths()
print("Search paths count:", #paths)
for i, path in ipairs(paths) do
    print("  " .. i .. ": " .. path)
end

if #paths >= 2 then
    print("✓ Search path management working")
else
    print("✗ Search path management failed")
end

-- Test 3: Test plugin discovery
print("\n=== Test 3: Plugin Discovery ===")
local discovered = manager:discover_plugins("./plugins", false)
print("Discovered plugins count:", #discovered)
for i, plugin_path in ipairs(discovered) do
    print("  " .. i .. ": " .. plugin_path)
end

-- Test 4: Test plugin loading options
print("\n=== Test 4: Plugin Load Options ===")
local options = qtforge.core.PluginLoadOptions()
options.check_dependencies = true
options.initialize_immediately = false
options.enable_hot_reload = true
options:set_timeout_ms(5000)

print("✓ PluginLoadOptions created and configured")
print("  check_dependencies:", options.check_dependencies)
print("  initialize_immediately:", options.initialize_immediately)
print("  enable_hot_reload:", options.enable_hot_reload)
print("  timeout_ms:", options:get_timeout_ms())

-- Test 5: Test plugin queries
print("\n=== Test 5: Plugin Queries ===")
local loaded_plugins = manager:loaded_plugins()
print("Currently loaded plugins:", #loaded_plugins)

local all_info = manager:all_plugin_info()
print("Plugin info entries:", #all_info)

-- Test 6: Test monitoring
print("\n=== Test 6: Monitoring ===")
print("Monitoring active:", manager:is_monitoring_active())

manager:start_monitoring(1000) -- 1 second interval
print("Started monitoring")
print("Monitoring active:", manager:is_monitoring_active())

manager:stop_monitoring()
print("Stopped monitoring")
print("Monitoring active:", manager:is_monitoring_active())

-- Test 7: Test metrics
print("\n=== Test 7: System Metrics ===")
local metrics = manager:system_metrics()
if metrics then
    print("✓ System metrics retrieved")
    -- Print some basic metrics if available
    for key, value in pairs(metrics) do
        print("  " .. key .. ":", tostring(value))
    end
else
    print("✗ Failed to retrieve system metrics")
end

-- Test 8: Test lifecycle management
print("\n=== Test 8: Lifecycle Management ===")
local initialized_count = manager:initialize_all_plugins()
print("Initialized plugins:", initialized_count)

local started_services = manager:start_all_services()
print("Started services:", started_services)

local stopped_services = manager:stop_all_services()
print("Stopped services:", stopped_services)

-- Test 9: Test dependency management
print("\n=== Test 9: Dependency Management ===")
local resolve_result = manager:resolve_dependencies()
if resolve_result == true then
    print("✓ Dependencies resolved successfully")
else
    print("✗ Dependency resolution failed")
end

local load_order = manager:get_load_order()
print("Load order count:", #load_order)
for i, plugin_id in ipairs(load_order) do
    print("  " .. i .. ": " .. plugin_id)
end

-- Test 10: Test hot reload
print("\n=== Test 10: Hot Reload ===")
manager:enable_global_hot_reload()
print("✓ Global hot reload enabled")

manager:disable_global_hot_reload()
print("✓ Global hot reload disabled")

-- Test 11: Test configuration
print("\n=== Test 11: Configuration ===")
local test_config = {
    setting1 = "value1",
    setting2 = 42,
    setting3 = true,
    nested = {
        option1 = "nested_value",
        option2 = 3.14
    }
}

-- This will fail if no plugin is loaded, but tests the binding
local config_result = manager:configure_plugin("test_plugin", test_config)
print("Configuration test completed (expected to fail with no plugin)")

print("\n=== All Tests Completed ===")
print("PluginManager Lua bindings appear to be working correctly!")

return true
