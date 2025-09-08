/**
 * @file test_http_plugin_loader.cpp
 * @brief Unit tests for HttpPluginLoader class
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QCoreApplication>
#include <QEventLoop>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTimer>
#include <qtplugin/remote/http_plugin_loader.hpp>
#include <qtplugin/remote/remote_plugin_configuration.hpp>
#include <memory>

using namespace qtplugin;
using namespace testing;

class HttpPluginLoaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directory for testing
        temp_dir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(temp_dir->isValid());
        
        // Create test configuration
        config = std::make_shared<RemotePluginConfiguration>(
            RemotePluginConfiguration::create_default());
        config->set_cache_directory(std::filesystem::path(temp_dir->path().toStdString()));
        
        // Create download manager
        download_manager = std::make_shared<PluginDownloadManager>();
        
        // Create validator
        validator = std::make_shared<RemotePluginValidator>(nullptr, config);
        
        // Create HTTP plugin loader
        http_loader = std::make_unique<HttpPluginLoader>(config, download_manager, validator);
        
        // Set up test data
        test_http_url = QUrl("https://plugins.example.com/test-plugin.zip");
        test_registry_url = QUrl("https://registry.example.com/api/v1/plugins");
        
        http_source = RemotePluginSource(test_http_url, RemoteSourceType::Http, "Test HTTP Source");
        registry_source = RemotePluginSource(test_registry_url, RemoteSourceType::Registry, "Test Registry");
    }

    void TearDown() override {
        http_loader.reset();
        validator.reset();
        download_manager.reset();
        config.reset();
        temp_dir.reset();
    }

    std::unique_ptr<QTemporaryDir> temp_dir;
    std::shared_ptr<RemotePluginConfiguration> config;
    std::shared_ptr<PluginDownloadManager> download_manager;
    std::shared_ptr<RemotePluginValidator> validator;
    std::unique_ptr<HttpPluginLoader> http_loader;
    
    QUrl test_http_url;
    QUrl test_registry_url;
    RemotePluginSource http_source;
    RemotePluginSource registry_source;
};

// === Basic Functionality Tests ===

TEST_F(HttpPluginLoaderTest, Construction) {
    EXPECT_NE(http_loader.get(), nullptr);
    EXPECT_EQ(http_loader->loader_name(), "HTTP Plugin Loader");
    
    auto supported_schemes = http_loader->supported_schemes();
    EXPECT_TRUE(std::find(supported_schemes.begin(), supported_schemes.end(), "http") != supported_schemes.end());
    EXPECT_TRUE(std::find(supported_schemes.begin(), supported_schemes.end(), "https") != supported_schemes.end());
}

TEST_F(HttpPluginLoaderTest, UrlSupport) {
    EXPECT_TRUE(HttpPluginLoader::is_http_url(test_http_url));
    EXPECT_TRUE(HttpPluginLoader::is_http_url(QUrl("http://example.com/plugin.zip")));
    EXPECT_FALSE(HttpPluginLoader::is_http_url(QUrl("ftp://example.com/plugin.zip")));
    EXPECT_FALSE(HttpPluginLoader::is_http_url(QUrl("file:///local/plugin.zip")));
    EXPECT_FALSE(HttpPluginLoader::is_http_url(QUrl()));
}

TEST_F(HttpPluginLoaderTest, SourceSupport) {
    EXPECT_TRUE(http_loader->supports_source(http_source));
    EXPECT_TRUE(http_loader->supports_source(registry_source));
    
    RemotePluginSource git_source(QUrl("git://github.com/user/repo"), RemoteSourceType::Git);
    EXPECT_FALSE(http_loader->supports_source(git_source));
}

// === Configuration Tests ===

TEST_F(HttpPluginLoaderTest, ConfigurationManagement) {
    auto initial_config = http_loader->configuration();
    EXPECT_EQ(initial_config, config);
    
    auto new_config = std::make_shared<RemotePluginConfiguration>(
        RemotePluginConfiguration::create_secure());
    
    http_loader->set_configuration(new_config);
    
    auto updated_config = http_loader->configuration();
    EXPECT_EQ(updated_config, new_config);
    EXPECT_NE(updated_config, config);
}

TEST_F(HttpPluginLoaderTest, TimeoutConfiguration) {
    std::chrono::seconds timeout(45);
    http_loader->set_timeout(timeout);
    
    // The timeout would be used in actual network requests
    // For now, we just verify the method can be called
    EXPECT_NO_THROW(http_loader->set_timeout(timeout));
}

TEST_F(HttpPluginLoaderTest, UserAgentConfiguration) {
    QString user_agent = "TestAgent/1.0 (QtForge Plugin Loader)";
    http_loader->set_user_agent(user_agent);
    
    // The user agent would be used in HTTP headers
    // For now, we just verify the method can be called
    EXPECT_NO_THROW(http_loader->set_user_agent(user_agent));
}

// === Source Management Tests ===

TEST_F(HttpPluginLoaderTest, AddRemoveSource) {
    auto initial_sources = http_loader->get_sources();
    size_t initial_count = initial_sources.size();
    
    auto add_result = http_loader->add_source(http_source);
    EXPECT_TRUE(add_result.has_value());
    
    auto sources_after_add = http_loader->get_sources();
    EXPECT_EQ(sources_after_add.size(), initial_count + 1);
    
    auto remove_result = http_loader->remove_source(http_source.id());
    EXPECT_TRUE(remove_result.has_value());
    
    auto sources_after_remove = http_loader->get_sources();
    EXPECT_EQ(sources_after_remove.size(), initial_count);
}

TEST_F(HttpPluginLoaderTest, DuplicateSourceHandling) {
    auto add_result1 = http_loader->add_source(http_source);
    EXPECT_TRUE(add_result1.has_value());
    
    // Adding the same source again should either succeed (update) or fail gracefully
    auto add_result2 = http_loader->add_source(http_source);
    // The behavior depends on implementation - either way should not crash
    EXPECT_NO_THROW(http_loader->add_source(http_source));
}

TEST_F(HttpPluginLoaderTest, RemoveNonExistentSource) {
    auto remove_result = http_loader->remove_source("non-existent-source-id");
    EXPECT_FALSE(remove_result.has_value());
    EXPECT_EQ(remove_result.error().code, PluginErrorCode::NotFound);
}

// === Plugin Discovery Tests ===

TEST_F(HttpPluginLoaderTest, DiscoverPluginsFromHttpSource) {
    // Add the HTTP source
    http_loader->add_source(http_source);
    
    // Attempt to discover plugins (this would normally make HTTP requests)
    auto discovery_result = http_loader->discover_plugins(http_source);
    
    // Since we don't have a real server, this might fail with network error
    // But it should not crash and should return a proper error
    if (!discovery_result.has_value()) {
        EXPECT_TRUE(discovery_result.error().code == PluginErrorCode::NetworkError ||
                   discovery_result.error().code == PluginErrorCode::NotSupported);
    }
}

TEST_F(HttpPluginLoaderTest, DiscoverPluginsFromRegistrySource) {
    // Add the registry source
    http_loader->add_source(registry_source);
    
    // Attempt to discover plugins from registry
    auto discovery_result = http_loader->discover_plugins(registry_source);
    
    // Since we don't have a real registry, this might fail
    // But it should handle the error gracefully
    if (!discovery_result.has_value()) {
        EXPECT_TRUE(discovery_result.error().code == PluginErrorCode::NetworkError ||
                   discovery_result.error().code == PluginErrorCode::NotSupported);
    }
}

TEST_F(HttpPluginLoaderTest, SearchPlugins) {
    // Add sources
    http_loader->add_source(http_source);
    http_loader->add_source(registry_source);
    
    QString search_query = "test plugin";
    int max_results = 10;
    
    auto search_result = http_loader->search_plugins(search_query, max_results);
    
    // Since we don't have real servers, this will likely fail
    // But it should handle the error gracefully
    if (!search_result.has_value()) {
        EXPECT_TRUE(search_result.error().code == PluginErrorCode::NetworkError ||
                   search_result.error().code == PluginErrorCode::NotSupported);
    }
}

// === Remote Plugin Loading Tests ===

TEST_F(HttpPluginLoaderTest, LoadRemotePluginOptions) {
    RemotePluginLoadOptions options;
    options.security_level = RemoteSecurityLevel::Standard;
    options.validate_source = true;
    options.validate_plugin = true;
    options.cache_plugin = true;
    options.auto_update = false;
    options.validation_timeout = std::chrono::seconds(30);
    
    // Test that options are properly structured
    EXPECT_EQ(options.security_level, RemoteSecurityLevel::Standard);
    EXPECT_TRUE(options.validate_source);
    EXPECT_TRUE(options.validate_plugin);
    EXPECT_TRUE(options.cache_plugin);
    EXPECT_FALSE(options.auto_update);
    EXPECT_EQ(options.validation_timeout.count(), 30);
}

TEST_F(HttpPluginLoaderTest, LoadRemotePluginFromSource) {
    RemotePluginLoadOptions options;
    options.security_level = RemoteSecurityLevel::Minimal; // Reduce security for testing
    options.validate_source = false;
    options.validate_plugin = false;
    
    // Attempt to load plugin (this would normally download and validate)
    auto load_result = http_loader->load_remote(http_source, options);
    
    // Since we don't have a real server, this will fail
    // But it should fail gracefully with appropriate error
    EXPECT_FALSE(load_result.has_value());
    EXPECT_TRUE(load_result.error().code == PluginErrorCode::NetworkError ||
               load_result.error().code == PluginErrorCode::FileNotFound ||
               load_result.error().code == PluginErrorCode::NotSupported);
}

// === Asynchronous Operations Tests ===

TEST_F(HttpPluginLoaderTest, AsyncPluginDiscovery) {
    http_loader->add_source(http_source);
    
    bool progress_called = false;
    bool completion_called = false;
    
    auto progress_callback = [&progress_called](const DiscoveryProgress& progress) {
        progress_called = true;
        EXPECT_GE(progress.progress_percentage, 0.0);
        EXPECT_LE(progress.progress_percentage, 100.0);
    };
    
    auto completion_callback = [&completion_called](const qtplugin::expected<std::vector<QJsonObject>, PluginError>& result) {
        completion_called = true;
        // Result might be success or failure depending on network availability
    };
    
    QString operation_id = http_loader->discover_plugins_async(
        http_source, progress_callback, completion_callback);
    
    EXPECT_FALSE(operation_id.isEmpty());
    
    // Wait a bit for async operation to start
    QEventLoop loop;
    QTimer::singleShot(100, &loop, &QEventLoop::quit);
    loop.exec();
    
    // Cancel the operation to clean up
    auto cancel_result = http_loader->cancel_discovery(operation_id);
    // Cancel might succeed or fail depending on timing
}

TEST_F(HttpPluginLoaderTest, AsyncPluginLoading) {
    RemotePluginLoadOptions options;
    options.security_level = RemoteSecurityLevel::Minimal;
    options.validate_source = false;
    options.validate_plugin = false;
    
    bool progress_called = false;
    bool completion_called = false;
    
    auto progress_callback = [&progress_called](const DownloadProgress& progress) {
        progress_called = true;
        EXPECT_GE(progress.percentage, 0.0);
        EXPECT_LE(progress.percentage, 100.0);
    };
    
    auto completion_callback = [&completion_called](const qtplugin::expected<RemotePluginLoadResult, PluginError>& result) {
        completion_called = true;
        // Result will likely be an error due to no real server
    };
    
    QString operation_id = http_loader->load_remote_async(
        http_source, options, progress_callback, completion_callback);
    
    EXPECT_FALSE(operation_id.isEmpty());
    
    // Wait a bit for async operation to start
    QEventLoop loop;
    QTimer::singleShot(100, &loop, &QEventLoop::quit);
    loop.exec();
    
    // Cancel the operation to clean up
    auto cancel_result = http_loader->cancel_remote_load(operation_id);
    // Cancel might succeed or fail depending on timing
}

// === Statistics Tests ===

TEST_F(HttpPluginLoaderTest, StatisticsTracking) {
    auto stats = http_loader->get_statistics();
    
    EXPECT_TRUE(stats.contains("total_requests"));
    EXPECT_TRUE(stats.contains("successful_requests"));
    EXPECT_TRUE(stats.contains("failed_requests"));
    EXPECT_TRUE(stats.contains("cache_hits"));
    EXPECT_TRUE(stats.contains("cache_misses"));
    EXPECT_TRUE(stats.contains("active_operations"));
    
    // Initially all should be zero
    EXPECT_EQ(stats["total_requests"].toInt(), 0);
    EXPECT_EQ(stats["successful_requests"].toInt(), 0);
    EXPECT_EQ(stats["failed_requests"].toInt(), 0);
    EXPECT_EQ(stats["cache_hits"].toInt(), 0);
    EXPECT_EQ(stats["cache_misses"].toInt(), 0);
    EXPECT_EQ(stats["active_operations"].toInt(), 0);
}

// === Error Handling Tests ===

TEST_F(HttpPluginLoaderTest, InvalidSourceHandling) {
    RemotePluginSource invalid_source(QUrl(), RemoteSourceType::Http, "Invalid Source");
    
    auto add_result = http_loader->add_source(invalid_source);
    EXPECT_FALSE(add_result.has_value());
    EXPECT_EQ(add_result.error().code, PluginErrorCode::InvalidConfiguration);
}

TEST_F(HttpPluginLoaderTest, UnsupportedSourceHandling) {
    RemotePluginSource ftp_source(QUrl("ftp://ftp.example.com/plugin.zip"), RemoteSourceType::Ftp);
    
    EXPECT_FALSE(http_loader->supports_source(ftp_source));
    
    auto load_result = http_loader->load_remote(ftp_source, RemotePluginLoadOptions{});
    EXPECT_FALSE(load_result.has_value());
    EXPECT_EQ(load_result.error().code, PluginErrorCode::NotSupported);
}

// === Factory Tests ===

TEST_F(HttpPluginLoaderTest, FactoryCreation) {
    auto default_loader = HttpPluginLoaderFactory::create_default();
    EXPECT_NE(default_loader.get(), nullptr);
    EXPECT_EQ(default_loader->loader_name(), "HTTP Plugin Loader");
    
    auto secure_loader = HttpPluginLoaderFactory::create_secure();
    EXPECT_NE(secure_loader.get(), nullptr);
    
    auto enterprise_loader = HttpPluginLoaderFactory::create_enterprise();
    EXPECT_NE(enterprise_loader.get(), nullptr);
}

TEST_F(HttpPluginLoaderTest, FactoryWithCustomConfig) {
    auto custom_config = std::make_shared<RemotePluginConfiguration>(
        RemotePluginConfiguration::create_secure());
    
    auto custom_loader = HttpPluginLoaderFactory::create_with_config(custom_config);
    EXPECT_NE(custom_loader.get(), nullptr);
    EXPECT_EQ(custom_loader->configuration(), custom_config);
}
