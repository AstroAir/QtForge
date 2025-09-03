#include <iostream>
#include <string>

// C-style API declarations
extern "C" {
    int qtforge_lua_init();
    void qtforge_lua_shutdown();
    int qtforge_lua_execute(const char* code, char* error_buffer, size_t buffer_size);
    int qtforge_lua_load_file(const char* file_path, char* error_buffer, size_t buffer_size);
}

int main() {
    std::cout << "=== QtForge Lua Bindings Test Program ===" << std::endl;
    
    // Initialize Lua bindings
    std::cout << "Initializing QtForge Lua bindings..." << std::endl;
    if (!qtforge_lua_init()) {
        std::cerr << "Failed to initialize QtForge Lua bindings!" << std::endl;
        return 1;
    }
    std::cout << "QtForge Lua bindings initialized successfully!" << std::endl;
    
    // Test basic Lua code execution
    std::cout << "\n--- Testing basic Lua code execution ---" << std::endl;
    char error_buffer[1024];
    
    const char* test_code = R"(
        print("Hello from Lua!")
        print("2 + 3 =", 2 + 3)
    )";
    
    if (qtforge_lua_execute(test_code, error_buffer, sizeof(error_buffer))) {
        std::cout << "Basic Lua code executed successfully!" << std::endl;
    } else {
        std::cerr << "Failed to execute basic Lua code: " << error_buffer << std::endl;
    }
    
    // Test QtForge bindings
    std::cout << "\n--- Testing QtForge bindings ---" << std::endl;
    const char* qtforge_test = R"(
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
    
    if (qtforge_lua_execute(qtforge_test, error_buffer, sizeof(error_buffer))) {
        std::cout << "QtForge bindings test executed successfully!" << std::endl;
    } else {
        std::cerr << "Failed to execute QtForge bindings test: " << error_buffer << std::endl;
    }
    
    // Test loading Lua file
    std::cout << "\n--- Testing Lua file loading ---" << std::endl;
    if (qtforge_lua_load_file("test_lua_bindings.lua", error_buffer, sizeof(error_buffer))) {
        std::cout << "Lua file loaded and executed successfully!" << std::endl;
    } else {
        std::cerr << "Failed to load Lua file: " << error_buffer << std::endl;
    }
    
    // Shutdown
    std::cout << "\nShutting down QtForge Lua bindings..." << std::endl;
    qtforge_lua_shutdown();
    std::cout << "Test completed!" << std::endl;
    
    return 0;
}
