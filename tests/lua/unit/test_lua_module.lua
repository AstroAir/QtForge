-- Test script for QtForge Lua module
print("=== QtForge Lua Module Test ===")

-- Set the package path to include the build directory
package.cpath = package.cpath .. ";../../?.dll;../../../build/?.dll"

-- Try to require the qtforge module
print("Attempting to load QtForge module...")
local success, qtforge = pcall(require, "qtforge")

if not success then
    print("Failed to load QtForge module:", qtforge)
    print("Trying alternative loading method...")

    -- Try to load as libqtforge_lua
    local success2, qtforge2 = pcall(require, "libqtforge_lua")
    if not success2 then
        print("Failed to load libqtforge_lua:", qtforge2)
        return
    else
        qtforge = qtforge2
        print("Successfully loaded libqtforge_lua module!")
    end
else
    print("Successfully loaded QtForge module!")
end

-- Test the module
if qtforge then
    print("\n--- Testing QtForge Module ---")
    print("QtForge module type:", type(qtforge))

    -- Print all available functions/properties
    print("\nAvailable QtForge properties:")
    for k, v in pairs(qtforge) do
        print("  " .. k .. ":", type(v))
    end

    -- Test version information
    if qtforge.version then
        print("\nVersion information:")
        print("  Version:", qtforge.version)
        print("  Version major:", qtforge.version_major)
        print("  Version minor:", qtforge.version_minor)
        print("  Version patch:", qtforge.version_patch)
    end

    -- Test logging function
    if qtforge.log then
        print("\nTesting logging function:")
        qtforge.log("Hello from QtForge Lua module!")
    end

    -- Test core module
    print("\n--- Testing Core Module ---")
    print("Core module exists:", qtforge.core ~= nil)
    if qtforge.core then
        print("Core module type:", type(qtforge.core))

        -- Print all available core functions
        print("Available core functions:")
        for k, v in pairs(qtforge.core) do
            print("  " .. k .. ":", type(v))
        end

        -- Test core functions
        if qtforge.core.test_function then
            print("\nCore test function:", qtforge.core.test_function())
        end

        if qtforge.core.add then
            print("Core add function (5+7):", qtforge.core.add(5, 7))
        end

        -- Test Version class
        if qtforge.core.version then
            print("\nTesting Version class:")
            local version = qtforge.core.version(1, 2, 3)
            if version then
                print("Created version:", tostring(version))
                if version.major then
                    print("Version major:", version:major())
                    print("Version minor:", version:minor())
                    print("Version patch:", version:patch())
                    print("Version string:", version:to_string())
                end
            end
        end
    else
        print("Core module not available")
    end

    -- Test utils module
    print("\n--- Testing Utils Module ---")
    print("Utils module exists:", qtforge.utils ~= nil)
    if qtforge.utils then
        print("Utils module type:", type(qtforge.utils))

        -- Print all available utils functions
        print("Available utils functions:")
        for k, v in pairs(qtforge.utils) do
            print("  " .. k .. ":", type(v))
        end

        -- Test utils functions
        if qtforge.utils.utils_test then
            print("\nUtils test:", qtforge.utils.utils_test())
        end

        if qtforge.utils.create_version then
            print("Utils create version:", qtforge.utils.create_version(2, 1, 0))
        end

        if qtforge.utils.parse_version then
            print("Utils parse version:", qtforge.utils.parse_version("1.2.3"))
        end

        if qtforge.utils.create_error then
            print("Utils create error:", qtforge.utils.create_error(404, "Not found"))
        end
    else
        print("Utils module not available")
    end

else
    print("QtForge module is nil!")
end

print("\n=== Test completed! ===")
