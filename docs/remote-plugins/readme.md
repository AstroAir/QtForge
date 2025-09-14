# QtForge Remote Plugin System

## Overview

The QtForge Remote Plugin System extends the existing local plugin architecture to support downloading, validating, and executing plugins from remote repositories. It provides enterprise-grade security, caching, version management, and fallback mechanisms while maintaining full backward compatibility with the existing plugin system.

## Key Features

### 🔐 Security & Trust

- **Digital Signature Verification**: RSA and ECDSA signature support
- **Certificate Chain Validation**: Full X.509 certificate verification
- **Trust Store Management**: Centralized publisher trust management
- **Sandboxed Execution**: Isolated plugin execution environments
- **Network Security**: HTTPS-only downloads with configurable domain whitelisting

### 📦 Plugin Management

- **Multi-Repository Support**: Connect to multiple plugin repositories
- **Version Management**: Automatic updates and version conflict resolution
- **Dependency Resolution**: Automatic dependency downloading and installation
- **Intelligent Caching**: Local caching with configurable size limits and expiration

### 🔄 Integration & Compatibility

- **Seamless Integration**: Works alongside existing local plugins
- **Fallback Mechanisms**: Automatic fallback to local/cached versions
- **Backward Compatibility**: Existing plugin code works unchanged
- **Unified API**: Single interface for both local and remote plugins

### 🚀 Performance & Reliability

- **Concurrent Downloads**: Multiple simultaneous plugin downloads
- **Network Resilience**: Retry mechanisms and circuit breakers
- **Resource Optimization**: Efficient memory and disk usage
- **Cross-Platform**: Full support for Windows, Linux, and macOS

## Architecture Overview

```txt
┌─────────────────────────────────────────────────────────────┐
│                 Unified Plugin Manager                      │
├─────────────────┬───────────────────┬───────────────────────┤
│  Local Plugins  │  Remote Plugins   │   Integration Layer   │
│                 │                   │                       │
│ ┌─────────────┐ │ ┌───────────────┐ │ ┌───────────────────┐ │
│ │   Plugin    │ │ │   Remote      │ │ │   Load Strategy   │ │
│ │   Manager   │ │ │   Plugin      │ │ │   Management      │ │
│ │             │ │ │   Manager     │ │ │                   │ │
│ └─────────────┘ │ └───────────────┘ │ └───────────────────┘ │
│                 │                   │                       │
│                 │ ┌───────────────┐ │ ┌───────────────────┐ │
│                 │ │   Security    │ │ │   Repository      │ │
│                 │ │   Manager     │ │ │   Manager         │ │
│                 │ │               │ │ │                   │ │
│                 │ └───────────────┘ │ └───────────────────┘ │
│                 │                   │                       │
│                 │ ┌───────────────┐ │ ┌───────────────────┐ │
│                 │ │   Cache       │ │ │   Update          │ │
│                 │ │   Manager     │ │ │   Manager         │ │
│                 │ │               │ │ │                   │ │
│                 │ └───────────────┘ │ └───────────────────┘ │
└─────────────────┴───────────────────┴───────────────────────┘
```

## Quick Start Guide

### 1. Basic Setup

```cpp
#include <qtplugin/remote/integration/unified_plugin_manager.hpp>

using namespace qtplugin::integration;

// Initialize the unified plugin manager
UnifiedPluginManager& manager = UnifiedPluginManager::instance();

// Initialize with local directories and remote cache
auto result = manager.initialize(
    {"./plugins", "/usr/local/lib/qtforge/plugins"},  // Local plugin directories
    "~/.cache/qtforge/remote_plugins",               // Remote plugin cache
    {}  // Default security configuration
);

if (!result) {
    qCritical() << "Failed to initialize plugin manager:" << result.error().message();
    return -1;
}
```

### 2. Add Plugin Repositories

```cpp
// Add a remote repository
remote::RemotePluginRepository repo;
repo.id = "official_qtforge_repo";
repo.name = "Official QtForge Repository";
repo.base_url = QUrl("https://plugins.qtforge.org/api/v1");
repo.is_enabled = true;
repo.default_trust_level = remote::security::PublisherTrustLevel::Verified;

auto repo_result = manager.add_remote_repository(repo);
if (!repo_result) {
    qWarning() << "Failed to add repository:" << repo_result.error().message();
}
```

