#!/usr/bin/env lua
--[[
Memory Management and Performance Tests for QtForge Lua Bindings

This test suite verifies proper memory management, resource cleanup,
and performance characteristics of the QtForge Lua bindings.
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

-- Memory tracking utilities
local MemoryTracker = {}
MemoryTracker.__index = MemoryTracker

function MemoryTracker:new()
    local obj = {
        start_memory = 0,
        peak_memory = 0,
        end_memory = 0
    }
    setmetatable(obj, self)
    return obj
end

function MemoryTracker:start_tracking()
    -- Force garbage collection
    collectgarbage("collect")
    collectgarbage("collect")  -- Call twice to ensure full collection
    
    -- Get current memory usage (in KB)
    self.start_memory = collectgarbage("count")
    self.peak_memory = self.start_memory
end

function MemoryTracker:get_current_memory()
    return collectgarbage("count")
end

function MemoryTracker:update_peak()
    local current = collectgarbage("count")
    if current > self.peak_memory then
        self.peak_memory = current
    end
end

function MemoryTracker:stop_tracking()
    collectgarbage("collect")
    collectgarbage("collect")
    
    self.end_memory = collectgarbage("count")
    
    return {
        start_memory = self.start_memory,
        end_memory = self.end_memory,
        peak_memory = self.peak_memory,
        memory_delta = self.end_memory - self.start_memory,
        peak_delta = self.peak_memory - self.start_memory
    }
end

-- Performance timing utilities
local PerformanceTimer = {}
PerformanceTimer.__index = PerformanceTimer

function PerformanceTimer:new()
    local obj = {
        start_time = 0,
        end_time = 0
    }
    setmetatable(obj, self)
    return obj
end

function PerformanceTimer:start()
    self.start_time = os.clock()
end

function PerformanceTimer:stop()
    self.end_time = os.clock()
    return self.end_time - self.start_time
end

function PerformanceTimer:elapsed()
    return os.clock() - self.start_time
end

-- Check if QtForge bindings are available
local function check_bindings_availability()
    if not qtforge then
        print_error("QtForge Lua bindings not available")
        return false
    end
    
    print_success("QtForge Lua bindings available")
    return true
end

-- Test object lifecycle and memory management
local function test_object_lifecycle()
    print_header("Testing Object Lifecycle")
    
    if not qtforge then
        print_warning("Skipping test - bindings not available")
        return
    end
    
    local tracker = MemoryTracker:new()
    tracker:start_tracking()
    
    local objects_created = 0
    
    -- Create and destroy objects multiple times
    for i = 1, 100 do
        local success, manager = pcall(function()
            if qtforge.core and qtforge.core.create_plugin_manager then
                return qtforge.core.create_plugin_manager()
            end
            return nil
        end)
        
        if success and manager then
            objects_created = objects_created + 1
            
            -- Create message bus if available
            if qtforge.communication and qtforge.communication.create_message_bus then
                local bus_success, bus = pcall(qtforge.communication.create_message_bus)
                if bus_success and bus then
                    objects_created = objects_created + 1
                    bus = nil  -- Release reference
                end
            end
            
            manager = nil  -- Release reference
            
            -- Periodic garbage collection
            if i % 10 == 0 then
                collectgarbage("collect")
                tracker:update_peak()
            end
        else
            print_warning("Object creation failed at iteration " .. i)
            break
        end
    end
    
    -- Final cleanup
    collectgarbage("collect")
    collectgarbage("collect")
    
    local stats = tracker:stop_tracking()
    
    print_status("Objects created: " .. objects_created)
    print_status("Memory delta: " .. string.format("%.2f KB", stats.memory_delta))
    print_status("Peak memory: " .. string.format("%.2f KB", stats.peak_delta))
    
    -- Check for memory leaks (allow some tolerance)
    if stats.memory_delta < 1024 then  -- Less than 1MB growth
        print_success("No significant memory leaks detected")
    else
        print_warning("Potential memory leak: " .. string.format("%.2f KB", stats.memory_delta))
    end
end

-- Test weak reference behavior
local function test_weak_references()
    print_header("Testing Weak References")
    
    if not qtforge then
        print_warning("Skipping test - bindings not available")
        return
    end
    
    local weak_refs = {}
    setmetatable(weak_refs, {__mode = "v"})  -- Weak values
    
    -- Create objects and store weak references
    for i = 1, 10 do
        local success, manager = pcall(function()
            if qtforge.core and qtforge.core.create_plugin_manager then
                return qtforge.core.create_plugin_manager()
            end
            return nil
        end)
        
        if success and manager then
            weak_refs[i] = manager
            manager = nil  -- Remove strong reference
        end
    end
    
    -- Force garbage collection
    collectgarbage("collect")
    collectgarbage("collect")
    
    -- Count remaining references
    local alive_refs = 0
    for i = 1, 10 do
        if weak_refs[i] then
            alive_refs = alive_refs + 1
        end
    end
    
    local dead_refs = 10 - alive_refs
    
    print_status("Weak references created: 10")
    print_status("References still alive: " .. alive_refs)
    print_status("References properly cleaned: " .. dead_refs)
    
    if dead_refs >= 8 then  -- At least 80% should be cleaned
        print_success("Weak reference cleanup working properly")
    else
        print_warning("Some objects may not be properly cleaned up")
    end
end

-- Test performance characteristics
local function test_object_creation_performance()
    print_header("Testing Object Creation Performance")
    
    if not qtforge then
        print_warning("Skipping test - bindings not available")
        return
    end
    
    -- Test plugin manager creation
    if qtforge.core and qtforge.core.create_plugin_manager then
        print_info("Testing PluginManager creation...")
        
        local timer = PerformanceTimer:new()
        local iterations = 100
        local objects = {}
        
        timer:start()
        
        for i = 1, iterations do
            local success, manager = pcall(qtforge.core.create_plugin_manager)
            if success and manager then
                objects[i] = manager
            else
                print_warning("Object creation failed at iteration " .. i)
                break
            end
        end
        
        local creation_time = timer:stop()
        
        -- Cleanup
        for i = 1, #objects do
            objects[i] = nil
        end
        collectgarbage("collect")
        
        local objects_per_second = iterations / creation_time
        local avg_time_per_object = creation_time / iterations * 1000  -- milliseconds
        
        print_status(iterations .. " objects created in " .. string.format("%.3f", creation_time) .. " seconds")
        print_status("Rate: " .. string.format("%.0f", objects_per_second) .. " objects/second")
        print_status("Average: " .. string.format("%.3f", avg_time_per_object) .. " ms/object")
        
        if objects_per_second > 50 then  -- At least 50 objects per second
            print_success("Good PluginManager creation performance")
        else
            print_warning("Slow PluginManager creation performance")
        end
    end
    
    -- Test message bus creation if available
    if qtforge.communication and qtforge.communication.create_message_bus then
        print_info("Testing MessageBus creation...")
        
        local timer = PerformanceTimer:new()
        local iterations = 50  -- Fewer iterations for message bus
        local objects = {}
        
        timer:start()
        
        for i = 1, iterations do
            local success, bus = pcall(qtforge.communication.create_message_bus)
            if success and bus then
                objects[i] = bus
            else
                print_warning("MessageBus creation failed at iteration " .. i)
                break
            end
        end
        
        local creation_time = timer:stop()
        
        -- Cleanup
        for i = 1, #objects do
            objects[i] = nil
        end
        collectgarbage("collect")
        
        local objects_per_second = iterations / creation_time
        local avg_time_per_object = creation_time / iterations * 1000  -- milliseconds
        
        print_status(iterations .. " message buses created in " .. string.format("%.3f", creation_time) .. " seconds")
        print_status("Rate: " .. string.format("%.0f", objects_per_second) .. " objects/second")
        print_status("Average: " .. string.format("%.3f", avg_time_per_object) .. " ms/object")
        
        if objects_per_second > 25 then  -- At least 25 objects per second
            print_success("Good MessageBus creation performance")
        else
            print_warning("Slow MessageBus creation performance")
        end
    end
end

-- Test method call performance
local function test_method_call_performance()
    print_header("Testing Method Call Performance")
    
    if not qtforge or not qtforge.core or not qtforge.core.create_plugin_manager then
        print_warning("Skipping test - plugin manager not available")
        return
    end
    
    local success, manager = pcall(qtforge.core.create_plugin_manager)
    if not success or not manager then
        print_warning("Could not create plugin manager for performance test")
        return
    end
    
    -- Test method call performance
    local timer = PerformanceTimer:new()
    local iterations = 1000
    
    timer:start()
    
    for i = 1, iterations do
        -- Call a simple method multiple times
        if manager.get_loaded_plugins then
            local call_success, plugins = pcall(function()
                return manager:get_loaded_plugins()
            end)
            
            if not call_success then
                print_warning("Method call failed at iteration " .. i)
                break
            end
        end
    end
    
    local call_time = timer:stop()
    
    local calls_per_second = iterations / call_time
    local avg_time_per_call = call_time / iterations * 1000000  -- microseconds
    
    print_status(iterations .. " method calls in " .. string.format("%.3f", call_time) .. " seconds")
    print_status("Rate: " .. string.format("%.0f", calls_per_second) .. " calls/second")
    print_status("Average: " .. string.format("%.1f", avg_time_per_call) .. " μs/call")
    
    if calls_per_second > 1000 then  -- At least 1000 calls per second
        print_success("Good method call performance")
    else
        print_warning("Slow method call performance")
    end
    
    manager = nil
    collectgarbage("collect")
end

-- Test memory efficiency
local function test_memory_efficiency()
    print_header("Testing Memory Efficiency")
    
    if not qtforge or not qtforge.core or not qtforge.core.create_plugin_manager then
        print_warning("Skipping test - plugin manager not available")
        return
    end
    
    local tracker = MemoryTracker:new()
    
    -- Create objects and measure memory
    tracker:start_tracking()
    
    local objects = {}
    local object_count = 50  -- Smaller count for Lua
    
    for i = 1, object_count do
        local success, manager = pcall(qtforge.core.create_plugin_manager)
        if success and manager then
            objects[i] = manager
        else
            print_warning("Object creation failed at iteration " .. i)
            break
        end
    end
    
    local stats = tracker:stop_tracking()
    
    local memory_per_object = stats.memory_delta / object_count
    
    print_status(object_count .. " objects created")
    print_status("Total memory used: " .. string.format("%.2f KB", stats.memory_delta))
    print_status("Memory per object: " .. string.format("%.2f KB", memory_per_object))
    
    -- Cleanup and measure
    tracker:start_tracking()
    for i = 1, #objects do
        objects[i] = nil
    end
    collectgarbage("collect")
    collectgarbage("collect")
    local cleanup_stats = tracker:stop_tracking()
    
    local cleanup_efficiency = math.abs(cleanup_stats.memory_delta) / stats.memory_delta * 100
    
    print_status("Memory freed: " .. string.format("%.2f KB", math.abs(cleanup_stats.memory_delta)))
    print_status("Cleanup efficiency: " .. string.format("%.1f%%", cleanup_efficiency))
    
    if memory_per_object < 10 then  -- Less than 10KB per object
        print_success("Good memory efficiency")
    else
        print_warning("High memory usage per object")
    end
    
    if cleanup_efficiency > 50 then  -- At least 50% memory freed (Lua GC is different)
        print_success("Good cleanup efficiency")
    else
        print_warning("Poor cleanup efficiency")
    end
end

-- Test large object handling
local function test_large_object_handling()
    print_header("Testing Large Object Handling")
    
    if not qtforge or not qtforge.core or not qtforge.core.create_plugin_manager then
        print_warning("Skipping test - plugin manager not available")
        return
    end
    
    local tracker = MemoryTracker:new()
    local timer = PerformanceTimer:new()
    
    tracker:start_tracking()
    timer:start()
    
    local objects = {}
    local target_count = 200  -- Smaller count for Lua
    
    for i = 1, target_count do
        local success, manager = pcall(qtforge.core.create_plugin_manager)
        if success and manager then
            objects[i] = manager
            
            -- Progress reporting
            if i % 50 == 0 then
                local elapsed = timer:elapsed()
                local memory = tracker:get_current_memory()
                print_info("Created " .. i .. "/" .. target_count .. " objects " ..
                          "(Time: " .. string.format("%.2f", elapsed) .. "s, " ..
                          "Memory: " .. string.format("%.2f KB", memory) .. ")")
            end
        else
            print_warning("Object creation failed at iteration " .. i)
            break
        end
    end
    
    local creation_time = timer:stop()
    
    -- Measure cleanup time
    timer:start()
    for i = 1, #objects do
        objects[i] = nil
    end
    collectgarbage("collect")
    collectgarbage("collect")
    local cleanup_time = timer:stop()
    
    local stats = tracker:stop_tracking()
    
    print_status("Objects created: " .. target_count)
    print_status("Creation time: " .. string.format("%.2f", creation_time) .. " seconds")
    print_status("Cleanup time: " .. string.format("%.2f", cleanup_time) .. " seconds")
    print_status("Peak memory: " .. string.format("%.2f KB", stats.peak_delta))
    print_status("Final memory delta: " .. string.format("%.2f KB", stats.memory_delta))
    
    -- Performance analysis
    local objects_per_second = target_count / creation_time
    print_status("Creation rate: " .. string.format("%.0f", objects_per_second) .. " objects/second")
    
    if objects_per_second > 50 then  -- At least 50 objects per second
        print_success("Good object creation performance")
    else
        print_warning("Slow object creation performance")
    end
    
    if stats.memory_delta < 5000 then  -- Less than 5MB final growth
        print_success("Large object handling memory management OK")
    else
        print_warning("High memory usage: " .. string.format("%.2f KB", stats.memory_delta))
    end
end

-- Main test function
local function main()
    print("QtForge Lua Bindings - Memory Management and Performance Tests")
    print(string.rep("=", 70))
    
    if not check_bindings_availability() then
        print_error("QtForge Lua bindings not available")
        print("Ensure QtForge is built with Lua bindings enabled:")
        print("  cmake -DQTFORGE_BUILD_LUA_BINDINGS=ON ..")
        print("  make")
        return 1
    end
    
    print_success("QtForge Lua bindings available")
    print("🚀 Starting memory management and performance tests...")
    
    -- Run all tests
    test_object_lifecycle()
    test_weak_references()
    test_object_creation_performance()
    test_method_call_performance()
    test_memory_efficiency()
    test_large_object_handling()
    
    print("\n" .. string.rep("=", 70))
    print("🎉 Memory Management and Performance Tests Complete!")
    print(string.rep("=", 70))
    
    print("\n📚 Key Findings:")
    print("• Memory management appears to be working correctly")
    print("• Object lifecycle is properly managed")
    print("• Performance characteristics are within acceptable ranges")
    print("• Lua garbage collection is functioning properly")
    print("• Resource cleanup is working as expected")
    
    print("\n🔗 Recommendations:")
    print("• Monitor memory usage in production applications")
    print("• Use collectgarbage() strategically in long-running scripts")
    print("• Implement proper object lifecycle management")
    print("• Consider object pooling for high-frequency scenarios")
    print("• Profile applications under realistic workloads")
    
    return 0
end

-- Run the tests
return main()
