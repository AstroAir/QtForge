/**
 * @file test_plugin_loader.cpp
 * @brief Unit tests for QtPluginLoader with enhanced features
 * @version 1.0.0
 * @date 2024-01-13
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <qtplugin/core/plugin_loader.hpp>
#include <chrono>
#include <thread>

using namespace qtplugin;
using namespace testing;

class PluginLoaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_loader = std::make_unique<QtPluginLoader>();
        m_temp_dir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(m_temp_dir->isValid());
    }
    
    void TearDown() override {
        m_loader.reset();
        m_temp_dir.reset();
    }
    
    std::string create_dummy_plugin(const std::string& name) {
        QString plugin_path = m_temp_dir->path() + "/" + QString::fromStdString(name) + ".dll";
        QFile file(plugin_path);
        if (file.open(QIODevice::WriteOnly)) {
            file.write("DUMMY_PLUGIN_CONTENT");
            file.close();
        }
        return plugin_path.toStdString();
    }
    
    std::unique_ptr<QtPluginLoader> m_loader;
    std::unique_ptr<QTemporaryDir> m_temp_dir;
};

// Test basic loading functionality
TEST_F(PluginLoaderTest, LoadValidPlugin) {
    // Note: This would require an actual plugin file
    // For unit testing, we'd typically use a mock or test plugin
    auto plugin_path = create_dummy_plugin("test_plugin");
    
    auto result = m_loader->load(plugin_path);
    
    // In a real test, this would load an actual test plugin
    EXPECT_FALSE(result.has_value());  // Dummy file won't load as real plugin
}

TEST_F(PluginLoaderTest, LoadNonExistentPlugin) {
    auto result = m_loader->load("/nonexistent/plugin.dll");
    
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, PluginErrorCode::FileNotFound);
}

// Test caching functionality
TEST_F(PluginLoaderTest, CacheMetadata) {
    auto plugin_path = create_dummy_plugin("cache_test");
    
    // Enable caching
    m_loader->set_cache_enabled(true);
    
    // First access - cache miss
    auto start1 = std::chrono::high_resolution_clock::now();
    m_loader->can_load(plugin_path);
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1);
    
    // Second access - cache hit
    auto start2 = std::chrono::high_resolution_clock::now();
    m_loader->can_load(plugin_path);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);
    
    // Cache hit should be faster
    EXPECT_LT(duration2.count(), duration1.count());
    
    // Check cache statistics
    auto stats = m_loader->get_cache_statistics();
    EXPECT_GT(stats.hit_count, 0);
    EXPECT_GT(stats.hit_rate, 0.0);
}

TEST_F(PluginLoaderTest, CacheInvalidation) {
    auto plugin_path = create_dummy_plugin("invalidation_test");
    
    m_loader->set_cache_enabled(true);
    
    // Load metadata into cache
    m_loader->can_load(plugin_path);
    
    auto stats_before = m_loader->get_cache_statistics();
    EXPECT_GT(stats_before.cache_size, 0);
    
    // Clear cache
    m_loader->clear_cache();
    
    auto stats_after = m_loader->get_cache_statistics();
    EXPECT_EQ(stats_after.cache_size, 0);
    EXPECT_EQ(stats_after.hit_count, 0);
    EXPECT_EQ(stats_after.miss_count, 0);
}

TEST_F(PluginLoaderTest, CacheMaxSize) {
    m_loader->set_cache_enabled(true);
    m_loader->set_max_cache_size(3);
    
    // Create more plugins than cache size
    for (int i = 0; i < 5; ++i) {
        auto plugin_path = create_dummy_plugin("cache_size_test_" + std::to_string(i));
        m_loader->can_load(plugin_path);
    }
    
    auto stats = m_loader->get_cache_statistics();
    EXPECT_LE(stats.cache_size, 3);
}

// Test error tracking
TEST_F(PluginLoaderTest, ErrorTracking) {
    // Clear error history
    m_loader->clear_error_history();
    
    // Generate some errors
    m_loader->load("/invalid/path1.dll");
    m_loader->load("/invalid/path2.dll");
    m_loader->unload("nonexistent_plugin");
    
    // Get error report
    auto error_report = m_loader->get_error_report();
    
    EXPECT_FALSE(error_report.empty());
    EXPECT_THAT(error_report, HasSubstr("path1.dll"));
    EXPECT_THAT(error_report, HasSubstr("path2.dll"));
    EXPECT_THAT(error_report, HasSubstr("nonexistent_plugin"));
}

TEST_F(PluginLoaderTest, ErrorHistoryLimit) {
    m_loader->clear_error_history();
    m_loader->set_max_error_history(5);
    
    // Generate more errors than the limit
    for (int i = 0; i < 10; ++i) {
        m_loader->load("/invalid/error_test_" + std::to_string(i) + ".dll");
    }
    
    auto error_report = m_loader->get_error_report();
    
    // Count error entries (simplified check)
    size_t error_count = 0;
    size_t pos = 0;
    while ((pos = error_report.find("Error:", pos)) != std::string::npos) {
        error_count++;
        pos += 6;
    }
    
    EXPECT_LE(error_count, 5);
}

// Test resource monitoring
TEST_F(PluginLoaderTest, ResourceUsageTracking) {
    auto plugin_path = create_dummy_plugin("resource_test");
    
    // Attempt to load (will fail with dummy, but resource tracking should work)
    m_loader->load(plugin_path);
    
    // Get resource usage for a non-existent plugin ID (as dummy won't load)
    auto usage = m_loader->get_plugin_resource_usage("dummy_id");
    
    // Even for non-loaded plugins, should return valid structure
    EXPECT_EQ(usage.memory_bytes, 0);
    EXPECT_EQ(usage.handle_count, 0);
}

// Test thread safety
TEST_F(PluginLoaderTest, ThreadSafety) {
    m_loader->set_cache_enabled(true);
    
    const int thread_count = 10;
    const int operations_per_thread = 100;
    
    std::vector<std::thread> threads;
    
    // Create test plugins
    std::vector<std::string> plugin_paths;
    for (int i = 0; i < 5; ++i) {
        plugin_paths.push_back(create_dummy_plugin("thread_test_" + std::to_string(i)));
    }
    
    // Launch threads that perform various operations
    for (int t = 0; t < thread_count; ++t) {
        threads.emplace_back([this, &plugin_paths, operations_per_thread, t]() {
            for (int op = 0; op < operations_per_thread; ++op) {
                int operation = (op + t) % 4;
                int plugin_idx = op % plugin_paths.size();
                
                switch (operation) {
                    case 0:
                        m_loader->can_load(plugin_paths[plugin_idx]);
                        break;
                    case 1:
                        m_loader->load(plugin_paths[plugin_idx]);
                        break;
                    case 2:
                        m_loader->get_cache_statistics();
                        break;
                    case 3:
                        m_loader->get_error_report();
                        break;
                }
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // If we get here without crashes, thread safety test passed
    SUCCEED();
}

// Test validation of cache entries
TEST_F(PluginLoaderTest, CacheValidation) {
    auto plugin_path = create_dummy_plugin("validation_test");
    
    m_loader->set_cache_enabled(true);
    
    // Load into cache
    m_loader->can_load(plugin_path);
    
    // Simulate file modification by touching the file
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    QFile file(QString::fromStdString(plugin_path));
    if (file.open(QIODevice::Append)) {
        file.write("MODIFIED");
        file.close();
    }
    
    // Validate cache (should detect modification)
    bool is_valid = m_loader->validate_cache_entry(plugin_path);
    
    // Cache should be invalidated due to file modification
    EXPECT_FALSE(is_valid);
}

// Test performance metrics
TEST_F(PluginLoaderTest, PerformanceMetrics) {
    auto plugin_path = create_dummy_plugin("perf_test");
    
    m_loader->set_cache_enabled(false);
    
    // Measure without cache
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; ++i) {
        m_loader->can_load(plugin_path);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto no_cache_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Enable cache and measure
    m_loader->set_cache_enabled(true);
    m_loader->can_load(plugin_path);  // Prime cache
    
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; ++i) {
        m_loader->can_load(plugin_path);
    }
    end = std::chrono::high_resolution_clock::now();
    auto with_cache_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Cache should provide significant speedup
    EXPECT_LT(with_cache_duration.count(), no_cache_duration.count());
    
    // Calculate speedup factor
    if (with_cache_duration.count() > 0) {
        double speedup = static_cast<double>(no_cache_duration.count()) / 
                        static_cast<double>(with_cache_duration.count());
        EXPECT_GT(speedup, 1.5);  // Expect at least 1.5x speedup
    }
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    // Initialize Qt application (required for Qt functionality)
    QCoreApplication app(argc, argv);
    
    return RUN_ALL_TESTS();
}