### 3. Load Plugins

```cpp
// Load plugin with unified options
UnifiedPluginLoadOptions options;
options.strategy = LoadStrategy::PreferRemote;  // Prefer remote over local
options.allow_fallback = true;                 // Allow fallback to local
options.cache_remote = true;                   // Cache downloaded plugins
options.check_for_updates = true;              // Check for newer versions

// Load the plugin asynchronously
auto future = manager.load_plugin("com.example.weather_service", options);

// Handle the result
auto watcher = new QFutureWatcher<expected<std::shared_ptr<IPlugin>, PluginError>>(this);
connect(watcher, &QFutureWatcherBase::finished, [=]() {
    auto result = watcher->result();
    if (result) {
        auto plugin = result.value();
        qDebug() << "Plugin loaded:" << QString::fromStdString(plugin->name());

        // Use the plugin
        auto weather_result = plugin->execute_command("get_weather", {{"location", "London"}});
        if (weather_result) {
            qDebug() << "Weather data:" << weather_result.value();
        }
    } else {
        qWarning() << "Plugin load failed:" << result.error().message();
    }
    watcher->deleteLater();
});
watcher->setFuture(future);
```

### 4. Search and Install Plugins

```cpp
// Search for plugins
auto search_future = manager.search_plugins("weather", "productivity");

connect(search_watcher, &QFutureWatcherBase::finished, [=]() {
    auto results = search_watcher->result();
    for (const auto& plugin_info : results) {
        qDebug() << "Found plugin:" << plugin_info.name
                 << "Version:" << plugin_info.version.toString()
                 << "Source:" << static_cast<int>(plugin_info.source);
    }
});

// Install a specific plugin
auto install_future = manager.install_plugin("com.example.advanced_calculator",
                                            QVersionNumber(2, 1, 0));
```

## Security Configuration

### Trust Store Management

```cpp
#include <qtplugin/remote/security/remote_security_manager.hpp>

using namespace qtplugin::remote::security;

// Get the security manager
RemoteSecurityManager& security = RemoteSecurityManager::instance();

// Add trusted publisher
QSslCertificate publisher_cert = loadCertificateFromFile("publisher.crt");
auto trust_result = security.get_trust_store()->add_trusted_publisher(
    "com.example.publisher",
    publisher_cert,
    PublisherTrustLevel::Trusted
);
```

### Security Configuration

```cpp
RemoteSecurityConfig security_config;
security_config.security_level = RemoteSecurityLevel::Strict;
security_config.require_signatures = true;
security_config.allow_self_signed = false;
security_config.check_certificate_revocation = true;
security_config.enable_sandbox = true;
security_config.allow_http_sources = false;  // HTTPS only
security_config.strict_tls_verification = true;

// Network security
security_config.network_timeout = std::chrono::seconds{30};
security_config.max_redirects = 3;
security_config.allowed_domains = {"plugins.qtforge.org", "secure-plugins.example.com"};
security_config.blocked_domains = {"malicious-site.com"};

// Trust settings
security_config.minimum_trust_level = PublisherTrustLevel::Verified;
security_config.allow_untrusted_development = false;

// Initialize security manager with configuration
auto security_result = security.initialize(security_config);
```

### Signature Verification

```cpp
// Verify plugin signature manually
RemotePluginSignature signature_info;
signature_info.algorithm = "RSA-SHA256";
signature_info.signature = loadSignatureFromFile("plugin.sig");
signature_info.certificate = loadCertificateFromFile("publisher.crt");
signature_info.publisher_id = "com.example.publisher";

QByteArray plugin_data = loadPluginFromFile("plugin.dll");
auto validation_result = security.validate_plugin_data(plugin_data, signature_info);

if (validation_result.is_valid) {
    qDebug() << "Plugin signature is valid";
    qDebug() << "Validated security level:" << static_cast<int>(validation_result.validated_level);
} else {
    qWarning() << "Plugin signature validation failed:";
    for (const auto& error : validation_result.errors) {
        qWarning() << "  -" << error;
    }
}
```

