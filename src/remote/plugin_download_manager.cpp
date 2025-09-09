/**
 * @file plugin_download_manager.cpp
 * @brief Implementation of plugin download and caching management
 */

#include <QCryptographicHash>
#include <QDir>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QTimer>
#include <QUuid>
#include <fstream>
#include <qtplugin/remote/plugin_download_manager.hpp>

namespace qtplugin {

// === DownloadProgress Implementation ===

QJsonObject DownloadProgress::to_json() const {
    QJsonObject json;
    json["bytes_received"] = bytes_received;
    json["bytes_total"] = bytes_total;
    json["percentage"] = percentage;
    json["bytes_per_second"] = bytes_per_second;
    json["elapsed_time"] = static_cast<qint64>(elapsed_time.count());
    json["estimated_time_remaining"] =
        static_cast<qint64>(estimated_time_remaining.count());
    return json;
}

// === DownloadResult Implementation ===

QJsonObject DownloadResult::to_json() const {
    QJsonObject json;
    json["file_path"] = QString::fromStdString(file_path.string());
    json["file_size"] = file_size;
    json["checksum"] = checksum;
    json["content_type"] = content_type;
    json["download_time"] =
        QDateTime::fromSecsSinceEpoch(
            std::chrono::duration_cast<std::chrono::seconds>(
                download_time.time_since_epoch())
                .count())
            .toString(Qt::ISODate);
    json["download_duration"] = static_cast<qint64>(download_duration.count());
    json["metadata"] = metadata;
    return json;
}

DownloadResult DownloadResult::from_json(const QJsonObject& json) {
    DownloadResult result;
    result.file_path = json["file_path"].toString().toStdString();
    result.file_size = json["file_size"].toInt();
    result.checksum = json["checksum"].toString();
    result.content_type = json["content_type"].toString();

    QDateTime dt =
        QDateTime::fromString(json["download_time"].toString(), Qt::ISODate);
    result.download_time =
        std::chrono::system_clock::from_time_t(dt.toSecsSinceEpoch());
    result.download_duration =
        std::chrono::milliseconds(json["download_duration"].toInt());
    result.metadata = json["metadata"].toObject();

    return result;
}

// === DownloadOptions Implementation ===

QJsonObject DownloadOptions::to_json() const {
    QJsonObject json;
    json["cache_directory"] = QString::fromStdString(cache_directory.string());
    json["timeout"] = static_cast<qint64>(timeout.count());
    json["max_retries"] = max_retries;
    json["max_file_size"] = max_file_size;
    json["verify_checksum"] = verify_checksum;
    json["use_cache"] = use_cache;
    json["resume_partial"] = resume_partial;
    json["expected_checksum"] = expected_checksum;
    json["user_agent"] = user_agent;
    json["custom_headers"] = custom_headers;
    return json;
}

DownloadOptions DownloadOptions::from_json(const QJsonObject& json) {
    DownloadOptions options;
    options.cache_directory = json["cache_directory"].toString().toStdString();
    options.timeout = std::chrono::seconds(json["timeout"].toInt());
    options.max_retries = json["max_retries"].toInt();
    options.max_file_size = json["max_file_size"].toInt();
    options.verify_checksum = json["verify_checksum"].toBool();
    options.use_cache = json["use_cache"].toBool();
    options.resume_partial = json["resume_partial"].toBool();
    options.expected_checksum = json["expected_checksum"].toString();
    options.user_agent = json["user_agent"].toString();
    options.custom_headers = json["custom_headers"].toObject();
    return options;
}

// === CacheEntry Implementation ===

bool CacheEntry::is_valid() const {
    return std::filesystem::exists(file_path) && !is_expired();
}

bool CacheEntry::is_expired() const {
    auto now = std::chrono::system_clock::now();
    return (now - cached_time) > ttl;
}

QJsonObject CacheEntry::to_json() const {
    QJsonObject json;
    json["file_path"] = QString::fromStdString(file_path.string());
    json["source_url"] = source_url.toString();
    json["cached_time"] = QDateTime::fromSecsSinceEpoch(
                              std::chrono::duration_cast<std::chrono::seconds>(
                                  cached_time.time_since_epoch())
                                  .count())
                              .toString(Qt::ISODate);
    json["ttl"] = static_cast<qint64>(ttl.count());
    json["checksum"] = checksum;
    json["file_size"] = file_size;
    json["metadata"] = metadata;
    return json;
}

CacheEntry CacheEntry::from_json(const QJsonObject& json) {
    CacheEntry entry;
    entry.file_path = json["file_path"].toString().toStdString();
    entry.source_url = QUrl(json["source_url"].toString());

    QDateTime dt =
        QDateTime::fromString(json["cached_time"].toString(), Qt::ISODate);
    entry.cached_time =
        std::chrono::system_clock::from_time_t(dt.toSecsSinceEpoch());
    entry.ttl = std::chrono::seconds(json["ttl"].toInt());
    entry.checksum = json["checksum"].toString();
    entry.file_size = json["file_size"].toInt();
    entry.metadata = json["metadata"].toObject();

    return entry;
}

// === PluginDownloadManager Implementation ===

PluginDownloadManager::PluginDownloadManager(QObject* parent)
    : QObject(parent),
      m_network_manager(std::make_unique<QNetworkAccessManager>(this)) {
    // Set default cache directory
    QString cache_path =
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    m_cache_directory =
        std::filesystem::path(cache_path.toStdString()) / "qtforge" / "plugins";

    // Create cache directory if it doesn't exist
    std::filesystem::create_directories(m_cache_directory);

    // Load cache index
    load_cache_index();

    // Set default options
    m_default_options.cache_directory = m_cache_directory;
}

PluginDownloadManager::~PluginDownloadManager() {
    // Cancel all active downloads
    std::lock_guard<std::mutex> lock(m_downloads_mutex);
    for (auto& pair : m_active_downloads) {
        if (pair.second->reply) {
            pair.second->reply->abort();
        }
    }

    // Save cache index
    save_cache_index();
}

qtplugin::expected<DownloadResult, PluginError>
PluginDownloadManager::download_plugin(const RemotePluginSource& source,
                                       const QUrl& plugin_url,
                                       const DownloadOptions& options) {
    // Validate options
    auto validation_result = validate_download_options(options);
    if (!validation_result) {
        return qtplugin::unexpected(validation_result.error());
    }

    QUrl target_url = plugin_url.isEmpty() ? source.url() : plugin_url;

    // Check cache first if enabled
    if (options.use_cache && is_cached(target_url)) {
        auto cached_path = get_cached_path(target_url);
        if (cached_path) {
            m_cache_hits++;

            DownloadResult result;
            result.file_path = *cached_path;
            result.file_size = std::filesystem::file_size(*cached_path);
            result.checksum = calculate_checksum(*cached_path);
            result.download_time = std::chrono::system_clock::now();
            result.download_duration = std::chrono::milliseconds{0};

            return result;
        }
    }

    // Perform synchronous download
    QNetworkRequest request(target_url);
    auto setup_result = setup_network_request(request, source, options);
    if (!setup_result) {
        return qtplugin::unexpected(setup_result.error());
    }

    QNetworkReply* reply = m_network_manager->get(request);

    // Set timeout
    QTimer timeout_timer;
    timeout_timer.setSingleShot(true);
    timeout_timer.start(static_cast<int>(options.timeout.count() * 1000));

    // Wait for completion
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timeout_timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    loop.exec();

    if (timeout_timer.isActive()) {
        timeout_timer.stop();

        if (reply->error() == QNetworkReply::NoError) {
            // Download successful
            QByteArray data = reply->readAll();

            // Generate cache path
            std::filesystem::path cache_path = generate_cache_path(target_url);

            // Write to file
            std::ofstream file(cache_path, std::ios::binary);
            if (!file) {
                reply->deleteLater();
                return qtplugin::make_error<DownloadResult>(
                    PluginErrorCode::FileSystemError,
                    "Failed to write downloaded file");
            }

            file.write(data.data(), data.size());
            file.close();

            // Calculate checksum
            QString checksum = calculate_checksum(cache_path);

            // Verify checksum if provided
            if (!options.expected_checksum.isEmpty() &&
                checksum != options.expected_checksum) {
                std::filesystem::remove(cache_path);
                reply->deleteLater();
                return qtplugin::make_error<DownloadResult>(
                    PluginErrorCode::SecurityViolation,
                    "Checksum verification failed");
            }

            // Update cache
            CacheEntry cache_entry;
            cache_entry.file_path = cache_path;
            cache_entry.source_url = target_url;
            cache_entry.cached_time = std::chrono::system_clock::now();
            cache_entry.ttl = std::chrono::seconds{3600};  // 1 hour
            cache_entry.checksum = checksum;
            cache_entry.file_size = data.size();

            {
                std::lock_guard<std::mutex> lock(m_cache_mutex);
                m_cache_entries[target_url.toString()] = cache_entry;
            }

            // Update statistics
            m_total_downloads++;
            m_successful_downloads++;
            m_bytes_downloaded += data.size();

            // Create result
            DownloadResult result;
            result.file_path = cache_path;
            result.file_size = data.size();
            result.checksum = checksum;
            result.content_type =
                reply->header(QNetworkRequest::ContentTypeHeader).toString();
            result.download_time = std::chrono::system_clock::now();
            result.download_duration =
                std::chrono::milliseconds{0};  // TODO: Track actual time

            reply->deleteLater();
            return result;

        } else {
            // Download failed
            QString error_msg = reply->errorString();
            reply->deleteLater();

            m_total_downloads++;
            m_failed_downloads++;

            return qtplugin::make_error<DownloadResult>(
                PluginErrorCode::NetworkError,
                "Download failed: " + error_msg.toStdString());
        }
    } else {
        // Timeout
        reply->abort();
        reply->deleteLater();

        m_total_downloads++;
        m_failed_downloads++;

        return qtplugin::make_error<DownloadResult>(
            PluginErrorCode::NetworkError, "Download timed out");
    }
}

QString PluginDownloadManager::download_plugin_async(
    const RemotePluginSource& source, const QUrl& plugin_url,
    const DownloadOptions& options,
    std::function<void(const DownloadProgress&)> progress_callback,
    std::function<void(const qtplugin::expected<DownloadResult, PluginError>&)>
        completion_callback) {
    // Validate options
    auto validation_result = validate_download_options(options);
    if (!validation_result) {
        if (completion_callback) {
            completion_callback(
                qtplugin::unexpected(validation_result.error()));
        }
        return QString();
    }

    QString download_id = generate_download_id();
    QUrl target_url = plugin_url.isEmpty() ? source.url() : plugin_url;

    // Check cache first if enabled
    if (options.use_cache && is_cached(target_url)) {
        auto cached_path = get_cached_path(target_url);
        if (cached_path) {
            m_cache_hits++;

            DownloadResult result;
            result.file_path = *cached_path;
            result.file_size = std::filesystem::file_size(*cached_path);
            result.checksum = calculate_checksum(*cached_path);
            result.download_time = std::chrono::system_clock::now();
            result.download_duration = std::chrono::milliseconds{0};

            if (completion_callback) {
                completion_callback(result);
            }

            emit download_completed(download_id, result);
            return download_id;
        }
    }

    // Create download info
    auto download_info = std::make_unique<DownloadInfo>(download_id, source);
    download_info->url = target_url;
    download_info->options = options;
    download_info->target_path = generate_cache_path(target_url);
    download_info->start_time = std::chrono::system_clock::now();
    download_info->progress_callback = progress_callback;
    download_info->completion_callback = completion_callback;

    // Setup network request
    QNetworkRequest request(target_url);
    auto setup_result = setup_network_request(request, source, options);
    if (!setup_result) {
        if (completion_callback) {
            completion_callback(qtplugin::unexpected(setup_result.error()));
        }
        return QString();
    }

    // Start download
    download_info->reply = m_network_manager->get(request);

    // Connect signals
    connect(download_info->reply, &QNetworkReply::downloadProgress, this,
            &PluginDownloadManager::on_download_progress);
    connect(download_info->reply, &QNetworkReply::finished, this,
            &PluginDownloadManager::on_download_finished);
    connect(download_info->reply,
            QOverload<QNetworkReply::NetworkError>::of(
                &QNetworkReply::errorOccurred),
            this, &PluginDownloadManager::on_download_error);

    // Store download info
    {
        std::lock_guard<std::mutex> lock(m_downloads_mutex);
        m_active_downloads[download_id] = std::move(download_info);
    }

    return download_id;
}

qtplugin::expected<void, PluginError> PluginDownloadManager::cancel_download(
    const QString& download_id) {
    std::lock_guard<std::mutex> lock(m_downloads_mutex);

    auto it = m_active_downloads.find(download_id);
    if (it == m_active_downloads.end()) {
        return qtplugin::make_error<void>(
            PluginErrorCode::NotFound,
            "Download not found: " + download_id.toStdString());
    }

    if (it->second->reply) {
        it->second->reply->abort();
    }

    emit download_cancelled(download_id);
    m_active_downloads.erase(it);

    return qtplugin::make_success();
}

std::optional<DownloadProgress> PluginDownloadManager::get_download_progress(
    const QString& download_id) const {
    std::lock_guard<std::mutex> lock(m_downloads_mutex);

    auto it = m_active_downloads.find(download_id);
    if (it != m_active_downloads.end()) {
        return it->second->progress;
    }

    return std::nullopt;
}

qtplugin::expected<void, PluginError>
PluginDownloadManager::set_cache_directory(
    const std::filesystem::path& directory) {
    if (!std::filesystem::exists(directory)) {
        std::error_code ec;
        std::filesystem::create_directories(directory, ec);
        if (ec) {
            return qtplugin::make_error<void>(
                PluginErrorCode::FileSystemError,
                "Failed to create cache directory: " + ec.message());
        }
    }

    m_cache_directory = directory;
    m_default_options.cache_directory = directory;

    return qtplugin::make_success();
}

bool PluginDownloadManager::is_cached(const QUrl& url) const {
    std::lock_guard<std::mutex> lock(m_cache_mutex);

    auto it = m_cache_entries.find(url.toString());
    if (it != m_cache_entries.end()) {
        return it->second.is_valid();
    }

    return false;
}

std::optional<std::filesystem::path> PluginDownloadManager::get_cached_path(
    const QUrl& url) const {
    std::lock_guard<std::mutex> lock(m_cache_mutex);

    auto it = m_cache_entries.find(url.toString());
    if (it != m_cache_entries.end() && it->second.is_valid()) {
        return it->second.file_path;
    }

    return std::nullopt;
}

// === Helper Methods ===

QString PluginDownloadManager::generate_download_id() const {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

std::filesystem::path PluginDownloadManager::generate_cache_path(
    const QUrl& url) const {
    // Generate a unique filename based on URL
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(url.toString().toUtf8());
    QString filename = hash.result().toHex() + ".qtplugin";

    return m_cache_directory / filename.toStdString();
}

QString PluginDownloadManager::calculate_checksum(
    const std::filesystem::path& file_path) const {
    QFile file(QString::fromStdString(file_path.string()));
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(&file);
    return hash.result().toHex();
}

qtplugin::expected<void, PluginError>
PluginDownloadManager::validate_download_options(
    const DownloadOptions& options) const {
    if (options.timeout.count() <= 0) {
        return qtplugin::make_error<void>(PluginErrorCode::InvalidConfiguration,
                                          "Invalid timeout value");
    }

    if (options.max_file_size == 0) {
        return qtplugin::make_error<void>(PluginErrorCode::InvalidConfiguration,
                                          "Invalid max file size");
    }

    if (options.max_retries < 0) {
        return qtplugin::make_error<void>(PluginErrorCode::InvalidConfiguration,
                                          "Invalid max retries value");
    }

    return qtplugin::make_success();
}

qtplugin::expected<void, PluginError>
PluginDownloadManager::setup_network_request(
    QNetworkRequest& request, const RemotePluginSource& source,
    const DownloadOptions& options) const {
    // Set user agent
    request.setHeader(QNetworkRequest::UserAgentHeader, options.user_agent);

    // Set timeout
    request.setTransferTimeout(
        static_cast<int>(options.timeout.count() * 1000));

    // Add authentication if configured
    if (source.has_authentication()) {
        const auto& auth = source.authentication();

        if (auth.type == AuthenticationType::Basic) {
            QString credentials = auth.username + ":" + auth.password;
            QString encoded = credentials.toUtf8().toBase64();
            request.setRawHeader("Authorization", "Basic " + encoded.toUtf8());
        } else if (auth.type == AuthenticationType::Bearer) {
            request.setRawHeader("Authorization",
                                 "Bearer " + auth.token.toUtf8());
        } else if (auth.type == AuthenticationType::ApiKey) {
            request.setRawHeader("X-API-Key", auth.api_key.toUtf8());
        }
    }

    // Add custom headers
    for (auto it = options.custom_headers.begin();
         it != options.custom_headers.end(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toString().toUtf8());
    }

    return qtplugin::make_success();
}

qtplugin::expected<void, PluginError>
PluginDownloadManager::load_cache_index() {
    std::filesystem::path index_path = m_cache_directory / "cache_index.json";

    if (!std::filesystem::exists(index_path)) {
        return qtplugin::make_success();  // No index file yet
    }

    QFile file(QString::fromStdString(index_path.string()));
    if (!file.open(QIODevice::ReadOnly)) {
        return qtplugin::make_error<void>(PluginErrorCode::FileSystemError,
                                          "Failed to open cache index file");
    }

    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parse_error);
    if (parse_error.error != QJsonParseError::NoError) {
        return qtplugin::make_error<void>(
            PluginErrorCode::InvalidFormat,
            "Failed to parse cache index: " +
                parse_error.errorString().toStdString());
    }

    QJsonObject root = doc.object();
    QJsonArray entries = root["entries"].toArray();

    std::lock_guard<std::mutex> lock(m_cache_mutex);
    m_cache_entries.clear();

    for (const auto& entry_value : entries) {
        if (!entry_value.isObject()) {
            continue;
        }

        try {
            CacheEntry entry = CacheEntry::from_json(entry_value.toObject());
            m_cache_entries[entry.source_url.toString()] = entry;
        } catch (const std::exception& e) {
            // Skip invalid entries
            continue;
        }
    }

    return qtplugin::make_success();
}

qtplugin::expected<void, PluginError> PluginDownloadManager::save_cache_index()
    const {
    std::filesystem::path index_path = m_cache_directory / "cache_index.json";

    QJsonArray entries;
    {
        std::lock_guard<std::mutex> lock(m_cache_mutex);
        for (const auto& pair : m_cache_entries) {
            entries.append(pair.second.to_json());
        }
    }

    QJsonObject root;
    root["version"] = "1.0";
    root["entries"] = entries;

    QJsonDocument doc(root);

    QFile file(QString::fromStdString(index_path.string()));
    if (!file.open(QIODevice::WriteOnly)) {
        return qtplugin::make_error<void>(
            PluginErrorCode::FileSystemError,
            "Failed to open cache index file for writing");
    }

    file.write(doc.toJson());
    return qtplugin::make_success();
}

// === Slot Implementations ===

void PluginDownloadManager::on_download_progress(qint64 bytes_received,
                                                 qint64 bytes_total) {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;

    // Find download info
    std::lock_guard<std::mutex> lock(m_downloads_mutex);
    for (auto& pair : m_active_downloads) {
        if (pair.second->reply == reply) {
            auto& progress = pair.second->progress;
            progress.bytes_received = bytes_received;
            progress.bytes_total = bytes_total;

            if (bytes_total > 0) {
                progress.percentage =
                    (static_cast<double>(bytes_received) / bytes_total) * 100.0;
            }

            // Calculate speed and time estimates
            auto now = std::chrono::system_clock::now();
            progress.elapsed_time =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - pair.second->start_time);

            if (progress.elapsed_time.count() > 0) {
                progress.bytes_per_second =
                    (bytes_received * 1000) / progress.elapsed_time.count();

                if (progress.bytes_per_second > 0 &&
                    bytes_total > bytes_received) {
                    qint64 remaining_bytes = bytes_total - bytes_received;
                    progress.estimated_time_remaining =
                        std::chrono::milliseconds((remaining_bytes * 1000) /
                                                  progress.bytes_per_second);
                }
            }

            // Emit signals
            emit download_progress(pair.first, progress);

            if (pair.second->progress_callback) {
                pair.second->progress_callback(progress);
            }

            break;
        }
    }
}

