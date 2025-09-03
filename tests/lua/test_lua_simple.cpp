#include <iostream>
#include <string>

// Include the QtForge Lua headers
namespace qtforge_lua {
    bool initialize_qtforge_lua();
    void shutdown_qtforge_lua();
    bool execute_lua_code(const std::string& code, std::string& error_message);
    bool load_lua_file(const std::string& file_path, std::string& error_message);
}

int main() {
    std::cout << "=== QtForge Lua Bindings Simple Test ===" << std::endl;
    
    // Initialize Lua bindings
    std::cout << "Initializing QtForge Lua bindings..." << std::endl;
    if (!qtforge_lua::initialize_qtforge_lua()) {
        std::cerr << "Failed to initialize QtForge Lua bindings!" << std::endl;
        return 1;
    }
    std::cout << "QtForge Lua bindings initialized successfully!" << std::endl;
    
    // Test basic Lua code execution
    std::cout << "\n--- Testing basic Lua code execution ---" << std::endl;
    std::string error_message;
    
    const std::string test_code = R"(
        print("Hello from Lua!")
        print("2 + 3 =", 2 + 3)
    )";
    
    if (qtforge_lua::execute_lua_code(test_code, error_message)) {
        std::cout << "Basic Lua code executed successfully!" << std::endl;
    } else {
        std::cerr << "Failed to execute basic Lua code: " << error_message << std::endl;
    }
    
    // Test QtForge bindings
    std::cout << "\n--- Testing QtForge bindings ---" << std::endl;
    const std::string qtforge_test = R"(
        if qtforge then
            print("QtForge module is available!")
            print("Version:", qtforge.version)
            qtforge.log("Testing QtForge logging from Lua")
            
            if qtforge.core then
                print("Core module test:", qtforge.core.test_function())
                print("Core add test:", qtforge.core.add(5, 7))
            end
            
            if qtforge.utils then
                print("Utils test:", qtforge.utils.utils_test())
                print("Utils create version:", qtforge.utils.create_version(2, 1, 0))
            end
        else
            print("QtForge module is not available!")
        end
    )";
    
    if (qtforge_lua::execute_lua_code(qtforge_test, error_message)) {
        std::cout << "QtForge bindings test executed successfully!" << std::endl;
    } else {
        std::cerr << "Failed to execute QtForge bindings test: " << error_message << std::endl;
    }
    
    // Test loading Lua file
    std::cout << "\n--- Testing Lua file loading ---" << std::endl;
    if (qtforge_lua::load_lua_file("../test_lua_bindings.lua", error_message)) {
        std::cout << "Lua file loaded and executed successfully!" << std::endl;
    } else {
        std::cerr << "Failed to load Lua file: " << error_message << std::endl;
    }
    
    // Shutdown
    std::cout << "\nShutting down QtForge Lua bindings..." << std::endl;
    qtforge_lua::shutdown_qtforge_lua();
    std::cout << "Test completed!" << std::endl;
    
    return 0;
}
