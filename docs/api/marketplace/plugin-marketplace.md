# PluginMarketplace API Reference

!!! warning "Experimental Feature"
**Status**: Experimental  
 **Since**: QtForge v3.2.0  
 **Note**: API complete, core implementation in development. Use for testing and feedback.

## Overview

The PluginMarketplace provides a comprehensive plugin marketplace system for discovering, installing, and managing plugins from remote repositories. It includes plugin search, installation management, rating systems, and security verification.

### Key Features

- **Plugin Discovery**: Search and browse plugins from marketplace repositories
- **Installation Management**: Download, install, update, and uninstall plugins
- **Rating and Reviews**: Community-driven plugin rating and review system
- **Security Verification**: Plugin signature verification and security scanning
- **Multiple Marketplaces**: Support for multiple marketplace sources
- **Offline Caching**: Local caching for improved performance

### Use Cases

- **Plugin Distribution**: Distribute plugins through centralized marketplaces
- **Plugin Discovery**: Find and explore available plugins
- **Automated Updates**: Keep plugins up-to-date automatically
- **Community Feedback**: Share and read plugin reviews and ratings
- **Enterprise Deployment**: Manage plugin deployment in enterprise environments

## Quick Start

```cpp
#include <qtplugin/marketplace/plugin_marketplace.hpp>

using namespace qtplugin;

// Create marketplace client
auto marketplace = std::make_shared<PluginMarketplace>("https://plugins.qtforge.org");

// Initialize with optional API key
auto init_result = marketplace->initialize("your-api-key");
if (!init_result) {
    qDebug() << "Failed to initialize marketplace:" << init_result.error().message();
    return;
}

// Search for plugins
SearchFilters filters;
filters.query = "data processing";
filters.categories = {"Data", "Analytics"};
filters.min_rating = 4.0;
filters.verified_only = true;

auto search_result = marketplace->search_plugins(filters);
if (search_result) {
    auto plugins = search_result.value();
    for (const auto& plugin : plugins) {
        qDebug() << "Found plugin:" << plugin.name << "by" << plugin.author;
    }

    // Install a plugin
    if (!plugins.empty()) {
        auto install_result = marketplace->install_plugin(plugins[0].id);
        if (install_result) {
            QString installation_id = install_result.value();
            qDebug() << "Installation started:" << installation_id;
        }
    }
}
```

## Data Structures

### MarketplacePlugin

```cpp
struct MarketplacePlugin {
    QString id;                     ///< Unique plugin identifier
    QString name;                   ///< Plugin name
    QString description;            ///< Plugin description
    QString author;                 ///< Plugin author
    QString version;                ///< Current version
    QString license;                ///< License type
    QStringList categories;         ///< Plugin categories
    QStringList tags;               ///< Plugin tags
    double rating{0.0};             ///< Average rating (0-5)
    int review_count{0};            ///< Number of reviews
    int download_count{0};          ///< Download count
    QString download_url;           ///< Download URL
    QString checksum;               ///< File checksum for verification
    qint64 file_size{0};           ///< File size in bytes
    QDateTime created_at;           ///< Creation timestamp
    QDateTime updated_at;           ///< Last update timestamp
    bool verified{false};           ///< Whether plugin is verified
    bool free{true};                ///< Whether plugin is free
    double price{0.0};              ///< Plugin price (if not free)
    QString currency{"USD"};        ///< Price currency
    QJsonObject metadata;           ///< Additional metadata

    QJsonObject to_json() const;
    static MarketplacePlugin from_json(const QJsonObject& json);
};
```

### SearchFilters

```cpp
struct SearchFilters {
    QString query;                  ///< Search query string
    QStringList categories;         ///< Filter by categories
    QStringList tags;               ///< Filter by tags
    QString author;                 ///< Filter by author
    QString license;                ///< Filter by license
    double min_rating{0.0};         ///< Minimum rating filter
    bool verified_only{false};      ///< Show only verified plugins
    bool free_only{false};          ///< Show only free plugins
    QString sort_by{"relevance"};   ///< Sort criteria
    bool ascending{false};          ///< Sort order
    int limit{50};                  ///< Maximum results
    int offset{0};                  ///< Result offset for pagination

    QJsonObject to_json() const;
    static SearchFilters from_json(const QJsonObject& json);
};
```

