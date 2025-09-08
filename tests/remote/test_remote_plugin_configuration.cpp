/**
 * @file test_remote_plugin_configuration.cpp
 * @brief Unit tests for RemotePluginConfiguration class
 */

#include <gtest/gtest.h>
#include <QJsonDocument>
#include <QTemporaryDir>
#include <qtplugin/remote/remote_plugin_configuration.hpp>
#include <memory>
#include <filesystem>

using namespace qtplugin;
using namespace testing;

class RemotePluginConfigurationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directory for testing
        temp_dir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(temp_dir->isValid());
        
        // Create test configurations
        minimal_config = std::make_shared<RemotePluginConfiguration>(
            RemotePluginConfiguration::create_minimal());
        
        default_config = std::make_shared<RemotePluginConfiguration>(
            RemotePluginConfiguration::create_default());
        
        secure_config = std::make_shared<RemotePluginConfiguration>(
            RemotePluginConfiguration::create_secure());
        
        enterprise_config = std::make_shared<RemotePluginConfiguration>(
            RemotePluginConfiguration::create_enterprise());
    }

    void TearDown() override {
        enterprise_config.reset();
        secure_config.reset();
        default_config.reset();
        minimal_config.reset();
        temp_dir.reset();
    }

    std::unique_ptr<QTemporaryDir> temp_dir;
    std::shared_ptr<RemotePluginConfiguration> minimal_config;
    std::shared_ptr<RemotePluginConfiguration> default_config;
    std::shared_ptr<RemotePluginConfiguration> secure_config;
    std::shared_ptr<RemotePluginConfiguration> enterprise_config;
};

// === Factory Method Tests ===

TEST_F(RemotePluginConfigurationTest, FactoryMethods) {
    EXPECT_NE(minimal_config.get(), nullptr);
    EXPECT_NE(default_config.get(), nullptr);
    EXPECT_NE(secure_config.get(), nullptr);
    EXPECT_NE(enterprise_config.get(), nullptr);
    
    // Test that different factory methods create different configurations
    EXPECT_EQ(minimal_config->security_level(), RemoteSecurityLevel::Minimal);
    EXPECT_EQ(default_config->security_level(), RemoteSecurityLevel::Standard);
    EXPECT_EQ(secure_config->security_level(), RemoteSecurityLevel::High);
    EXPECT_EQ(enterprise_config->security_level(), RemoteSecurityLevel::Paranoid);
}

// === Security Level Tests ===

TEST_F(RemotePluginConfigurationTest, SecurityLevelProperties) {
    // Minimal security should be permissive
    EXPECT_FALSE(minimal_config->require_signature_validation());
    EXPECT_FALSE(minimal_config->require_source_verification());
    EXPECT_TRUE(minimal_config->allow_http_sources());
    
    // Default security should be balanced
    EXPECT_TRUE(default_config->require_source_verification());
    EXPECT_FALSE(default_config->allow_http_sources()); // Should prefer HTTPS
    
    // Secure configuration should be restrictive
    EXPECT_TRUE(secure_config->require_signature_validation());
    EXPECT_TRUE(secure_config->require_source_verification());
    EXPECT_FALSE(secure_config->allow_http_sources());
    
    // Enterprise should be most restrictive
    EXPECT_TRUE(enterprise_config->require_signature_validation());
    EXPECT_TRUE(enterprise_config->require_source_verification());
    EXPECT_FALSE(enterprise_config->allow_http_sources());
    EXPECT_TRUE(enterprise_config->require_whitelist_verification());
}

// === Cache Configuration Tests ===

TEST_F(RemotePluginConfigurationTest, CacheConfiguration) {
    std::filesystem::path cache_dir = std::filesystem::path(temp_dir->path().toStdString()) / "cache";
    
    default_config->set_cache_directory(cache_dir);
    EXPECT_EQ(default_config->cache_directory(), cache_dir);
    
    // Test cache TTL
    std::chrono::hours ttl(24);
    default_config->set_cache_ttl(ttl);
    EXPECT_EQ(default_config->cache_ttl(), ttl);
    
    // Test cache size limit
    size_t cache_limit = 100 * 1024 * 1024; // 100MB
    default_config->set_max_cache_size(cache_limit);
    EXPECT_EQ(default_config->max_cache_size(), cache_limit);
    
    // Test cache enabled/disabled
    EXPECT_TRUE(default_config->is_cache_enabled());
    default_config->set_cache_enabled(false);
    EXPECT_FALSE(default_config->is_cache_enabled());
}

