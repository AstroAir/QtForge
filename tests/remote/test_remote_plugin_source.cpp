/**
 * @file test_remote_plugin_source.cpp
 * @brief Unit tests for RemotePluginSource class
 */

#include <gtest/gtest.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <qtplugin/remote/remote_plugin_source.hpp>

using namespace qtplugin;

class RemotePluginSourceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test data
        test_url = QUrl("https://plugins.example.com/api/v1");
        test_name = "Example Plugin Registry";
        test_type = RemoteSourceType::Registry;
    }

    QUrl test_url;
    QString test_name;
    RemoteSourceType test_type;
};

// === Basic Construction Tests ===

TEST_F(RemotePluginSourceTest, DefaultConstruction) {
    RemotePluginSource source(test_url);
    
    EXPECT_EQ(source.url(), test_url);
    EXPECT_EQ(source.type(), RemoteSourceType::Http);  // Auto-detected from HTTPS URL
    EXPECT_FALSE(source.name().isEmpty());
    EXPECT_TRUE(source.is_enabled());
    EXPECT_FALSE(source.id().isEmpty());
}

TEST_F(RemotePluginSourceTest, ConstructionWithParameters) {
    RemotePluginSource source(test_url, test_type, test_name);
    
    EXPECT_EQ(source.url(), test_url);
    EXPECT_EQ(source.type(), test_type);
    EXPECT_EQ(source.name(), test_name);
    EXPECT_TRUE(source.is_enabled());
    EXPECT_FALSE(source.id().isEmpty());
}

TEST_F(RemotePluginSourceTest, CopyConstruction) {
    RemotePluginSource original(test_url, test_type, test_name);
    RemotePluginSource copy(original);
    
    EXPECT_EQ(copy.url(), original.url());
    EXPECT_EQ(copy.type(), original.type());
    EXPECT_EQ(copy.name(), original.name());
    EXPECT_EQ(copy.is_enabled(), original.is_enabled());
    EXPECT_EQ(copy.id(), original.id());
}

TEST_F(RemotePluginSourceTest, Assignment) {
    RemotePluginSource original(test_url, test_type, test_name);
    RemotePluginSource assigned(QUrl("http://other.com"));
    
    assigned = original;
    
    EXPECT_EQ(assigned.url(), original.url());
    EXPECT_EQ(assigned.type(), original.type());
    EXPECT_EQ(assigned.name(), original.name());
    EXPECT_EQ(assigned.is_enabled(), original.is_enabled());
    EXPECT_EQ(assigned.id(), original.id());
}

// === Property Tests ===

TEST_F(RemotePluginSourceTest, UrlModification) {
    RemotePluginSource source(test_url);
    QUrl new_url("https://new.example.com/api");
    
    source.set_url(new_url);
    
    EXPECT_EQ(source.url(), new_url);
    // Type should be auto-updated
    EXPECT_EQ(source.type(), RemoteSourceType::Http);
}

TEST_F(RemotePluginSourceTest, TypeModification) {
    RemotePluginSource source(test_url);
    
    source.set_type(RemoteSourceType::Git);
    
    EXPECT_EQ(source.type(), RemoteSourceType::Git);
}

TEST_F(RemotePluginSourceTest, NameModification) {
    RemotePluginSource source(test_url);
    QString new_name = "New Plugin Source";
    
    source.set_name(new_name);
    
    EXPECT_EQ(source.name(), new_name);
}

TEST_F(RemotePluginSourceTest, EnabledState) {
    RemotePluginSource source(test_url);
    
    EXPECT_TRUE(source.is_enabled());
    
    source.set_enabled(false);
    EXPECT_FALSE(source.is_enabled());
    
    source.set_enabled(true);
    EXPECT_TRUE(source.is_enabled());
}

// === Authentication Tests ===

TEST_F(RemotePluginSourceTest, BasicAuthentication) {
    RemotePluginSource source(test_url);
    AuthenticationCredentials auth;
    auth.type = AuthenticationType::Basic;
    auth.username = "testuser";
    auth.password = "testpass";
    
    source.set_authentication(auth);
    
    auto retrieved_auth = source.authentication();
    EXPECT_EQ(retrieved_auth.type, AuthenticationType::Basic);
    EXPECT_EQ(retrieved_auth.username, "testuser");
    EXPECT_EQ(retrieved_auth.password, "testpass");
}

