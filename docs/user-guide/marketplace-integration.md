# Marketplace Integration Guide

!!! warning "Experimental Feature"
**Difficulty**: Intermediate  
 **Prerequisites**: Basic plugin development, QtForge setup  
 **Estimated Time**: 1-2 hours  
 **QtForge Version**: v3.2.0+  
 **Status**: Experimental - API complete, implementation in progress

## Overview

This guide covers how to integrate with the QtForge plugin marketplace system for discovering, installing, and managing plugins from remote repositories. You'll learn how to set up marketplace clients, search for plugins, handle installations, and manage plugin updates.

### What You'll Learn

- [ ] Setting up marketplace clients and authentication
- [ ] Searching and discovering plugins from marketplaces
- [ ] Installing and managing plugins from remote sources
- [ ] Handling plugin updates and version management
- [ ] Managing multiple marketplace sources
- [ ] Preparing plugins for marketplace distribution

### Prerequisites

Before starting this guide, you should have:

- [x] QtForge installed and configured
- [x] Basic understanding of plugin management
- [x] Network connectivity for marketplace access
- [x] Understanding of plugin metadata and packaging

## Getting Started

### Setting Up Marketplace Client

```cpp
#include <qtplugin/marketplace/plugin_marketplace.hpp>

class MarketplaceIntegration {
private:
    std::shared_ptr<PluginMarketplace> m_marketplace;

public:
    bool initialize() {
        // Create marketplace client
        m_marketplace = std::make_shared<PluginMarketplace>("https://plugins.qtforge.org");

        // Connect to marketplace signals
        connect(m_marketplace.get(), &PluginMarketplace::installation_completed,
                this, &MarketplaceIntegration::on_installation_completed);

        connect(m_marketplace.get(), &PluginMarketplace::installation_failed,
                this, &MarketplaceIntegration::on_installation_failed);

        connect(m_marketplace.get(), &PluginMarketplace::installation_progress,
                this, &MarketplaceIntegration::on_installation_progress);

        // Initialize with optional API key
        auto init_result = m_marketplace->initialize("your-api-key-here");
        if (!init_result) {
            qWarning() << "Failed to initialize marketplace:" << init_result.error().message();
            return false;
        }

        qDebug() << "Marketplace client initialized successfully";
        return true;
    }

private slots:
    void on_installation_completed(const QString& installation_id, const QString& plugin_id) {
        qDebug() << "Plugin installation completed:" << plugin_id;
        // Handle successful installation
    }

    void on_installation_failed(const QString& installation_id, const QString& error) {
        qWarning() << "Plugin installation failed:" << error;
        // Handle installation failure
    }

    void on_installation_progress(const QString& installation_id, double progress) {
        qDebug() << "Installation progress:" << (progress * 100) << "%";
        // Update progress UI
    }
};
```

### Basic Plugin Search and Discovery

```cpp
void search_plugins() {
    // Create search filters
    SearchFilters filters;
    filters.query = "data processing";
    filters.categories = {"Data", "Analytics"};
    filters.min_rating = 4.0;
    filters.verified_only = true;
    filters.sort_by = "rating";
    filters.limit = 20;

    // Search for plugins
    auto search_result = m_marketplace->search_plugins(filters);
    if (!search_result) {
        qWarning() << "Search failed:" << search_result.error().message();
        return;
    }

    auto plugins = search_result.value();
    qDebug() << "Found" << plugins.size() << "plugins";

    // Display search results
    for (const auto& plugin : plugins) {
        qDebug() << "Plugin:" << plugin.name;
        qDebug() << "  Author:" << plugin.author;
        qDebug() << "  Version:" << plugin.version;
        qDebug() << "  Rating:" << plugin.rating << "(" << plugin.review_count << "reviews)";
        qDebug() << "  Downloads:" << plugin.download_count;
        qDebug() << "  Description:" << plugin.description;
        qDebug() << "  Categories:" << plugin.categories.join(", ");
        qDebug() << "  Verified:" << (plugin.verified ? "Yes" : "No");
        qDebug() << "  Price:" << (plugin.free ? "Free" : QString("$%1").arg(plugin.price));
        qDebug() << "---";
    }
}
```

