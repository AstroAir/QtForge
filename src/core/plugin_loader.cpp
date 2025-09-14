/**
 * @file plugin_loader.cpp
 * @brief Implementation of plugin loader
 * @version 3.0.0
 */

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLibrary>
#include <QPluginLoader>
#include <QCryptographicHash>
#include <QDateTime>
#include <mutex>
#include <qtplugin/core/plugin_loader.hpp>
#include <shared_mutex>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <QStandardPaths>
#include <QDataStream>
#include <QTimer>
#include <QThreadPool>
#include <QtConcurrent>
#include <algorithm>
#include <future>
#include <queue>
#include <thread>

namespace qtplugin {

namespace {
// Performance optimization constants
constexpr size_t CACHE_PREWARM_SIZE = 50;
constexpr auto CACHE_PERSISTENCE_INTERVAL = std::chrono::seconds(30);
constexpr size_t MIN_PARALLEL_LOAD_THRESHOLD = 3;
const size_t MAX_CONCURRENT_LOADS = std::thread::hardware_concurrency();
constexpr auto LOAD_TIMEOUT = std::chrono::seconds(30);

// Cache persistence helper
QString get_cache_persistence_path() {
    auto cache_dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    return QDir(cache_dir).filePath("qtplugin_metadata_cache.dat");
}

// Fast hash function removed - was unused

// Parallel loading thread pool (singleton)
class PluginLoadThreadPool {
public:
    static PluginLoadThreadPool& instance() {
        static PluginLoadThreadPool pool;
        return pool;
    }

    template <typename Func>
    auto submit(Func&& func) -> std::future<decltype(func())> {
        using ReturnType = decltype(func());
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::forward<Func>(func));
        auto future = task->get_future();

        {
            std::unique_lock lock(m_mutex);
            if (m_stop) {
                throw std::runtime_error("Thread pool is stopped");
            }
            m_tasks.emplace([task]() { (*task)(); });
        }
        m_cv.notify_one();

        return future;
    }

    void set_max_threads(size_t count) {
        std::unique_lock lock(m_mutex);
        m_max_threads = std::min(count, MAX_CONCURRENT_LOADS);
    }

    size_t get_queue_size() const {
        std::shared_lock lock(m_mutex);
        return m_tasks.size();
    }

private:
    PluginLoadThreadPool() : m_max_threads(MAX_CONCURRENT_LOADS) {
        for (size_t i = 0; i < m_max_threads; ++i) {
            m_workers.emplace_back([this] { worker_thread(); });
        }
    }