### PluginReview

```cpp
struct PluginReview {
    QString id;                     ///< Review identifier
    QString plugin_id;              ///< Plugin being reviewed
    QString author;                 ///< Review author
    double rating{0.0};             ///< Review rating (0-5)
    QString title;                  ///< Review title
    QString content;                ///< Review content
    QDateTime created_at;           ///< Review timestamp
    int helpful_count{0};           ///< Number of helpful votes

    QJsonObject to_json() const;
    static PluginReview from_json(const QJsonObject& json);
};
```

### InstallationProgress

```cpp
struct InstallationProgress {
    QString plugin_id;              ///< Plugin being installed
    QString operation;              ///< Current operation
    double progress{0.0};           ///< Progress percentage (0.0-1.0)
    QString status_message;         ///< Status message
    qint64 bytes_downloaded{0};     ///< Bytes downloaded
    qint64 total_bytes{0};          ///< Total bytes to download

    QJsonObject to_json() const;
};
```

## Class: PluginMarketplace

Main marketplace client for interacting with plugin repositories.

### Constructor

```cpp
explicit PluginMarketplace(const QString& marketplace_url = "https://plugins.qtforge.org",
                          QObject* parent = nullptr);
```

Creates a marketplace client for the specified URL.

**Parameters:**

- `marketplace_url` - Marketplace API base URL
- `parent` - Qt parent object

### Initialization

#### `initialize()`

```cpp
qtplugin::expected<void, PluginError> initialize(const QString& api_key = {});
```

Initializes the marketplace client.

**Parameters:**

- `api_key` - Optional API key for authenticated requests

**Returns:**

- `expected<void, PluginError>` - Success or error

**Note:** Currently returns success immediately. Full implementation pending.

### Plugin Discovery

#### `search_plugins()`

```cpp
qtplugin::expected<std::vector<MarketplacePlugin>, PluginError> search_plugins(
    const SearchFilters& filters = {});
```

Searches for plugins in the marketplace.

**Parameters:**

- `filters` - Search filters and criteria

**Returns:**

- `expected<std::vector<MarketplacePlugin>, PluginError>` - Search results or error

**Note:** Currently returns empty results. Implementation in progress.

**Example:**

```cpp
SearchFilters filters;
filters.query = "image processing";
filters.categories = {"Graphics", "Media"};
filters.min_rating = 3.5;
filters.sort_by = "rating";
filters.limit = 20;

auto results = marketplace->search_plugins(filters);
if (results) {
    for (const auto& plugin : results.value()) {
        qDebug() << plugin.name << "- Rating:" << plugin.rating;
    }
}
```

#### `get_plugin_details()`

```cpp
qtplugin::expected<MarketplacePlugin, PluginError> get_plugin_details(const QString& plugin_id);
```

Gets detailed information about a specific plugin.

**Parameters:**

- `plugin_id` - Plugin identifier

**Returns:**

- `expected<MarketplacePlugin, PluginError>` - Plugin details or error

#### `get_featured_plugins()`

```cpp
qtplugin::expected<std::vector<MarketplacePlugin>, PluginError> get_featured_plugins(int limit = 10);
```

Gets featured/recommended plugins.

**Parameters:**

- `limit` - Maximum number of plugins to return

**Returns:**

- `expected<std::vector<MarketplacePlugin>, PluginError>` - Featured plugins or error

#### `get_categories()`

```cpp
qtplugin::expected<QStringList, PluginError> get_categories();
```

Gets available plugin categories.

**Returns:**

- `expected<QStringList, PluginError>` - Category list or error

### Plugin Management

#### `install_plugin()`

```cpp
qtplugin::expected<QString, PluginError> install_plugin(
    const QString& plugin_id,
    const QString& version = {});
```

