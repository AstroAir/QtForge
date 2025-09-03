-- Test script to load QtForge Lua bindings using package.loadlib
print("=== QtForge Lua Bindings LoadLib Test ===")

-- Try to load the library
local lib_path = "../../libqtforge_lua.dll"
if not io.open(lib_path, "r") then
    lib_path = "../../../build/libqtforge_lua.dll"
end
print("Attempting to load library:", lib_path)

-- Try to load the init function
local init_func, err = package.loadlib(lib_path, "qtforge_lua_init")
if not init_func then
    print("Failed to load qtforge_lua_init:", err)
    return
end

print("Successfully loaded qtforge_lua_init function")

-- Try to initialize
local result = init_func()
if result == 1 then
    print("QtForge Lua bindings initialized successfully!")
else
    print("Failed to initialize QtForge Lua bindings")
    return
end

-- Try to load the execute function
local execute_func, err = package.loadlib(lib_path, "qtforge_lua_execute")
if not execute_func then
    print("Failed to load qtforge_lua_execute:", err)
    return
end

print("Successfully loaded qtforge_lua_execute function")

-- Test basic Lua code execution
print("\n--- Testing Lua code execution ---")
local test_code = [[
    print("Hello from executed Lua code!")
    print("Math test: 5 + 3 =", 5 + 3)
]]

-- Create error buffer (this is tricky in Lua, so we'll just test without error handling for now)
-- For now, let's just see if we can call the function
print("Attempting to execute Lua code...")

-- Try to load the shutdown function
local shutdown_func, err = package.loadlib(lib_path, "qtforge_lua_shutdown")
if shutdown_func then
    print("Successfully loaded qtforge_lua_shutdown function")
    -- Call shutdown
    shutdown_func()
    print("QtForge Lua bindings shut down")
else
    print("Failed to load qtforge_lua_shutdown:", err)
end

print("Test completed!")