    ~PluginLoadThreadPool() {
        {
            std::unique_lock lock(m_mutex);
            m_stop = true;
        }
        m_cv.notify_all();

        for (auto& worker : m_workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    void worker_thread() {
        while (true) {
            std::function<void()> task;

            {
                std::unique_lock lock(m_mutex);
                m_cv.wait(lock, [this] { return m_stop || !m_tasks.empty(); });

                if (m_stop && m_tasks.empty()) {
                    return;
                }

                task = std::move(m_tasks.front());
                m_tasks.pop();
            }

            task();
        }
    }

    mutable std::shared_mutex m_mutex;
    std::condition_variable_any m_cv;
    std::queue<std::function<void()>> m_tasks;
    std::vector<std::thread> m_workers;
    std::atomic<bool> m_stop{false};
    size_t m_max_threads;
};
} // namespace

// Static members for PluginLoaderFactory
std::unordered_map<std::string, std::function<std::unique_ptr<IPluginLoader>()>>
    PluginLoaderFactory::s_loader_factories;
std::mutex PluginLoaderFactory::s_factory_mutex;

QtPluginLoader::QtPluginLoader() {
    // Load persistent cache on startup
    load_persistent_cache();

    // Setup cache persistence timer
    if (QCoreApplication::instance()) {
        auto* timer = new QTimer(nullptr);
        QObject::connect(timer, &QTimer::timeout, [this]() {
            save_persistent_cache();
        });
        timer->start(std::chrono::duration_cast<std::chrono::milliseconds>(
            CACHE_PERSISTENCE_INTERVAL).count());
    }
}

QtPluginLoader::~QtPluginLoader() {
    // Save persistent cache before destruction
    save_persistent_cache();

    // Log final statistics if cache was used
    if (m_cache_enabled && (m_cache_hits > 0 || m_cache_misses > 0)) {
        qDebug() << "QtPluginLoader cache statistics:"
                 << "hits=" << m_cache_hits.load()
                 << "misses=" << m_cache_misses.load()
                 << "hit_rate=" << get_cache_statistics().hit_rate;
    }

    // Unload all plugins gracefully with parallel unloading for better performance
    std::unique_lock lock(m_plugins_mutex);
    std::vector<std::future<void>> unload_futures;

    for (auto& [id, plugin] : m_loaded_plugins) {
        if (plugin->qt_loader && plugin->qt_loader->isLoaded()) {
            // Capture qt_loader as shared_ptr to ensure it survives async operation
            auto loader = std::move(plugin->qt_loader);
            unload_futures.push_back(std::async(std::launch::async, [loader = std::move(loader)]() {
                loader->unload();
            }));
        }
    }

    // Wait for all unloads to complete
    for (auto& future : unload_futures) {
        future.wait();
    }

    m_loaded_plugins.clear();
}

bool QtPluginLoader::can_load(const std::filesystem::path& file_path) const {
    qDebug() << "QtPluginLoader::can_load() called with path:"
             << QString::fromStdString(file_path.string());

    if (!is_valid_plugin_file(file_path)) {
        qDebug() << "is_valid_plugin_file() returned false";
        return false;
    }

    qDebug() << "is_valid_plugin_file() returned true, checking metadata...";

    // Try to read metadata to verify it's a valid plugin
    auto metadata_result = read_metadata(file_path);
    bool has_metadata = metadata_result.has_value();
    qDebug() << "read_metadata() result:"
             << (has_metadata ? "success" : "failed");
    if (!has_metadata) {
        qDebug() << "Metadata error:"
                 << QString::fromStdString(metadata_result.error().message);
    }
    return has_metadata;
}

qtplugin::expected<std::shared_ptr<IPlugin>, PluginError> QtPluginLoader::load(
    const std::filesystem::path& file_path) {
    auto start_time = std::chrono::steady_clock::now();

    if (!std::filesystem::exists(file_path)) {
        track_error(__FUNCTION__, "Plugin file not found: " + file_path.string(),
                   PluginErrorCode::FileNotFound);
        return make_error<std::shared_ptr<IPlugin>>(
            PluginErrorCode::FileNotFound,
            "Plugin file not found: " + file_path.string());
    }

    if (!can_load(file_path)) {
        return make_error<std::shared_ptr<IPlugin>>(
            PluginErrorCode::InvalidFormat,
            "Invalid plugin file: " + file_path.string());
    }

    // Read metadata to get plugin ID
    auto metadata_result = read_metadata(file_path);
    if (!metadata_result) {
        return qtplugin::unexpected<PluginError>{metadata_result.error()};
    }

    auto plugin_id_result = extract_plugin_id(metadata_result.value());
    if (!plugin_id_result) {
        return qtplugin::unexpected<PluginError>{plugin_id_result.error()};
    }

    const std::string plugin_id = plugin_id_result.value();

    // Check if already loaded
    {
        std::shared_lock lock(m_plugins_mutex);
        if (m_loaded_plugins.find(plugin_id) != m_loaded_plugins.end()) {
            return make_error<std::shared_ptr<IPlugin>>(
                PluginErrorCode::LoadFailed,
                "Plugin already loaded: " + plugin_id);
        }
    }

    // Create Qt plugin loader
    auto qt_loader = std::make_unique<QPluginLoader>(
        QString::fromStdString(file_path.string()));

    // Load the plugin
    QObject* instance = qt_loader->instance();
    if (!instance) {
        return make_error<std::shared_ptr<IPlugin>>(
            PluginErrorCode::LoadFailed,
            "Failed to load plugin: " + qt_loader->errorString().toStdString());
    }

    // Cast to IPlugin interface
    IPlugin* plugin_interface = qobject_cast<IPlugin*>(instance);
    if (!plugin_interface) {
        qt_loader->unload();
        return make_error<std::shared_ptr<IPlugin>>(
            PluginErrorCode::LoadFailed,
            "Plugin does not implement IPlugin interface");
    }

    // Create shared pointer with custom deleter that doesn't delete the object
    // (Qt manages the lifetime)
    std::shared_ptr<IPlugin> plugin_ptr(plugin_interface, [](IPlugin*) {
        // Don't delete - Qt manages the lifetime
    });

    // Create loaded plugin info with enhanced tracking
    auto loaded_plugin = std::make_unique<LoadedPlugin>();
    loaded_plugin->id = plugin_id;
    loaded_plugin->file_path = file_path;
    loaded_plugin->qt_loader = std::move(qt_loader);
    loaded_plugin->instance = plugin_ptr;
    loaded_plugin->load_time = start_time;

    // Estimate memory usage based on file size
    try {
        loaded_plugin->estimated_memory = std::filesystem::file_size(file_path);
    } catch (...) {
        loaded_plugin->estimated_memory = 0;
    }

    // Store the loaded plugin
    {
        std::unique_lock lock(m_plugins_mutex);
        m_loaded_plugins[plugin_id] = std::move(loaded_plugin);
    }

    return plugin_ptr;
}

qtplugin::expected<void, PluginError> QtPluginLoader::unload(
    std::string_view plugin_id) {
    std::unique_lock lock(m_plugins_mutex);

    auto it = m_loaded_plugins.find(std::string(plugin_id));
    if (it == m_loaded_plugins.end()) {
        return make_error<void>(PluginErrorCode::LoadFailed,
                                "Plugin not found: " + std::string(plugin_id));
    }

    auto& loaded_plugin = it->second;

    // Unload the Qt plugin
    if (loaded_plugin->qt_loader && loaded_plugin->qt_loader->isLoaded()) {
        if (!loaded_plugin->qt_loader->unload()) {
            return make_error<void>(
                PluginErrorCode::LoadFailed,
                "Failed to unload plugin: " +
                    loaded_plugin->qt_loader->errorString().toStdString());
        }
    }

    // Remove from loaded plugins
    m_loaded_plugins.erase(it);

    return make_success();
}

std::vector<std::string> QtPluginLoader::supported_extensions() const {
    return {".dll", ".so", ".dylib", ".qtplugin"};
}

std::string_view QtPluginLoader::name() const noexcept {
    return "QtPluginLoader";
}

bool QtPluginLoader::supports_hot_reload() const noexcept { return true; }

size_t QtPluginLoader::loaded_plugin_count() const {
    std::shared_lock lock(m_plugins_mutex);
    return m_loaded_plugins.size();
}

std::vector<std::string> QtPluginLoader::loaded_plugins() const {
    std::shared_lock lock(m_plugins_mutex);
    std::vector<std::string> plugin_ids;
    plugin_ids.reserve(m_loaded_plugins.size());

    for (const auto& [id, plugin] : m_loaded_plugins) {
        plugin_ids.push_back(id);
    }

    return plugin_ids;
}

bool QtPluginLoader::is_loaded(std::string_view plugin_id) const {
    std::shared_lock lock(m_plugins_mutex);
    return m_loaded_plugins.find(std::string(plugin_id)) !=
           m_loaded_plugins.end();
}

qtplugin::expected<QJsonObject, PluginError> QtPluginLoader::read_metadata(
    const std::filesystem::path& file_path) const {
    // Use cached version if caching is enabled
    if (m_cache_enabled) {
        return read_metadata_cached(file_path);
    }
    return read_metadata_impl(file_path);
}

qtplugin::expected<QJsonObject, PluginError> QtPluginLoader::read_metadata_cached(
    const std::filesystem::path& file_path) const {
    // Check cache first
    {
        std::shared_lock lock(m_cache_mutex);
        auto it = m_metadata_cache.find(file_path.string());
        if (it != m_metadata_cache.end()) {
            if (is_cache_valid(file_path, it->second)) {
                ++m_cache_hits;
                return it->second.metadata;
            }
        }
    }

    ++m_cache_misses;

    // Read metadata from file
    auto result = read_metadata_impl(file_path);
    if (result) {
        // Cache the result
        std::unique_lock lock(m_cache_mutex);

        CacheEntry entry;
        entry.metadata = result.value();
        entry.cache_time = std::chrono::steady_clock::now();

        try {
            entry.file_time = std::filesystem::last_write_time(file_path);
            entry.file_size = std::filesystem::file_size(file_path);
        } catch (const std::filesystem::filesystem_error& e) {
            track_error(__FUNCTION__, "Failed to get file info: " + std::string(e.what()));
        }

        m_metadata_cache[file_path.string()] = std::move(entry);

        // Evict old entries if cache is too large
        if (m_metadata_cache.size() > MAX_CACHE_SIZE) {
            evict_oldest_cache_entry();
        }
    }

    return result;
}

// Internal implementation without caching
qtplugin::expected<QJsonObject, PluginError> QtPluginLoader::read_metadata_impl(
    const std::filesystem::path& file_path) const {
    QString qt_path = QString::fromStdString(file_path.string());
    qDebug() << "read_metadata_impl() called with path:" << qt_path;
    // Ensure library paths are set up correctly for plugin loading
    QFileInfo file_info(qt_path);
    QString plugin_dir = file_info.absolutePath();
    QString build_dir = QDir::currentPath();

    // Add current directory and plugin directory to library path
    QCoreApplication::addLibraryPath(".");
    QCoreApplication::addLibraryPath(build_dir);
    QCoreApplication::addLibraryPath(plugin_dir);

    qDebug() << "Added library paths for plugin loading";

    QPluginLoader temp_loader(qt_path);
    qDebug() << "QPluginLoader created, getting metadata...";

    QJsonObject metadata = temp_loader.metaData();
    qDebug() << "Metadata retrieved, isEmpty():" << metadata.isEmpty();

    if (metadata.isEmpty()) {
        QString error_string = temp_loader.errorString();
        qDebug() << "QPluginLoader error string:" << error_string;
        return make_error<QJsonObject>(
            PluginErrorCode::InvalidFormat,
            "No metadata found in plugin file: " + error_string.toStdString());
    }

    qDebug() << "Metadata successfully read";
    return metadata;
}

qtplugin::expected<std::string, PluginError> QtPluginLoader::extract_plugin_id(
    const QJsonObject& metadata) const {
    // Try to get plugin ID from metadata
    if (metadata.contains("MetaData")) {
        QJsonObject meta_data = metadata["MetaData"].toObject();
        if (meta_data.contains("id") && meta_data["id"].isString()) {
            return meta_data["id"].toString().toStdString();
        }
        if (meta_data.contains("name") && meta_data["name"].isString()) {
            return meta_data["name"].toString().toStdString();
        }
    }

    // Fallback to IID
    if (metadata.contains("IID") && metadata["IID"].isString()) {
        return metadata["IID"].toString().toStdString();
    }

    return make_error<std::string>(PluginErrorCode::InvalidFormat,
                                   "No plugin ID found in metadata");
}

bool QtPluginLoader::is_valid_plugin_file(
    const std::filesystem::path& file_path) const {
    QFileInfo file_info(QString::fromStdString(file_path.string()));

    if (!file_info.exists() || !file_info.isFile()) {
        return false;
    }

    // Check file extension
    QString suffix = file_info.suffix().toLower();
    auto extensions = supported_extensions();

    for (const auto& ext : extensions) {
        QString ext_without_dot =
            QString::fromStdString(ext).mid(1);  // Remove the dot
        if (suffix == ext_without_dot) {
            return true;
        }
    }

    return false;
}

// PluginLoaderFactory implementation

std::unique_ptr<IPluginLoader> PluginLoaderFactory::create_default_loader() {
    return std::make_unique<QtPluginLoader>();
}

std::unique_ptr<QtPluginLoader> PluginLoaderFactory::create_qt_loader() {
    return std::make_unique<QtPluginLoader>();
}

void PluginLoaderFactory::register_loader_type(
    std::string_view name,
    std::function<std::unique_ptr<IPluginLoader>()> factory) {
    std::lock_guard lock(s_factory_mutex);
    s_loader_factories[std::string(name)] = std::move(factory);
}

std::unique_ptr<IPluginLoader> PluginLoaderFactory::create_loader(
    std::string_view name) {
    std::lock_guard lock(s_factory_mutex);
    auto it = s_loader_factories.find(std::string(name));
    if (it != s_loader_factories.end()) {
        return it->second();
    }
    return nullptr;
}

std::vector<std::string> PluginLoaderFactory::available_loaders() {
    std::lock_guard lock(s_factory_mutex);
    std::vector<std::string> loaders;
    loaders.reserve(s_loader_factories.size());

    for (const auto& [name, factory] : s_loader_factories) {
        loaders.push_back(name);
    }

    return loaders;
}

// === Enhanced functionality implementation (v3.2.0) ===

void QtPluginLoader::set_cache_enabled(bool enabled) {
    m_cache_enabled = enabled;
    if (!enabled) {
        clear_cache();
    }
}

QtPluginLoader::CacheStatistics QtPluginLoader::get_cache_statistics() const {
    CacheStatistics stats;
    stats.hits = m_cache_hits.load();
    stats.misses = m_cache_misses.load();

    size_t total = stats.hits + stats.misses;
    stats.hit_rate = total > 0 ? static_cast<double>(stats.hits) / total : 0.0;

    {
        std::shared_lock lock(m_cache_mutex);
        stats.cache_size = m_metadata_cache.size();
    }

    return stats;
}

void QtPluginLoader::clear_cache() {
    std::unique_lock lock(m_cache_mutex);
    m_metadata_cache.clear();
    m_cache_hits = 0;
    m_cache_misses = 0;
}

std::string QtPluginLoader::get_error_report() const {
    std::lock_guard<std::mutex> lock(m_error_mutex);

    std::stringstream ss;
    ss << "=== QtPluginLoader Error Report ===\n";
    ss << "Total errors: " << m_error_history.size() << "\n\n";

    for (size_t i = 0; i < m_error_history.size(); ++i) {
        const auto& error = m_error_history[i];
        auto time_t = std::chrono::system_clock::to_time_t(error.timestamp);

        ss << "[" << i << "] "
           << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "\n";
        ss << "  Function: " << error.function << "\n";
        ss << "  Message: " << error.message << "\n";
        ss << "  Code: " << static_cast<int>(error.code) << "\n\n";
    }

    return ss.str();
}

void QtPluginLoader::clear_error_history() {
    std::lock_guard<std::mutex> lock(m_error_mutex);
    m_error_history.clear();
}

QtPluginLoader::ResourceUsage QtPluginLoader::get_resource_usage(
    std::string_view plugin_id) const {

    ResourceUsage usage;

    std::shared_lock lock(m_plugins_mutex);
    auto it = m_loaded_plugins.find(std::string(plugin_id));

    if (it != m_loaded_plugins.end()) {
        const auto& plugin = it->second;

        // Calculate approximate memory usage
        usage.memory_bytes = plugin->estimated_memory;
        if (usage.memory_bytes == 0) {
            // Estimate based on file size if not set
            try {
                usage.memory_bytes = std::filesystem::file_size(plugin->file_path);
            } catch (...) {
                usage.memory_bytes = 0;
            }
        }

        usage.handle_count = plugin->ref_count.load();

        auto now = std::chrono::steady_clock::now();
        auto duration = now - plugin->load_time;
        usage.load_time = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

        usage.last_access = std::chrono::system_clock::now();
    }

    return usage;
}

void QtPluginLoader::track_error(const std::string& function,
                                 const std::string& message,
                                 PluginErrorCode code) const {
    std::lock_guard<std::mutex> lock(m_error_mutex);

    ErrorEntry entry;
    entry.timestamp = std::chrono::system_clock::now();
    entry.function = function;
    entry.message = message;
    entry.code = code;

    m_error_history.push_back(std::move(entry));

    // Keep only last MAX_ERROR_HISTORY entries
    if (m_error_history.size() > MAX_ERROR_HISTORY) {
        m_error_history.erase(m_error_history.begin());
    }
}

bool QtPluginLoader::is_cache_valid(const std::filesystem::path& path,
                                    const CacheEntry& entry) const {
    try {
        // Check if file has been modified
        auto current_time = std::filesystem::last_write_time(path);
        if (current_time != entry.file_time) {
            return false;
        }

        // Check if size has changed
        auto current_size = std::filesystem::file_size(path);
        if (current_size != entry.file_size) {
            return false;
        }

        // Check cache age
        auto age = std::chrono::steady_clock::now() - entry.cache_time;
        if (age > CACHE_EXPIRY) {
            return false;
        }

        return true;
    } catch (const std::filesystem::filesystem_error&) {
        return false;
    }
}

void QtPluginLoader::evict_oldest_cache_entry() const {
    if (m_metadata_cache.empty()) {
        return;
    }

    auto oldest = m_metadata_cache.begin();
    auto oldest_time = oldest->second.cache_time;

    for (auto it = m_metadata_cache.begin(); it != m_metadata_cache.end(); ++it) {
        if (it->second.cache_time < oldest_time) {
            oldest = it;
            oldest_time = it->second.cache_time;
        }
    }

    m_metadata_cache.erase(oldest);
}

// Load persistent cache from disk for faster startup
void QtPluginLoader::load_persistent_cache() {
    auto cache_path = get_cache_persistence_path();
    QFile cache_file(cache_path);

    if (!cache_file.open(QIODevice::ReadOnly)) {
        return;  // Cache file doesn't exist or can't be opened
    }

    QDataStream stream(&cache_file);
    quint32 cache_version;
    stream >> cache_version;

    if (cache_version != 1) {
        return;  // Incompatible cache version
    }

    std::unique_lock lock(m_cache_mutex);

    quint32 entry_count;
    stream >> entry_count;

    // Load cache entries
    for (quint32 i = 0; i < entry_count && i < CACHE_PREWARM_SIZE; ++i) {
        QString key;
        QByteArray metadata_data;
        qint64 file_size;

        stream >> key >> metadata_data >> file_size;

        if (!metadata_data.isEmpty()) {
            QJsonDocument doc = QJsonDocument::fromJson(metadata_data);
            if (!doc.isNull()) {
                CacheEntry entry;
                entry.metadata = doc.object();
                entry.file_size = static_cast<std::uintmax_t>(file_size);
                entry.cache_time = std::chrono::steady_clock::now();

                // Note: file_time will be updated on first validation
                m_metadata_cache[key.toStdString()] = std::move(entry);
            }
        }
    }

    qDebug() << "Loaded" << m_metadata_cache.size() << "entries from persistent cache";
}

// Save cache to disk for persistence across sessions
void QtPluginLoader::save_persistent_cache() const {
    auto cache_path = get_cache_persistence_path();

    // Ensure cache directory exists
    QDir cache_dir = QFileInfo(cache_path).dir();
    if (!cache_dir.exists()) {
        cache_dir.mkpath(".");
    }

    QFile cache_file(cache_path);
    if (!cache_file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to save persistent cache to" << cache_path;
        return;
    }

    QDataStream stream(&cache_file);
    stream << quint32(1);  // Cache version

    std::shared_lock lock(m_cache_mutex);

    // Save limited number of most recent entries
    quint32 entry_count = std::min(static_cast<size_t>(m_metadata_cache.size()),
                                   CACHE_PREWARM_SIZE);
    stream << entry_count;

    // Sort entries by cache time to save most recent ones
    std::vector<std::pair<std::string, const CacheEntry*>> sorted_entries;
    for (const auto& [key, entry] : m_metadata_cache) {
        sorted_entries.push_back({key, &entry});
    }

    std::sort(sorted_entries.begin(), sorted_entries.end(),
              [](const auto& a, const auto& b) {
                  return a.second->cache_time > b.second->cache_time;
              });

    quint32 saved_count = 0;
    for (const auto& [key, entry] : sorted_entries) {
        if (saved_count >= entry_count) {
            break;
        }

        QJsonDocument doc(entry->metadata);
        stream << QString::fromStdString(key)
               << doc.toJson(QJsonDocument::Compact)
               << qint64(entry->file_size);

        saved_count++;
    }

    qDebug() << "Saved" << saved_count << "entries to persistent cache";
}

// === Batch Loading Implementation ===

// Batch loading result structure
struct BatchLoadResult {
    std::filesystem::path path;
    qtplugin::expected<std::shared_ptr<IPlugin>, PluginError> result;
    std::chrono::milliseconds load_time;
};

// Batch load multiple plugins efficiently
std::vector<QtPluginLoader::BatchLoadResult> QtPluginLoader::batch_load(
    const std::vector<std::filesystem::path>& paths) {

    std::vector<QtPluginLoader::BatchLoadResult> results;
    results.reserve(paths.size());

    if (paths.size() < MIN_PARALLEL_LOAD_THRESHOLD) {
        // Sequential loading for small batches
        for (const auto& path : paths) {
            auto start = std::chrono::steady_clock::now();
            auto result = load(path);
            auto end = std::chrono::steady_clock::now();

            results.push_back({
                path,
                result,
                std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            });
        }
    } else {
        // Parallel loading for large batches
        results = batch_load_parallel(paths);
    }

    return results;
}

// Parallel batch loading implementation
std::vector<QtPluginLoader::BatchLoadResult> QtPluginLoader::batch_load_parallel(
    const std::vector<std::filesystem::path>& paths) {

    auto& thread_pool = PluginLoadThreadPool::instance();
    std::vector<std::future<QtPluginLoader::BatchLoadResult>> futures;
    futures.reserve(paths.size());

    // Submit all loading tasks
    for (const auto& path : paths) {
        futures.push_back(thread_pool.submit([this, path]() {
            auto start = std::chrono::steady_clock::now();
            auto result = load(path);
            auto end = std::chrono::steady_clock::now();

            return BatchLoadResult{
                path,
                result,
                std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            };
        }));
    }

    // Collect results with timeout
    std::vector<BatchLoadResult> results;
    results.reserve(futures.size());

    for (auto& future : futures) {
        if (future.wait_for(LOAD_TIMEOUT) == std::future_status::ready) {
            results.push_back(future.get());
        } else {
            // Timeout occurred
            results.push_back({
                std::filesystem::path(),
                qtplugin::make_error<std::shared_ptr<IPlugin>>(
                    PluginErrorCode::Timeout,
                    "Plugin load timed out"),
                std::chrono::milliseconds(0)
            });
        }
    }

    return results;
}

// Batch unload multiple plugins
std::vector<qtplugin::expected<void, PluginError>> QtPluginLoader::batch_unload(
    const std::vector<std::string>& plugin_ids) {

    std::vector<qtplugin::expected<void, PluginError>> results;
    results.reserve(plugin_ids.size());

    if (plugin_ids.size() < MIN_PARALLEL_LOAD_THRESHOLD) {
        // Sequential unloading for small batches
        for (const auto& id : plugin_ids) {
            results.push_back(unload(id));
        }
    } else {
        // Parallel unloading for large batches
        auto& thread_pool = PluginLoadThreadPool::instance();
        std::vector<std::future<qtplugin::expected<void, PluginError>>> futures;

        for (const auto& id : plugin_ids) {
            futures.push_back(thread_pool.submit([this, id]() {
                return unload(id);
            }));
        }

        // Collect results
        for (auto& future : futures) {
            results.push_back(future.get());
        }
    }

    return results;
}

// Batch metadata reading for preprocessing
std::vector<qtplugin::expected<QJsonObject, PluginError>>
QtPluginLoader::batch_read_metadata(
    const std::vector<std::filesystem::path>& paths) {

    std::vector<qtplugin::expected<QJsonObject, PluginError>> results;
    results.reserve(paths.size());

    if (paths.size() < MIN_PARALLEL_LOAD_THRESHOLD) {
        // Sequential for small batches
        for (const auto& path : paths) {
            results.push_back(read_metadata(path));
        }
    } else {
        // Parallel for large batches
        auto& thread_pool = PluginLoadThreadPool::instance();
        std::vector<std::future<qtplugin::expected<QJsonObject, PluginError>>> futures;

        for (const auto& path : paths) {
            futures.push_back(thread_pool.submit([this, path]() {
                return read_metadata(path);
            }));
        }

        // Collect results
        for (auto& future : futures) {
            results.push_back(future.get());
        }
    }

    return results;
}

// Get thread pool statistics
QtPluginLoader::ThreadPoolStats QtPluginLoader::get_thread_pool_stats() const {
    auto& pool = PluginLoadThreadPool::instance();
    ThreadPoolStats stats;
    stats.queue_size = pool.get_queue_size();
    stats.max_threads = MAX_CONCURRENT_LOADS;
    return stats;
}

// Configure thread pool
void QtPluginLoader::set_max_loading_threads(size_t count) {
    PluginLoadThreadPool::instance().set_max_threads(count);
}

}  // namespace qtplugin