Installs a plugin from the marketplace.

**Parameters:**

- `plugin_id` - Plugin identifier
- `version` - Specific version to install (latest if empty)

**Returns:**

- `expected<QString, PluginError>` - Installation ID for tracking or error

**Note:** Framework complete, core download/install logic pending implementation.

#### `update_plugin()`

```cpp
qtplugin::expected<QString, PluginError> update_plugin(const QString& plugin_id);
```

Updates an installed plugin to the latest version.

#### `uninstall_plugin()`

```cpp
qtplugin::expected<void, PluginError> uninstall_plugin(const QString& plugin_id);
```

Uninstalls a plugin.

#### `get_installation_progress()`

```cpp
qtplugin::expected<InstallationProgress, PluginError> get_installation_progress(
    const QString& installation_id);
```

Gets progress information for an ongoing installation.

#### `get_installed_plugins()`

```cpp
std::vector<QString> get_installed_plugins() const;
```

Gets list of installed plugin IDs.

#### `check_for_updates()`

```cpp
qtplugin::expected<std::vector<MarketplacePlugin>, PluginError> check_for_updates();
```

Checks for available updates to installed plugins.

### Reviews and Ratings

#### `get_plugin_reviews()`

```cpp
qtplugin::expected<std::vector<PluginReview>, PluginError> get_plugin_reviews(
    const QString& plugin_id,
    int limit = 10,
    int offset = 0);
```

Gets reviews for a plugin.

#### `submit_review()`

```cpp
qtplugin::expected<void, PluginError> submit_review(
    const QString& plugin_id,
    double rating,
    const QString& title,
    const QString& content);
```

Submits a review for a plugin.

**Parameters:**

- `plugin_id` - Plugin to review
- `rating` - Rating from 0.0 to 5.0
- `title` - Review title
- `content` - Review content

## Signals

The PluginMarketplace emits the following Qt signals:

```cpp
signals:
    void installation_started(const QString& installation_id, const QString& plugin_id);
    void installation_progress(const QString& installation_id, double progress);
    void installation_completed(const QString& installation_id, const QString& plugin_id);
    void installation_failed(const QString& installation_id, const QString& error);
    void plugin_updated(const QString& plugin_id, const QString& new_version);
    void plugin_uninstalled(const QString& plugin_id);
```

## Class: MarketplaceManager

Singleton manager for handling multiple marketplace sources.

### Static Methods

#### `instance()`

```cpp
static MarketplaceManager& instance();
```

Gets the singleton instance of the marketplace manager.

### Marketplace Management

#### `add_marketplace()`

```cpp
void add_marketplace(const QString& name, std::shared_ptr<PluginMarketplace> marketplace);
```

Adds a marketplace to the manager.

**Parameters:**

- `name` - Marketplace identifier
- `marketplace` - Marketplace instance

#### `remove_marketplace()`

```cpp
void remove_marketplace(const QString& name);
```

Removes a marketplace from the manager.

#### `get_marketplace()`

```cpp
std::shared_ptr<PluginMarketplace> get_marketplace(const QString& name);
```

Gets a specific marketplace by name.

#### `get_marketplace_names()`

```cpp
std::vector<QString> get_marketplace_names() const;
```

Gets names of all registered marketplaces.

### Aggregated Operations

#### `search_all_marketplaces()`

```cpp
qtplugin::expected<std::vector<MarketplacePlugin>, PluginError> search_all_marketplaces(
    const SearchFilters& filters = {});
```

Searches across all registered marketplaces.

**Note:** Implementation pending - currently returns empty results.

#### `check_all_updates()`

```cpp
qtplugin::expected<std::vector<MarketplacePlugin>, PluginError> check_all_updates();
```

Checks for updates across all marketplaces.

### Signals

```cpp
signals:
    void marketplace_added(const QString& name);
    void marketplace_removed(const QString& name);
```

## Error Handling

Common error codes and their meanings:

| Error Code             | Description                     | Resolution                                           |
| ---------------------- | ------------------------------- | ---------------------------------------------------- |
| `NotImplemented`       | Feature not yet implemented     | Wait for future release or contribute implementation |
| `NetworkError`         | Network connectivity issues     | Check internet connection and marketplace URL        |
| `AuthenticationFailed` | API key authentication failed   | Verify API key is valid                              |
| `PluginNotFound`       | Plugin not found in marketplace | Check plugin ID and marketplace availability         |
| `InstallationFailed`   | Plugin installation failed      | Check disk space and permissions                     |
| `InvalidChecksum`      | Plugin file checksum mismatch   | Re-download plugin or report security issue          |

## Thread Safety

- **Thread-safe methods**: All public methods are thread-safe
- **Signal emissions**: Signals are emitted from the main thread
- **Network operations**: Network requests run in background threads
- **Installation tracking**: Installation progress is synchronized

## Performance Considerations

- **Network usage**: Marketplace operations require internet connectivity
- **Caching**: Local caching reduces repeated network requests
- **Download size**: Plugin downloads may be large, consider bandwidth
- **Installation time**: Installation time varies with plugin complexity

## Integration Examples

### Basic Marketplace Usage

```cpp
#include <qtplugin/marketplace/plugin_marketplace.hpp>

class PluginStore {
private:
    std::shared_ptr<PluginMarketplace> m_marketplace;

public:
    bool initialize() {
        m_marketplace = std::make_shared<PluginMarketplace>();

        // Connect to marketplace signals
        connect(m_marketplace.get(), &PluginMarketplace::installation_completed,
                this, &PluginStore::on_installation_completed);

        connect(m_marketplace.get(), &PluginMarketplace::installation_failed,
                this, &PluginStore::on_installation_failed);

        // Initialize marketplace
        auto result = m_marketplace->initialize();
        if (!result) {
            qWarning() << "Failed to initialize marketplace:" << result.error().message();
            return false;
        }

        return true;
    }

    void search_and_install_plugin(const QString& search_term) {
        // Search for plugins
        SearchFilters filters;
        filters.query = search_term;
        filters.verified_only = true;
        filters.sort_by = "rating";
        filters.limit = 10;

        auto search_result = m_marketplace->search_plugins(filters);
        if (!search_result) {
            qWarning() << "Search failed:" << search_result.error().message();
            return;
        }

        auto plugins = search_result.value();
        if (plugins.empty()) {
            qDebug() << "No plugins found for:" << search_term;
            return;
        }

        // Install the highest-rated plugin
        const auto& best_plugin = plugins[0];
        qDebug() << "Installing plugin:" << best_plugin.name << "by" << best_plugin.author;

        auto install_result = m_marketplace->install_plugin(best_plugin.id);
        if (install_result) {
            QString installation_id = install_result.value();
            qDebug() << "Installation started with ID:" << installation_id;

            // Monitor installation progress
            monitor_installation(installation_id);
        } else {
            qWarning() << "Failed to start installation:" << install_result.error().message();
        }
    }

private:
    void monitor_installation(const QString& installation_id) {
        // Create timer to check installation progress
        auto timer = new QTimer(this);
        timer->setInterval(1000); // Check every second

        connect(timer, &QTimer::timeout, [this, installation_id, timer]() {
            auto progress_result = m_marketplace->get_installation_progress(installation_id);
            if (progress_result) {
                auto progress = progress_result.value();
                qDebug() << "Installation progress:" << (progress.progress * 100) << "%"
                         << progress.status_message;

                if (progress.progress >= 1.0) {
                    timer->stop();
                    timer->deleteLater();
                }
            }
        });

        timer->start();
    }

private slots:
    void on_installation_completed(const QString& installation_id, const QString& plugin_id) {
        qDebug() << "Plugin installation completed:" << plugin_id;
        // Refresh plugin list, notify user, etc.
    }

    void on_installation_failed(const QString& installation_id, const QString& error) {
        qWarning() << "Plugin installation failed:" << error;
        // Handle installation failure
    }
};
```

### Multi-Marketplace Management

