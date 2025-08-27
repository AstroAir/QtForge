/**
 * @file plugin_marketplace.hpp
 * @brief Plugin marketplace client for discovery, installation, and updates
 * @version 3.2.0
 * @author QtPlugin Development Team
 *
 * This file provides a comprehensive plugin marketplace system including:
 * - Plugin discovery and search
 * - Installation and update management
 * - Rating and review system
 * - License and payment integration
 * - Security verification
 */

#pragma once

#include "../core/dynamic_plugin_interface.hpp"
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QTimer>
#include <QMutex>
#include <QCryptographicHash>
#include <QDateTime>
#include <QSslError>
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>

namespace qtplugin {

/**
 * @brief Plugin marketplace entry information
 */
struct MarketplacePlugin {
    QString plugin_id;                  ///< Unique plugin identifier
    QString name;                       ///< Plugin name
    QString description;                ///< Plugin description
    QString author;                     ///< Plugin author
    QString version;                    ///< Latest version
    QString category;                   ///< Plugin category
    QStringList tags;                   ///< Plugin tags
    QString license;                    ///< Plugin license
    QString homepage;                   ///< Plugin homepage URL
    QString repository;                 ///< Source repository URL
    QString download_url;               ///< Download URL
    qint64 download_size;               ///< Download size in bytes
    QString checksum;                   ///< File checksum (SHA-256)
    double rating{0.0};                 ///< Average rating (0-5)
    int review_count{0};                ///< Number of reviews
    int download_count{0};              ///< Number of downloads
    QDateTime created_date;             ///< Creation date
    QDateTime updated_date;             ///< Last update date
    bool verified{false};               ///< Whether plugin is verified
    bool premium{false};                ///< Whether plugin is premium
    double price{0.0};                  ///< Plugin price (if premium)
    QString currency{"USD"};            ///< Price currency
    QJsonObject metadata;               ///< Additional metadata

    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON representation
     */
    static qtplugin::expected<MarketplacePlugin, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Plugin review information
 */
struct PluginReview {
    QString review_id;                  ///< Unique review identifier
    QString plugin_id;                  ///< Associated plugin identifier
    QString user_id;                    ///< Reviewer user identifier
    QString username;                   ///< Reviewer username
    double rating{0.0};                 ///< Review rating (0-5)
    QString title;                      ///< Review title
    QString content;                    ///< Review content
    QDateTime created_date;             ///< Review creation date
    bool verified_purchase{false};      ///< Whether reviewer purchased plugin
    int helpful_count{0};               ///< Number of helpful votes

    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON representation
     */
    static qtplugin::expected<PluginReview, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Search filters for marketplace queries
 */
struct SearchFilters {
    QString query;                      ///< Search query string
    QStringList categories;             ///< Filter by categories
    QStringList tags;                   ///< Filter by tags
    QString author;                     ///< Filter by author
    QString license;                    ///< Filter by license
    double min_rating{0.0};             ///< Minimum rating filter
    bool verified_only{false};          ///< Show only verified plugins
    bool free_only{false};              ///< Show only free plugins
    QString sort_by{"relevance"};       ///< Sort criteria
    bool ascending{false};              ///< Sort order
    int limit{50};                      ///< Maximum results
    int offset{0};                      ///< Result offset for pagination

    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;
};

/**
 * @brief Installation progress information
 */
struct InstallationProgress {
    QString plugin_id;                  ///< Plugin being installed
    QString operation;                  ///< Current operation
    int progress_percent{0};            ///< Progress percentage (0-100)
    qint64 bytes_downloaded{0};         ///< Bytes downloaded
    qint64 total_bytes{0};              ///< Total bytes to download
    QString status_message;             ///< Current status message
    bool completed{false};              ///< Whether installation is complete
    bool failed{false};                 ///< Whether installation failed
    QString error_message;              ///< Error message if failed

    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;
};

/**
 * @brief Plugin marketplace client
 */
class PluginMarketplace : public QObject {
    Q_OBJECT

public:
    explicit PluginMarketplace(const QString& marketplace_url = "https://plugins.qtforge.org",
                              QObject* parent = nullptr);
    ~PluginMarketplace() override;

    /**
     * @brief Initialize the marketplace client
     * @param api_key Optional API key for authenticated requests
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> initialize(const QString& api_key = {});

    /**
     * @brief Search for plugins in the marketplace
     * @param filters Search filters
     * @return Search results or error
     */
    qtplugin::expected<std::vector<MarketplacePlugin>, PluginError> search_plugins(
        const SearchFilters& filters = {});

    /**
     * @brief Get plugin details
     * @param plugin_id Plugin identifier
     * @return Plugin details or error
     */
    qtplugin::expected<MarketplacePlugin, PluginError> get_plugin_details(const QString& plugin_id);

    /**
     * @brief Get plugin reviews
     * @param plugin_id Plugin identifier
     * @param limit Maximum number of reviews
     * @param offset Review offset for pagination
     * @return Plugin reviews or error
     */
    qtplugin::expected<std::vector<PluginReview>, PluginError> get_plugin_reviews(
        const QString& plugin_id, int limit = 10, int offset = 0);

    /**
     * @brief Install a plugin from the marketplace
     * @param plugin_id Plugin identifier
     * @param version Specific version to install (empty for latest)
     * @return Installation ID or error
     */
    qtplugin::expected<QString, PluginError> install_plugin(
        const QString& plugin_id, const QString& version = {});

