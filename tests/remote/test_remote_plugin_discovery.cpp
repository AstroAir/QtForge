/**
 * @file test_remote_plugin_discovery.cpp
 * @brief Unit tests for remote plugin discovery system
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QEventLoop>
#include <QSignalSpy>
#include <QTimer>
#include <qtplugin/remote/remote_plugin_discovery.hpp>
#include <memory>

using namespace qtplugin;
using namespace testing;

class RemotePluginDiscoveryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create discovery manager
        discovery_manager = std::make_unique<RemotePluginDiscoveryManager>();
        
        // Create HTTP discovery engine
        http_engine = std::make_shared<HttpDiscoveryEngine>();
        
        // Set up test data
        setupTestData();
    }

    void TearDown() override {
        http_engine.reset();
        discovery_manager.reset();
    }

    void setupTestData() {
        // Create test sources
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
        
        // Create test discovery filter
        test_filter.query = "test plugin";
        test_filter.category = "utility";
        test_filter.max_results = 10;
        test_filter.sort_by = "name";
        test_filter.sort_ascending = true;
    }

    std::unique_ptr<RemotePluginDiscoveryManager> discovery_manager;
    std::shared_ptr<HttpDiscoveryEngine> http_engine;
    
    RemotePluginSource http_source;
    RemotePluginSource registry_source;
    PluginDiscoveryFilter test_filter;
};

// === Discovery Filter Tests ===

TEST_F(RemotePluginDiscoveryTest, DiscoveryFilterJsonSerialization) {
    test_filter.name_pattern = "test.*";
    test_filter.required_tags = {"utility", "tool"};
    test_filter.excluded_tags = {"deprecated"};
    test_filter.author_pattern = "Test.*Author";
    test_filter.license = "MIT";
    test_filter.min_rating = 4.0;
    test_filter.version_range = ">=1.0.0";
    test_filter.max_size_bytes = 1024 * 1024; // 1MB
    test_filter.verified_only = true;
    test_filter.free_only = false;
    
    QJsonObject json = test_filter.to_json();
    
    EXPECT_EQ(json["query"].toString(), "test plugin");
    EXPECT_EQ(json["category"].toString(), "utility");
    EXPECT_EQ(json["name_pattern"].toString(), "test.*");
    EXPECT_EQ(json["author_pattern"].toString(), "Test.*Author");
    EXPECT_EQ(json["license"].toString(), "MIT");
    EXPECT_DOUBLE_EQ(json["min_rating"].toDouble(), 4.0);
    EXPECT_EQ(json["version_range"].toString(), ">=1.0.0");
    EXPECT_EQ(json["max_size_bytes"].toInt(), 1024 * 1024);
    EXPECT_TRUE(json["verified_only"].toBool());
    EXPECT_FALSE(json["free_only"].toBool());
    
    // Check arrays
    QJsonArray required_tags = json["required_tags"].toArray();
    EXPECT_EQ(required_tags.size(), 2);
    EXPECT_EQ(required_tags[0].toString(), "utility");
    EXPECT_EQ(required_tags[1].toString(), "tool");
    
    QJsonArray excluded_tags = json["excluded_tags"].toArray();
    EXPECT_EQ(excluded_tags.size(), 1);
    EXPECT_EQ(excluded_tags[0].toString(), "deprecated");
}

TEST_F(RemotePluginDiscoveryTest, DiscoveryFilterJsonDeserialization) {
    QJsonObject json;
    json["query"] = "search term";
    json["category"] = "development";
    json["name_pattern"] = "dev.*";
    json["author_pattern"] = "Dev.*Team";
    json["license"] = "Apache-2.0";
    json["min_rating"] = 3.5;
    json["version_range"] = "^2.0.0";
    json["max_size_bytes"] = 2048;
    json["verified_only"] = false;
    json["free_only"] = true;
    json["max_results"] = 25;
    json["offset"] = 10;
    json["sort_by"] = "rating";
    json["sort_ascending"] = false;
    
    QJsonArray required_tags;
    required_tags.append("development");
    required_tags.append("framework");
    json["required_tags"] = required_tags;
    
    QJsonArray excluded_tags;
    excluded_tags.append("beta");
    json["excluded_tags"] = excluded_tags;
    
    PluginDiscoveryFilter filter = PluginDiscoveryFilter::from_json(json);
    
    EXPECT_EQ(filter.query.value(), "search term");
    EXPECT_EQ(filter.category.value(), "development");
    EXPECT_EQ(filter.name_pattern.value(), "dev.*");
    EXPECT_EQ(filter.author_pattern.value(), "Dev.*Team");
    EXPECT_EQ(filter.license.value(), "Apache-2.0");
    EXPECT_DOUBLE_EQ(filter.min_rating.value(), 3.5);
    EXPECT_EQ(filter.version_range.value(), "^2.0.0");
    EXPECT_EQ(filter.max_size_bytes.value(), 2048);
    EXPECT_FALSE(filter.verified_only);
    EXPECT_TRUE(filter.free_only);
    EXPECT_EQ(filter.max_results, 25);
    EXPECT_EQ(filter.offset, 10);
    EXPECT_EQ(filter.sort_by, "rating");
    EXPECT_FALSE(filter.sort_ascending);
    
    EXPECT_EQ(filter.required_tags.size(), 2);
    EXPECT_EQ(filter.required_tags[0], "development");
    EXPECT_EQ(filter.required_tags[1], "framework");
    
    EXPECT_EQ(filter.excluded_tags.size(), 1);
    EXPECT_EQ(filter.excluded_tags[0], "beta");
}

TEST_F(RemotePluginDiscoveryTest, DiscoveryFilterMatching) {
    // Create a test discovery result
    RemotePluginDiscoveryResult result;
    result.name = "Test Development Plugin";
    result.category = "development";
    result.author = "Test Author";
    result.tags = {"development", "utility", "tool"};
    result.rating = 4.5;
    result.file_size = 512 * 1024; // 512KB
    result.metadata["verified"] = true;
    result.metadata["free"] = true;
    
    // Test matching filter
    PluginDiscoveryFilter matching_filter;
    matching_filter.name_pattern = "Test.*Plugin";
    matching_filter.category = "development";
    matching_filter.required_tags = {"development", "utility"};
    matching_filter.min_rating = 4.0;
    matching_filter.max_size_bytes = 1024 * 1024; // 1MB
    matching_filter.verified_only = true;
    matching_filter.free_only = true;
    
    EXPECT_TRUE(matching_filter.matches(result));
    
    // Test non-matching filter
    PluginDiscoveryFilter non_matching_filter;
    non_matching_filter.category = "graphics"; // Different category
    
    EXPECT_FALSE(non_matching_filter.matches(result));
    
    // Test excluded tags
    PluginDiscoveryFilter excluding_filter;
    excluding_filter.excluded_tags = {"utility"}; // Result has utility tag
    
    EXPECT_FALSE(excluding_filter.matches(result));
}

// === Discovery Progress Tests ===

TEST_F(RemotePluginDiscoveryTest, DiscoveryProgressSerialization) {
    DiscoveryProgress progress;
    progress.sources_total = 5;
    progress.sources_completed = 3;
    progress.plugins_found = 15;
    progress.current_source = "Test Registry";
    progress.status_message = "Searching plugins...";
    progress.progress_percentage = 60.0;
    
    QJsonObject json = progress.to_json();
    
    EXPECT_EQ(json["sources_total"].toInt(), 5);
    EXPECT_EQ(json["sources_completed"].toInt(), 3);
    EXPECT_EQ(json["plugins_found"].toInt(), 15);
    EXPECT_EQ(json["current_source"].toString(), "Test Registry");
    EXPECT_EQ(json["status_message"].toString(), "Searching plugins...");
    EXPECT_DOUBLE_EQ(json["progress_percentage"].toDouble(), 60.0);
}

// === Discovery Result Tests ===

TEST_F(RemotePluginDiscoveryTest, DiscoveryResultSerialization) {
    DiscoveryResult result;
    result.total_sources_queried = 3;
    result.failed_sources = {"Failed Source 1", "Failed Source 2"};
    result.error_messages = {"Network error", "Timeout"};
    result.total_time = std::chrono::milliseconds(5000);
    
    // Add some mock plugins
    RemotePluginDiscoveryResult plugin1;
    plugin1.plugin_id = "plugin1";
    plugin1.name = "Plugin 1";
    plugin1.source = http_source;
    result.plugins.push_back(plugin1);
    
    RemotePluginDiscoveryResult plugin2;
    plugin2.plugin_id = "plugin2";
    plugin2.name = "Plugin 2";
    plugin2.source = registry_source;
    result.plugins.push_back(plugin2);
    
    QJsonObject json = result.to_json();
    
    EXPECT_EQ(json["total_sources_queried"].toInt(), 3);
    EXPECT_EQ(json["total_time_ms"].toInt(), 5000);
    EXPECT_DOUBLE_EQ(json["success_rate"].toDouble(), result.success_rate());
    
    QJsonArray plugins = json["plugins"].toArray();
    EXPECT_EQ(plugins.size(), 2);
    
    QJsonArray failed_sources = json["failed_sources"].toArray();
    EXPECT_EQ(failed_sources.size(), 2);
    EXPECT_EQ(failed_sources[0].toString(), "Failed Source 1");
    
    QJsonArray errors = json["error_messages"].toArray();
    EXPECT_EQ(errors.size(), 2);
    EXPECT_EQ(errors[0].toString(), "Network error");
    
    // Test success methods
    EXPECT_TRUE(result.is_successful()); // Has plugins
    EXPECT_DOUBLE_EQ(result.success_rate(), 1.0/3.0); // 1 successful out of 3 total
}

// === HTTP Discovery Engine Tests ===

TEST_F(RemotePluginDiscoveryTest, HttpDiscoveryEngineBasics) {
    EXPECT_EQ(http_engine->engine_name(), "HTTP Discovery Engine");
    
    auto supported_types = http_engine->supported_source_types();
    EXPECT_TRUE(std::find(supported_types.begin(), supported_types.end(), "http") != supported_types.end());
    EXPECT_TRUE(std::find(supported_types.begin(), supported_types.end(), "https") != supported_types.end());
    EXPECT_TRUE(std::find(supported_types.begin(), supported_types.end(), "registry") != supported_types.end());
    
    EXPECT_TRUE(http_engine->supports_source(http_source));
    EXPECT_TRUE(http_engine->supports_source(registry_source));
    
    RemotePluginSource git_source(QUrl("git://github.com/user/repo"), RemoteSourceType::Git);
    EXPECT_FALSE(http_engine->supports_source(git_source));
}

TEST_F(RemotePluginDiscoveryTest, HttpDiscoveryEngineConfiguration) {
    http_engine->set_timeout(std::chrono::seconds(60));
    http_engine->set_max_concurrent_requests(10);
    http_engine->set_user_agent("TestAgent/1.0");
    
    // Configuration methods should not throw
    EXPECT_NO_THROW(http_engine->set_timeout(std::chrono::seconds(30)));
    EXPECT_NO_THROW(http_engine->set_max_concurrent_requests(5));
    EXPECT_NO_THROW(http_engine->set_user_agent("TestAgent/2.0"));
}

TEST_F(RemotePluginDiscoveryTest, HttpDiscoveryEngineDiscovery) {
    // Attempt discovery (will likely fail due to no real server)
    auto result = http_engine->discover_from_source(http_source, test_filter);
    
    if (!result.has_value()) {
        // Should fail gracefully with appropriate error
        EXPECT_TRUE(result.error().code == PluginErrorCode::NetworkError ||
                   result.error().code == PluginErrorCode::NotSupported);
    } else {
        // If it succeeds (unlikely), should return valid results
        EXPECT_GE(result->size(), 0);
    }
}

TEST_F(RemotePluginDiscoveryTest, HttpDiscoveryEngineAsyncDiscovery) {
    bool progress_called = false;
    bool completion_called = false;
    
    auto progress_callback = [&progress_called](const DiscoveryProgress& progress) {
        progress_called = true;
        EXPECT_GE(progress.progress_percentage, 0.0);
        EXPECT_LE(progress.progress_percentage, 100.0);
    };
    
    auto completion_callback = [&completion_called](const qtplugin::expected<DiscoveryResult, PluginError>& result) {
        completion_called = true;
        // Result might be success or failure
    };
    
    QString operation_id = http_engine->discover_from_source_async(
        http_source, test_filter, progress_callback, completion_callback);
    
    EXPECT_FALSE(operation_id.isEmpty());
    
    // Wait a bit for async operation
    QEventLoop loop;
    QTimer::singleShot(100, &loop, &QEventLoop::quit);
    loop.exec();
    
    // The operation might still be running or completed
    // Either way, it should not crash
}

// === Discovery Manager Tests ===

TEST_F(RemotePluginDiscoveryTest, DiscoveryManagerEngineRegistration) {
    auto initial_engines = discovery_manager->get_registered_engines();
    size_t initial_count = initial_engines.size();
    
    discovery_manager->register_engine(http_engine);
    
    auto engines_after_add = discovery_manager->get_registered_engines();
    EXPECT_EQ(engines_after_add.size(), initial_count + 1);
    EXPECT_TRUE(std::find(engines_after_add.begin(), engines_after_add.end(), 
                         http_engine->engine_name()) != engines_after_add.end());
    
    discovery_manager->unregister_engine(http_engine->engine_name());
    
    auto engines_after_remove = discovery_manager->get_registered_engines();
    EXPECT_EQ(engines_after_remove.size(), initial_count);
}

TEST_F(RemotePluginDiscoveryTest, DiscoveryManagerDiscovery) {
    discovery_manager->register_engine(http_engine);
    
    std::vector<RemotePluginSource> sources = {http_source, registry_source};
    
    auto result = discovery_manager->discover_plugins(sources, test_filter);
    
    if (!result.has_value()) {
        // Should fail gracefully with network error
        EXPECT_TRUE(result.error().code == PluginErrorCode::NetworkError ||
                   result.error().code == PluginErrorCode::NotSupported);
    } else {
        // If successful, should have valid structure
        EXPECT_GE(result->plugins.size(), 0);
        EXPECT_EQ(result->total_sources_queried, sources.size());
    }
}

TEST_F(RemotePluginDiscoveryTest, DiscoveryManagerAsyncDiscovery) {
    discovery_manager->register_engine(http_engine);
    
    QSignalSpy progress_spy(discovery_manager.get(), &RemotePluginDiscoveryManager::discovery_progress);
    QSignalSpy completion_spy(discovery_manager.get(), &RemotePluginDiscoveryManager::discovery_completed);
    
    bool progress_called = false;
    bool completion_called = false;
    
    auto progress_callback = [&progress_called](const DiscoveryProgress& progress) {
        progress_called = true;
        EXPECT_GE(progress.progress_percentage, 0.0);
        EXPECT_LE(progress.progress_percentage, 100.0);
    };
    
    auto completion_callback = [&completion_called](const qtplugin::expected<DiscoveryResult, PluginError>& result) {
        completion_called = true;
        // Result might be success or failure
    };
    
    std::vector<RemotePluginSource> sources = {http_source};
    QString operation_id = discovery_manager->discover_plugins_async(
        sources, test_filter, progress_callback, completion_callback);
    
    EXPECT_FALSE(operation_id.isEmpty());
    
    auto active_operations = discovery_manager->get_active_operations();
    EXPECT_TRUE(std::find(active_operations.begin(), active_operations.end(), 
                         operation_id) != active_operations.end());
    
    // Wait for async operation
    QEventLoop loop;
    QTimer::singleShot(200, &loop, &QEventLoop::quit);
    loop.exec();
    
    // Cancel operation to clean up
    auto cancel_result = discovery_manager->cancel_discovery(operation_id);
    // Cancel might succeed or fail depending on timing
}

TEST_F(RemotePluginDiscoveryTest, DiscoveryManagerWithNoEngines) {
    // Test discovery with no registered engines
    std::vector<RemotePluginSource> sources = {http_source};
    
    auto result = discovery_manager->discover_plugins(sources, test_filter);
    
    if (!result.has_value()) {
        EXPECT_TRUE(result.error().code == PluginErrorCode::NotSupported ||
                   result.error().code == PluginErrorCode::InvalidConfiguration);
    }
}

TEST_F(RemotePluginDiscoveryTest, DiscoveryManagerCancellation) {
    discovery_manager->register_engine(http_engine);
    
    std::vector<RemotePluginSource> sources = {http_source, registry_source};
    QString operation_id = discovery_manager->discover_plugins_async(sources, test_filter);
    
    EXPECT_FALSE(operation_id.isEmpty());
    
    // Cancel immediately
    auto cancel_result = discovery_manager->cancel_discovery(operation_id);
    // Cancel might succeed or fail depending on timing, but should not crash
    
    // Try to cancel non-existent operation
    auto invalid_cancel = discovery_manager->cancel_discovery("non-existent-id");
    EXPECT_FALSE(invalid_cancel.has_value());
    EXPECT_EQ(invalid_cancel.error().code, PluginErrorCode::NotFound);
}
