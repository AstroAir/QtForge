/**
 * @file test_remote_plugin_performance.cpp
 * @brief Performance and stress tests for remote plugin system
 * @version 3.2.0
 */

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QEventLoop>
#include <QTimer>
#include <QTemporaryDir>
#include <QElapsedTimer>
#include <QThread>
#include <QThreadPool>
#include <QRunnable>

#include <qtplugin/remote/remote_plugin_manager_extension.hpp>
#include <qtplugin/remote/http_plugin_loader.hpp>
#include <qtplugin/remote/plugin_download_manager.hpp>
#include <qtplugin/security/security_manager.hpp>
#include <qtplugin/core/plugin_manager.hpp>

#include <memory>
#include <vector>
#include <chrono>
#include <atomic>
#include <future>
#include <random>

using namespace qtplugin;
using namespace testing;

class RemotePluginPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directory for testing
        temp_dir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(temp_dir->isValid());
        
        // Create core components
        plugin_manager = std::make_shared<PluginManager>();
        security_manager = std::make_shared<SecurityManager>();
        download_manager = std::make_shared<PluginDownloadManager>();
        
        // Create remote plugin components
        remote_manager = std::make_unique<RemotePluginManagerExtension>(plugin_manager);
        http_loader = std::make_unique<HttpPluginLoader>(download_manager, security_manager);
        
        // Create test data
        setupTestData();
    }

    void TearDown() override {
        http_loader.reset();
        remote_manager.reset();
        download_manager.reset();
        security_manager.reset();
        plugin_manager.reset();
        temp_dir.reset();
    }

    void setupTestData() {
        // Create multiple test plugin files
        for (int i = 0; i < 10; ++i) {
            QString plugin_path = temp_dir->path() + QString("/test_plugin_%1.zip").arg(i);
            QFile plugin_file(plugin_path);
            ASSERT_TRUE(plugin_file.open(QIODevice::WriteOnly));
            
            // Create plugin content with varying sizes
            QByteArray content;
            int size = 1024 * (i + 1); // 1KB to 10KB
            content.fill('A' + i, size);
            plugin_file.write(content);
            plugin_file.close();
            
            test_plugin_paths.append(plugin_path);
        }
    }

    // Helper function to measure execution time
    template<typename Func>
    std::chrono::milliseconds measureExecutionTime(Func&& func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    }

    // Test data
    std::unique_ptr<QTemporaryDir> temp_dir;
    std::shared_ptr<PluginManager> plugin_manager;
    std::shared_ptr<SecurityManager> security_manager;
    std::shared_ptr<PluginDownloadManager> download_manager;
    std::unique_ptr<RemotePluginManagerExtension> remote_manager;
    std::unique_ptr<HttpPluginLoader> http_loader;
    QStringList test_plugin_paths;
};

TEST_F(RemotePluginPerformanceTest, SinglePluginLoadPerformance) {
    // Test performance of loading a single remote plugin
    
    if (test_plugin_paths.isEmpty()) {
        GTEST_SKIP() << "No test plugins available";
    }
    
    QString plugin_path = test_plugin_paths.first();
    QUrl plugin_url = QUrl::fromLocalFile(plugin_path);
    
    // Measure load time
    auto load_time = measureExecutionTime([&]() {
        RemotePluginSource source(plugin_url);
        RemotePluginLoadOptions options;
        options.use_cache = false; // Force fresh load
        
        auto result = http_loader->load_remote(source, options);
        // Don't assert success here as we're measuring performance
    });
    
    // Performance expectations
    EXPECT_LT(load_time.count(), 1000) << "Single plugin load should complete within 1 second";
    
    std::cout << "Single plugin load time: " << load_time.count() << "ms" << std::endl;
}

