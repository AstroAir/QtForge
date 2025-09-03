-- Debug test for QtForge Lua module
print("=== QtForge Lua Module Debug Test ===")

-- Set the package path to include the build directory
package.cpath = package.cpath .. ";../../?.dll;../../../build/?.dll"

-- Load the qtforge module
local qtforge = require("qtforge")

if qtforge then
    print("QtForge module loaded successfully!")
    print("QtForge type:", type(qtforge))

    -- Debug all properties
    print("\n--- Debugging All Properties ---")
    for k, v in pairs(qtforge) do
        print(string.format("  %s: %s = %s", k, type(v), tostring(v)))

        -- If it's a table, show its contents
        if type(v) == "table" then
            print("    Contents of " .. k .. ":")
            for k2, v2 in pairs(v) do
                print(string.format("      %s: %s = %s", k2, type(v2), tostring(v2)))
            end
        end
    end

    -- Test direct access
    print("\n--- Testing Direct Access ---")
    print("qtforge.core:", qtforge.core)
    print("qtforge.utils:", qtforge.utils)
    print("qtforge.version:", qtforge.version)
    print("qtforge.log:", qtforge.log)

    -- Test function calls
    print("\n--- Testing Function Calls ---")
    if qtforge.log then
        print("Calling qtforge.log...")
        qtforge.log("Test message from debug script")
    end

    -- Try to access core functions directly
    print("\n--- Testing Core Access ---")
    local core = qtforge.core
    if core then
        print("Core table accessed successfully!")
        print("Core type:", type(core))
        for k, v in pairs(core) do
            print(string.format("  core.%s: %s = %s", k, type(v), tostring(v)))
        end

        -- Test core functions
        if core.test_function then
            print("Calling core.test_function:", core.test_function())
        end

        if core.add then
            print("Calling core.add(3, 4):", core.add(3, 4))
        end
    else
        print("Core table is nil or false")
    end

    -- Try to access utils functions directly
    print("\n--- Testing Utils Access ---")
    local utils = qtforge.utils
    if utils then
        print("Utils table accessed successfully!")
        print("Utils type:", type(utils))
        for k, v in pairs(utils) do
            print(string.format("  utils.%s: %s = %s", k, type(v), tostring(v)))
        end

        -- Test utils functions
        if utils.utils_test then
            print("Calling utils.utils_test:", utils.utils_test())
        end

        if utils.create_version then
            print("Calling utils.create_version(1,2,3):", utils.create_version(1, 2, 3))
        end
    else
        print("Utils table is nil or false")
    end

else
    print("Failed to load QtForge module!")
end

print("\n=== Debug test completed! ===")
