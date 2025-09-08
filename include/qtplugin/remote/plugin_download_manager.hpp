/**
 * @file plugin_download_manager.hpp
 * @brief Plugin download and caching management
 * @version 3.0.0
 */

#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QTimer>
#include <QUrl>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include "../utils/error_handling.hpp"
#include "remote_plugin_source.hpp"

namespace qtplugin {

/**
 * @brief Download progress information
 */
struct DownloadProgress {
    qint64 bytes_received = 0;
    qint64 bytes_total = 0;
    double percentage = 0.0;
    qint64 bytes_per_second = 0;
    std::chrono::milliseconds elapsed_time{0};
    std::chrono::milliseconds estimated_time_remaining{0};
    
    /**
     * @brief Check if download is complete
     */
    bool is_complete() const { return bytes_total > 0 && bytes_received >= bytes_total; }
    
    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;
};

/**
 * @brief Download result information
 */
struct DownloadResult {
    std::filesystem::path file_path;
    qint64 file_size = 0;
    QString checksum;
    QString content_type;
    std::chrono::system_clock::time_point download_time;
    std::chrono::milliseconds download_duration{0};
    QJsonObject metadata;
    
    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;
    
    /**
     * @brief Create from JSON representation
     */
    static DownloadResult from_json(const QJsonObject& json);
};

/**
 * @brief Download options and configuration
 */
struct DownloadOptions {
    std::filesystem::path cache_directory;
    std::chrono::seconds timeout{30};
    int max_retries = 3;
    qint64 max_file_size = 100 * 1024 * 1024;  // 100MB
    bool verify_checksum = true;
    bool use_cache = true;
    bool resume_partial = true;
    QString expected_checksum;
    QString user_agent = "QtForge-PluginDownloader/3.0.0";
    QJsonObject custom_headers;
    
    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;
    
    /**
     * @brief Create from JSON representation
     */
    static DownloadOptions from_json(const QJsonObject& json);
};

/**
 * @brief Cache entry information
 */
struct CacheEntry {
    std::filesystem::path file_path;
    QUrl source_url;
    std::chrono::system_clock::time_point cached_time;
    std::chrono::seconds ttl{3600};  // 1 hour default
    QString checksum;
    qint64 file_size = 0;
    QJsonObject metadata;
    
    /**
     * @brief Check if cache entry is valid (not expired)
     */
    bool is_valid() const;
    
    /**
     * @brief Check if cache entry is expired
     */
    bool is_expired() const;
    
    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;
    