TEST_F(RemotePluginSourceTest, ApiKeyAuthentication) {
    RemotePluginSource source(test_url);
    AuthenticationCredentials auth;
    auth.type = AuthenticationType::ApiKey;
    auth.api_key = "test-api-key-123";
    auth.api_key_header = "X-API-Key";
    
    source.set_authentication(auth);
    
    auto retrieved_auth = source.authentication();
    EXPECT_EQ(retrieved_auth.type, AuthenticationType::ApiKey);
    EXPECT_EQ(retrieved_auth.api_key, "test-api-key-123");
    EXPECT_EQ(retrieved_auth.api_key_header, "X-API-Key");
}

TEST_F(RemotePluginSourceTest, BearerTokenAuthentication) {
    RemotePluginSource source(test_url);
    AuthenticationCredentials auth;
    auth.type = AuthenticationType::Bearer;
    auth.bearer_token = "bearer-token-xyz";
    
    source.set_authentication(auth);
    
    auto retrieved_auth = source.authentication();
    EXPECT_EQ(retrieved_auth.type, AuthenticationType::Bearer);
    EXPECT_EQ(retrieved_auth.bearer_token, "bearer-token-xyz");
}

// === Configuration Tests ===

TEST_F(RemotePluginSourceTest, ConfigurationOptions) {
    RemotePluginSource source(test_url);
    
    source.set_config_option("timeout", 60);
    source.set_config_option("max_retries", 5);
    source.set_config_option("verify_ssl", true);
    
    EXPECT_EQ(source.get_config_option("timeout").toInt(), 60);
    EXPECT_EQ(source.get_config_option("max_retries").toInt(), 5);
    EXPECT_EQ(source.get_config_option("verify_ssl").toBool(), true);
}

// === Validation Tests ===

TEST_F(RemotePluginSourceTest, ValidSource) {
    RemotePluginSource source(test_url, test_type, test_name);
    
    auto validation_result = source.validate();
    
    EXPECT_TRUE(validation_result.has_value());
}

TEST_F(RemotePluginSourceTest, InvalidUrl) {
    QUrl invalid_url("not-a-valid-url");
    RemotePluginSource source(invalid_url);
    
    auto validation_result = source.validate();
    
    EXPECT_FALSE(validation_result.has_value());
    EXPECT_EQ(validation_result.error().code, PluginErrorCode::InvalidConfiguration);
}

// === Serialization Tests ===

TEST_F(RemotePluginSourceTest, JsonSerialization) {
    RemotePluginSource original(test_url, test_type, test_name);
    original.set_enabled(false);
    
    AuthenticationCredentials auth;
    auth.type = AuthenticationType::ApiKey;
    auth.api_key = "test-key";
    original.set_authentication(auth);
    
    original.set_config_option("timeout", 30);
    
    QJsonObject json = original.to_json();
    
    EXPECT_EQ(json["url"].toString(), test_url.toString());
    EXPECT_EQ(json["type"].toInt(), static_cast<int>(test_type));
    EXPECT_EQ(json["name"].toString(), test_name);
    EXPECT_EQ(json["enabled"].toBool(), false);
    
    // Check authentication
    QJsonObject auth_json = json["authentication"].toObject();
    EXPECT_EQ(auth_json["type"].toInt(), static_cast<int>(AuthenticationType::ApiKey));
    EXPECT_EQ(auth_json["api_key"].toString(), "test-key");
    
    // Check configuration
    QJsonObject config_json = json["configuration"].toObject();
    EXPECT_EQ(config_json["timeout"].toInt(), 30);
}

TEST_F(RemotePluginSourceTest, JsonDeserialization) {
    QJsonObject json;
    json["url"] = test_url.toString();
    json["type"] = static_cast<int>(test_type);
    json["name"] = test_name;
    json["enabled"] = false;
    
    QJsonObject auth_json;
    auth_json["type"] = static_cast<int>(AuthenticationType::Basic);
    auth_json["username"] = "user";
    auth_json["password"] = "pass";
    json["authentication"] = auth_json;
    
    QJsonObject config_json;
    config_json["timeout"] = 45;
    json["configuration"] = config_json;
    
    RemotePluginSource source = RemotePluginSource::from_json(json);
    
    EXPECT_EQ(source.url(), test_url);
    EXPECT_EQ(source.type(), test_type);
    EXPECT_EQ(source.name(), test_name);
    EXPECT_FALSE(source.is_enabled());
    
    auto auth = source.authentication();
    EXPECT_EQ(auth.type, AuthenticationType::Basic);
    EXPECT_EQ(auth.username, "user");
    EXPECT_EQ(auth.password, "pass");
    
    EXPECT_EQ(source.get_config_option("timeout").toInt(), 45);
}