void PluginDownloadManager::on_download_finished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;

    // Find and process download
    std::lock_guard<std::mutex> lock(m_downloads_mutex);
    for (auto it = m_active_downloads.begin(); it != m_active_downloads.end();
         ++it) {
        if (it->second->reply == reply) {
            QString download_id = it->first;
            auto& download_info = it->second;

            if (reply->error() == QNetworkReply::NoError) {
                // Download successful - process the result
                QByteArray data = reply->readAll();

                // Write to file
                std::ofstream file(download_info->target_path,
                                   std::ios::binary);
                if (!file) {
                    PluginError error{PluginErrorCode::FileSystemError,
                                      "Failed to write downloaded file"};
                    emit download_failed(download_id, error);
                    if (download_info->completion_callback) {
                        download_info->completion_callback(
                            qtplugin::unexpected(error));
                    }
                    return;
                }

                file.write(data.data(), data.size());
                file.close();

                // Calculate checksum
                QString checksum =
                    calculate_checksum(download_info->target_path);

                // Verify checksum if provided
                if (!download_info->options.expected_checksum.isEmpty() &&
                    checksum != download_info->options.expected_checksum) {
                    std::filesystem::remove(download_info->target_path);
                    PluginError error{PluginErrorCode::SecurityViolation,
                                      "Checksum verification failed"};
                    emit download_failed(download_id, error);
                    if (download_info->completion_callback) {
                        download_info->completion_callback(
                            qtplugin::unexpected(error));
                    }
                    return;
                }

                // Update cache
                CacheEntry cache_entry;
                cache_entry.file_path = download_info->target_path;
                cache_entry.source_url = download_info->url;
                cache_entry.cached_time = std::chrono::system_clock::now();
                cache_entry.ttl = std::chrono::seconds{3600};  // 1 hour
                cache_entry.checksum = checksum;
                cache_entry.file_size = data.size();

                {
                    std::lock_guard<std::mutex> cache_lock(m_cache_mutex);
                    m_cache_entries[download_info->url.toString()] =
                        cache_entry;
                }

                // Create result
                DownloadResult result;
                result.file_path = download_info->target_path;
                result.file_size = data.size();
                result.checksum = checksum;
                result.content_type =
                    reply->header(QNetworkRequest::ContentTypeHeader)
                        .toString();
                result.download_time = std::chrono::system_clock::now();
                result.download_duration =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        result.download_time - download_info->start_time);

                // Update statistics
                m_total_downloads++;
                m_successful_downloads++;
                m_bytes_downloaded += data.size();

                // Emit signals and call callback
                emit download_completed(download_id, result);
                if (download_info->completion_callback) {
                    download_info->completion_callback(result);
                }
            }

            reply->deleteLater();
            m_active_downloads.erase(it);
            break;
        }
    }
}