// === Network Configuration Tests ===

TEST_F(RemotePluginConfigurationTest, NetworkConfiguration) {
    // Test timeout settings
    std::chrono::seconds timeout(45);
    default_config->set_network_timeout(timeout);
    EXPECT_EQ(default_config->network_timeout(), timeout);
    
    // Test retry settings
    int max_retries = 5;
    default_config->set_max_retries(max_retries);
    EXPECT_EQ(default_config->max_retries(), max_retries);
    
    // Test concurrent downloads
    int max_concurrent = 3;
    default_config->set_max_concurrent_downloads(max_concurrent);
    EXPECT_EQ(default_config->max_concurrent_downloads(), max_concurrent);
    
    // Test user agent
    QString user_agent = "TestAgent/1.0";
    default_config->set_user_agent(user_agent);
    EXPECT_EQ(default_config->user_agent(), user_agent);
}

// === Source Management Tests ===

TEST_F(RemotePluginConfigurationTest, SourceManagement) {
    RemotePluginSource test_source(
        QUrl("https://registry.example.com/api/v1"),
        RemoteSourceType::Registry,
        "Test Registry"
    );
    
    // Add source
    auto add_result = default_config->add_source(test_source);
    EXPECT_TRUE(add_result.has_value());
    
    // Get sources
    auto sources = default_config->get_all_sources();
    EXPECT_EQ(sources.size(), 1);
    EXPECT_EQ(sources[0].url(), test_source.url());
    
    // Get specific source
    auto retrieved_source = default_config->get_source(test_source.id());
    EXPECT_TRUE(retrieved_source.has_value());
    EXPECT_EQ(retrieved_source->url(), test_source.url());
    
    // Remove source
    auto remove_result = default_config->remove_source(test_source.id());
    EXPECT_TRUE(remove_result.has_value());
    
    auto sources_after_remove = default_config->get_all_sources();
    EXPECT_TRUE(sources_after_remove.empty());
}

TEST_F(RemotePluginConfigurationTest, DuplicateSourceHandling) {
    RemotePluginSource test_source(
        QUrl("https://registry.example.com/api/v1"),
        RemoteSourceType::Registry,
        "Test Registry"
    );
    
    // Add source twice
    auto add_result1 = default_config->add_source(test_source);
    EXPECT_TRUE(add_result1.has_value());
    
    auto add_result2 = default_config->add_source(test_source);
    // Should either succeed (update) or fail gracefully
    // But should not crash
    EXPECT_NO_THROW(default_config->add_source(test_source));
}

TEST_F(RemotePluginConfigurationTest, InvalidSourceHandling) {
    RemotePluginSource invalid_source(QUrl(), RemoteSourceType::Http, "Invalid");
    
    auto add_result = default_config->add_source(invalid_source);
    EXPECT_FALSE(add_result.has_value());
    EXPECT_EQ(add_result.error().code, PluginErrorCode::InvalidConfiguration);
}

// === Whitelist/Blacklist Tests ===

TEST_F(RemotePluginConfigurationTest, WhitelistManagement) {
    QString domain = "trusted.example.com";
    
    // Add to whitelist
    secure_config->add_to_whitelist(domain);
    EXPECT_TRUE(secure_config->is_whitelisted(domain));
    
    // Remove from whitelist
    secure_config->remove_from_whitelist(domain);
    EXPECT_FALSE(secure_config->is_whitelisted(domain));
    
    // Test URL whitelist checking
    QUrl whitelisted_url("https://trusted.example.com/plugin.zip");
    QUrl non_whitelisted_url("https://untrusted.example.com/plugin.zip");
    
    secure_config->add_to_whitelist("trusted.example.com");
    EXPECT_TRUE(secure_config->is_url_whitelisted(whitelisted_url));
    EXPECT_FALSE(secure_config->is_url_whitelisted(non_whitelisted_url));
}

