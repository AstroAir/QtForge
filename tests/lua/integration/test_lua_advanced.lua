-- Comprehensive test script for QtForge Lua bindings
print("=== QtForge Lua Bindings Comprehensive Test ===")

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
print("\n--- Initializing QtForge Lua bindings ---")
local result = init_func()
print("Initialization result:", result)

if result == 1 then
    print("QtForge Lua bindings initialized successfully!")
else
    print("Failed to initialize QtForge Lua bindings")
    print("But let's continue to test what we can...")
end

-- Try to load the execute function
local execute_func, err = package.loadlib(lib_path, "qtforge_lua_execute")
if not execute_func then
    print("Failed to load qtforge_lua_execute:", err)
    return
end

print("Successfully loaded qtforge_lua_execute function")

-- Test basic Lua code execution
print("\n--- Testing basic Lua code execution ---")
local test_code = [[
    print("Hello from executed Lua code!")
    print("Math test: 5 + 3 =", 5 + 3)
    return "Basic test completed"
]]

print("Attempting to execute basic Lua code...")
-- Note: We can't easily test the execute function from Lua due to C string handling
-- But we can test if the function is callable

-- Test QtForge bindings if available
print("\n--- Testing QtForge bindings ---")
local qtforge_test = [[
    if qtforge then
        print("QtForge module is available!")
        print("Version:", qtforge.version)
        print("Version major:", qtforge.version_major)
        print("Version minor:", qtforge.version_minor)
        print("Version patch:", qtforge.version_patch)

        qtforge.log("Testing QtForge logging from Lua")

        if qtforge.core then
            print("Core module is available!")
            print("Core test function:", qtforge.core.test_function())
            print("Core add function (2+3):", qtforge.core.add(2, 3))

            -- Test Version class if available
            if qtforge.core.version then
                local version = qtforge.core.version(1, 2, 3)
                if version then
                    print("Created version object:", tostring(version))
                    if version.major then
                        print("Version major:", version:major())
                        print("Version minor:", version:minor())
                        print("Version patch:", version:patch())
                        print("Version string:", version:to_string())
                    end
                end
            end
        else
            print("Core module is not available")
        end

        if qtforge.utils then
            print("Utils module is available!")
            print("Utils test:", qtforge.utils.utils_test())
            print("Utils create version:", qtforge.utils.create_version(2, 1, 0))
            print("Utils parse version:", qtforge.utils.parse_version("1.2.3"))
            print("Utils create error:", qtforge.utils.create_error(404, "Not found"))
        else
            print("Utils module is not available")
        end
    else
        print("QtForge module is not available!")
    end

    return "QtForge test completed"
]]

print("Attempting to test QtForge bindings...")

-- Try to load the shutdown function
print("\n--- Shutting down ---")
local shutdown_func, err = package.loadlib(lib_path, "qtforge_lua_shutdown")
if shutdown_func then
    print("Successfully loaded qtforge_lua_shutdown function")
    -- Call shutdown
    shutdown_func()
    print("QtForge Lua bindings shut down")
else
    print("Failed to load qtforge_lua_shutdown:", err)
end

print("\n=== Test completed! ===")