TEST_F(RemotePluginSourceTest, RoundTripSerialization) {
    RemotePluginSource original(test_url, test_type, test_name);
    original.set_enabled(false);
    
    AuthenticationCredentials auth;
    auth.type = AuthenticationType::Bearer;
    auth.bearer_token = "token123";
    original.set_authentication(auth);
    
    original.set_config_option("max_retries", 3);
    original.set_config_option("verify_ssl", false);
    
    QJsonObject json = original.to_json();
    RemotePluginSource deserialized = RemotePluginSource::from_json(json);
    
    EXPECT_EQ(deserialized.url(), original.url());
    EXPECT_EQ(deserialized.type(), original.type());
    EXPECT_EQ(deserialized.name(), original.name());
    EXPECT_EQ(deserialized.is_enabled(), original.is_enabled());
    
    auto orig_auth = original.authentication();
    auto deser_auth = deserialized.authentication();
    EXPECT_EQ(deser_auth.type, orig_auth.type);
    EXPECT_EQ(deser_auth.bearer_token, orig_auth.bearer_token);
    
    EXPECT_EQ(deserialized.get_config_option("max_retries").toInt(), 3);
    EXPECT_EQ(deserialized.get_config_option("verify_ssl").toBool(), false);
}

// === Utility Tests ===

TEST_F(RemotePluginSourceTest, SourceTypeDetection) {
    EXPECT_EQ(RemotePluginSource::detect_source_type(QUrl("http://example.com")), RemoteSourceType::Http);
    EXPECT_EQ(RemotePluginSource::detect_source_type(QUrl("https://example.com")), RemoteSourceType::Http);
    EXPECT_EQ(RemotePluginSource::detect_source_type(QUrl("git://github.com/user/repo")), RemoteSourceType::Git);
    EXPECT_EQ(RemotePluginSource::detect_source_type(QUrl("git+https://github.com/user/repo")), RemoteSourceType::Git);
    EXPECT_EQ(RemotePluginSource::detect_source_type(QUrl("ftp://ftp.example.com")), RemoteSourceType::Ftp);
    EXPECT_EQ(RemotePluginSource::detect_source_type(QUrl("registry://npm.example.com")), RemoteSourceType::Registry);
    EXPECT_EQ(RemotePluginSource::detect_source_type(QUrl("custom://special.protocol")), RemoteSourceType::Custom);
}

TEST_F(RemotePluginSourceTest, SupportedUrlCheck) {
    EXPECT_TRUE(RemotePluginSource::is_supported_url(QUrl("https://example.com")));
    EXPECT_TRUE(RemotePluginSource::is_supported_url(QUrl("git://github.com/user/repo")));
    EXPECT_TRUE(RemotePluginSource::is_supported_url(QUrl("ftp://ftp.example.com")));
    EXPECT_FALSE(RemotePluginSource::is_supported_url(QUrl("invalid-url")));
    EXPECT_FALSE(RemotePluginSource::is_supported_url(QUrl()));
}

TEST_F(RemotePluginSourceTest, SupportedSchemes) {
    auto schemes = RemotePluginSource::supported_schemes();
    
    EXPECT_TRUE(std::find(schemes.begin(), schemes.end(), "http") != schemes.end());
    EXPECT_TRUE(std::find(schemes.begin(), schemes.end(), "https") != schemes.end());
    EXPECT_TRUE(std::find(schemes.begin(), schemes.end(), "git") != schemes.end());
    EXPECT_TRUE(std::find(schemes.begin(), schemes.end(), "ftp") != schemes.end());
    EXPECT_TRUE(std::find(schemes.begin(), schemes.end(), "registry") != schemes.end());
}

// === Comparison Tests ===

TEST_F(RemotePluginSourceTest, EqualityComparison) {
    RemotePluginSource source1(test_url, test_type, test_name);
    RemotePluginSource source2(test_url, test_type, test_name);
    RemotePluginSource source3(QUrl("https://different.com"), test_type, test_name);
    
    EXPECT_TRUE(source1 == source2);
    EXPECT_FALSE(source1 == source3);
    EXPECT_TRUE(source1 != source3);
    EXPECT_FALSE(source1 != source2);
}

TEST_F(RemotePluginSourceTest, StringRepresentation) {
    RemotePluginSource source(test_url, test_type, test_name);
    
    QString str = source.to_string();
    
    EXPECT_TRUE(str.contains(test_name));
    EXPECT_TRUE(str.contains(test_url.toString()));
}