void PluginDownloadManager::on_download_error(
    QNetworkReply::NetworkError error) {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;

    // Find download and handle error
    std::lock_guard<std::mutex> lock(m_downloads_mutex);
    for (auto it = m_active_downloads.begin(); it != m_active_downloads.end();
         ++it) {
        if (it->second->reply == reply) {
            QString download_id = it->first;
            auto& download_info = it->second;

            // Check if we should retry
            if (should_retry_download(*download_info, error)) {
                download_info->retry_count++;
                // Retry logic would go here
                return;
            }

            // Create error and notify
            PluginError plugin_error{
                PluginErrorCode::NetworkError,
                "Download failed: " + reply->errorString().toStdString()};

            emit download_failed(download_id, plugin_error);

            if (download_info->completion_callback) {
                download_info->completion_callback(
                    qtplugin::unexpected(plugin_error));
            }

            m_total_downloads++;
            m_failed_downloads++;

            reply->deleteLater();
            m_active_downloads.erase(it);
            break;
        }
    }
}

bool PluginDownloadManager::should_retry_download(
    const DownloadInfo& info, QNetworkReply::NetworkError error) const {
    if (info.retry_count >= info.options.max_retries) {
        return false;
    }

    // Retry on certain network errors
    switch (error) {
        case QNetworkReply::TimeoutError:
        case QNetworkReply::TemporaryNetworkFailureError:
        case QNetworkReply::NetworkSessionFailedError:
        case QNetworkReply::BackgroundRequestNotAllowedError:
            return true;
        default:
            return false;
    }
}