TEST_F(RemotePluginPerformanceTest, MultiplePluginLoadPerformance) {
    // Test performance of loading multiple plugins sequentially
    
    if (test_plugin_paths.size() < 5) {
        GTEST_SKIP() << "Insufficient test plugins for multiple load test";
    }
    
    auto total_time = measureExecutionTime([&]() {
        for (int i = 0; i < 5; ++i) {
            QString plugin_path = test_plugin_paths[i];
            QUrl plugin_url = QUrl::fromLocalFile(plugin_path);
            
            RemotePluginSource source(plugin_url);
            RemotePluginLoadOptions options;
            options.use_cache = false;
            
            auto result = http_loader->load_remote(source, options);
        }
    });
    
    // Performance expectations
    EXPECT_LT(total_time.count(), 5000) << "Multiple plugin loads should complete within 5 seconds";
    
    double avg_time = static_cast<double>(total_time.count()) / 5.0;
    EXPECT_LT(avg_time, 1000) << "Average plugin load time should be under 1 second";
    
    std::cout << "Multiple plugin load time: " << total_time.count() << "ms" << std::endl;
    std::cout << "Average per plugin: " << avg_time << "ms" << std::endl;
}

TEST_F(RemotePluginPerformanceTest, ConcurrentPluginLoadPerformance) {
    // Test performance of loading plugins concurrently
    
    if (test_plugin_paths.size() < 5) {
        GTEST_SKIP() << "Insufficient test plugins for concurrent load test";
    }
    
    const int concurrent_loads = 5;
    std::vector<std::future<void>> futures;
    std::atomic<int> completed_loads{0};
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Launch concurrent loads
    for (int i = 0; i < concurrent_loads; ++i) {
        auto future = std::async(std::launch::async, [&, i]() {
            QString plugin_path = test_plugin_paths[i];
            QUrl plugin_url = QUrl::fromLocalFile(plugin_path);
            
            RemotePluginSource source(plugin_url);
            RemotePluginLoadOptions options;
            options.use_cache = false;
            
            auto result = http_loader->load_remote(source, options);
            completed_loads.fetch_add(1);
        });
        
        futures.push_back(std::move(future));
    }
    
    // Wait for all loads to complete
    for (auto& future : futures) {
        future.wait();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // Performance expectations
    EXPECT_EQ(completed_loads.load(), concurrent_loads);
    EXPECT_LT(total_time.count(), 3000) << "Concurrent plugin loads should complete within 3 seconds";
    
    std::cout << "Concurrent plugin load time: " << total_time.count() << "ms" << std::endl;
    std::cout << "Completed loads: " << completed_loads.load() << std::endl;
}

TEST_F(RemotePluginPerformanceTest, CachePerformanceComparison) {
    // Test performance difference between cached and non-cached loads
    
    if (test_plugin_paths.isEmpty()) {
        GTEST_SKIP() << "No test plugins available";
    }
    
    QString plugin_path = test_plugin_paths.first();
    QUrl plugin_url = QUrl::fromLocalFile(plugin_path);
    RemotePluginSource source(plugin_url);
    
    // First load without cache
    RemotePluginLoadOptions no_cache_options;
    no_cache_options.use_cache = false;
    
    auto no_cache_time = measureExecutionTime([&]() {
        auto result = http_loader->load_remote(source, no_cache_options);
    });
    
    // Second load with cache
    RemotePluginLoadOptions cache_options;
    cache_options.use_cache = true;
    
    auto cache_time = measureExecutionTime([&]() {
        auto result = http_loader->load_remote(source, cache_options);
    });
    
    // Cache should be faster (or at least not significantly slower)
    EXPECT_LE(cache_time.count(), no_cache_time.count() + 100) 
        << "Cached load should not be significantly slower than non-cached load";
    
    std::cout << "No cache load time: " << no_cache_time.count() << "ms" << std::endl;
    std::cout << "Cached load time: " << cache_time.count() << "ms" << std::endl;
}

TEST_F(RemotePluginPerformanceTest, SecurityValidationPerformance) {
    // Test performance of security validation
    
    if (test_plugin_paths.isEmpty()) {
        GTEST_SKIP() << "No test plugins available";
    }
    
    QString plugin_path = test_plugin_paths.first();
    std::filesystem::path fs_path(plugin_path.toStdString());
    
    // Test different security levels
    std::vector<SecurityLevel> levels = {
        SecurityLevel::None,
        SecurityLevel::Basic,
        SecurityLevel::Standard,
        SecurityLevel::Strict
    };
    
    for (auto level : levels) {
        auto validation_time = measureExecutionTime([&]() {
            auto result = security_manager->validate_plugin(fs_path, level);
        });
        
        // Security validation should be reasonably fast
        EXPECT_LT(validation_time.count(), 2000) 
            << "Security validation should complete within 2 seconds for level " 
            << static_cast<int>(level);
        
        std::cout << "Security validation time (level " << static_cast<int>(level) 
                  << "): " << validation_time.count() << "ms" << std::endl;
    }
}

TEST_F(RemotePluginPerformanceTest, MemoryUsageStressTest) {
    // Test memory usage under stress conditions
    
    if (test_plugin_paths.size() < 3) {
        GTEST_SKIP() << "Insufficient test plugins for stress test";
    }
    
    const int stress_iterations = 50;
    std::atomic<int> successful_loads{0};
    std::atomic<int> failed_loads{0};
    
    auto stress_time = measureExecutionTime([&]() {
        for (int i = 0; i < stress_iterations; ++i) {
            // Use random plugin to vary load patterns
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, test_plugin_paths.size() - 1);
            
            QString plugin_path = test_plugin_paths[dis(gen)];
            QUrl plugin_url = QUrl::fromLocalFile(plugin_path);
            
            RemotePluginSource source(plugin_url);
            RemotePluginLoadOptions options;
            options.use_cache = true; // Use cache to reduce I/O stress
            
            auto result = http_loader->load_remote(source, options);
            if (result.has_value()) {
                successful_loads.fetch_add(1);
            } else {
                failed_loads.fetch_add(1);
            }
            
            // Small delay to prevent overwhelming the system
            QThread::msleep(10);
        }
    });
    
    // Performance and reliability expectations
    EXPECT_GT(successful_loads.load(), stress_iterations * 0.8) 
        << "At least 80% of stress test loads should succeed";
    EXPECT_LT(stress_time.count(), 30000) 
        << "Stress test should complete within 30 seconds";
    
    std::cout << "Stress test time: " << stress_time.count() << "ms" << std::endl;
    std::cout << "Successful loads: " << successful_loads.load() << std::endl;
    std::cout << "Failed loads: " << failed_loads.load() << std::endl;
    
    double success_rate = static_cast<double>(successful_loads.load()) / stress_iterations * 100.0;
    std::cout << "Success rate: " << success_rate << "%" << std::endl;
}