```cpp
class EnterprisePluginManager {
private:
    MarketplaceManager& m_manager;

public:
    EnterprisePluginManager() : m_manager(MarketplaceManager::instance()) {
        setup_marketplaces();
    }

    void setup_marketplaces() {
        // Add official marketplace
        auto official = std::make_shared<PluginMarketplace>("https://plugins.qtforge.org");
        official->initialize();
        m_manager.add_marketplace("official", official);

        // Add enterprise marketplace
        auto enterprise = std::make_shared<PluginMarketplace>("https://enterprise.company.com/plugins");
        enterprise->initialize("enterprise-api-key");
        m_manager.add_marketplace("enterprise", enterprise);

        // Add development marketplace
        auto dev = std::make_shared<PluginMarketplace>("https://dev.company.com/plugins");
        dev->initialize("dev-api-key");
        m_manager.add_marketplace("development", dev);
    }

    void search_all_sources(const QString& query) {
        SearchFilters filters;
        filters.query = query;
        filters.verified_only = true;

        // Search across all marketplaces
        auto results = m_manager.search_all_marketplaces(filters);
        if (results) {
            qDebug() << "Found" << results.value().size() << "plugins across all marketplaces";
            for (const auto& plugin : results.value()) {
                qDebug() << "  -" << plugin.name << "by" << plugin.author;
            }
        }
    }
};
```

## Python Bindings

!!! note "Python Support"
This component is available in Python through the `qtforge.marketplace` module.

```python
import qtforge

# Create marketplace client
marketplace = qtforge.marketplace.PluginMarketplace("https://plugins.qtforge.org")
result = marketplace.initialize("your-api-key")

if result:
    # Search for plugins
    filters = qtforge.marketplace.SearchFilters()
    filters.query = "data processing"
    filters.categories = ["Data", "Analytics"]
    filters.min_rating = 4.0
    filters.verified_only = True

    search_results = marketplace.search_plugins(filters)
    if search_results:
        plugins = search_results.value()
        print(f"Found {len(plugins)} plugins")

        for plugin in plugins:
            print(f"- {plugin.name} by {plugin.author} (Rating: {plugin.rating})")

        # Install first plugin
        if plugins:
            installation_id = marketplace.install_plugin(plugins[0].id)
            if installation_id:
                print(f"Installation started: {installation_id.value()}")

# Use marketplace manager
manager = qtforge.marketplace.MarketplaceManager.instance()
manager.add_marketplace("custom", marketplace)

# Search across all marketplaces
all_results = manager.search_all_marketplaces(filters)
```

## Related Components

- **[PluginManager](../core/plugin-manager.md)**: Core plugin management for installed plugins
- **[SecurityManager](../security/security-manager.md)**: Plugin verification and security
- **[PluginLoader](../core/plugin-loader.md)**: Loading installed marketplace plugins
- **[NetworkManager](../optional/network.md)**: Network operations for marketplace communication

## Migration Notes

### From v3.1 to v3.2

- **New Features**: Marketplace system introduction, plugin discovery and installation
- **API Status**: Experimental - API complete, implementation in progress
- **Future Changes**: Core implementation will be added in upcoming releases

## Implementation Status

!!! info "Current Implementation Status"
The marketplace system is currently in **experimental** status:

    **✅ Complete:**
    - Full API definition and interfaces
    - Data structures and type definitions
    - Python bindings
    - Signal/slot system
    - Installation progress tracking framework

    **🚧 In Progress:**
    - Network API implementation
    - Plugin download and installation logic
    - Security verification system
    - Marketplace server communication

    **📋 Planned:**
    - Plugin signature verification
    - Payment processing integration
    - Advanced search algorithms
    - Offline mode support

## See Also

- [Marketplace Integration User Guide](../../user-guide/marketplace-integration.md)
- [Plugin Distribution Guide](../../developer-guide/plugin-distribution.md)
- [Marketplace Examples](../../examples/marketplace-examples.md)
- [Security Considerations](../../security/marketplace-security.md)

---

_Last updated: December 2024 | QtForge v3.2.0_
