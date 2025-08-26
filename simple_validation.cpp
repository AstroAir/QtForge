#include <iostream>
#include <qtplugin/utils/version.hpp>
#include <qtplugin/core/plugin_interface.hpp>

using namespace qtplugin;

int main() {
    std::cout << "QtForge Library Validation Test" << std::endl;
    std::cout << "===============================" << std::endl;
    
    try {
        // Test version functionality
        std::cout << "Testing Version class..." << std::endl;
        Version v1(1, 2, 3);
        Version v2("1.2.3");
        
        if (v1 == v2) {
            std::cout << "✓ Version creation and parsing works" << std::endl;
        } else {
            std::cout << "✗ Version creation failed" << std::endl;
            return 1;
        }
        
        // Test version comparison
        Version v3(1, 2, 4);
        if (v1 < v3) {
            std::cout << "✓ Version comparison works" << std::endl;
        } else {
            std::cout << "✗ Version comparison failed" << std::endl;
            return 1;
        }
        
        std::cout << "✓ All basic tests passed!" << std::endl;
        std::cout << "QtForge libraries are working correctly." << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cout << "✗ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
