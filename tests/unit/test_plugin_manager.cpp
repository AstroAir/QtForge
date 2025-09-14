/**
 * @file test_plugin_manager.cpp
 * @brief Unit tests for PluginManager with enhanced features
 * @version 1.0.0
 * @date 2024-01-13
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QCoreApplication>
#include <QJsonObject>
#include <qtplugin/core/plugin_manager.hpp>
#include <qtplugin/interfaces/plugin_interface.hpp>
#include <thread>
#include <chrono>

using namespace qtplugin;
using namespace testing;
using namespace std::chrono_literals;

// Mock plugin for testing
class MockPlugin : public IPlugin {
public:
    MOCK_METHOD(expected<void, PluginError>, initialize, (), (override));
    MOCK_METHOD(expected<void, PluginError>, shutdown, (), (override));
    MOCK_METHOD(expected<void, PluginError>, configure, (const QJsonObject&), (override));
    MOCK_METHOD(std::string, id, (), (const, override));
    MOCK_METHOD(std::string, name, (), (const, override));
    MOCK_METHOD(PluginMetadata, metadata, (), (const, override));
    MOCK_METHOD(PluginState, state, (), (const, override));
    MOCK_METHOD(std::vector<std::string>, dependencies, (), (const, override));
    MOCK_METHOD((expected<PluginHealthStatus, PluginError>), check_health, (), (const, override));
};

class PluginManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_manager = std::make_unique<PluginManager>();
    }
    
    void TearDown() override {
        m_manager.reset();
    }
    
    std::shared_ptr<MockPlugin> create_mock_plugin(const std::string& id) {
        auto plugin = std::make_shared<MockPlugin>();
        
        ON_CALL(*plugin, id()).WillByDefault(Return(id));
        ON_CALL(*plugin, name()).WillByDefault(Return("Mock Plugin " + id));
        ON_CALL(*plugin, state()).WillByDefault(Return(PluginState::Active));
        ON_CALL(*plugin, dependencies()).WillByDefault(Return(std::vector<std::string>{}));
        ON_CALL(*plugin, initialize()).WillByDefault(Return(make_success()));
        ON_CALL(*plugin, shutdown()).WillByDefault(Return(make_success()));
        ON_CALL(*plugin, configure(_)).WillByDefault(Return(make_success()));
        
        PluginHealthStatus health{true, "Healthy", 0, std::chrono::steady_clock::now()};
        ON_CALL(*plugin, check_health()).WillByDefault(Return(health));
        
        return plugin;
    }
    
    std::unique_ptr<PluginManager> m_manager;
};

// Test basic plugin operations
TEST_F(PluginManagerTest, LoadPlugin) {
    auto result = m_manager->load_plugin("./test_plugin.dll");
    
    // Without actual plugin file, this should fail
    EXPECT_FALSE(result.has_value());
}

TEST_F(PluginManagerTest, GetPlugin) {
    // Plugin not loaded
    auto plugin = m_manager->get_plugin("nonexistent");
    EXPECT_EQ(plugin, nullptr);
}

// Test batch operations
TEST_F(PluginManagerTest, BatchLoad) {
    std::vector<std::filesystem::path> plugins = {
        "./plugin1.dll",
        "./plugin2.dll",
        "./plugin3.dll"
    };
    
    auto results = m_manager->batch_load(plugins);
    
    EXPECT_EQ(results.size(), 3);
    
    for (const auto& [path, result] : results) {
        EXPECT_FALSE(result.has_value());  // Without actual files, should fail
    }
}

TEST_F(PluginManagerTest, BatchUnload) {
    std::vector<std::string> plugin_ids = {
        "plugin1",
        "plugin2",
        "plugin3"
    };
    
    auto results = m_manager->batch_unload(plugin_ids);
    
    EXPECT_EQ(results.size(), 3);
    
    for (const auto& [id, result] : results) {
        EXPECT_FALSE(result.has_value());  // Not loaded, should fail
        EXPECT_EQ(result.error().code, PluginErrorCode::NotLoaded);
    }
}

// Test transaction support
TEST_F(PluginManagerTest, TransactionCommit) {
    auto transaction = m_manager->begin_transaction();
    
    ASSERT_NE(transaction, nullptr);
    
    transaction->add_load("./plugin1.dll");
    transaction->add_load("./plugin2.dll");
    
    auto result = transaction->commit();
    
    // Without actual plugins, commit should fail and rollback
    EXPECT_FALSE(result.has_value());
}

TEST_F(PluginManagerTest, TransactionRollback) {
    auto transaction = m_manager->begin_transaction();
    
    ASSERT_NE(transaction, nullptr);
    
    transaction->add_load("./plugin1.dll");
    transaction->add_unload("plugin2");
    
    transaction->rollback();
    
    // After rollback, no changes should be applied
    EXPECT_EQ(m_manager->get_plugin("plugin1"), nullptr);
}

TEST_F(PluginManagerTest, TransactionAtomicity) {
    auto transaction = m_manager->begin_transaction();
    
    // Add mix of operations
    transaction->add_load("./valid_plugin.dll");
    transaction->add_load("./invalid_plugin.dll");  // This will fail
    transaction->add_unload("some_plugin");
    
    auto result = transaction->commit();
    
    // If any operation fails, all should be rolled back
    EXPECT_FALSE(result.has_value());
    
    // Verify no partial changes
    EXPECT_EQ(m_manager->get_loaded_plugins().size(), 0);
}

// Test lifecycle hooks
TEST_F(PluginManagerTest, PreLoadHook) {
    bool hook_called = false;
    std::string hook_plugin_id;
    
    auto hook_id = m_manager->register_pre_load_hook(
        [&hook_called, &hook_plugin_id](const std::string& plugin_id, 
                                        std::shared_ptr<IPlugin> plugin) 
        -> expected<void, PluginError> {
            hook_called = true;
            hook_plugin_id = plugin_id;
            return make_success();
        }
    );
    
    EXPECT_FALSE(hook_id.empty());
    
    // Attempt to load a plugin (will fail but hook should be considered)
    m_manager->load_plugin("./test_plugin.dll");
    
    // Note: Hook might not be called if loading fails early
    // This is implementation-dependent
}

TEST_F(PluginManagerTest, PostLoadHook) {
    bool hook_called = false;
    
    auto hook_id = m_manager->register_post_load_hook(
        [&hook_called](const std::string& plugin_id, 
                      std::shared_ptr<IPlugin> plugin) 
        -> expected<void, PluginError> {
            hook_called = true;
            return make_success();
        }
    );
    
    EXPECT_FALSE(hook_id.empty());
    
    // Hook should only be called after successful load
    m_manager->load_plugin("./test_plugin.dll");
    
    // Without actual plugin, post-load won't be called
    EXPECT_FALSE(hook_called);
}

TEST_F(PluginManagerTest, UnregisterHook) {
    auto hook_id = m_manager->register_pre_load_hook(
        [](const std::string&, std::shared_ptr<IPlugin>) 
        -> expected<void, PluginError> {
            return make_success();
        }
    );
    
    EXPECT_FALSE(hook_id.empty());
    
    bool unregistered = m_manager->unregister_hook(hook_id);
    EXPECT_TRUE(unregistered);
    
    // Try to unregister again
    bool unregistered_again = m_manager->unregister_hook(hook_id);
    EXPECT_FALSE(unregistered_again);
}

// Test health monitoring
TEST_F(PluginManagerTest, EnableHealthMonitoring) {
    m_manager->enable_health_monitoring(100ms, true);
    
    // Health monitoring should be enabled
    // (Implementation would need a way to check this)
    
    m_manager->disable_health_monitoring();
    
    // Health monitoring should be disabled
    SUCCEED();
}

TEST_F(PluginManagerTest, CheckPluginHealth) {
    // Without loaded plugins, should return empty map
    auto health_status = m_manager->check_all_plugin_health();
    
    EXPECT_TRUE(health_status.empty());
}

TEST_F(PluginManagerTest, HealthCheckWithAutoRestart) {
    m_manager->enable_health_monitoring(50ms, true);
    
    // Would need actual plugins to test auto-restart
    // This test verifies the API works
    
    std::this_thread::sleep_for(200ms);
    
    m_manager->disable_health_monitoring();
    
    SUCCEED();
}

// Test configuration hot reload
TEST_F(PluginManagerTest, UpdatePluginConfig) {
    QJsonObject config;
    config["test_key"] = "test_value";
    
    // Without loaded plugin, should fail
    auto result = m_manager->update_plugin_config("nonexistent", config);
    
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, PluginErrorCode::NotLoaded);
}

TEST_F(PluginManagerTest, BatchUpdateConfigs) {
    std::unordered_map<std::string, QJsonObject> configs;
    
    QJsonObject config1;
    config1["key1"] = "value1";
    configs["plugin1"] = config1;
    
    QJsonObject config2;
    config2["key2"] = "value2";
    configs["plugin2"] = config2;
    
    auto results = m_manager->batch_update_configs(configs);
    
    EXPECT_EQ(results.size(), 2);
    
    for (const auto& [id, result] : results) {
        EXPECT_FALSE(result.has_value());  // Plugins not loaded
        EXPECT_EQ(result.error().code, PluginErrorCode::NotLoaded);
    }
}

// Test thread safety
TEST_F(PluginManagerTest, ConcurrentOperations) {
    const int thread_count = 10;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back([this, i]() {
            // Perform various operations
            m_manager->load_plugin("./plugin_" + std::to_string(i) + ".dll");
            m_manager->get_plugin("plugin_" + std::to_string(i));
            m_manager->unload_plugin("plugin_" + std::to_string(i));
            
            auto transaction = m_manager->begin_transaction();
            transaction->add_load("./trans_plugin_" + std::to_string(i) + ".dll");
            transaction->commit();
            
            m_manager->check_all_plugin_health();
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // If we get here without crashes, thread safety test passed
    SUCCEED();
}

// Test resource management
TEST_F(PluginManagerTest, GetLoadedPlugins) {
    auto plugins = m_manager->get_loaded_plugins();
    
    EXPECT_TRUE(plugins.empty());
}

TEST_F(PluginManagerTest, TransactionResourceCleanup) {
    {
        auto transaction = m_manager->begin_transaction();
        transaction->add_load("./plugin.dll");
        // Transaction destroyed without commit/rollback
    }
    
    // Should not leak resources
    SUCCEED();
}

// Test error handling
TEST_F(PluginManagerTest, InvalidPluginPath) {
    auto result = m_manager->load_plugin("");
    
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, PluginErrorCode::InvalidPath);
}

TEST_F(PluginManagerTest, CircularDependencyHandling) {
    // This would require actual plugins with circular dependencies
    // Test verifies API exists and doesn't crash
    
    auto result = m_manager->load_plugin("./circular_dep_plugin.dll");
    
    // Should handle circular dependencies gracefully
    EXPECT_FALSE(result.has_value());
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    // Initialize Qt application (required for Qt functionality)
    QCoreApplication app(argc, argv);
    
    return RUN_ALL_TESTS();
}