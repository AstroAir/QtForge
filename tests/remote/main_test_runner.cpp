/**
 * @file main_test_runner.cpp
 * @brief Main test runner for remote plugin system tests
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QCoreApplication>
#include <QDir>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <iostream>
#include <filesystem>

// Custom test listener for better output formatting
class RemotePluginTestListener : public ::testing::EmptyTestEventListener {
public:
    void OnTestStart(const ::testing::TestInfo& test_info) override {
        std::cout << "\n[RUNNING] " << test_info.test_suite_name() 
                  << "." << test_info.name() << std::endl;
    }
    
    void OnTestEnd(const ::testing::TestInfo& test_info) override {
        if (test_info.result()->Passed()) {
            std::cout << "[  PASS  ] " << test_info.test_suite_name() 
                      << "." << test_info.name() << std::endl;
        } else {
            std::cout << "[  FAIL  ] " << test_info.test_suite_name() 
                      << "." << test_info.name() << std::endl;
        }
    }
    
    void OnTestSuiteStart(const ::testing::TestSuite& test_suite) override {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "Running test suite: " << test_suite.name() << std::endl;
        std::cout << std::string(60, '=') << std::endl;
    }
    
    void OnTestSuiteEnd(const ::testing::TestSuite& test_suite) override {
        std::cout << "\nTest suite " << test_suite.name() << " completed:" << std::endl;
        std::cout << "  Tests run: " << test_suite.total_test_count() << std::endl;
        std::cout << "  Passed: " << test_suite.successful_test_count() << std::endl;
        std::cout << "  Failed: " << test_suite.failed_test_count() << std::endl;
        std::cout << std::string(60, '-') << std::endl;
    }
};

// Test environment setup
class RemotePluginTestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        std::cout << "Setting up remote plugin test environment..." << std::endl;
        
        // Set up Qt application
        setupQtEnvironment();
        
        // Create test directories
        setupTestDirectories();
        
        // Configure logging
        setupLogging();
        
        std::cout << "Remote plugin test environment ready." << std::endl;
    }
    
    void TearDown() override {
        std::cout << "Cleaning up remote plugin test environment..." << std::endl;
        
        // Clean up test directories
        cleanupTestDirectories();
        
        std::cout << "Remote plugin test environment cleaned up." << std::endl;
    }

private:
    void setupQtEnvironment() {
        // Set Qt platform to offscreen for headless testing
        qputenv("QT_QPA_PLATFORM", "offscreen");
        
        // Disable Qt debug output during tests
        qputenv("QT_LOGGING_RULES", "*.debug=false");
        
        // Set test-specific paths
        QStandardPaths::setTestModeEnabled(true);
    }
    
    void setupTestDirectories() {
        // Create temporary directories for testing
        test_cache_dir = std::filesystem::temp_directory_path() / "qtforge_remote_plugin_tests";
        test_data_dir = test_cache_dir / "test_data";
        test_plugins_dir = test_cache_dir / "plugins";
        
        std::filesystem::create_directories(test_cache_dir);
        std::filesystem::create_directories(test_data_dir);
        std::filesystem::create_directories(test_plugins_dir);
        
        // Set environment variables for tests
        qputenv("QTFORGE_TEST_CACHE_DIR", test_cache_dir.string().c_str());
        qputenv("QTFORGE_TEST_DATA_DIR", test_data_dir.string().c_str());
        qputenv("QTFORGE_TEST_PLUGINS_DIR", test_plugins_dir.string().c_str());
    }
    
    void cleanupTestDirectories() {
        try {
            if (std::filesystem::exists(test_cache_dir)) {
                std::filesystem::remove_all(test_cache_dir);
            }
        } catch (const std::exception& e) {
            std::cerr << "Warning: Failed to clean up test directories: " << e.what() << std::endl;
        }
    }
    
    void setupLogging() {
        // Configure Qt logging for tests
        QLoggingCategory::setFilterRules(
            "qtforge.remote.debug=false\n"
            "qtforge.remote.info=true\n"
            "qtforge.remote.warning=true\n"
            "qtforge.remote.critical=true"
        );
    }
    
    std::filesystem::path test_cache_dir;
    std::filesystem::path test_data_dir;
    std::filesystem::path test_plugins_dir;
};

// Custom main function for Qt integration
int main(int argc, char** argv) {
    // Initialize Qt application
    QCoreApplication app(argc, argv);
    app.setApplicationName("QtForge Remote Plugin Tests");
    app.setApplicationVersion("3.0.0");
    app.setOrganizationName("QtForge");
    
    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    
    // Add custom test environment
    ::testing::AddGlobalTestEnvironment(new RemotePluginTestEnvironment);
    
    // Add custom test listener for better output
    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new RemotePluginTestListener);
    
    // Parse command line arguments for test-specific options
    bool verbose = false;
    bool list_tests = false;
    std::string filter;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--verbose" || arg == "-v") {
            verbose = true;
        } else if (arg == "--list-tests") {
            list_tests = true;
        } else if (arg.starts_with("--gtest_filter=")) {
            filter = arg.substr(15);
        }
    }
    
    if (verbose) {
        std::cout << "Running in verbose mode" << std::endl;
        qputenv("QT_LOGGING_RULES", "*.debug=true");
    }
    
    if (list_tests) {
        std::cout << "Available test suites:" << std::endl;
        std::cout << "  RemotePluginSourceTest - Tests for RemotePluginSource class" << std::endl;
        std::cout << "  PluginDownloadManagerTest - Tests for PluginDownloadManager class" << std::endl;
        std::cout << "  HttpPluginLoaderTest - Tests for HttpPluginLoader class" << std::endl;
        std::cout << "  RemotePluginIntegrationTest - Integration tests for complete system" << std::endl;
        std::cout << "  RemotePluginSecurityTest - Security and validation tests" << std::endl;
        return 0;
    }
    
    if (!filter.empty()) {
        std::cout << "Running tests matching filter: " << filter << std::endl;
    }
    
    // Print test configuration
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "QtForge Remote Plugin System Test Suite" << std::endl;
    std::cout << "Version: 3.0.0" << std::endl;
    std::cout << "Build: " << __DATE__ << " " << __TIME__ << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    // Print environment information
    std::cout << "\nTest Environment:" << std::endl;
    std::cout << "  Qt Version: " << qVersion() << std::endl;
    std::cout << "  Platform: " << qgetenv("QT_QPA_PLATFORM") << std::endl;
    std::cout << "  Test Mode: " << (QStandardPaths::isTestModeEnabled() ? "Enabled" : "Disabled") << std::endl;
    std::cout << "  Cache Dir: " << qgetenv("QTFORGE_TEST_CACHE_DIR") << std::endl;
    
    // Run the tests
    std::cout << "\nStarting test execution..." << std::endl;
    int result = RUN_ALL_TESTS();
    
    // Print summary
    std::cout << "\n" << std::string(80, '=') << std::endl;
    if (result == 0) {
        std::cout << "All tests PASSED!" << std::endl;
    } else {
        std::cout << "Some tests FAILED!" << std::endl;
    }
    std::cout << std::string(80, '=') << std::endl;
    
    return result;
}

// Alternative simple main for individual test files
#ifdef SIMPLE_TEST_MAIN
int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#endif