### Plugin Installation and Management

```cpp
class PluginInstallationManager {
private:
    std::shared_ptr<PluginMarketplace> m_marketplace;
    std::unordered_map<QString, QString> m_active_installations; // installation_id -> plugin_id

public:
    bool install_plugin(const QString& plugin_id, const QString& version = {}) {
        qDebug() << "Starting installation of plugin:" << plugin_id;

        // Start installation
        auto install_result = m_marketplace->install_plugin(plugin_id, version);
        if (!install_result) {
            qWarning() << "Failed to start installation:" << install_result.error().message();
            return false;
        }

        QString installation_id = install_result.value();
        m_active_installations[installation_id] = plugin_id;

        qDebug() << "Installation started with ID:" << installation_id;

        // Monitor installation progress
        monitor_installation(installation_id);

        return true;
    }

    void monitor_installation(const QString& installation_id) {
        // Create timer to check installation progress
        auto timer = new QTimer(this);
        timer->setInterval(1000); // Check every second

        connect(timer, &QTimer::timeout, [this, installation_id, timer]() {
            auto progress_result = m_marketplace->get_installation_progress(installation_id);
            if (!progress_result) {
                qWarning() << "Failed to get installation progress:"
                           << progress_result.error().message();
                timer->stop();
                timer->deleteLater();
                return;
            }

            auto progress = progress_result.value();
            qDebug() << "Installation progress:" << (progress.progress * 100) << "%"
                     << progress.status_message;

            // Update UI with progress
            update_installation_ui(installation_id, progress);

            // Check if installation is complete
            if (progress.progress >= 1.0) {
                timer->stop();
                timer->deleteLater();

                // Installation completed
                handle_installation_completion(installation_id);
            }
        });

        timer->start();
    }

    void handle_installation_completion(const QString& installation_id) {
        auto it = m_active_installations.find(installation_id);
        if (it != m_active_installations.end()) {
            QString plugin_id = it->second;
            m_active_installations.erase(it);

            qDebug() << "Plugin installation completed:" << plugin_id;

            // Verify installation
            verify_plugin_installation(plugin_id);
        }
    }

    bool verify_plugin_installation(const QString& plugin_id) {
        // Check if plugin is now available in the system
        auto installed_plugins = m_marketplace->get_installed_plugins();

        bool found = std::find(installed_plugins.begin(), installed_plugins.end(),
                              plugin_id) != installed_plugins.end();

        if (found) {
            qDebug() << "Plugin" << plugin_id << "successfully installed and verified";
            emit plugin_installation_verified(plugin_id);
            return true;
        } else {
            qWarning() << "Plugin" << plugin_id << "installation verification failed";
            return false;
        }
    }

    void check_for_updates() {
        qDebug() << "Checking for plugin updates...";

        auto updates_result = m_marketplace->check_for_updates();
        if (!updates_result) {
            qWarning() << "Failed to check for updates:" << updates_result.error().message();
            return;
        }

        auto available_updates = updates_result.value();
        if (available_updates.empty()) {
            qDebug() << "No updates available";
            return;
        }

        qDebug() << "Found" << available_updates.size() << "available updates:";
        for (const auto& plugin : available_updates) {
            qDebug() << "  -" << plugin.name << ":" << plugin.version;
        }

        // Optionally auto-update or prompt user
        prompt_for_updates(available_updates);
    }

    void update_plugin(const QString& plugin_id) {
        qDebug() << "Updating plugin:" << plugin_id;

        auto update_result = m_marketplace->update_plugin(plugin_id);
        if (update_result) {
            QString installation_id = update_result.value();
            m_active_installations[installation_id] = plugin_id;
            monitor_installation(installation_id);
        } else {
            qWarning() << "Failed to start plugin update:" << update_result.error().message();
        }
    }

signals:
    void plugin_installation_verified(const QString& plugin_id);
    void installation_progress_updated(const QString& installation_id, double progress);

private:
    void update_installation_ui(const QString& installation_id, const InstallationProgress& progress) {
        // Update progress bars, status messages, etc.
        emit installation_progress_updated(installation_id, progress.progress);
    }

    void prompt_for_updates(const std::vector<MarketplacePlugin>& updates) {
        // Show update dialog or notification to user
        for (const auto& plugin : updates) {
            // Could show individual update prompts or batch update option
            qDebug() << "Update available for" << plugin.name << "to version" << plugin.version;
        }
    }
};
```

