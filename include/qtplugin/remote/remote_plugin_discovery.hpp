/**
 * @file remote_plugin_discovery.hpp
 * @brief Remote plugin discovery system
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QUrl>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "../utils/error_handling.hpp"
#include "remote_plugin_registry_extension.hpp"
#include "remote_plugin_source.hpp"

namespace qtplugin {

/**
 * @brief Plugin discovery filter options
 */
struct PluginDiscoveryFilter {
    std::optional<std::string> name_pattern;     ///< Name pattern (regex)
    std::optional<std::string> category;         ///< Category filter
    std::vector<std::string> required_tags;      ///< Tags that must be present
    std::vector<std::string> excluded_tags;      ///< Tags that must not be present
    std::optional<std::string> author_pattern;   ///< Author pattern (regex)
    std::optional<std::string> license;          ///< License filter
    std::optional<double> min_rating;            ///< Minimum rating
    std::optional<std::string> version_range;    ///< Version range (semver)
    std::optional<qint64> max_size_bytes;        ///< Maximum plugin size
    bool verified_only = false;                  ///< Only verified plugins
    bool free_only = false;                      ///< Only free plugins
    
    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;
    
    /**
     * @brief Create from JSON representation
     */
    static PluginDiscoveryFilter from_json(const QJsonObject& json);
    
    /**
     * @brief Check if a discovery result matches this filter
     */
    bool matches(const RemotePluginDiscoveryResult& result) const;
};

/**
 * @brief Discovery operation progress information
 */
struct DiscoveryProgress {
    int sources_total = 0;        ///< Total number of sources to query
    int sources_completed = 0;    ///< Number of sources completed
    int plugins_found = 0;        ///< Total plugins found so far
    QString current_source;       ///< Currently processing source
    QString status_message;       ///< Current status message
    double progress_percentage = 0.0;  ///< Overall progress (0-100)
    
    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;
};

/**
 * @brief Discovery operation result
 */
struct DiscoveryResult {
    std::vector<RemotePluginDiscoveryResult> plugins;  ///< Discovered plugins
    std::vector<QString> failed_sources;               ///< Sources that failed
    std::vector<QString> error_messages;               ///< Error messages
    std::chrono::milliseconds total_time{0};           ///< Total discovery time
    int total_sources_queried = 0;                     ///< Number of sources queried
    
    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;
    
    /**
     * @brief Check if discovery was successful
     */
    bool is_successful() const { return !plugins.empty() || failed_sources.empty(); }
    
    /**
     * @brief Get success rate
     */
    double success_rate() const {
        if (total_sources_queried == 0) return 0.0;
        return static_cast<double>(total_sources_queried - failed_sources.size()) / total_sources_queried;
    }
};

/**
 * @brief Callback function types for discovery operations
 */
using DiscoveryProgressCallback = std::function<void(const DiscoveryProgress&)>;
using DiscoveryCompletionCallback = std::function<void(const qtplugin::expected<DiscoveryResult, PluginError>&)>;

/**
 * @brief Interface for plugin discovery engines
 */
class IPluginDiscoveryEngine {
public:
    virtual ~IPluginDiscoveryEngine() = default;
    
    /**
     * @brief Discover plugins from a specific source
     * @param source Remote plugin source
     * @param filter Discovery filter
     * @return Discovery results or error
     */
    virtual qtplugin::expected<std::vector<RemotePluginDiscoveryResult>, PluginError>
        discover_from_source(const RemotePluginSource& source, 
                           const PluginDiscoveryFilter& filter = {}) = 0;
    
    /**
     * @brief Discover plugins asynchronously from a specific source
     * @param source Remote plugin source
     * @param filter Discovery filter
     * @param progress_callback Progress callback
     * @param completion_callback Completion callback
     * @return Operation ID for tracking
     */
    virtual QString discover_from_source_async(
        const RemotePluginSource& source,
        const PluginDiscoveryFilter& filter = {},
        DiscoveryProgressCallback progress_callback = nullptr,
        DiscoveryCompletionCallback completion_callback = nullptr) = 0;
    
    /**
     * @brief Check if engine supports the given source type
     * @param source Remote plugin source
     * @return true if supported
     */
    virtual bool supports_source(const RemotePluginSource& source) const = 0;
    
    /**
     * @brief Get engine name
     * @return Engine name
     */
    virtual QString engine_name() const = 0;
    
    /**
     * @brief Get supported source types
     * @return Vector of supported source types
     */
    virtual std::vector<QString> supported_source_types() const = 0;
};

/**
 * @brief HTTP-based plugin discovery engine
 */
class HttpDiscoveryEngine : public QObject, public IPluginDiscoveryEngine {
    Q_OBJECT
    
public:
    explicit HttpDiscoveryEngine(QObject* parent = nullptr);
    ~HttpDiscoveryEngine() override;
    
    // IPluginDiscoveryEngine interface
    qtplugin::expected<std::vector<RemotePluginDiscoveryResult>, PluginError>
        discover_from_source(const RemotePluginSource& source, 
                           const PluginDiscoveryFilter& filter = {}) override;
    
    QString discover_from_source_async(
        const RemotePluginSource& source,
        const PluginDiscoveryFilter& filter = {},
        DiscoveryProgressCallback progress_callback = nullptr,
        DiscoveryCompletionCallback completion_callback = nullptr) override;
    
    bool supports_source(const RemotePluginSource& source) const override;
    QString engine_name() const override { return "HTTP Discovery Engine"; }
    std::vector<QString> supported_source_types() const override;
    