## Plugin Repository Protocol

### Repository API Endpoints

Remote plugin repositories should implement the following REST API:

#### 1. Repository Information

```
GET /api/v1/info
```

Response:

```json
{
  "id": "official_qtforge_repo",
  "name": "Official QtForge Repository",
  "version": "1.0",
  "description": "Official repository for QtForge plugins",
  "capabilities": {
    "search": true,
    "categories": true,
    "versions": true,
    "dependencies": true
  }
}
```

#### 2. Plugin Catalog

```
GET /api/v1/plugins?category=<category>&search=<query>&page=<page>&limit=<limit>
```

Response:

```json
{
  "plugins": [
    {
      "id": "com.example.weather_service",
      "name": "Weather Service Plugin",
      "version": "1.2.0",
      "description": "Advanced weather service integration",
      "author": "Example Corp",
      "license": "MIT",
      "category": "productivity",
      "tags": ["weather", "api", "service"],
      "download_url": "https://plugins.qtforge.org/download/weather_service/1.2.0",
      "signature_url": "https://plugins.qtforge.org/signature/weather_service/1.2.0",
      "metadata_url": "https://plugins.qtforge.org/metadata/weather_service/1.2.0",
      "checksum_sha256": "a1b2c3d4e5f6...",
      "size_bytes": 2048576,
      "published_date": "2024-01-15T10:30:00Z",
      "last_updated": "2024-01-20T14:45:00Z",
      "required_dependencies": ["qtforge.core >= 3.2.0"],
      "optional_dependencies": ["qtforge.network >= 1.1.0"],
      "min_qtforge_version": "3.2.0",
      "publisher_id": "com.example.publisher",
      "requires_signature": true
    }
  ],
  "total_count": 1,
  "page": 1,
  "limit": 50
}
```

#### 3. Plugin Metadata

```
GET /api/v1/plugins/<plugin_id>
GET /api/v1/plugins/<plugin_id>/<version>
```

Response: Single plugin metadata object (same format as catalog entry)

#### 4. Plugin Download

```
GET /download/<plugin_id>/<version>
```

Response: Binary plugin file

#### 5. Signature Download

```
GET /signature/<plugin_id>/<version>
```

Response: Digital signature file

### Plugin Metadata Format

Plugin metadata files should follow this format:

```json
{
  "id": "com.example.weather_service",
  "name": "Weather Service Plugin",
  "version": "1.2.0",
  "description": "Advanced weather service integration with caching and rate limiting",
  "author": "Example Corporation",
  "license": "MIT",
  "website": "https://github.com/example/weather-service-plugin",
  "category": "productivity",
  "tags": ["weather", "api", "service", "remote"],

  "plugin_interface": "qtplugin.IPlugin",
  "plugin_type": "service",

  "capabilities": ["Service", "Configuration", "Networking", "Caching"],

  "priority": "Normal",
  "thread_safe": true,
  "thread_model": "multi-threaded",

  "dependencies": {
    "required": ["qtforge.core >= 3.2.0", "qtforge.network >= 1.1.0"],
    "optional": ["qtforge.ui >= 2.0.0"]
  },

  "commands": [
    {
      "name": "get_weather",
      "description": "Get current weather for location",
      "parameters": {
        "location": {
          "type": "string",
          "required": true,
          "description": "Location name or coordinates"
        }
      }
    },
    {
      "name": "get_forecast",
      "description": "Get weather forecast",
      "parameters": {
        "location": {
          "type": "string",
          "required": true
        },
        "days": {
          "type": "integer",
          "required": false,
          "default": 5,
          "min": 1,
          "max": 7
        }
      }
    }
  ],

  "configuration": {
    "api_key": {
      "type": "string",
      "required": true,
      "description": "Weather service API key"
    },
    "cache_duration": {
      "type": "integer",
      "default": 30,
      "min": 5,
      "max": 120,
      "description": "Cache duration in minutes"
    },
    "rate_limit": {
      "type": "integer",
      "default": 1000,
      "description": "API requests per hour"
    }
  },

  "security": {
    "trust_level": "verified",
    "signature_required": true,
    "permissions": ["network_access", "file_system_read", "file_system_write"],
    "sandbox": false
  },

  "build_info": {
    "qt_version": "6.2+",
    "cmake_minimum": "3.21",
    "cpp_standard": "20",
    "compiler_features": ["concepts", "ranges"],
    "target_platforms": ["windows", "linux", "macos"]
  },

  "publisher": {
    "id": "com.example.publisher",
    "name": "Example Corporation",
    "email": "plugins@example.com",
    "website": "https://example.com",
    "certificate_fingerprint": "sha256:1234567890abcdef..."
  },

  "signature": {
    "algorithm": "RSA-SHA256",
    "signature": "base64-encoded-signature",
    "certificate": "base64-encoded-certificate",
    "timestamp": "2024-01-15T10:30:00Z"
  }
}
```