TEST_F(RemotePluginConfigurationTest, BlacklistManagement) {
    QString domain = "malicious.example.com";
    
    // Add to blacklist
    default_config->add_to_blacklist(domain);
    EXPECT_TRUE(default_config->is_blacklisted(domain));
    
    // Remove from blacklist
    default_config->remove_from_blacklist(domain);
    EXPECT_FALSE(default_config->is_blacklisted(domain));
    
    // Test URL blacklist checking
    QUrl blacklisted_url("https://malicious.example.com/malware.zip");
    QUrl safe_url("https://safe.example.com/plugin.zip");
    
    default_config->add_to_blacklist("malicious.example.com");
    EXPECT_TRUE(default_config->is_url_blacklisted(blacklisted_url));
    EXPECT_FALSE(default_config->is_url_blacklisted(safe_url));
}

// === Validation Configuration Tests ===

TEST_F(RemotePluginConfigurationTest, ValidationConfiguration) {
    // Test signature validation requirements
    EXPECT_FALSE(minimal_config->require_signature_validation());
    minimal_config->set_require_signature_validation(true);
    EXPECT_TRUE(minimal_config->require_signature_validation());
    
    // Test source verification requirements
    EXPECT_FALSE(minimal_config->require_source_verification());
    minimal_config->set_require_source_verification(true);
    EXPECT_TRUE(minimal_config->require_source_verification());
    
    // Test checksum validation
    EXPECT_TRUE(default_config->require_checksum_validation());
    default_config->set_require_checksum_validation(false);
    EXPECT_FALSE(default_config->require_checksum_validation());
    
    // Test size limits
    size_t max_size = 50 * 1024 * 1024; // 50MB
    default_config->set_max_plugin_size(max_size);
    EXPECT_EQ(default_config->max_plugin_size(), max_size);
}

// === JSON Serialization Tests ===

TEST_F(RemotePluginConfigurationTest, JsonSerialization) {
    // Configure the default config with specific settings
    default_config->set_cache_enabled(true);
    default_config->set_cache_ttl(std::chrono::hours(12));
    default_config->set_network_timeout(std::chrono::seconds(60));
    default_config->set_max_retries(3);
    default_config->set_user_agent("TestAgent/1.0");
    default_config->set_require_signature_validation(true);
    default_config->add_to_whitelist("trusted.example.com");
    default_config->add_to_blacklist("malicious.example.com");
    
    QJsonObject json = default_config->to_json();
    
    EXPECT_EQ(json["security_level"].toInt(), static_cast<int>(RemoteSecurityLevel::Standard));
    EXPECT_TRUE(json["cache_enabled"].toBool());
    EXPECT_EQ(json["cache_ttl_hours"].toInt(), 12);
    EXPECT_EQ(json["network_timeout_seconds"].toInt(), 60);
    EXPECT_EQ(json["max_retries"].toInt(), 3);
    EXPECT_EQ(json["user_agent"].toString(), "TestAgent/1.0");
    EXPECT_TRUE(json["require_signature_validation"].toBool());
    
    QJsonArray whitelist = json["whitelist"].toArray();
    EXPECT_EQ(whitelist.size(), 1);
    EXPECT_EQ(whitelist[0].toString(), "trusted.example.com");
    
    QJsonArray blacklist = json["blacklist"].toArray();
    EXPECT_EQ(blacklist.size(), 1);
    EXPECT_EQ(blacklist[0].toString(), "malicious.example.com");
}