// === Additional Methods ===

qtplugin::expected<void, PluginError> PluginDownloadManager::clear_cache_entry(
    const QUrl& url) {
    std::lock_guard<std::mutex> lock(m_cache_mutex);

    auto it = m_cache_entries.find(url.toString());
    if (it != m_cache_entries.end()) {
        // Remove file if it exists
        std::error_code ec;
        std::filesystem::remove(it->second.file_path, ec);

        // Remove from cache entries
        m_cache_entries.erase(it);

        return qtplugin::make_success();
    }

    return qtplugin::make_error<void>(
        PluginErrorCode::NotFound,
        "Cache entry not found for URL: " + url.toString().toStdString());
}

int PluginDownloadManager::clear_cache() {
    std::lock_guard<std::mutex> lock(m_cache_mutex);

    int cleared_count = 0;
    for (const auto& pair : m_cache_entries) {
        std::error_code ec;
        if (std::filesystem::remove(pair.second.file_path, ec)) {
            cleared_count++;
        }
    }

    m_cache_entries.clear();
    return cleared_count;
}

qint64 PluginDownloadManager::cache_size() const {
    std::lock_guard<std::mutex> lock(m_cache_mutex);

    qint64 total_size = 0;
    for (const auto& pair : m_cache_entries) {
        if (std::filesystem::exists(pair.second.file_path)) {
            std::error_code ec;
            total_size += std::filesystem::file_size(pair.second.file_path, ec);
        }
    }

    return total_size;
}