### Advanced Search and Filtering

```cpp
class AdvancedPluginSearch {
private:
    std::shared_ptr<PluginMarketplace> m_marketplace;

public:
    void search_by_category(const QString& category) {
        SearchFilters filters;
        filters.categories = {category};
        filters.sort_by = "download_count";
        filters.limit = 50;

        perform_search(filters, QString("Category: %1").arg(category));
    }

    void search_free_plugins() {
        SearchFilters filters;
        filters.free_only = true;
        filters.sort_by = "rating";
        filters.min_rating = 3.5;

        perform_search(filters, "Free Plugins");
    }

    void search_verified_plugins() {
        SearchFilters filters;
        filters.verified_only = true;
        filters.sort_by = "rating";

        perform_search(filters, "Verified Plugins");
    }

    void search_by_author(const QString& author) {
        SearchFilters filters;
        filters.author = author;
        filters.sort_by = "updated_at";

        perform_search(filters, QString("Author: %1").arg(author));
    }

    void get_featured_plugins() {
        auto featured_result = m_marketplace->get_featured_plugins(10);
        if (featured_result) {
            auto plugins = featured_result.value();
            qDebug() << "Featured plugins:";
            display_plugin_list(plugins);
        }
    }

    void get_plugin_details(const QString& plugin_id) {
        auto details_result = m_marketplace->get_plugin_details(plugin_id);
        if (!details_result) {
            qWarning() << "Failed to get plugin details:" << details_result.error().message();
            return;
        }

        auto plugin = details_result.value();
        display_detailed_plugin_info(plugin);

        // Get reviews
        get_plugin_reviews(plugin_id);
    }

    void get_plugin_reviews(const QString& plugin_id) {
        auto reviews_result = m_marketplace->get_plugin_reviews(plugin_id, 10, 0);
        if (reviews_result) {
            auto reviews = reviews_result.value();
            qDebug() << "Reviews for plugin" << plugin_id << ":";

            for (const auto& review : reviews) {
                qDebug() << "  Rating:" << review.rating << "/5";
                qDebug() << "  Title:" << review.title;
                qDebug() << "  Author:" << review.author;
                qDebug() << "  Content:" << review.content;
                qDebug() << "  Helpful votes:" << review.helpful_count;
                qDebug() << "  Date:" << review.created_at.toString();
                qDebug() << "  ---";
            }
        }
    }

private:
    void perform_search(const SearchFilters& filters, const QString& search_name) {
        qDebug() << "Performing search:" << search_name;

        auto search_result = m_marketplace->search_plugins(filters);
        if (search_result) {
            auto plugins = search_result.value();
            qDebug() << "Found" << plugins.size() << "plugins for" << search_name;
            display_plugin_list(plugins);
        } else {
            qWarning() << "Search failed:" << search_result.error().message();
        }
    }

    void display_plugin_list(const std::vector<MarketplacePlugin>& plugins) {
        for (const auto& plugin : plugins) {
            qDebug() << QString("  %1 v%2 by %3 (Rating: %4, Downloads: %5)")
                        .arg(plugin.name)
                        .arg(plugin.version)
                        .arg(plugin.author)
                        .arg(plugin.rating)
                        .arg(plugin.download_count);
        }
    }

    void display_detailed_plugin_info(const MarketplacePlugin& plugin) {
        qDebug() << "=== Plugin Details ===";
        qDebug() << "ID:" << plugin.id;
        qDebug() << "Name:" << plugin.name;
        qDebug() << "Version:" << plugin.version;
        qDebug() << "Author:" << plugin.author;
        qDebug() << "License:" << plugin.license;
        qDebug() << "Description:" << plugin.description;
        qDebug() << "Categories:" << plugin.categories.join(", ");
        qDebug() << "Tags:" << plugin.tags.join(", ");
        qDebug() << "Rating:" << plugin.rating << "(" << plugin.review_count << "reviews)";
        qDebug() << "Downloads:" << plugin.download_count;
        qDebug() << "File Size:" << (plugin.file_size / 1024.0 / 1024.0) << "MB";
        qDebug() << "Created:" << plugin.created_at.toString();
        qDebug() << "Updated:" << plugin.updated_at.toString();
        qDebug() << "Verified:" << (plugin.verified ? "Yes" : "No");
        qDebug() << "Price:" << (plugin.free ? "Free" : QString("$%1 %2").arg(plugin.price).arg(plugin.currency));
        qDebug() << "======================";
    }
};
```

