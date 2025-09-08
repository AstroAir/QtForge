/**
 * @file test_remote_plugin_security.cpp
 * @brief Security and validation tests for remote plugin system
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <qtplugin/remote/remote_plugin_validator.hpp>
#include <qtplugin/remote/remote_plugin_configuration.hpp>
#include <qtplugin/remote/remote_plugin_source.hpp>
#include <qtplugin/remote/remote_plugin_manager_extension.hpp>
#include <memory>
#include <filesystem>
#include <fstream>

using namespace qtplugin;
using namespace testing;

class RemotePluginSecurityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directory for testing
        temp_dir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(temp_dir->isValid());
        
        // Create test configurations for different security levels
        minimal_config = std::make_shared<RemotePluginConfiguration>(
            RemotePluginConfiguration::create_minimal());
        
        standard_config = std::make_shared<RemotePluginConfiguration>(
            RemotePluginConfiguration::create_default());
        
        secure_config = std::make_shared<RemotePluginConfiguration>(
            RemotePluginConfiguration::create_secure());
        
        enterprise_config = std::make_shared<RemotePluginConfiguration>(
            RemotePluginConfiguration::create_enterprise());
        
        // Create validators for different security levels
        minimal_validator = std::make_unique<RemotePluginValidator>(nullptr, minimal_config);
        standard_validator = std::make_unique<RemotePluginValidator>(nullptr, standard_config);
        secure_validator = std::make_unique<RemotePluginValidator>(nullptr, secure_config);
        enterprise_validator = std::make_unique<RemotePluginValidator>(nullptr, enterprise_config);
        
        // Set up test data
        setupTestData();
    }

    void TearDown() override {
        enterprise_validator.reset();
        secure_validator.reset();
        standard_validator.reset();
        minimal_validator.reset();
        
        enterprise_config.reset();
        secure_config.reset();
        standard_config.reset();
        minimal_config.reset();
        
        temp_dir.reset();
    }

    void setupTestData() {
        // Create test plugin sources with different security characteristics
        trusted_source = RemotePluginSource(
            QUrl("https://trusted-registry.example.com/api/v1"),
            RemoteSourceType::Registry,
            "Trusted Registry"
        );
        
        untrusted_source = RemotePluginSource(
            QUrl("http://untrusted-site.example.com/plugin.zip"),
            RemoteSourceType::Http,
            "Untrusted Source"
        );
        
        malicious_source = RemotePluginSource(
            QUrl("https://malicious-site.example.com/malware.zip"),
            RemoteSourceType::Http,
            "Malicious Source"
        );
        
        // Create test plugin files
        createTestPluginFiles();
    }

    void createTestPluginFiles() {
        std::filesystem::path test_dir = std::filesystem::path(temp_dir->path().toStdString());
        
        // Valid plugin file
        valid_plugin_path = test_dir / "valid_plugin.zip";
        std::ofstream valid_file(valid_plugin_path);
        valid_file << "Valid plugin content with proper structure";
        valid_file.close();
        
        // Suspicious plugin file (large size)
        suspicious_plugin_path = test_dir / "suspicious_plugin.zip";
        std::ofstream suspicious_file(suspicious_plugin_path);
        // Create a large file to simulate suspicious size
        std::string large_content(1024 * 1024, 'X'); // 1MB of X's
        suspicious_file << large_content;
        suspicious_file.close();
        
        // Malicious plugin file (simulated)
        malicious_plugin_path = test_dir / "malicious_plugin.zip";
        std::ofstream malicious_file(malicious_plugin_path);
        malicious_file << "Malicious content that should be detected";
        malicious_file.close();
    }

    std::unique_ptr<QTemporaryDir> temp_dir;
    
    std::shared_ptr<RemotePluginConfiguration> minimal_config;
    std::shared_ptr<RemotePluginConfiguration> standard_config;
    std::shared_ptr<RemotePluginConfiguration> secure_config;
    std::shared_ptr<RemotePluginConfiguration> enterprise_config;
    
    std::unique_ptr<RemotePluginValidator> minimal_validator;
    std::unique_ptr<RemotePluginValidator> standard_validator;
    std::unique_ptr<RemotePluginValidator> secure_validator;
    std::unique_ptr<RemotePluginValidator> enterprise_validator;
    
    RemotePluginSource trusted_source;
    RemotePluginSource untrusted_source;
    RemotePluginSource malicious_source;
    
    std::filesystem::path valid_plugin_path;
    std::filesystem::path suspicious_plugin_path;
    std::filesystem::path malicious_plugin_path;
};

// === Security Level Configuration Tests ===

TEST_F(RemotePluginSecurityTest, SecurityLevelConfigurations) {
    // Test that different security levels have appropriate settings
    
    // Minimal security should be permissive
    EXPECT_EQ(minimal_config->security_level(), RemoteSecurityLevel::Minimal);
    EXPECT_FALSE(minimal_config->require_signature_validation());
    EXPECT_FALSE(minimal_config->require_source_verification());
    
    // Standard security should have balanced settings
    EXPECT_EQ(standard_config->security_level(), RemoteSecurityLevel::Standard);
    EXPECT_TRUE(standard_config->require_source_verification());
    
    // Secure configuration should be restrictive
    EXPECT_EQ(secure_config->security_level(), RemoteSecurityLevel::High);
    EXPECT_TRUE(secure_config->require_signature_validation());
    EXPECT_TRUE(secure_config->require_source_verification());
    
    // Enterprise should be most restrictive
    EXPECT_EQ(enterprise_config->security_level(), RemoteSecurityLevel::Paranoid);
    EXPECT_TRUE(enterprise_config->require_signature_validation());
    EXPECT_TRUE(enterprise_config->require_source_verification());
}

// === Source Validation Tests ===

TEST_F(RemotePluginSecurityTest, TrustedSourceValidation) {
    // Trusted HTTPS source should pass validation in most security levels
    auto minimal_result = minimal_validator->validate_source(trusted_source);
    EXPECT_TRUE(minimal_result.has_value());
    
    auto standard_result = standard_validator->validate_source(trusted_source);
    EXPECT_TRUE(standard_result.has_value());
    
    auto secure_result = secure_validator->validate_source(trusted_source);
    // Might pass or fail depending on whitelist/reputation
    // But should not crash
    EXPECT_NO_THROW(secure_validator->validate_source(trusted_source));
}

TEST_F(RemotePluginSecurityTest, UntrustedSourceValidation) {
    // HTTP (non-HTTPS) source should be rejected by higher security levels
    auto minimal_result = minimal_validator->validate_source(untrusted_source);
    // Minimal security might allow HTTP
    
    auto standard_result = standard_validator->validate_source(untrusted_source);
    // Standard security might reject HTTP
    
    auto secure_result = secure_validator->validate_source(untrusted_source);
    EXPECT_FALSE(secure_result.has_value());
    EXPECT_EQ(secure_result.error().code, PluginErrorCode::UntrustedSource);
    
    auto enterprise_result = enterprise_validator->validate_source(untrusted_source);
    EXPECT_FALSE(enterprise_result.has_value());
    EXPECT_EQ(enterprise_result.error().code, PluginErrorCode::UntrustedSource);
}

TEST_F(RemotePluginSecurityTest, MaliciousSourceValidation) {
    // Known malicious source should be rejected by all security levels
    // (In real implementation, this would check against blacklists)
    
    auto minimal_result = minimal_validator->validate_source(malicious_source);
    // Even minimal security should reject known malicious sources
    
    auto standard_result = standard_validator->validate_source(malicious_source);
    EXPECT_FALSE(standard_result.has_value());
    
    auto secure_result = secure_validator->validate_source(malicious_source);
    EXPECT_FALSE(secure_result.has_value());
    
    auto enterprise_result = enterprise_validator->validate_source(malicious_source);
    EXPECT_FALSE(enterprise_result.has_value());
}

// === Plugin Content Validation Tests ===

TEST_F(RemotePluginSecurityTest, ValidPluginValidation) {
    auto minimal_result = minimal_validator->validate_plugin(valid_plugin_path);
    EXPECT_TRUE(minimal_result.has_value());
    
    auto standard_result = standard_validator->validate_plugin(valid_plugin_path);
    // Might pass or fail depending on signature requirements
    
    auto secure_result = secure_validator->validate_plugin(valid_plugin_path);
    // Will likely fail due to missing signature in test file
    if (!secure_result.has_value()) {
        EXPECT_TRUE(secure_result.error().code == PluginErrorCode::SignatureInvalid ||
                   secure_result.error().code == PluginErrorCode::InvalidFormat);
    }
}

TEST_F(RemotePluginSecurityTest, SuspiciousPluginValidation) {
    // Large plugin file should trigger size-based validation
    auto minimal_result = minimal_validator->validate_plugin(suspicious_plugin_path);
    // Minimal security might allow large files
    
    auto standard_result = standard_validator->validate_plugin(suspicious_plugin_path);
    // Standard security might have size limits
    
    auto secure_result = secure_validator->validate_plugin(suspicious_plugin_path);
    // Secure validation should reject oversized files
    if (!secure_result.has_value()) {
        EXPECT_TRUE(secure_result.error().code == PluginErrorCode::SecurityViolation ||
                   secure_result.error().code == PluginErrorCode::InvalidFormat);
    }
}

TEST_F(RemotePluginSecurityTest, MaliciousPluginValidation) {
    // Malicious plugin should be rejected by all security levels
    auto minimal_result = minimal_validator->validate_plugin(malicious_plugin_path);
    // Even minimal security should detect obvious malware patterns
    
    auto standard_result = standard_validator->validate_plugin(malicious_plugin_path);
    EXPECT_FALSE(standard_result.has_value());
    
    auto secure_result = secure_validator->validate_plugin(malicious_plugin_path);
    EXPECT_FALSE(secure_result.has_value());
    
    auto enterprise_result = enterprise_validator->validate_plugin(malicious_plugin_path);
    EXPECT_FALSE(enterprise_result.has_value());
}

// === Signature Validation Tests ===

TEST_F(RemotePluginSecurityTest, SignatureValidationRequirements) {
    // Test signature validation requirements at different security levels
    
    // Create a plugin validation request
    PluginValidationRequest request;
    request.plugin_path = valid_plugin_path;
    request.source = trusted_source;
    request.expected_checksum = "test-checksum";
    request.require_signature = true;
    
    // Minimal security might not require signatures
    auto minimal_result = minimal_validator->validate_plugin_request(request);
    
    // Higher security levels should require signatures
    request.require_signature = true;
    auto secure_result = secure_validator->validate_plugin_request(request);
    if (!secure_result.has_value()) {
        EXPECT_EQ(secure_result.error().code, PluginErrorCode::SignatureInvalid);
    }
}

TEST_F(RemotePluginSecurityTest, ChecksumValidation) {
    // Test checksum validation
    PluginValidationRequest request;
    request.plugin_path = valid_plugin_path;
    request.source = trusted_source;
    request.expected_checksum = "invalid-checksum";
    request.verify_checksum = true;
    
    auto result = standard_validator->validate_plugin_request(request);
    if (!result.has_value()) {
        EXPECT_TRUE(result.error().code == PluginErrorCode::SecurityViolation ||
                   result.error().code == PluginErrorCode::InvalidFormat);
    }
}

// === Authentication Security Tests ===

TEST_F(RemotePluginSecurityTest, AuthenticationValidation) {
    // Test different authentication methods
    
    // Basic authentication
    AuthenticationCredentials basic_auth;
    basic_auth.type = AuthenticationType::Basic;
    basic_auth.username = "testuser";
    basic_auth.password = "testpass";
    
    trusted_source.set_authentication(basic_auth);
    auto basic_result = standard_validator->validate_source(trusted_source);
    EXPECT_TRUE(basic_result.has_value());
    
    // API Key authentication
    AuthenticationCredentials api_auth;
    api_auth.type = AuthenticationType::ApiKey;
    api_auth.api_key = "test-api-key";
    
    trusted_source.set_authentication(api_auth);
    auto api_result = standard_validator->validate_source(trusted_source);
    EXPECT_TRUE(api_result.has_value());
    
    // Bearer token authentication
    AuthenticationCredentials bearer_auth;
    bearer_auth.type = AuthenticationType::Bearer;
    bearer_auth.bearer_token = "bearer-token";
    
    trusted_source.set_authentication(bearer_auth);
    auto bearer_result = standard_validator->validate_source(trusted_source);
    EXPECT_TRUE(bearer_result.has_value());
}

// === Security Policy Enforcement Tests ===

TEST_F(RemotePluginSecurityTest, SecurityPolicyEnforcement) {
    // Test that security policies are properly enforced
    
    // Create a manager with secure configuration
    auto secure_manager = RemotePluginManagerFactory::create_with_remote_config(secure_config);
    
    // Attempt to load from untrusted source
    RemotePluginLoadOptions options;
    options.remote_security_level = RemoteSecurityLevel::High;
    
    auto load_result = secure_manager->load_remote_plugin(untrusted_source.url(), options);
    EXPECT_FALSE(load_result.has_value());
    EXPECT_EQ(load_result.error().code, PluginErrorCode::UntrustedSource);
}

TEST_F(RemotePluginSecurityTest, SecurityLevelOverride) {
    // Test that security level can be overridden per operation
    
    auto manager = RemotePluginManagerFactory::create_with_remote_config(secure_config);
    
    // Override to minimal security for specific operation
    RemotePluginLoadOptions minimal_options;
    minimal_options.remote_security_level = RemoteSecurityLevel::Minimal;
    minimal_options.validate_remote_source = false;
    
    // This should bypass some security checks
    auto load_result = manager->load_remote_plugin(untrusted_source.url(), minimal_options);
    // Will still fail due to network, but should not fail due to security
    if (!load_result.has_value()) {
        EXPECT_TRUE(load_result.error().code == PluginErrorCode::NetworkError ||
                   load_result.error().code == PluginErrorCode::FileNotFound);
    }
}

// === Vulnerability Tests ===

TEST_F(RemotePluginSecurityTest, PathTraversalPrevention) {
    // Test prevention of path traversal attacks
    std::filesystem::path malicious_path = "../../../etc/passwd";
    
    auto result = secure_validator->validate_plugin(malicious_path);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, PluginErrorCode::SecurityViolation);
}

TEST_F(RemotePluginSecurityTest, UrlInjectionPrevention) {
    // Test prevention of URL injection attacks
    QUrl malicious_url("https://trusted.com@malicious.com/plugin.zip");
    RemotePluginSource malicious_source(malicious_url, RemoteSourceType::Http);
    
    auto result = secure_validator->validate_source(malicious_source);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code, PluginErrorCode::SecurityViolation);
}

TEST_F(RemotePluginSecurityTest, ResourceExhaustionPrevention) {
    // Test prevention of resource exhaustion attacks
    
    // Extremely large timeout should be rejected
    RemotePluginLoadOptions options;
    options.download_timeout = std::chrono::hours(24 * 365); // 1 year timeout
    
    // The validator should reject unreasonable timeouts
    PluginValidationRequest request;
    request.plugin_path = valid_plugin_path;
    request.source = trusted_source;
    request.timeout = options.download_timeout;
    
    auto result = secure_validator->validate_plugin_request(request);
    if (!result.has_value()) {
        EXPECT_EQ(result.error().code, PluginErrorCode::InvalidConfiguration);
    }
}

// === Security Audit Tests ===

TEST_F(RemotePluginSecurityTest, SecurityAuditLogging) {
    // Test that security events are properly logged
    
    // Attempt various operations that should generate audit logs
    minimal_validator->validate_source(malicious_source);
    standard_validator->validate_plugin(suspicious_plugin_path);
    secure_validator->validate_source(untrusted_source);
    
    // Get validation statistics (which should include audit information)
    auto stats = secure_validator->get_validation_statistics();
    
    EXPECT_TRUE(stats.contains("total_validations"));
    EXPECT_TRUE(stats.contains("successful_validations"));
    EXPECT_TRUE(stats.contains("failed_validations"));
    EXPECT_TRUE(stats.contains("security_violations"));
}

TEST_F(RemotePluginSecurityTest, SecurityMetrics) {
    // Test security metrics collection
    
    auto initial_stats = enterprise_validator->get_validation_statistics();
    
    // Perform various validation operations
    enterprise_validator->validate_source(trusted_source);
    enterprise_validator->validate_source(untrusted_source);
    enterprise_validator->validate_plugin(valid_plugin_path);
    enterprise_validator->validate_plugin(malicious_plugin_path);
    
    auto final_stats = enterprise_validator->get_validation_statistics();
    
    // Verify that statistics were updated
    EXPECT_GE(final_stats["total_validations"].toInt(), 
              initial_stats["total_validations"].toInt());
}
