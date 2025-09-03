#include <iostream>
#include <sol/sol.hpp>

// Directly include the binding functions
namespace qtforge_lua {
    void register_core_bindings(sol::state& lua);
    void register_utils_bindings(sol::state& lua);
}

int main() {
    std::cout << "=== QtForge Lua Bindings Direct Test ===" << std::endl;
    
    try {
        // Create Lua state
        sol::state lua;
        
        // Open standard libraries
        lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::math, sol::lib::table);
        
        std::cout << "Lua state created successfully!" << std::endl;
        
        // Set up QtForge module table
        sol::table qtforge = lua.create_table();
        lua["qtforge"] = qtforge;
        
        // Add version information
        qtforge["version"] = "3.2.0";
        qtforge["version_major"] = 3;
        qtforge["version_minor"] = 2;
        qtforge["version_patch"] = 0;
        
        // Add logging function
        qtforge["log"] = [](const std::string& message) {
            std::cout << "Lua: " << message << std::endl;
        };
        
        std::cout << "QtForge module table created!" << std::endl;
        
        // Register bindings
        std::cout << "Registering core bindings..." << std::endl;
        qtforge_lua::register_core_bindings(lua);
        
        std::cout << "Registering utils bindings..." << std::endl;
        qtforge_lua::register_utils_bindings(lua);
        
        std::cout << "All bindings registered successfully!" << std::endl;
        
        // Test basic functionality
        std::cout << "\n--- Testing basic Lua functionality ---" << std::endl;
        lua.script("print('Hello from Lua!')");
        lua.script("print('2 + 3 =', 2 + 3)");
        
        // Test QtForge bindings
        std::cout << "\n--- Testing QtForge bindings ---" << std::endl;
        lua.script(R"(
            print("QtForge version:", qtforge.version)
            qtforge.log("Testing QtForge logging from Lua")
            
            if qtforge.core then
                print("Core module test:", qtforge.core.test_function())
                print("Core add test:", qtforge.core.add(5, 7))
            end
            
            if qtforge.utils then
                print("Utils test:", qtforge.utils.utils_test())
                print("Utils create version:", qtforge.utils.create_version(2, 1, 0))
            end
        )");
        
        std::cout << "\nAll tests completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
