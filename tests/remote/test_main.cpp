/**
 * @file test_main.cpp
 * @brief Simple main function for individual test files
 */

#include <gtest/gtest.h>
#include <QCoreApplication>

int main(int argc, char** argv) {
    // Initialize Qt application for tests that need it
    QCoreApplication app(argc, argv);
    
    // Set up test environment
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_LOGGING_RULES", "*.debug=false");
    
    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);
    
    // Run tests
    return RUN_ALL_TESTS();
}