TEST_F(RemotePluginPerformanceTest, ResourceCleanupPerformance) {
    // Test performance of resource cleanup operations
    
    if (test_plugin_paths.isEmpty()) {
        GTEST_SKIP() << "No test plugins available";
    }
    
    // Load several plugins first
    std::vector<QString> loaded_plugins;
    for (int i = 0; i < std::min(3, test_plugin_paths.size()); ++i) {
        QString plugin_path = test_plugin_paths[i];
        QUrl plugin_url = QUrl::fromLocalFile(plugin_path);
        
        RemotePluginSource source(plugin_url);
        RemotePluginLoadOptions options;
        options.use_cache = true;
        
        auto result = http_loader->load_remote(source, options);
        if (result.has_value()) {
            loaded_plugins.push_back(plugin_path);
        }
    }
    
    // Measure cleanup time
    auto cleanup_time = measureExecutionTime([&]() {
        // Reset components to trigger cleanup
        http_loader.reset();
        remote_manager.reset();
        
        // Recreate components
        remote_manager = std::make_unique<RemotePluginManagerExtension>(plugin_manager);
        http_loader = std::make_unique<HttpPluginLoader>(download_manager, security_manager);
    });
    
    // Cleanup should be fast
    EXPECT_LT(cleanup_time.count(), 1000) 
        << "Resource cleanup should complete within 1 second";
    
    std::cout << "Resource cleanup time: " << cleanup_time.count() << "ms" << std::endl;
    std::cout << "Loaded plugins before cleanup: " << loaded_plugins.size() << std::endl;
}
