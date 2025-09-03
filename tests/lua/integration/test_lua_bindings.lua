-- Test script for QtForge Lua bindings
print("=== QtForge Lua Bindings Test ===")

-- Set up module path for integration tests
package.cpath = package.cpath .. ";../../?.dll;../../../build/?.dll"

-- Test basic qtforge module
print("QtForge version:", qtforge.version)
print("QtForge version major:", qtforge.version_major)
print("QtForge version minor:", qtforge.version_minor)
print("QtForge version patch:", qtforge.version_patch)

-- Test logging function
qtforge.log("Testing QtForge Lua logging")

-- Test core module
print("\n--- Core Module Tests ---")
print("Core test function:", qtforge.core.test_function())
print("Core add function (2+3):", qtforge.core.add(2, 3))

-- Test Version class
print("\n--- Version Class Tests ---")
local version = qtforge.core.version(1, 2, 3)
print("Created version:", tostring(version))
print("Version major:", version:major())
print("Version minor:", version:minor())
print("Version patch:", version:patch())
print("Version string:", version:to_string())

-- Test utils module
print("\n--- Utils Module Tests ---")
print("Utils test:", qtforge.utils.utils_test())
print("Create version:", qtforge.utils.create_version(1, 2, 3))
print("Parse version:", qtforge.utils.parse_version("1.2.3"))
print("Create error:", qtforge.utils.create_error(404, "Not found"))

print("\n=== All tests completed! ===")