    // Configuration
    void set_timeout(std::chrono::seconds timeout) { m_timeout = timeout; }
    void set_max_concurrent_requests(int max_requests) { m_max_concurrent_requests = max_requests; }
    void set_user_agent(const QString& user_agent) { m_user_agent = user_agent; }

private slots:
    void on_network_reply_finished();
    void on_discovery_timeout();

private:
    QNetworkAccessManager* m_network_manager;
    std::chrono::seconds m_timeout{30};
    int m_max_concurrent_requests = 5;
    QString m_user_agent = "QtForge-PluginDiscovery/3.0.0";
    
    // Active operations
    struct DiscoveryOperation {
        QString operation_id;
        RemotePluginSource source;
        PluginDiscoveryFilter filter;
        DiscoveryProgressCallback progress_callback;
        DiscoveryCompletionCallback completion_callback;
        QNetworkReply* reply = nullptr;
        QTimer* timeout_timer = nullptr;
        std::chrono::steady_clock::time_point start_time;
    };
    
    std::unordered_map<QString, std::unique_ptr<DiscoveryOperation>> m_active_operations;
    
    // Helper methods
    QString generate_operation_id() const;
    QNetworkRequest create_discovery_request(const RemotePluginSource& source, 
                                            const PluginDiscoveryFilter& filter) const;
    qtplugin::expected<std::vector<RemotePluginDiscoveryResult>, PluginError>
        parse_discovery_response(const QByteArray& response, const RemotePluginSource& source) const;
    
    // Registry API support
    qtplugin::expected<std::vector<RemotePluginDiscoveryResult>, PluginError>
        discover_from_registry_api(const RemotePluginSource& source, 
                                 const PluginDiscoveryFilter& filter) const;
    
    // Direct URL support
    qtplugin::expected<std::vector<RemotePluginDiscoveryResult>, PluginError>
        discover_from_direct_url(const RemotePluginSource& source, 
                               const PluginDiscoveryFilter& filter) const;
    
    void cleanup_operation(const QString& operation_id);
};

/**
 * @brief Main plugin discovery manager
 */
class RemotePluginDiscoveryManager : public QObject {
    Q_OBJECT
    
public:
    explicit RemotePluginDiscoveryManager(QObject* parent = nullptr);
    ~RemotePluginDiscoveryManager() override;
    
    /**
     * @brief Register a discovery engine
     * @param engine Discovery engine
     */
    void register_engine(std::shared_ptr<IPluginDiscoveryEngine> engine);
    
    /**
     * @brief Unregister a discovery engine
     * @param engine_name Engine name
     */
    void unregister_engine(const QString& engine_name);
    
    /**
     * @brief Get registered engines
     * @return Vector of engine names
     */
    std::vector<QString> get_registered_engines() const;
    
    /**
     * @brief Discover plugins from all configured sources
     * @param sources Remote plugin sources
     * @param filter Discovery filter
     * @return Discovery result
     */
    qtplugin::expected<DiscoveryResult, PluginError> discover_plugins(
        const std::vector<RemotePluginSource>& sources,
        const PluginDiscoveryFilter& filter = {});
    
    /**
     * @brief Discover plugins asynchronously from all configured sources
     * @param sources Remote plugin sources
     * @param filter Discovery filter
     * @param progress_callback Progress callback
     * @param completion_callback Completion callback
     * @return Operation ID for tracking
     */
    QString discover_plugins_async(
        const std::vector<RemotePluginSource>& sources,
        const PluginDiscoveryFilter& filter = {},
        DiscoveryProgressCallback progress_callback = nullptr,
        DiscoveryCompletionCallback completion_callback = nullptr);
    
    /**
     * @brief Cancel discovery operation
     * @param operation_id Operation identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> cancel_discovery(const QString& operation_id);
    
    /**
     * @brief Get active discovery operations
     * @return Vector of operation IDs
     */
    std::vector<QString> get_active_operations() const;

signals:
    /**
     * @brief Emitted when plugins are discovered
     */
    void plugins_discovered(const QJsonObject& discovery_result);
    
    /**
     * @brief Emitted when discovery progress updates
     */
    void discovery_progress(const QString& operation_id, const QJsonObject& progress);
    
    /**
     * @brief Emitted when discovery completes
     */
    void discovery_completed(const QString& operation_id, bool success, const QString& error_message);

private:
    std::unordered_map<QString, std::shared_ptr<IPluginDiscoveryEngine>> m_engines;
    
    // Active discovery operations
    struct ManagedDiscoveryOperation {
        QString operation_id;
        std::vector<RemotePluginSource> sources;
        PluginDiscoveryFilter filter;
        DiscoveryProgressCallback progress_callback;
        DiscoveryCompletionCallback completion_callback;
        std::chrono::steady_clock::time_point start_time;
        
        // Per-source operation tracking
        std::unordered_map<QString, QString> source_operation_ids;  // source_id -> engine_operation_id
        DiscoveryResult partial_result;
        int completed_sources = 0;
    };
    
    std::unordered_map<QString, std::unique_ptr<ManagedDiscoveryOperation>> m_active_operations;
    mutable std::mutex m_operations_mutex;
    
    // Helper methods
    QString generate_operation_id() const;
    std::shared_ptr<IPluginDiscoveryEngine> find_engine_for_source(const RemotePluginSource& source) const;
    void handle_source_discovery_completed(const QString& managed_operation_id, 
                                         const QString& source_id,
                                         const qtplugin::expected<std::vector<RemotePluginDiscoveryResult>, PluginError>& result);
    void finalize_discovery_operation(const QString& operation_id);
};

}  // namespace qtplugin