TEST_F(RemotePluginConfigurationTest, JsonDeserialization) {
    QJsonObject json;
    json["security_level"] = static_cast<int>(RemoteSecurityLevel::High);
    json["cache_enabled"] = false;
    json["cache_ttl_hours"] = 6;
    json["network_timeout_seconds"] = 30;
    json["max_retries"] = 5;
    json["user_agent"] = "DeserializedAgent/2.0";
    json["require_signature_validation"] = false;
    json["require_source_verification"] = true;
    
    QJsonArray whitelist;
    whitelist.append("safe1.example.com");
    whitelist.append("safe2.example.com");
    json["whitelist"] = whitelist;
    
    QJsonArray blacklist;
    blacklist.append("bad1.example.com");
    json["blacklist"] = blacklist;
    
    auto config = RemotePluginConfiguration::from_json(json);
    
    EXPECT_EQ(config.security_level(), RemoteSecurityLevel::High);
    EXPECT_FALSE(config.is_cache_enabled());
    EXPECT_EQ(config.cache_ttl(), std::chrono::hours(6));
    EXPECT_EQ(config.network_timeout(), std::chrono::seconds(30));
    EXPECT_EQ(config.max_retries(), 5);
    EXPECT_EQ(config.user_agent(), "DeserializedAgent/2.0");
    EXPECT_FALSE(config.require_signature_validation());
    EXPECT_TRUE(config.require_source_verification());
    
    EXPECT_TRUE(config.is_whitelisted("safe1.example.com"));
    EXPECT_TRUE(config.is_whitelisted("safe2.example.com"));
    EXPECT_FALSE(config.is_whitelisted("unknown.example.com"));
    
    EXPECT_TRUE(config.is_blacklisted("bad1.example.com"));
    EXPECT_FALSE(config.is_blacklisted("safe1.example.com"));
}

TEST_F(RemotePluginConfigurationTest, RoundTripSerialization) {
    // Configure with various settings
    secure_config->set_cache_enabled(false);
    secure_config->set_network_timeout(std::chrono::seconds(90));
    secure_config->set_max_concurrent_downloads(2);
    secure_config->add_to_whitelist("trusted1.example.com");
    secure_config->add_to_whitelist("trusted2.example.com");
    secure_config->add_to_blacklist("malicious.example.com");
    
    // Serialize to JSON
    QJsonObject json = secure_config->to_json();
    
    // Deserialize from JSON
    auto deserialized_config = RemotePluginConfiguration::from_json(json);
    
    // Verify all settings are preserved
    EXPECT_EQ(deserialized_config.security_level(), secure_config->security_level());
    EXPECT_EQ(deserialized_config.is_cache_enabled(), secure_config->is_cache_enabled());
    EXPECT_EQ(deserialized_config.network_timeout(), secure_config->network_timeout());
    EXPECT_EQ(deserialized_config.max_concurrent_downloads(), secure_config->max_concurrent_downloads());
    
    EXPECT_EQ(deserialized_config.is_whitelisted("trusted1.example.com"), 
              secure_config->is_whitelisted("trusted1.example.com"));
    EXPECT_EQ(deserialized_config.is_whitelisted("trusted2.example.com"), 
              secure_config->is_whitelisted("trusted2.example.com"));
    EXPECT_EQ(deserialized_config.is_blacklisted("malicious.example.com"), 
              secure_config->is_blacklisted("malicious.example.com"));
}

// === Configuration Validation Tests ===

TEST_F(RemotePluginConfigurationTest, ConfigurationValidation) {
    // Valid configuration should pass validation
    auto validation_result = default_config->validate();
    EXPECT_TRUE(validation_result.has_value());
    
    // Invalid configuration should fail validation
    default_config->set_network_timeout(std::chrono::seconds(0)); // Invalid timeout
    validation_result = default_config->validate();
    EXPECT_FALSE(validation_result.has_value());
    EXPECT_EQ(validation_result.error().code, PluginErrorCode::InvalidConfiguration);
}

// === Configuration Copying Tests ===

TEST_F(RemotePluginConfigurationTest, ConfigurationCopying) {
    // Configure original
    default_config->set_cache_enabled(false);
    default_config->set_user_agent("OriginalAgent/1.0");
    default_config->add_to_whitelist("original.example.com");
    
    // Create copy
    auto copied_config = std::make_shared<RemotePluginConfiguration>(*default_config);
    
    // Verify copy has same settings
    EXPECT_EQ(copied_config->is_cache_enabled(), default_config->is_cache_enabled());
    EXPECT_EQ(copied_config->user_agent(), default_config->user_agent());
    EXPECT_EQ(copied_config->is_whitelisted("original.example.com"), 
              default_config->is_whitelisted("original.example.com"));
    
    // Modify original
    default_config->set_user_agent("ModifiedAgent/2.0");
    
    // Copy should remain unchanged
    EXPECT_EQ(copied_config->user_agent(), "OriginalAgent/1.0");
    EXPECT_EQ(default_config->user_agent(), "ModifiedAgent/2.0");
}