    /**
     * @brief Create from JSON representation
     */
    static CacheEntry from_json(const QJsonObject& json);
};

/**
 * @brief Plugin download manager
 * 
 * Handles downloading plugins from remote sources with caching,
 * progress tracking, retry logic, and resume capabilities.
 */
class PluginDownloadManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent QObject
     */
    explicit PluginDownloadManager(QObject* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~PluginDownloadManager() override;

    // === Download Operations ===
    
    /**
     * @brief Download plugin from remote source
     * @param source Remote plugin source
     * @param plugin_url Specific plugin URL (if different from source URL)
     * @param options Download options
     * @return Download result or error
     */
    qtplugin::expected<DownloadResult, PluginError> download_plugin(
        const RemotePluginSource& source,
        const QUrl& plugin_url = QUrl(),
        const DownloadOptions& options = {});
    
    /**
     * @brief Download plugin asynchronously
     * @param source Remote plugin source
     * @param plugin_url Specific plugin URL (if different from source URL)
     * @param options Download options
     * @param progress_callback Progress callback function
     * @param completion_callback Completion callback function
     * @return Download ID for tracking
     */
    QString download_plugin_async(
        const RemotePluginSource& source,
        const QUrl& plugin_url = QUrl(),
        const DownloadOptions& options = {},
        std::function<void(const DownloadProgress&)> progress_callback = nullptr,
        std::function<void(const qtplugin::expected<DownloadResult, PluginError>&)> completion_callback = nullptr);
    
    /**
     * @brief Cancel ongoing download
     * @param download_id Download identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> cancel_download(const QString& download_id);
    
    /**
     * @brief Get download progress
     * @param download_id Download identifier
     * @return Progress information or nullopt if not found
     */
    std::optional<DownloadProgress> get_download_progress(const QString& download_id) const;

    // === Cache Management ===
    
    /**
     * @brief Set cache directory
     * @param directory Cache directory path
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> set_cache_directory(const std::filesystem::path& directory);
    
    /**
     * @brief Get cache directory
     */
    std::filesystem::path cache_directory() const { return m_cache_directory; }
    
    /**
     * @brief Check if plugin is cached
     * @param url Plugin URL
     * @return true if cached and valid
     */
    bool is_cached(const QUrl& url) const;
    
    /**
     * @brief Get cached plugin path
     * @param url Plugin URL
     * @return Cached file path or nullopt if not cached
     */
    std::optional<std::filesystem::path> get_cached_path(const QUrl& url) const;
    
    /**
     * @brief Clear cache entry
     * @param url Plugin URL
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> clear_cache_entry(const QUrl& url);
    
    /**
     * @brief Clear all cache
     * @return Number of entries cleared
     */
    int clear_cache();
    
    /**
     * @brief Get cache size in bytes
     */
    qint64 cache_size() const;
    
    /**
     * @brief Get cache entry count
     */
    int cache_entry_count() const;
    
    /**
     * @brief Cleanup expired cache entries
     * @return Number of entries cleaned up
     */
    int cleanup_expired_cache();

    // === Configuration ===
    
    /**
     * @brief Set default download options
     */
    void set_default_options(const DownloadOptions& options);
    
    /**
     * @brief Get default download options
     */
    const DownloadOptions& default_options() const { return m_default_options; }
    
    /**
     * @brief Set network access manager
     * @param manager Custom network access manager
     */
    void set_network_manager(std::unique_ptr<QNetworkAccessManager> manager);
    
    /**
     * @brief Get network access manager
     */
    QNetworkAccessManager* network_manager() const { return m_network_manager.get(); }

    // === Statistics ===
    
    /**
     * @brief Get download statistics
     */
    QJsonObject get_statistics() const;
    
    /**
     * @brief Reset statistics
     */
    void reset_statistics();

signals:
    /**
     * @brief Emitted when download progress updates
     * @param download_id Download identifier
     * @param progress Progress information
     */
    void download_progress(const QString& download_id, const DownloadProgress& progress);
    
    /**
     * @brief Emitted when download completes successfully
     * @param download_id Download identifier
     * @param result Download result
     */
    void download_completed(const QString& download_id, const DownloadResult& result);
    
    /**
     * @brief Emitted when download fails
     * @param download_id Download identifier
     * @param error Error information
     */
    void download_failed(const QString& download_id, const PluginError& error);
    
    /**
     * @brief Emitted when download is cancelled
     * @param download_id Download identifier
     */
    void download_cancelled(const QString& download_id);

private slots:
    void on_download_progress(qint64 bytes_received, qint64 bytes_total);
    void on_download_finished();
    void on_download_error(QNetworkReply::NetworkError error);

private:
    // Network management
    std::unique_ptr<QNetworkAccessManager> m_network_manager;
    
    // Cache management
    std::filesystem::path m_cache_directory;
    mutable std::mutex m_cache_mutex;
    std::unordered_map<QString, CacheEntry> m_cache_entries;
    
    // Download tracking
    struct DownloadInfo {
        QString id;
        QNetworkReply* reply;
        RemotePluginSource source;
        QUrl url;
        DownloadOptions options;
        std::filesystem::path target_path;
        DownloadProgress progress;
        std::chrono::system_clock::time_point start_time;
        int retry_count = 0;
        std::function<void(const DownloadProgress&)> progress_callback;
        std::function<void(const qtplugin::expected<DownloadResult, PluginError>&)> completion_callback;
    };
    
    mutable std::mutex m_downloads_mutex;
    std::unordered_map<QString, std::unique_ptr<DownloadInfo>> m_active_downloads;
    
    // Configuration
    DownloadOptions m_default_options;
    
    // Statistics
    mutable std::mutex m_stats_mutex;
    std::atomic<qint64> m_total_downloads{0};
    std::atomic<qint64> m_successful_downloads{0};
    std::atomic<qint64> m_failed_downloads{0};
    std::atomic<qint64> m_bytes_downloaded{0};
    std::atomic<qint64> m_cache_hits{0};
    
    // Helper methods
    QString generate_download_id() const;
    std::filesystem::path generate_cache_path(const QUrl& url) const;
    QString calculate_checksum(const std::filesystem::path& file_path) const;
    qtplugin::expected<void, PluginError> validate_download_options(const DownloadOptions& options) const;
    qtplugin::expected<void, PluginError> setup_network_request(QNetworkRequest& request, 
                                                               const RemotePluginSource& source,
                                                               const DownloadOptions& options) const;
    qtplugin::expected<void, PluginError> load_cache_index();
    qtplugin::expected<void, PluginError> save_cache_index() const;
    void cleanup_download(const QString& download_id);
    bool should_retry_download(const DownloadInfo& info, QNetworkReply::NetworkError error) const;
    qtplugin::expected<void, PluginError> retry_download(const QString& download_id);
};

}  // namespace qtplugin
