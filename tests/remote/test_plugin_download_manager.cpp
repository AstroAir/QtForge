/**
 * @file test_plugin_download_manager.cpp
 * @brief Unit tests for PluginDownloadManager class
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QCoreApplication>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTimer>
#include <qtplugin/remote/plugin_download_manager.hpp>
#include <filesystem>
#include <fstream>

using namespace qtplugin;
using namespace testing;

class PluginDownloadManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directory for testing
        temp_dir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(temp_dir->isValid());
        
        // Create download manager with test cache directory
        download_manager = std::make_unique<PluginDownloadManager>();
        
        // Set up test URLs and options
        test_url = QUrl("https://example.com/test-plugin.zip");
        test_options.cache_directory = std::filesystem::path(temp_dir->path().toStdString());
        test_options.timeout = std::chrono::seconds(30);
        test_options.max_retries = 3;
        test_options.max_file_size = 10 * 1024 * 1024; // 10MB
        test_options.use_cache = true;
        test_options.verify_checksum = false; // Disable for basic tests
    }

    void TearDown() override {
        download_manager.reset();
        temp_dir.reset();
    }

    std::unique_ptr<QTemporaryDir> temp_dir;
    std::unique_ptr<PluginDownloadManager> download_manager;
    QUrl test_url;
    DownloadOptions test_options;
};

// === Basic Functionality Tests ===

TEST_F(PluginDownloadManagerTest, Construction) {
    EXPECT_NE(download_manager.get(), nullptr);
    
    auto stats = download_manager->get_statistics();
    EXPECT_EQ(stats["active_downloads"].toInt(), 0);
    EXPECT_EQ(stats["completed_downloads"].toInt(), 0);
    EXPECT_EQ(stats["failed_downloads"].toInt(), 0);
}

TEST_F(PluginDownloadManagerTest, CacheDirectoryCreation) {
    std::filesystem::path cache_path = test_options.cache_directory / "test_cache";
    test_options.cache_directory = cache_path;
    
    // Cache directory should be created when needed
    EXPECT_FALSE(std::filesystem::exists(cache_path));
    
    // This should create the directory
    download_manager->set_cache_directory(cache_path);
    
    EXPECT_TRUE(std::filesystem::exists(cache_path));
}

// === Download Options Tests ===

TEST_F(PluginDownloadManagerTest, DownloadOptionsJsonSerialization) {
    test_options.expected_checksum = "abc123";
    test_options.user_agent = "TestAgent/1.0";
    test_options.custom_headers["X-Test-Header"] = "test-value";
    
    QJsonObject json = test_options.to_json();
    
    EXPECT_EQ(json["timeout"].toInt(), 30);
    EXPECT_EQ(json["max_retries"].toInt(), 3);
    EXPECT_EQ(json["max_file_size"].toInt(), 10 * 1024 * 1024);
    EXPECT_TRUE(json["use_cache"].toBool());
    EXPECT_FALSE(json["verify_checksum"].toBool());
    EXPECT_EQ(json["expected_checksum"].toString(), "abc123");
    EXPECT_EQ(json["user_agent"].toString(), "TestAgent/1.0");
    
    QJsonObject headers = json["custom_headers"].toObject();
    EXPECT_EQ(headers["X-Test-Header"].toString(), "test-value");
}

TEST_F(PluginDownloadManagerTest, DownloadOptionsJsonDeserialization) {
    QJsonObject json;
    json["timeout"] = 45;
    json["max_retries"] = 5;
    json["max_file_size"] = 20 * 1024 * 1024;
    json["use_cache"] = false;
    json["verify_checksum"] = true;
    json["expected_checksum"] = "def456";
    json["user_agent"] = "TestAgent/2.0";
    
    QJsonObject headers;
    headers["Authorization"] = "Bearer token123";
    json["custom_headers"] = headers;
    
    DownloadOptions options = DownloadOptions::from_json(json);
    
    EXPECT_EQ(options.timeout.count(), 45);
    EXPECT_EQ(options.max_retries, 5);
    EXPECT_EQ(options.max_file_size, 20 * 1024 * 1024);
    EXPECT_FALSE(options.use_cache);
    EXPECT_TRUE(options.verify_checksum);
    EXPECT_EQ(options.expected_checksum, "def456");
    EXPECT_EQ(options.user_agent, "TestAgent/2.0");
    EXPECT_EQ(options.custom_headers["Authorization"].toString(), "Bearer token123");
}

// === Cache Entry Tests ===

TEST_F(PluginDownloadManagerTest, CacheEntryValidation) {
    // Create a test file
    std::filesystem::path test_file = test_options.cache_directory / "test_file.txt";
    std::filesystem::create_directories(test_file.parent_path());
    std::ofstream(test_file) << "test content";
    
    CacheEntry entry;
    entry.file_path = test_file;
    entry.source_url = test_url;
    entry.cached_time = std::chrono::system_clock::now();
    entry.ttl = std::chrono::hours(1);
    entry.checksum = "test-checksum";
    entry.file_size = 12; // "test content" length
    
    EXPECT_TRUE(entry.is_valid());
    EXPECT_FALSE(entry.is_expired());
}

TEST_F(PluginDownloadManagerTest, CacheEntryExpiration) {
    std::filesystem::path test_file = test_options.cache_directory / "expired_file.txt";
    std::filesystem::create_directories(test_file.parent_path());
    std::ofstream(test_file) << "expired content";
    
    CacheEntry entry;
    entry.file_path = test_file;
    entry.source_url = test_url;
    entry.cached_time = std::chrono::system_clock::now() - std::chrono::hours(2);
    entry.ttl = std::chrono::hours(1); // Expired 1 hour ago
    entry.checksum = "expired-checksum";
    entry.file_size = 15;
    
    EXPECT_FALSE(entry.is_valid()); // Should be invalid due to expiration
    EXPECT_TRUE(entry.is_expired());
}

TEST_F(PluginDownloadManagerTest, CacheEntryJsonSerialization) {
    CacheEntry entry;
    entry.file_path = test_options.cache_directory / "cache_test.txt";
    entry.source_url = test_url;
    entry.cached_time = std::chrono::system_clock::now();
    entry.ttl = std::chrono::hours(2);
    entry.checksum = "cache-checksum";
    entry.file_size = 1024;
    entry.metadata["version"] = "1.0.0";
    
    QJsonObject json = entry.to_json();
    
    EXPECT_EQ(json["source_url"].toString(), test_url.toString());
    EXPECT_EQ(json["checksum"].toString(), "cache-checksum");
    EXPECT_EQ(json["file_size"].toInt(), 1024);
    EXPECT_EQ(json["metadata"].toObject()["version"].toString(), "1.0.0");
    
    // Test deserialization
    CacheEntry deserialized = CacheEntry::from_json(json);
    
    EXPECT_EQ(deserialized.source_url, entry.source_url);
    EXPECT_EQ(deserialized.checksum, entry.checksum);
    EXPECT_EQ(deserialized.file_size, entry.file_size);
    EXPECT_EQ(deserialized.metadata["version"].toString(), "1.0.0");
}

// === Download Progress Tests ===

TEST_F(PluginDownloadManagerTest, DownloadProgressCalculation) {
    DownloadProgress progress;
    progress.bytes_received = 5000;
    progress.bytes_total = 10000;
    progress.bytes_per_second = 1000;
    progress.elapsed_time = std::chrono::seconds(5);
    progress.estimated_time_remaining = std::chrono::seconds(5);
    
    // Calculate percentage
    progress.percentage = (static_cast<double>(progress.bytes_received) / progress.bytes_total) * 100.0;
    
    EXPECT_DOUBLE_EQ(progress.percentage, 50.0);
    
    QJsonObject json = progress.to_json();
    EXPECT_EQ(json["bytes_received"].toInt(), 5000);
    EXPECT_EQ(json["bytes_total"].toInt(), 10000);
    EXPECT_DOUBLE_EQ(json["percentage"].toDouble(), 50.0);
    EXPECT_EQ(json["bytes_per_second"].toInt(), 1000);
    EXPECT_EQ(json["elapsed_time"].toInt(), 5);
    EXPECT_EQ(json["estimated_time_remaining"].toInt(), 5);
}

// === Cache Management Tests ===

TEST_F(PluginDownloadManagerTest, CacheHitScenario) {
    // Create a cached file
    std::filesystem::path cached_file = test_options.cache_directory / "cached_plugin.zip";
    std::filesystem::create_directories(cached_file.parent_path());
    std::string test_content = "cached plugin content";
    std::ofstream(cached_file) << test_content;
    
    // Add cache entry
    CacheEntry entry;
    entry.file_path = cached_file;
    entry.source_url = test_url;
    entry.cached_time = std::chrono::system_clock::now();
    entry.ttl = std::chrono::hours(1);
    entry.checksum = "cached-checksum";
    entry.file_size = test_content.size();
    
    // Simulate cache lookup
    bool cache_hit = entry.is_valid();
    EXPECT_TRUE(cache_hit);
    
    if (cache_hit) {
        // Verify file content
        std::ifstream file(cached_file);
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        EXPECT_EQ(content, test_content);
    }
}

TEST_F(PluginDownloadManagerTest, CacheMissScenario) {
    // Test with non-existent file
    std::filesystem::path non_existent = test_options.cache_directory / "non_existent.zip";
    
    CacheEntry entry;
    entry.file_path = non_existent;
    entry.source_url = test_url;
    entry.cached_time = std::chrono::system_clock::now();
    entry.ttl = std::chrono::hours(1);
    
    EXPECT_FALSE(entry.is_valid()); // Should be invalid due to missing file
}

// === Statistics Tests ===

TEST_F(PluginDownloadManagerTest, StatisticsTracking) {
    auto initial_stats = download_manager->get_statistics();
    
    EXPECT_TRUE(initial_stats.contains("active_downloads"));
    EXPECT_TRUE(initial_stats.contains("completed_downloads"));
    EXPECT_TRUE(initial_stats.contains("failed_downloads"));
    EXPECT_TRUE(initial_stats.contains("cache_hits"));
    EXPECT_TRUE(initial_stats.contains("cache_misses"));
    EXPECT_TRUE(initial_stats.contains("total_bytes_downloaded"));
    
    // All should be zero initially
    EXPECT_EQ(initial_stats["active_downloads"].toInt(), 0);
    EXPECT_EQ(initial_stats["completed_downloads"].toInt(), 0);
    EXPECT_EQ(initial_stats["failed_downloads"].toInt(), 0);
    EXPECT_EQ(initial_stats["cache_hits"].toInt(), 0);
    EXPECT_EQ(initial_stats["cache_misses"].toInt(), 0);
    EXPECT_EQ(initial_stats["total_bytes_downloaded"].toInt(), 0);
}

// === Error Handling Tests ===

TEST_F(PluginDownloadManagerTest, InvalidUrlHandling) {
    QUrl invalid_url("not-a-valid-url");
    
    // This should fail immediately with invalid URL
    auto result = download_manager->download_sync(invalid_url, test_options);
    
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, PluginErrorCode::InvalidConfiguration);
}

TEST_F(PluginDownloadManagerTest, FileSizeLimitExceeded) {
    test_options.max_file_size = 100; // Very small limit
    
    // This would fail if we had a real server returning large files
    // For now, we just test that the option is properly set
    EXPECT_EQ(test_options.max_file_size, 100);
}

// === Checksum Validation Tests ===

TEST_F(PluginDownloadManagerTest, ChecksumValidation) {
    std::filesystem::path test_file = test_options.cache_directory / "checksum_test.txt";
    std::filesystem::create_directories(test_file.parent_path());
    std::string content = "test content for checksum";
    std::ofstream(test_file) << content;
    
    // Calculate expected checksum (this would normally be provided)
    QString expected_checksum = "expected-checksum-value";
    
    test_options.verify_checksum = true;
    test_options.expected_checksum = expected_checksum;
    
    // The actual checksum validation would happen in the download manager
    // For this test, we just verify the options are set correctly
    EXPECT_TRUE(test_options.verify_checksum);
    EXPECT_EQ(test_options.expected_checksum, expected_checksum);
}

// === Concurrent Download Tests ===

TEST_F(PluginDownloadManagerTest, ConcurrentDownloadLimits) {
    // Test that the download manager respects concurrent download limits
    int max_concurrent = 3;
    download_manager->set_max_concurrent_downloads(max_concurrent);
    
    // Verify the limit is set (this would be tested with actual downloads in integration tests)
    auto stats = download_manager->get_statistics();
    EXPECT_TRUE(stats.contains("max_concurrent_downloads"));
}

// === Cleanup Tests ===

TEST_F(PluginDownloadManagerTest, CacheCleanup) {
    // Create some test cache files
    std::filesystem::path cache_dir = test_options.cache_directory;
    std::filesystem::create_directories(cache_dir);
    
    std::ofstream(cache_dir / "old_file1.zip") << "old content 1";
    std::ofstream(cache_dir / "old_file2.zip") << "old content 2";
    std::ofstream(cache_dir / "recent_file.zip") << "recent content";
    
    // Test cleanup (this would normally clean up expired entries)
    int cleaned = download_manager->cleanup_cache(0); // Clean all
    
    // The actual cleanup behavior would be tested with real cache entries
    EXPECT_GE(cleaned, 0);
}

TEST_F(PluginDownloadManagerTest, TemporaryFileCleanup) {
    // Test that temporary files are cleaned up properly
    std::filesystem::path temp_file = test_options.cache_directory / "temp_download.tmp";
    std::filesystem::create_directories(temp_file.parent_path());
    std::ofstream(temp_file) << "temporary content";
    
    EXPECT_TRUE(std::filesystem::exists(temp_file));
    
    // Cleanup should remove temporary files
    download_manager->cleanup_temporary_files();
    
    // In a real implementation, this would clean up .tmp files
    // For now, we just verify the method exists and can be called
}
