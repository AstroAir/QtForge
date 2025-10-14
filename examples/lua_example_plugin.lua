-- Simple Lua plugin example for QtForge
-- This demonstrates the basic structure expected by the Lua plugin loader

-- Plugin metadata table (required)
plugin = {
    name = "Simple Lua Plugin",
    version = "1.0.0",
    description = "A simple example Lua plugin for QtForge",
    author = "QtForge Team",

    -- Plugin capabilities
    capabilities = {
        "basic_lua_plugin",
        "example_plugin"
    },

    -- Plugin dependencies (optional)
    dependencies = {
        -- "core_system"
    },

    -- Entry points (optional)
    entry_points = {
        main = "main_function"
    },

    -- Custom initialization and shutdown functions (optional)
    initialization_function = "initialize",
    shutdown_function = "shutdown"
}

-- Plugin state
local plugin_state = {
    initialized = false,
    data = {}
}

-- Initialize function (called when plugin is loaded)
function initialize()
    print("Simple Lua Plugin: Initializing...")
    plugin_state.initialized = true
    plugin_state.data.initialized_at = os.time()
    print("Simple Lua Plugin: Initialized successfully!")
    return true
end

-- Shutdown function (called when plugin is unloaded)
function shutdown()
    print("Simple Lua Plugin: Shutting down...")
    plugin_state.initialized = false
    plugin_state.data = {}
    print("Simple Lua Plugin: Shutdown complete!")
    return true
end

-- Main plugin function
function main_function(args)
    if not plugin_state.initialized then
        error("Plugin not initialized")
    end

    print("Simple Lua Plugin: Main function called with args:", args)

    -- Return a result
    return {
        success = true,
        message = "Hello from Simple Lua Plugin!",
        timestamp = os.time(),
        input_args = args
    }
end

-- Additional plugin functions
function get_status()
    return {
        initialized = plugin_state.initialized,
        uptime = plugin_state.initialized and (os.time() - plugin_state.data.initialized_at) or 0,
        data_count = #plugin_state.data
    }
end

-- Export functions for external use
if _G.qtforge_plugin_api then
    _G.qtforge_plugin_api.get_status = get_status
    _G.qtforge_plugin_api.main_function = main_function
end

print("Simple Lua Plugin: Script loaded successfully!")