## Development Guide

### Creating Remote Plugins

1. **Implement the IPlugin Interface**: Follow the existing plugin interface
2. **Add Security Metadata**: Include signature and trust information
3. **Configure Build System**: Support remote deployment
4. **Test Thoroughly**: Use the comprehensive test suite

Example CMakeLists.txt for remote plugin:

```cmake
cmake_minimum_required(VERSION 3.21)
project(WeatherServicePlugin VERSION 1.2.0)

# Find QtForge
find_package(QtForge REQUIRED COMPONENTS Core Remote)

# Create plugin
qtforge_add_plugin(weather_service
    SOURCES
        weather_service_plugin.cpp
        weather_service_plugin.hpp
    METADATA
        weather_service_plugin.json
    REMOTE_ENABLED
    SIGNATURE_REQUIRED
    PUBLISHER_ID "com.example.publisher"
)

# Link dependencies
target_link_libraries(weather_service
    QtForge::Core
    QtForge::Remote
    Qt6::Network
)

# Configure for remote deployment
qtforge_configure_remote_plugin(weather_service
    REPOSITORY_URL "https://plugins.qtforge.org"
    CATEGORY "productivity"
    TAGS "weather" "api" "service"
)
```

### Plugin Signing

```bash
# Generate signing key pair
qtforge-keygen --output publisher_key.pem --algorithm RSA --bits 2048

# Generate certificate
qtforge-cert --key publisher_key.pem --subject "CN=Example Corp" --output publisher.crt

# Sign plugin
qtforge-sign --plugin weather_service.dll --key publisher_key.pem --cert publisher.crt --output weather_service.sig

# Verify signature
qtforge-verify --plugin weather_service.dll --signature weather_service.sig --cert publisher.crt
```

## Testing

### Running the Test Suite

```bash
# Build tests
cmake -DQTFORGE_BUILD_REMOTE_TESTS=ON ..
make -j

# Run all remote plugin tests
./tests/remote/remote_plugin_test_suite

# Run specific test categories
./tests/remote/remote_plugin_test_suite --category security
./tests/remote/remote_plugin_test_suite --category performance
./tests/remote/remote_plugin_test_suite --category network

# Generate test report
./tests/remote/remote_plugin_test_suite --report test_results.xml
```

### Test Coverage

The test suite includes:

- **Security Tests**: Signature verification, certificate validation, trust management
- **Performance Tests**: Loading speed, memory usage, concurrent operations
- **Network Tests**: Connection failures, timeouts, malformed responses
- **Integration Tests**: Local/remote interaction, fallback mechanisms
- **Cross-Platform Tests**: Windows, Linux, macOS compatibility

## Best Practices

### Security

1. **Always Verify Signatures**: Never load unsigned remote plugins in production
2. **Use Trusted Publishers**: Maintain a curated list of trusted publishers
3. **Enable Sandboxing**: Use sandboxed execution for untrusted plugins
4. **Regular Updates**: Keep trust stores and security configurations updated
5. **Monitor Activity**: Log all remote plugin activities

### Performance