    /**
     * @brief Update an installed plugin
     * @param plugin_id Plugin identifier
     * @return Update ID or error
     */
    qtplugin::expected<QString, PluginError> update_plugin(const QString& plugin_id);

    /**
     * @brief Uninstall a plugin
     * @param plugin_id Plugin identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> uninstall_plugin(const QString& plugin_id);

    /**
     * @brief Get installation progress
     * @param installation_id Installation identifier
     * @return Installation progress or error
     */
    qtplugin::expected<InstallationProgress, PluginError> get_installation_progress(
        const QString& installation_id);

    /**
     * @brief Cancel installation
     * @param installation_id Installation identifier
     */
    void cancel_installation(const QString& installation_id);

    /**
     * @brief Check for plugin updates
     * @return List of plugins with available updates
     */
    qtplugin::expected<std::vector<MarketplacePlugin>, PluginError> check_for_updates();

    /**
     * @brief Submit a plugin review
     * @param plugin_id Plugin identifier
     * @param rating Review rating (0-5)
     * @param title Review title
     * @param content Review content
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> submit_review(
        const QString& plugin_id, double rating, const QString& title, const QString& content);

    /**
     * @brief Get installed plugins
     * @return List of installed plugins
     */
    std::vector<QString> get_installed_plugins() const;

    /**
     * @brief Get marketplace categories
     * @return List of available categories
     */
    qtplugin::expected<QStringList, PluginError> get_categories();

    /**
     * @brief Get popular plugins
     * @param limit Maximum number of plugins
     * @return Popular plugins or error
     */
    qtplugin::expected<std::vector<MarketplacePlugin>, PluginError> get_popular_plugins(int limit = 20);

    /**
     * @brief Get featured plugins
     * @param limit Maximum number of plugins
     * @return Featured plugins or error
     */
    qtplugin::expected<std::vector<MarketplacePlugin>, PluginError> get_featured_plugins(int limit = 10);

signals:
    /**
     * @brief Emitted when installation starts
     */
    void installation_started(const QString& installation_id, const QString& plugin_id);

    /**
     * @brief Emitted when installation progress updates
     */
    void installation_progress(const QString& installation_id, const InstallationProgress& progress);

    /**
     * @brief Emitted when installation completes
     */
    void installation_completed(const QString& installation_id, const QString& plugin_id);

    /**
     * @brief Emitted when installation fails
     */
    void installation_failed(const QString& installation_id, const QString& error);

    /**
     * @brief Emitted when plugin updates are available
     */
    void updates_available(const std::vector<MarketplacePlugin>& plugins);

private slots:
    void handle_network_reply();
    void handle_download_progress(qint64 bytes_received, qint64 bytes_total);
    void handle_ssl_errors(QNetworkReply* reply, const QList<QSslError>& errors);

private:
    QString m_marketplace_url;
    QString m_api_key;
    std::unique_ptr<QNetworkAccessManager> m_network_manager;
    std::unordered_map<QString, InstallationProgress> m_installations;
    std::unordered_map<QNetworkReply*, QString> m_reply_to_installation;
    QStringList m_installed_plugins;
    mutable QMutex m_mutex;

    qtplugin::expected<QJsonObject, PluginError> make_api_request(
        const QString& endpoint, const QJsonObject& data = {}, const QString& method = "GET");
    qtplugin::expected<void, PluginError> download_and_install_plugin(
        const QString& installation_id, const MarketplacePlugin& plugin);
    qtplugin::expected<void, PluginError> verify_plugin_signature(
        const QString& file_path, const QString& expected_checksum);
    QString generate_installation_id();
    void load_installed_plugins();
    void save_installed_plugins();
};

/**
 * @brief Plugin marketplace manager for handling multiple marketplace sources
 */
class MarketplaceManager : public QObject {
    Q_OBJECT

public:
    static MarketplaceManager& instance();

    /**
     * @brief Add a marketplace source
     * @param name Marketplace name
     * @param marketplace Marketplace client
     */
    void add_marketplace(const QString& name, std::shared_ptr<PluginMarketplace> marketplace);

    /**
     * @brief Remove a marketplace source
     * @param name Marketplace name
     */
    void remove_marketplace(const QString& name);

    /**
     * @brief Get marketplace by name
     * @param name Marketplace name
     * @return Marketplace client or nullptr
     */
    std::shared_ptr<PluginMarketplace> get_marketplace(const QString& name);

    /**
     * @brief Search across all marketplaces
     * @param filters Search filters
     * @return Aggregated search results
     */
    qtplugin::expected<std::vector<MarketplacePlugin>, PluginError> search_all_marketplaces(
        const SearchFilters& filters = {});

    /**
     * @brief Get all registered marketplaces
     * @return List of marketplace names
     */
    std::vector<QString> get_marketplace_names() const;

    /**
     * @brief Check for updates across all marketplaces
     * @return List of plugins with available updates
     */
    qtplugin::expected<std::vector<MarketplacePlugin>, PluginError> check_all_updates();

signals:
    /**
     * @brief Emitted when a marketplace is added
     */
    void marketplace_added(const QString& name);

    /**
     * @brief Emitted when a marketplace is removed
     */
    void marketplace_removed(const QString& name);

private:
    MarketplaceManager() = default;
    std::unordered_map<QString, std::shared_ptr<PluginMarketplace>> m_marketplaces;
    mutable QMutex m_mutex;
};

} // namespace qtplugin
