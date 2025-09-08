/**
 * @file test_remote_plugin_integration.cpp
 * @brief Integration tests for the complete remote plugin system
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QCoreApplication>
#include <QEventLoop>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTimer>
#include <qtplugin/remote/remote_plugin_manager_extension.hpp>
#include <qtplugin/remote/remote_plugin_registry_extension.hpp>
#include <qtplugin/remote/remote_plugin_discovery.hpp>
#include <qtplugin/core/plugin_manager.hpp>
#include <memory>
#include <filesystem>
#include <fstream>

using namespace qtplugin;
using namespace testing;

class RemotePluginIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directory for testing
        temp_dir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(temp_dir->isValid());
        
        // Create base plugin manager
        base_manager = std::make_shared<PluginManager>();
        
        // Create enhanced plugin manager with remote support
        remote_manager = RemotePluginManagerFactory::create_with_remote_support();
        
        // Create enhanced plugin registry
        remote_registry = std::make_unique<RemotePluginRegistry>();
        
        // Create discovery manager
        discovery_manager = std::make_unique<RemotePluginDiscoveryManager>();
        
        // Set up test data
        setupTestData();
    }

    void TearDown() override {
        discovery_manager.reset();
        remote_registry.reset();
        remote_manager.reset();
        base_manager.reset();
        temp_dir.reset();
    }

    void setupTestData() {
        // Create test plugin sources
        http_source = RemotePluginSource(
            QUrl("https://plugins.example.com/test-plugin.zip"),
            RemoteSourceType::Http,
            "Test HTTP Source"
        );
        
        registry_source = RemotePluginSource(
            QUrl("https://registry.example.com/api/v1"),
            RemoteSourceType::Registry,
            "Test Registry"
        );
        
        // Create test plugin file
        createTestPluginFile();
    }

    void createTestPluginFile() {
        test_plugin_path = std::filesystem::path(temp_dir->path().toStdString()) / "test_plugin.zip";
        
        // Create a simple test file (in real scenario, this would be a valid plugin)
        std::ofstream test_file(test_plugin_path);
        test_file << "Test plugin content - this would be a real plugin binary";
        test_file.close();
        
        ASSERT_TRUE(std::filesystem::exists(test_plugin_path));
    }

    std::unique_ptr<QTemporaryDir> temp_dir;
    std::shared_ptr<PluginManager> base_manager;
    std::unique_ptr<RemotePluginManagerExtension> remote_manager;
    std::unique_ptr<RemotePluginRegistry> remote_registry;
    std::unique_ptr<RemotePluginDiscoveryManager> discovery_manager;
    
    RemotePluginSource http_source;
    RemotePluginSource registry_source;
    std::filesystem::path test_plugin_path;
};

// === Basic Integration Tests ===

TEST_F(RemotePluginIntegrationTest, ManagerExtensionIntegration) {
    EXPECT_NE(remote_manager.get(), nullptr);
    EXPECT_NE(remote_manager->base_manager().get(), nullptr);
    EXPECT_TRUE(remote_manager->is_remote_plugins_enabled());
    
    // Test URL detection
    std::string http_url = "https://example.com/plugin.zip";
    std::string local_path = "/local/path/plugin.zip";
    
    // These would normally load plugins, but we're testing the URL detection logic
    EXPECT_NO_THROW(remote_manager->load_plugin(http_url, RemotePluginLoadOptions{}));
    EXPECT_NO_THROW(remote_manager->load_plugin(local_path, RemotePluginLoadOptions{}));
}

TEST_F(RemotePluginIntegrationTest, RegistryExtensionIntegration) {
    EXPECT_NE(remote_registry.get(), nullptr);
    
    // Test remote source management
    auto add_result = remote_registry->add_remote_source(http_source);
    EXPECT_TRUE(add_result.has_value());
    
    auto sources = remote_registry->get_remote_sources();
    EXPECT_EQ(sources.size(), 1);
    EXPECT_EQ(sources[0].url(), http_source.url());
    
    auto remove_result = remote_registry->remove_remote_source(http_source.id().toStdString());
    EXPECT_TRUE(remove_result.has_value());
    
    auto sources_after_remove = remote_registry->get_remote_sources();
    EXPECT_EQ(sources_after_remove.size(), 0);
}

TEST_F(RemotePluginIntegrationTest, DiscoveryManagerIntegration) {
    EXPECT_NE(discovery_manager.get(), nullptr);
    
    // Register HTTP discovery engine
    auto http_engine = std::make_shared<HttpDiscoveryEngine>();
    discovery_manager->register_engine(http_engine);
    
    auto engines = discovery_manager->get_registered_engines();
    EXPECT_EQ(engines.size(), 1);
    EXPECT_EQ(engines[0], http_engine->engine_name());
    
    // Test discovery with no real sources (should handle gracefully)
    std::vector<RemotePluginSource> sources = {http_source};
    auto discovery_result = discovery_manager->discover_plugins(sources);
    
    // This will likely fail due to no real server, but should not crash
    if (!discovery_result.has_value()) {
        EXPECT_TRUE(discovery_result.error().code == PluginErrorCode::NetworkError ||
                   discovery_result.error().code == PluginErrorCode::NotSupported);
    }
}

// === End-to-End Workflow Tests ===

TEST_F(RemotePluginIntegrationTest, CompleteRemotePluginWorkflow) {
    // 1. Add remote source to registry
    auto add_source_result = remote_registry->add_remote_source(http_source);
    EXPECT_TRUE(add_source_result.has_value());
    
    // 2. Discover plugins from sources
    RemotePluginSearchCriteria criteria;
    criteria.query = "test";
    criteria.max_results = 10;
    
    auto discovery_result = remote_registry->discover_remote_plugins(criteria);
    // This will likely fail due to no real server, but should handle gracefully
    
    // 3. Attempt to load a plugin (would normally succeed with real server)
    RemotePluginLoadOptions load_options;
    load_options.security_level = RemoteSecurityLevel::Minimal;
    load_options.validate_source = false;
    load_options.validate_plugin = false;
    
    auto load_result = remote_manager->load_remote_plugin(http_source.url(), load_options);
    // This will fail due to no real server, but should not crash
    EXPECT_FALSE(load_result.has_value());
    
    // 4. Check statistics
    auto stats = remote_registry->get_cache_statistics();
    EXPECT_TRUE(stats.contains("total_remote_plugins"));
    EXPECT_TRUE(stats.contains("cached_plugins"));
}

TEST_F(RemotePluginIntegrationTest, LocalFileLoadingWorkflow) {
    // Test that local file loading still works through the enhanced manager
    std::string local_path = test_plugin_path.string();
    
    RemotePluginLoadOptions options;
    auto load_result = remote_manager->load_plugin(local_path, options);
    
    // This might fail due to the test file not being a valid plugin,
    // but it should attempt to load it as a local file
    if (!load_result.has_value()) {
        // Should fail with plugin-related error, not URL-related error
        EXPECT_TRUE(load_result.error().code == PluginErrorCode::InvalidFormat ||
                   load_result.error().code == PluginErrorCode::LoadFailed);
    }
}

// === Asynchronous Integration Tests ===

TEST_F(RemotePluginIntegrationTest, AsyncDiscoveryWorkflow) {
    // Register discovery engine
    auto http_engine = std::make_shared<HttpDiscoveryEngine>();
    discovery_manager->register_engine(http_engine);
    
    bool progress_received = false;
    bool completion_received = false;
    
    auto progress_callback = [&progress_received](const DiscoveryProgress& progress) {
        progress_received = true;
        EXPECT_GE(progress.progress_percentage, 0.0);
        EXPECT_LE(progress.progress_percentage, 100.0);
    };
    
    auto completion_callback = [&completion_received](const qtplugin::expected<DiscoveryResult, PluginError>& result) {
        completion_received = true;
        // Result might be success or failure
    };
    
    std::vector<RemotePluginSource> sources = {http_source};
    QString operation_id = discovery_manager->discover_plugins_async(
        sources, PluginDiscoveryFilter{}, progress_callback, completion_callback);
    
    EXPECT_FALSE(operation_id.isEmpty());
    
    // Wait for async operation to start
    QEventLoop loop;
    QTimer::singleShot(200, &loop, &QEventLoop::quit);
    loop.exec();
    
    // Cancel operation to clean up
    auto cancel_result = discovery_manager->cancel_discovery(operation_id);
    // Cancel might succeed or fail depending on timing
}

TEST_F(RemotePluginIntegrationTest, AsyncLoadingWorkflow) {
    bool progress_received = false;
    bool completion_received = false;
    
    auto progress_callback = [&progress_received](const DownloadProgress& progress) {
        progress_received = true;
        EXPECT_GE(progress.percentage, 0.0);
        EXPECT_LE(progress.percentage, 100.0);
    };
    
    auto completion_callback = [&completion_received](const qtplugin::expected<std::string, PluginError>& result) {
        completion_received = true;
        // Result will likely be an error due to no real server
    };
    
    RemotePluginLoadOptions options;
    options.security_level = RemoteSecurityLevel::Minimal;
    
    QString operation_id = remote_manager->load_remote_plugin_async(
        http_source.url(), options, progress_callback, completion_callback);
    
    EXPECT_FALSE(operation_id.isEmpty());
    
    // Wait for async operation to start
    QEventLoop loop;
    QTimer::singleShot(200, &loop, &QEventLoop::quit);
    loop.exec();
    
    // Cancel operation to clean up
    auto cancel_result = remote_manager->cancel_remote_load(operation_id);
    // Cancel might succeed or fail depending on timing
}

// === Configuration Integration Tests ===

TEST_F(RemotePluginIntegrationTest, ConfigurationPropagation) {
    // Test that configuration changes propagate through the system
    auto config = std::make_shared<RemotePluginConfiguration>(
        RemotePluginConfiguration::create_secure());
    
    remote_manager->set_remote_configuration(config);
    
    auto retrieved_config = remote_manager->remote_configuration();
    EXPECT_EQ(retrieved_config, config);
    
    // Test enabling/disabling remote plugins
    EXPECT_TRUE(remote_manager->is_remote_plugins_enabled());
    
    remote_manager->set_remote_plugins_enabled(false);
    EXPECT_FALSE(remote_manager->is_remote_plugins_enabled());
    
    remote_manager->set_remote_plugins_enabled(true);
    EXPECT_TRUE(remote_manager->is_remote_plugins_enabled());
}

// === Error Handling Integration Tests ===

TEST_F(RemotePluginIntegrationTest, ErrorHandlingAcrossComponents) {
    // Test that errors are properly propagated across components
    
    // 1. Invalid URL handling
    QUrl invalid_url("not-a-valid-url");
    auto load_result = remote_manager->load_remote_plugin(invalid_url, RemotePluginLoadOptions{});
    EXPECT_FALSE(load_result.has_value());
    EXPECT_EQ(load_result.error().code, PluginErrorCode::InvalidConfiguration);
    
    // 2. Non-existent source removal
    auto remove_result = remote_registry->remove_remote_source("non-existent-id");
    EXPECT_FALSE(remove_result.has_value());
    EXPECT_EQ(remove_result.error().code, PluginErrorCode::NotFound);
    
    // 3. Discovery with no engines
    RemotePluginDiscoveryManager empty_discovery_manager;
    std::vector<RemotePluginSource> sources = {http_source};
    auto discovery_result = empty_discovery_manager.discover_plugins(sources);
    
    // Should handle gracefully even with no engines
    if (!discovery_result.has_value()) {
        EXPECT_TRUE(discovery_result.error().code == PluginErrorCode::NotSupported ||
                   discovery_result.error().code == PluginErrorCode::InvalidConfiguration);
    }
}

// === Performance and Resource Management Tests ===

TEST_F(RemotePluginIntegrationTest, ResourceManagement) {
    // Test that resources are properly managed across components
    
    // 1. Multiple source additions and removals
    for (int i = 0; i < 10; ++i) {
        RemotePluginSource source(
            QUrl(QString("https://example%1.com/api").arg(i)),
            RemoteSourceType::Registry,
            QString("Test Source %1").arg(i)
        );
        
        auto add_result = remote_registry->add_remote_source(source);
        EXPECT_TRUE(add_result.has_value());
    }
    
    auto sources = remote_registry->get_remote_sources();
    EXPECT_EQ(sources.size(), 10);
    
    // Remove all sources
    for (const auto& source : sources) {
        auto remove_result = remote_registry->remove_remote_source(source.id().toStdString());
        EXPECT_TRUE(remove_result.has_value());
    }
    
    auto sources_after_cleanup = remote_registry->get_remote_sources();
    EXPECT_EQ(sources_after_cleanup.size(), 0);
    
    // 2. Cache management
    int cleared = remote_registry->clear_remote_cache(0); // Clear all
    EXPECT_GE(cleared, 0);
    
    auto cache_stats = remote_registry->get_cache_statistics();
    EXPECT_TRUE(cache_stats.contains("total_remote_plugins"));
    EXPECT_TRUE(cache_stats.contains("cached_plugins"));
}

// === Factory Integration Tests ===

TEST_F(RemotePluginIntegrationTest, FactoryIntegration) {
    // Test different factory configurations
    auto default_manager = RemotePluginManagerFactory::create_with_remote_support();
    EXPECT_NE(default_manager.get(), nullptr);
    EXPECT_TRUE(default_manager->is_remote_plugins_enabled());
    
    auto enterprise_manager = RemotePluginManagerFactory::create_enterprise();
    EXPECT_NE(enterprise_manager.get(), nullptr);
    EXPECT_TRUE(enterprise_manager->is_remote_plugins_enabled());
    
    // Test enhancing existing manager
    auto existing_manager = std::make_shared<PluginManager>();
    auto enhanced_manager = RemotePluginManagerFactory::enhance_existing_manager(existing_manager);
    EXPECT_NE(enhanced_manager.get(), nullptr);
    EXPECT_EQ(enhanced_manager->base_manager(), existing_manager);
}
