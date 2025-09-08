-- QtForge Lua Test Plugin
-- This script demonstrates the enhanced Lua plugin bridge functionality

-- Plugin metadata
plugin = {
    name = "LuaTestPlugin",
    version = "1.0.0",
    description = "Test plugin for QtForge Lua bindings",
    author = "QtForge Development Team",

    -- Plugin properties
    counter = 0,
    message = "Hello from Lua!",
    enabled = true,

    -- Configuration
    config = {
        debug = true,
        max_iterations = 10,
        timeout = 5000
    }
}

-- Plugin initialization
function plugin.initialize()
    qtforge.log("Initializing Lua test plugin...")

    -- Test core bindings
    local version = qtforge.core.version(1, 0, 0)
    qtforge.log("Created version: " .. version:to_string())

    -- Test utils bindings
    local test_result = qtforge.utils.test()
    qtforge.log("Utils test: " .. test_result)

    -- Test string utilities
    local trimmed = qtforge.utils.trim("  hello world  ")
    qtforge.log("Trimmed string: '" .. trimmed .. "'")

    local parts = qtforge.utils.split("one,two,three", ",")
    qtforge.log("Split result has " .. #parts .. " parts")

    -- Test UUID generation
    local uuid = qtforge.utils.generate_uuid()
    qtforge.log("Generated UUID: " .. uuid)

    -- Test version parsing
    local parsed = qtforge.utils.parse_version("2.1.3")
    qtforge.log("Parsed version - major: " .. parsed.major .. ", minor: " .. parsed.minor .. ", patch: " .. parsed.patch)

    -- Test threading utilities
    local thread_count = qtforge.threading.get_thread_count()
    qtforge.log("Available threads: " .. thread_count)

    local is_main = qtforge.threading.is_main_thread()
    qtforge.log("Is main thread: " .. tostring(is_main))

    plugin.counter = 1
    qtforge.log("Plugin initialized successfully!")
    return true
end

-- Plugin shutdown
function plugin.shutdown()
    qtforge.log("Shutting down Lua test plugin...")
    plugin.enabled = false
    qtforge.log("Plugin shut down successfully!")
end

-- Test method with parameters
function plugin.test_method(param1, param2)
    qtforge.log("test_method called with: " .. tostring(param1) .. ", " .. tostring(param2))
    plugin.counter = plugin.counter + 1

    return {
        success = true,
        result = param1 + param2,
        counter = plugin.counter,
        message = "Method executed successfully"
    }
end

-- Test method without parameters
function plugin.get_status()
    return {
        name = plugin.name,
        version = plugin.version,
        counter = plugin.counter,
        enabled = plugin.enabled,
        uptime = os.time()
    }
end

-- Test method that demonstrates error handling
function plugin.test_error()
    error("This is a test error from Lua")
end

-- Test method that uses QtForge utilities
function plugin.process_data(data)
    if type(data) ~= "string" then
        return {
            success = false,
            error = "Data must be a string"
        }
    end

    -- Use QtForge utilities
    local trimmed = qtforge.utils.trim(data)
    local upper = qtforge.utils.to_upper(trimmed)
    local parts = qtforge.utils.split(upper, " ")
    local joined = qtforge.utils.join(parts, "_")

    return {
        success = true,
        original = data,
        processed = joined,
        word_count = #parts
    }
end

-- Test method that demonstrates configuration access
function plugin.update_config(key, value)
    if plugin.config[key] ~= nil then
        plugin.config[key] = value
        qtforge.log("Updated config: " .. key .. " = " .. tostring(value))
        return {
            success = true,
            message = "Configuration updated"
        }
    else
        return {
            success = false,
            error = "Unknown configuration key: " .. key
        }
    end
end

-- Test method that demonstrates async-like behavior
function plugin.delayed_operation(delay_ms)
    delay_ms = delay_ms or 1000

    qtforge.log("Starting delayed operation (" .. delay_ms .. "ms)...")

    -- Simulate work (in real scenario, this might be actual async work)
    qtforge.threading.sleep(delay_ms)

    qtforge.log("Delayed operation completed!")

    return {
        success = true,
        delay = delay_ms,
        timestamp = qtforge.utils.current_timestamp()
    }
end

-- Test method that demonstrates security features
function plugin.validate_input(input)
    if type(input) ~= "string" then
        return {
            success = false,
            error = "Input must be a string"
        }
    end

    local is_email = qtforge.utils.is_valid_email(input)
    local is_url = qtforge.utils.is_valid_url(input)

    return {
        success = true,
        input = input,
        is_email = is_email,
        is_url = is_url,
        length = string.len(input)
    }
end

-- Test method that demonstrates monitoring
function plugin.get_metrics()
    return {
        counter = plugin.counter,
        memory_usage = "N/A", -- Would be actual memory usage in real scenario
        cpu_usage = "N/A",    -- Would be actual CPU usage in real scenario
        last_activity = qtforge.utils.current_timestamp(),
        health_status = plugin.enabled and "Healthy" or "Disabled"
    }
end

-- Test method that demonstrates all binding modules
function plugin.test_all_bindings()
    local results = {}

    -- Test core bindings
    results.core = {
        test = qtforge.core and "Available" or "Not Available"
    }

    -- Test utils bindings
    results.utils = {
        test = qtforge.utils.test(),
        uuid = qtforge.utils.generate_uuid(),
        timestamp = qtforge.utils.current_timestamp(),
        trim = qtforge.utils.trim("  test  "),
        version_parse = qtforge.utils.parse_version("1.2.3")
    }

    -- Test threading bindings
    results.threading = {
        thread_count = qtforge.threading.get_thread_count(),
        is_main_thread = qtforge.threading.is_main_thread(),
        current_thread_id = qtforge.threading.current_thread_id()
    }

    -- Test security bindings (if available)
    if qtforge.security then
        results.security = {
            available = true,
            test = "Security module loaded"
        }
    else
        results.security = {
            available = false,
            test = "Security module not available"
        }
    end

    -- Test communication bindings (if available)
    if qtforge.communication then
        results.communication = {
            available = true,
            test = "Communication module loaded"
        }
    else
        results.communication = {
            available = false,
            test = "Communication module not available"
        }
    end

    -- Test managers bindings (if available)
    if qtforge.managers then
        results.managers = {
            available = true,
            test = "Managers module loaded"
        }
    else
        results.managers = {
            available = false,
            test = "Managers module not available"
        }
    end

    -- Test monitoring bindings (if available)
    if qtforge.monitoring then
        results.monitoring = {
            available = true,
            test = "Monitoring module loaded"
        }
    else
        results.monitoring = {
            available = false,
            test = "Monitoring module not available"
        }
    end

    -- Test orchestration bindings (if available)
    if qtforge.orchestration then
        results.orchestration = {
            available = true,
            test = "Orchestration module loaded"
        }
    else
        results.orchestration = {
            available = false,
            test = "Orchestration module not available"
        }
    end

    -- Test marketplace bindings (if available)
    if qtforge.marketplace then
        results.marketplace = {
            available = true,
            test = "Marketplace module loaded"
        }
    else
        results.marketplace = {
            available = false,
            test = "Marketplace module not available"
        }
    end

    -- Test composition bindings (if available)
    if qtforge.composition then
        results.composition = {
            available = true,
            test = "Composition module loaded"
        }
    else
        results.composition = {
            available = false,
            test = "Composition module not available"
        }
    end

    -- Test transactions bindings (if available)
    if qtforge.transactions then
        results.transactions = {
            available = true,
            test = "Transactions module loaded"
        }
    else
        results.transactions = {
            available = false,
            test = "Transactions module not available"
        }
    end

    return {
        success = true,
        binding_tests = results,
        total_modules = 10,
        available_modules = 0 -- Will be calculated by counting available modules
    }
end

-- Plugin command handler
function plugin.execute_command(command, params)
    qtforge.log("Executing command: " .. command)

    if command == "test" then
        return plugin.test_method(params.param1 or 0, params.param2 or 0)
    elseif command == "status" then
        return plugin.get_status()
    elseif command == "process" then
        return plugin.process_data(params.data or "")
    elseif command == "config" then
        return plugin.update_config(params.key, params.value)
    elseif command == "delay" then
        return plugin.delayed_operation(params.delay)
    elseif command == "validate" then
        return plugin.validate_input(params.input or "")
    elseif command == "metrics" then
        return plugin.get_metrics()
    elseif command == "test_bindings" then
        return plugin.test_all_bindings()
    else
        return {
            success = false,
            error = "Unknown command: " .. command,
            available_commands = {"test", "status", "process", "config", "delay", "validate", "metrics", "test_bindings"}
        }
    end
end

qtforge.log("Lua test plugin loaded successfully!")