1. **Enable Caching**: Use local caching to reduce network requests
2. **Limit Concurrent Downloads**: Avoid overwhelming the network
3. **Use Appropriate Timeouts**: Set reasonable timeouts for network operations
4. **Monitor Resource Usage**: Track memory and disk usage
5. **Clean Up Regularly**: Remove expired cache entries

### Development

1. **Test Thoroughly**: Use the comprehensive test suite
2. **Document APIs**: Provide clear documentation for plugin interfaces
3. **Handle Errors Gracefully**: Implement proper error handling and fallbacks
4. **Follow Security Guidelines**: Adhere to security best practices
5. **Version Carefully**: Use semantic versioning for compatibility

## Troubleshooting

### Common Issues

#### Plugin Loading Failures

```cpp
// Check plugin availability
auto info = manager.get_plugin_info("com.example.weather_service");
if (!info) {
    qDebug() << "Plugin not found in any repository";
    return;
}

// Check security validation
if (info->source == PluginSource::Remote && !info->remote_metadata) {
    qDebug() << "Remote metadata not available";
    return;
}

// Check trust level
auto security_manager = manager.get_security_manager();
auto publisher_trust = security_manager->get_trust_store()
                       ->get_trust_level(info->remote_metadata->publisher_id);
if (publisher_trust < PublisherTrustLevel::Basic) {
    qDebug() << "Publisher not trusted";
    return;
}
```

#### Network Issues

```cpp
// Check network connectivity
auto security_manager = manager.get_security_manager();
if (!security_manager->is_url_allowed(QUrl("https://plugins.qtforge.org"))) {
    qDebug() << "Repository URL not allowed";
    return;
}

// Configure network timeout
RemoteSecurityConfig config = security_manager->get_config();
config.network_timeout = std::chrono::seconds{60};  // Increase timeout
security_manager->update_config(config);
```

#### Cache Issues

```cpp
// Check cache status
auto cache = manager.get_remote_manager()->get_cache();
auto cache_stats = cache->get_cache_statistics();
qDebug() << "Cache size:" << cache_stats["total_size"].toInt();
qDebug() << "Cache entries:" << cache_stats["entry_count"].toInt();

// Clear cache if needed
cache->clear_cache();
```

### Debugging

Enable debug logging:

```cpp
// Enable detailed logging
QLoggingCategory::setFilterRules("qtforge.remote.debug=true\n"
                                "qtforge.security.debug=true\n"
                                "qtforge.network.debug=true");

// Set log file
QLoggingCategory remoteCategory("qtforge.remote");
qSetMessagePattern("[%{time yyyyMMdd h:mm:ss.zzz}] %{category} %{type}: %{message}");
```

## Migration Guide

### From Local-Only to Remote-Enabled

1. **Update Dependencies**: Add QtForge Remote components
2. **Configure Security**: Set up trust store and security policies
3. **Add Repositories**: Configure remote plugin repositories
4. **Update Loading Code**: Use UnifiedPluginManager instead of PluginManager
5. **Test Thoroughly**: Verify both local and remote functionality

Example migration:

```cpp
// Before (local only)
qtplugin::PluginManager manager;
auto result = manager.load_plugin("./plugins/weather_service.dll");

// After (unified)
qtplugin::integration::UnifiedPluginManager& manager =
    UnifiedPluginManager::instance();

UnifiedPluginLoadOptions options;
options.strategy = LoadStrategy::PreferLocal;  // Maintain existing behavior
options.allow_fallback = true;

auto future = manager.load_plugin("com.example.weather_service", options);
// Handle future result...
```

## API Reference

For detailed API documentation, see:

- [Remote Security Manager API](api/remote_security_manager.md)
- [Remote Plugin Manager API](api/remote_plugin_manager.md)
- [Unified Plugin Manager API](api/unified_plugin_manager.md)
- [Plugin Repository Protocol](api/repository_protocol.md)

## License

The QtForge Remote Plugin System is released under the MIT License. See [LICENSE](../LICENSE) for details.

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.

For bug reports and feature requests, please use the [GitHub Issues](https://github.com/QtForge/QtPlugin/issues) tracker.
