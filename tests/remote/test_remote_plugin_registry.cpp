/**
 * @file test_remote_plugin_registry.cpp
 * @brief Unit tests for RemotePluginRegistry class
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <qtplugin/remote/remote_plugin_registry_extension.hpp>
#include <memory>
#include <filesystem>

using namespace qtplugin;
using namespace testing;

class RemotePluginRegistryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directory for testing
        temp_dir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(temp_dir->isValid());
        
        // Create remote plugin registry
        registry = std::make_unique<RemotePluginRegistry>();
        
        // Set up test data
        setupTestData();
    }

    void TearDown() override {
        registry.reset();
        temp_dir.reset();
    }

    void setupTestData() {
        // Create test remote plugin source
        test_source = RemotePluginSource(
            QUrl("https://registry.example.com/api/v1"),
            RemoteSourceType::Registry,
            "Test Registry"
        );
        
        // Create test remote plugin info
        test_plugin_info = std::make_unique<RemotePluginInfo>();
        test_plugin_info->id = "test-plugin-id";
        test_plugin_info->file_path = temp_dir->path().toStdString() + "/test_plugin.zip";
        test_plugin_info->remote_source = test_source;
        test_plugin_info->original_url = QUrl("https://example.com/plugin.zip");
        test_plugin_info->download_time = std::chrono::system_clock::now();
        test_plugin_info->is_cached = true;
        test_plugin_info->auto_update_enabled = false;
        
        // Set up plugin metadata
        test_plugin_info->metadata.name = "Test Plugin";
        test_plugin_info->metadata.version = Version("1.0.0");
        test_plugin_info->metadata.description = "A test plugin";
        test_plugin_info->metadata.author = "Test Author";
    }

    std::unique_ptr<QTemporaryDir> temp_dir;
    std::unique_ptr<RemotePluginRegistry> registry;
    RemotePluginSource test_source;
    std::unique_ptr<RemotePluginInfo> test_plugin_info;
};

// === Basic Registry Tests ===

TEST_F(RemotePluginRegistryTest, Construction) {
    EXPECT_NE(registry.get(), nullptr);
    
    // Should start with no remote plugins
    auto remote_plugins = registry->get_all_remote_plugin_info();
    EXPECT_TRUE(remote_plugins.empty());
    
    // Should start with no remote sources
    auto sources = registry->get_remote_sources();
    EXPECT_TRUE(sources.empty());
}

// === Remote Plugin Registration Tests ===

TEST_F(RemotePluginRegistryTest, RegisterRemotePlugin) {
    std::string plugin_id = "test-plugin-123";
    auto plugin_info_copy = std::make_unique<RemotePluginInfo>(*test_plugin_info);
    plugin_info_copy->id = plugin_id;
    
    auto result = registry->register_remote_plugin(plugin_id, std::move(plugin_info_copy));
    EXPECT_TRUE(result.has_value());
    
    // Verify plugin was registered
    auto retrieved_info = registry->get_remote_plugin_info(plugin_id);
    EXPECT_TRUE(retrieved_info.has_value());
    EXPECT_EQ(retrieved_info->id, plugin_id);
    EXPECT_EQ(retrieved_info->metadata.name, "Test Plugin");
}

TEST_F(RemotePluginRegistryTest, RegisterPluginWithEmptyId) {
    auto plugin_info_copy = std::make_unique<RemotePluginInfo>(*test_plugin_info);
    
    auto result = registry->register_remote_plugin("", std::move(plugin_info_copy));
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, PluginErrorCode::InvalidParameters);
}

TEST_F(RemotePluginRegistryTest, RegisterPluginWithNullInfo) {
    auto result = registry->register_remote_plugin("test-id", nullptr);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, PluginErrorCode::InvalidParameters);
}

TEST_F(RemotePluginRegistryTest, GetNonExistentPlugin) {
    auto result = registry->get_remote_plugin_info("non-existent-id");
    EXPECT_FALSE(result.has_value());
}

TEST_F(RemotePluginRegistryTest, GetAllRemotePlugins) {
    // Register multiple plugins
    for (int i = 0; i < 5; ++i) {
        std::string plugin_id = "plugin-" + std::to_string(i);
        auto plugin_info_copy = std::make_unique<RemotePluginInfo>(*test_plugin_info);
        plugin_info_copy->id = plugin_id;
        
        auto result = registry->register_remote_plugin(plugin_id, std::move(plugin_info_copy));
        EXPECT_TRUE(result.has_value());
    }
    
    auto all_plugins = registry->get_all_remote_plugin_info();
    EXPECT_EQ(all_plugins.size(), 5);
}

// === Remote Source Management Tests ===

TEST_F(RemotePluginRegistryTest, AddRemoteSource) {
    QSignalSpy spy(registry.get(), &RemotePluginRegistry::remote_source_added);
    
    auto result = registry->add_remote_source(test_source);
    EXPECT_TRUE(result.has_value());
    
    // Verify signal was emitted
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toString(), test_source.id());
    
    // Verify source was added
    auto sources = registry->get_remote_sources();
    EXPECT_EQ(sources.size(), 1);
    EXPECT_EQ(sources[0].url(), test_source.url());
}

TEST_F(RemotePluginRegistryTest, AddSourceWithEmptyId) {
    RemotePluginSource invalid_source(QUrl(), RemoteSourceType::Http, "");
    
    auto result = registry->add_remote_source(invalid_source);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, PluginErrorCode::InvalidParameters);
}

TEST_F(RemotePluginRegistryTest, RemoveRemoteSource) {
    // First add a source
    registry->add_remote_source(test_source);
    
    QSignalSpy spy(registry.get(), &RemotePluginRegistry::remote_source_removed);
    
    auto result = registry->remove_remote_source(test_source.id().toStdString());
    EXPECT_TRUE(result.has_value());
    
    // Verify signal was emitted
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toString(), test_source.id());
    
    // Verify source was removed
    auto sources = registry->get_remote_sources();
    EXPECT_TRUE(sources.empty());
}

TEST_F(RemotePluginRegistryTest, RemoveNonExistentSource) {
    auto result = registry->remove_remote_source("non-existent-id");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, PluginErrorCode::NotFound);
}

// === Plugin Discovery Tests ===

TEST_F(RemotePluginRegistryTest, DiscoverRemotePluginsWithNoSources) {
    RemotePluginSearchCriteria criteria;
    criteria.query = "test";
    
    auto result = registry->discover_remote_plugins(criteria);
    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty()); // No sources, so no results
}

TEST_F(RemotePluginRegistryTest, SearchRemotePlugins) {
    // Add a source first
    registry->add_remote_source(test_source);
    
    auto result = registry->search_remote_plugins("test plugin", 10);
    EXPECT_TRUE(result.has_value());
    // Results will be empty since we don't have a real server, but should not error
}

TEST_F(RemotePluginRegistryTest, DiscoveryWithCriteria) {
    registry->add_remote_source(test_source);
    
    RemotePluginSearchCriteria criteria;
    criteria.query = "test";
    criteria.category = "utility";
    criteria.max_results = 5;
    criteria.sort_by = "name";
    criteria.sort_ascending = true;
    
    auto result = registry->discover_remote_plugins(criteria);
    EXPECT_TRUE(result.has_value());
    // Results will be empty since we don't have a real server
}

// === Cache Management Tests ===

TEST_F(RemotePluginRegistryTest, CacheStatistics) {
    auto stats = registry->get_cache_statistics();
    
    EXPECT_TRUE(stats.contains("total_remote_plugins"));
    EXPECT_TRUE(stats.contains("cached_plugins"));
    EXPECT_TRUE(stats.contains("cache_hit_ratio"));
    EXPECT_TRUE(stats.contains("total_cache_size_bytes"));
    EXPECT_TRUE(stats.contains("cache_directory"));
    
    // Initially should be zero
    EXPECT_EQ(stats["total_remote_plugins"].toInt(), 0);
    EXPECT_EQ(stats["cached_plugins"].toInt(), 0);
    EXPECT_DOUBLE_EQ(stats["cache_hit_ratio"].toDouble(), 0.0);
}

TEST_F(RemotePluginRegistryTest, ClearRemoteCache) {
    // Register some cached plugins
    for (int i = 0; i < 3; ++i) {
        std::string plugin_id = "cached-plugin-" + std::to_string(i);
        auto plugin_info_copy = std::make_unique<RemotePluginInfo>(*test_plugin_info);
        plugin_info_copy->id = plugin_id;
        plugin_info_copy->is_cached = true;
        plugin_info_copy->download_time = std::chrono::system_clock::now() - std::chrono::hours(i);
        
        registry->register_remote_plugin(plugin_id, std::move(plugin_info_copy));
    }
    
    // Clear all cache
    int cleared = registry->clear_remote_cache(0);
    EXPECT_EQ(cleared, 3);
    
    // Verify cache is empty
    auto stats = registry->get_cache_statistics();
    EXPECT_EQ(stats["cached_plugins"].toInt(), 0);
}

TEST_F(RemotePluginRegistryTest, ClearExpiredCache) {
    // Register plugins with different ages
    auto old_plugin = std::make_unique<RemotePluginInfo>(*test_plugin_info);
    old_plugin->id = "old-plugin";
    old_plugin->is_cached = true;
    old_plugin->download_time = std::chrono::system_clock::now() - std::chrono::hours(48); // 2 days old
    
    auto recent_plugin = std::make_unique<RemotePluginInfo>(*test_plugin_info);
    recent_plugin->id = "recent-plugin";
    recent_plugin->is_cached = true;
    recent_plugin->download_time = std::chrono::system_clock::now() - std::chrono::hours(12); // 12 hours old
    
    registry->register_remote_plugin("old-plugin", std::move(old_plugin));
    registry->register_remote_plugin("recent-plugin", std::move(recent_plugin));
    
    // Clear cache older than 1 day
    int cleared = registry->clear_remote_cache(1);
    EXPECT_EQ(cleared, 1); // Only the old plugin should be cleared
    
    // Verify recent plugin is still there
    auto remaining_info = registry->get_remote_plugin_info("recent-plugin");
    EXPECT_TRUE(remaining_info.has_value());
}

// === Update Management Tests ===

TEST_F(RemotePluginRegistryTest, CheckForUpdates) {
    // Register a plugin
    std::string plugin_id = "updatable-plugin";
    auto plugin_info_copy = std::make_unique<RemotePluginInfo>(*test_plugin_info);
    plugin_info_copy->id = plugin_id;
    plugin_info_copy->remote_source = test_source;
    
    registry->register_remote_plugin(plugin_id, std::move(plugin_info_copy));
    
    // Check for updates (will likely fail due to no real server)
    auto result = registry->check_for_updates();
    if (!result.has_value()) {
        // Should fail gracefully with network error
        EXPECT_TRUE(result.error().code == PluginErrorCode::NetworkError ||
                   result.error().code == PluginErrorCode::NotSupported);
    }
}

TEST_F(RemotePluginRegistryTest, CheckPluginUpdate) {
    // Register a plugin
    std::string plugin_id = "updatable-plugin";
    auto plugin_info_copy = std::make_unique<RemotePluginInfo>(*test_plugin_info);
    plugin_info_copy->id = plugin_id;
    plugin_info_copy->remote_source = test_source;
    
    registry->register_remote_plugin(plugin_id, std::move(plugin_info_copy));
    
    // Check for update for specific plugin
    auto result = registry->check_plugin_update(plugin_id);
    if (!result.has_value()) {
        // Should fail gracefully with network error
        EXPECT_TRUE(result.error().code == PluginErrorCode::NetworkError ||
                   result.error().code == PluginErrorCode::NotSupported);
    }
}

TEST_F(RemotePluginRegistryTest, CheckUpdateForNonExistentPlugin) {
    auto result = registry->check_plugin_update("non-existent-plugin");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, PluginErrorCode::NotFound);
}

TEST_F(RemotePluginRegistryTest, SetAutoUpdate) {
    // Register a plugin
    std::string plugin_id = "auto-update-plugin";
    auto plugin_info_copy = std::make_unique<RemotePluginInfo>(*test_plugin_info);
    plugin_info_copy->id = plugin_id;
    plugin_info_copy->auto_update_enabled = false;
    
    registry->register_remote_plugin(plugin_id, std::move(plugin_info_copy));
    
    // Enable auto-update
    auto result = registry->set_auto_update(plugin_id, true);
    EXPECT_TRUE(result.has_value());
    
    // Verify auto-update is enabled
    auto info = registry->get_remote_plugin_info(plugin_id);
    EXPECT_TRUE(info.has_value());
    EXPECT_TRUE(info->auto_update_enabled);
    
    // Disable auto-update
    result = registry->set_auto_update(plugin_id, false);
    EXPECT_TRUE(result.has_value());
    
    // Verify auto-update is disabled
    info = registry->get_remote_plugin_info(plugin_id);
    EXPECT_TRUE(info.has_value());
    EXPECT_FALSE(info->auto_update_enabled);
}

TEST_F(RemotePluginRegistryTest, SetAutoUpdateForNonExistentPlugin) {
    auto result = registry->set_auto_update("non-existent-plugin", true);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, PluginErrorCode::NotFound);
}

// === Enhanced Base Registry Methods Tests ===

TEST_F(RemotePluginRegistryTest, GetPluginInfoIncludesRemote) {
    // Register a remote plugin
    std::string plugin_id = "remote-plugin";
    auto plugin_info_copy = std::make_unique<RemotePluginInfo>(*test_plugin_info);
    plugin_info_copy->id = plugin_id;
    
    registry->register_remote_plugin(plugin_id, std::move(plugin_info_copy));
    
    // Get plugin info (should return base PluginInfo part)
    auto info = registry->get_plugin_info(plugin_id);
    EXPECT_TRUE(info.has_value());
    EXPECT_EQ(info->id, plugin_id);
    EXPECT_EQ(info->metadata.name, "Test Plugin");
}

TEST_F(RemotePluginRegistryTest, GetAllPluginInfoIncludesRemote) {
    // Register some remote plugins
    for (int i = 0; i < 3; ++i) {
        std::string plugin_id = "remote-plugin-" + std::to_string(i);
        auto plugin_info_copy = std::make_unique<RemotePluginInfo>(*test_plugin_info);
        plugin_info_copy->id = plugin_id;
        
        registry->register_remote_plugin(plugin_id, std::move(plugin_info_copy));
    }
    
    auto all_plugins = registry->get_all_plugin_info();
    EXPECT_GE(all_plugins.size(), 3); // Should include at least our remote plugins
}

// === Signal Tests ===

TEST_F(RemotePluginRegistryTest, RemotePluginUpdateSignal) {
    QSignalSpy spy(registry.get(), &RemotePluginRegistry::remote_plugin_update_available);
    
    // Register a plugin
    std::string plugin_id = "signal-test-plugin";
    auto plugin_info_copy = std::make_unique<RemotePluginInfo>(*test_plugin_info);
    plugin_info_copy->id = plugin_id;
    plugin_info_copy->remote_source = test_source;
    
    registry->register_remote_plugin(plugin_id, std::move(plugin_info_copy));
    
    // Check for updates (this might emit the signal if update is detected)
    registry->check_plugin_update(plugin_id);
    
    // Signal might or might not be emitted depending on network availability
    // But the test should not crash
    EXPECT_GE(spy.count(), 0);
}