## Best Practices

### Do's ✅

- **Handle Network Errors**: Always check for network connectivity issues
- **Validate Plugin Integrity**: Verify checksums and signatures before installation
- **Monitor Installation Progress**: Provide user feedback during installations
- **Cache Search Results**: Cache frequently accessed data to reduce network calls
- **Handle API Rate Limits**: Implement proper rate limiting and retry logic
- **Secure API Keys**: Store API keys securely and never expose them in logs

### Don'ts ❌

- **Don't Block UI**: Use asynchronous operations for all marketplace calls
- **Don't Ignore Errors**: Always handle marketplace operation failures gracefully
- **Don't Auto-Install**: Always get user consent before installing plugins
- **Don't Skip Verification**: Always verify plugin authenticity and integrity
- **Don't Hardcode URLs**: Make marketplace URLs configurable

## Implementation Status

!!! info "Current Implementation Status"
The marketplace system is currently in **experimental** status:

    **✅ Complete:**
    - Full API definition and interfaces
    - Search and discovery framework
    - Installation progress tracking
    - Multi-marketplace management
    - Python bindings

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

## Troubleshooting

### Common Issues

#### Issue 1: Marketplace Connection Failed

**Symptoms:**

- Cannot connect to marketplace
- Network timeout errors
- Authentication failures

**Solutions:**

1. Check internet connectivity
2. Verify marketplace URL is correct
3. Check API key validity
4. Review firewall settings

#### Issue 2: Plugin Installation Fails

**Symptoms:**

- Installation starts but never completes
- Checksum verification errors
- Permission denied errors

**Solutions:**

1. Check available disk space
2. Verify file permissions in plugin directory
3. Re-download plugin if checksum fails
4. Run with appropriate privileges

## Next Steps

After completing this guide, you might want to:

- [ ] [Advanced Security Guide](advanced-security.md) - Secure marketplace usage
- [ ] [Plugin Distribution Guide](../developer-guide/plugin-distribution.md) - Prepare plugins for marketplace
- [ ] [Performance Optimization](performance-optimization.md) - Optimize marketplace operations

## Related Resources

### Documentation

- [PluginMarketplace API](../api/marketplace/plugin-marketplace.md) - Detailed API reference
- [MarketplaceManager API](../api/marketplace/marketplace-manager.md) - Multi-marketplace management

### Examples

- [Marketplace Examples](../examples/marketplace-examples.md) - Complete usage examples
- [Plugin Distribution Examples](../examples/distribution-examples.md) - Distribution patterns

---

_Last updated: December 2024 | QtForge v3.2.0_