int PluginDownloadManager::cache_entry_count() const {
    std::lock_guard<std::mutex> lock(m_cache_mutex);
    return static_cast<int>(m_cache_entries.size());
}

int PluginDownloadManager::cleanup_expired_cache() {
    std::lock_guard<std::mutex> lock(m_cache_mutex);

    int cleaned_count = 0;
    auto it = m_cache_entries.begin();
    while (it != m_cache_entries.end()) {
        if (it->second.is_expired()) {
            std::error_code ec;
            std::filesystem::remove(it->second.file_path, ec);
            it = m_cache_entries.erase(it);
            cleaned_count++;
        } else {
            ++it;
        }
    }

    return cleaned_count;
}

void PluginDownloadManager::set_default_options(
    const DownloadOptions& options) {
    m_default_options = options;
}

void PluginDownloadManager::set_network_manager(
    std::unique_ptr<QNetworkAccessManager> manager) {
    m_network_manager = std::move(manager);
}

QJsonObject PluginDownloadManager::get_statistics() const {
    QJsonObject stats;
    stats["total_downloads"] = static_cast<qint64>(m_total_downloads.load());
    stats["successful_downloads"] =
        static_cast<qint64>(m_successful_downloads.load());
    stats["failed_downloads"] = static_cast<qint64>(m_failed_downloads.load());
    stats["bytes_downloaded"] = static_cast<qint64>(m_bytes_downloaded.load());
    stats["cache_hits"] = static_cast<qint64>(m_cache_hits.load());
    stats["cache_size"] = cache_size();
    stats["cache_entries"] = cache_entry_count();

    // Calculate success rate
    qint64 total = m_total_downloads.load();
    if (total > 0) {
        stats["success_rate"] =
            static_cast<double>(m_successful_downloads.load()) / total;
    } else {
        stats["success_rate"] = 0.0;
    }

    return stats;
}

void PluginDownloadManager::reset_statistics() {
    m_total_downloads = 0;
    m_successful_downloads = 0;
    m_failed_downloads = 0;
    m_bytes_downloaded = 0;
    m_cache_hits = 0;
}

}  // namespace qtplugin
