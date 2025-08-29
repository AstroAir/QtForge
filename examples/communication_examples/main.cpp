/**
 * @file main.cpp
 * @brief Main entry point for communication examples
 * @version 3.0.0
 */

#include <QCoreApplication>
#include <iostream>

#include "core/message_bus_example.hpp"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    std::cout << "QtForge Communication Examples\n";
    std::cout << "==============================\n\n";

    try {
        qtplugin::examples::AdvancedMessageBusExample example;
        int result = example.run_example();

        if (result == 0) {
            std::cout << "\n✅ All communication examples completed successfully!\n";
        } else {
            std::cout << "\n❌ Communication examples failed with code: " << result << "\n";
        }

        return result;

    } catch (const std::exception& e) {
        std::cerr << "❌ Fatal error: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "❌ Unknown fatal error occurred\n";
        return 1;
    }
}
