/**
 * @file http_plugin_loader.hpp
 * @brief HTTP/HTTPS plugin loader implementation
 * @version 3.0.0
 */

#pragma once

#include <QtNetwork/QNetworkAccessManager>

#include <QTimer>
#include <memory>
#include <unordered_map>

#include "../workflow/workflow_validator.hpp"
#include "remote_plugin_loader.hpp"

namespace qtplugin {
using qtplugin::workflow::validation::ValidationResult;

/**
 * @brief HTTP/HTTPS plugin loader implementation
 *
 * Specialized remote plugin loader for HTTP and HTTPS sources.
 * Provides optimized handling for web-based plugin repositories.
 */
class HttpPluginLoader : public RemotePluginLoaderBase {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param configuration Remote plugin configuration
     * @param download_manager Download manager
     * @param validator Plugin validator
     * @param parent Parent QObject
     */
    explicit HttpPluginLoader(
        std::shared_ptr<RemotePluginConfiguration> configuration = nullptr,
        std::shared_ptr<PluginDownloadManager> download_manager = nullptr,
        std::shared_ptr<RemotePluginValidator> validator = nullptr,
        QObject* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~HttpPluginLoader() override;

    // === IRemotePluginLoader Implementation ===

    bool can_load_remote(const QUrl& url) const override;

    qtplugin::expected<RemotePluginLoadResult, PluginError> load_remote(
        const RemotePluginSource& source,
        const RemotePluginLoadOptions& options = {}) override;

    qtplugin::expected<RemotePluginLoadResult, PluginError> load_remote(
        const QUrl& url, const RemotePluginLoadOptions& options = {}) override;

    QString load_remote_async(
        const RemotePluginSource& source,
        const RemotePluginLoadOptions& options = {},
        std::function<void(const DownloadProgress&)> progress_callback =
            nullptr,
        std::function<void(
            const qtplugin::expected<RemotePluginLoadResult, PluginError>&)>
            completion_callback = nullptr) override;

    qtplugin::expected<void, PluginError> cancel_remote_load(
        const QString& operation_id) override;

    qtplugin::expected<std::vector<QJsonObject>, PluginError> discover_plugins(
        const RemotePluginSource& source) override;

    qtplugin::expected<std::vector<QJsonObject>, PluginError> search_plugins(
        const QString& query, int max_results = 50) override;

    // === HTTP-Specific Methods ===

    /**
     * @brief Set custom network access manager
     * @param network_manager Custom network access manager
     */
    void set_network_manager(
        std::unique_ptr<QNetworkAccessManager> network_manager);

    /**
     * @brief Get network access manager
     * @return Current network access manager
     */
    QNetworkAccessManager* network_manager() const {
        return m_network_manager.get();
    }

    /**
     * @brief Check if URL is HTTP/HTTPS
     * @param url URL to check
     * @return true if HTTP/HTTPS
     */
    static bool is_http_url(const QUrl& url);

    /**
     * @brief Get plugin metadata from HTTP source
     * @param url Plugin URL
     * @param timeout Request timeout
     * @return Plugin metadata or error
     */
    qtplugin::expected<QJsonObject, PluginError> get_plugin_metadata(
        const QUrl& url,
        std::chrono::seconds timeout = std::chrono::seconds{30});

    // === Registry Support ===

    /**
     * @brief Check if source supports plugin registry API
     * @param source Remote plugin source
     * @return true if registry API is supported
     */
    bool supports_registry_api(const RemotePluginSource& source) const;

    /**
     * @brief Get registry API endpoint
     * @param source Remote plugin source
     * @param endpoint Endpoint name (e.g., "search", "info", "download")
     * @return Full endpoint URL
     */
    QUrl get_registry_endpoint(const RemotePluginSource& source,
                               const QString& endpoint) const;

protected:
    // === RemotePluginLoaderBase Implementation ===

    qtplugin::expected<RemotePluginLoadResult, PluginError> perform_remote_load(
        const RemotePluginSource& source,
        const RemotePluginLoadOptions& options) override;

private slots:
    void on_async_load_progress(const QString& download_id,
                                const DownloadProgress& progress);
    void on_async_load_completed(const QString& download_id,
                                 const DownloadResult& result);
    void on_async_load_failed(const QString& download_id,
                              const PluginError& error);

private:
    // Network management
    std::unique_ptr<QNetworkAccessManager> m_network_manager;

    // Async operation tracking
    struct AsyncOperation {
        QString operation_id;
        QString download_id;
        RemotePluginSource source;
        RemotePluginLoadOptions options;
        std::function<void(const DownloadProgress&)> progress_callback;
        std::function<void(
            const qtplugin::expected<RemotePluginLoadResult, PluginError>&)>
            completion_callback;
        std::chrono::system_clock::time_point start_time;

        // Explicitly define move semantics
        AsyncOperation() = default;
        AsyncOperation(const AsyncOperation&) = delete;
        AsyncOperation& operator=(const AsyncOperation&) = delete;
        AsyncOperation(AsyncOperation&&) = default;
        AsyncOperation& operator=(AsyncOperation&&) = default;
    };

    mutable std::mutex m_async_operations_mutex;
    std::unordered_map<QString, std::unique_ptr<AsyncOperation>>
        m_async_operations;
    std::unordered_map<QString, QString>
        m_download_to_operation_map;  // download_id -> operation_id

    // Helper methods
    RemotePluginSource create_source_from_url(const QUrl& url) const;
    qtplugin::expected<RemoteValidationResult, PluginError>
    validate_http_source(const RemotePluginSource& source,
                         const RemotePluginLoadOptions& options);
    qtplugin::expected<DownloadResult, PluginError> download_plugin_file(
        const RemotePluginSource& source,
        const RemotePluginLoadOptions& options);
    qtplugin::expected<std::shared_ptr<IPlugin>, PluginError>
    load_downloaded_plugin(const DownloadResult& download_result,
                           const RemotePluginSource& source);

    QString register_async_operation(
        const RemotePluginSource& source,
        const RemotePluginLoadOptions& options,
        std::function<void(const DownloadProgress&)> progress_callback,
        std::function<void(
            const qtplugin::expected<RemotePluginLoadResult, PluginError>&)>
            completion_callback);

    void complete_async_operation(
        const QString& operation_id,
        const qtplugin::expected<RemotePluginLoadResult, PluginError>& result);
    void cleanup_async_operation(const QString& operation_id);

    // Registry API helpers
    qtplugin::expected<QJsonObject, PluginError> make_registry_request(
        const QUrl& endpoint, const QJsonObject& params = {},
        std::chrono::seconds timeout = std::chrono::seconds{30});

    qtplugin::expected<QJsonArray, PluginError> parse_plugin_list_response(
        const QJsonObject& response);
    qtplugin::expected<QJsonObject, PluginError> parse_plugin_info_response(
        const QJsonObject& response);

    // HTTP request helpers
    qtplugin::expected<QJsonObject, PluginError> make_http_request(
        const QUrl& url, const QString& method = "GET",
        const QJsonObject& data = {}, const QJsonObject& headers = {},
        std::chrono::seconds timeout = std::chrono::seconds{30});

    void setup_network_request(QNetworkRequest& request,
                               const RemotePluginSource& source) const;
    void apply_authentication(QNetworkRequest& request,
                              const RemotePluginSource& source) const;
    void apply_custom_headers(QNetworkRequest& request,
                              const QJsonObject& headers) const;

    // URL manipulation helpers
    QUrl resolve_plugin_url(const RemotePluginSource& source,
                            const QString& plugin_path) const;
    QUrl build_search_url(const RemotePluginSource& source,
                          const QString& query, int max_results) const;
    QUrl build_discovery_url(const RemotePluginSource& source) const;

    // Response parsing helpers
    qtplugin::expected<QJsonObject, PluginError> parse_json_response(
        const QByteArray& data) const;
    bool is_error_response(const QJsonObject& response) const;
    PluginError extract_error_from_response(const QJsonObject& response) const;

    // Validation helpers
    bool is_valid_plugin_response(const QJsonObject& response) const;
    bool is_valid_search_response(const QJsonObject& response) const;
    bool is_valid_discovery_response(const QJsonObject& response) const;
};

/**
 * @brief Factory for creating HTTP plugin loaders
 */
class HttpPluginLoaderFactory {
public:
    /**
     * @brief Create HTTP plugin loader with default configuration
     * @param parent Parent QObject
     * @return HTTP plugin loader instance
     */
    static std::unique_ptr<HttpPluginLoader> create_default(
        QObject* parent = nullptr);

    /**
     * @brief Create HTTP plugin loader with custom configuration
     * @param configuration Remote plugin configuration
     * @param parent Parent QObject
     * @return HTTP plugin loader instance
     */
    static std::unique_ptr<HttpPluginLoader> create_with_config(
        std::shared_ptr<RemotePluginConfiguration> configuration,
        QObject* parent = nullptr);

    /**
     * @brief Create HTTP plugin loader for enterprise use
     * @param parent Parent QObject
     * @return HTTP plugin loader instance with enterprise configuration
     */
    static std::unique_ptr<HttpPluginLoader> create_enterprise(
        QObject* parent = nullptr);

    /**
     * @brief Create HTTP plugin loader with custom components
     * @param configuration Remote plugin configuration
     * @param download_manager Download manager
     * @param validator Plugin validator
     * @param parent Parent QObject
     * @return HTTP plugin loader instance
     */
    static std::unique_ptr<HttpPluginLoader> create_custom(
        std::shared_ptr<RemotePluginConfiguration> configuration,
        std::shared_ptr<PluginDownloadManager> download_manager,
        std::shared_ptr<RemotePluginValidator> validator,
        QObject* parent = nullptr);
};

}  // namespace qtplugin
