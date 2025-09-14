/**
 * @file lua_plugin_loader.cpp
 * @brief Implementation of Lua plugin loader
 * @version 3.2.0
 */

#include "../../include/qtplugin/core/lua_plugin_loader.hpp"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QTextStream>
#include <QUuid>
#include <regex>

Q_LOGGING_CATEGORY(luaLoaderLog, "qtplugin.lua.loader");

namespace qtplugin {

// === LuaPluginLoader Implementation ===

LuaPluginLoader::LuaPluginLoader() {
    qCDebug(luaLoaderLog) << "LuaPluginLoader created";
}

LuaPluginLoader::~LuaPluginLoader() {
    // Unload all plugins
    std::unique_lock lock(m_plugins_mutex);
    for (auto& [id, info] : m_loaded_plugins) {
        if (info->bridge) {
            info->bridge->shutdown();
        }
    }
    m_loaded_plugins.clear();
    qCDebug(luaLoaderLog) << "LuaPluginLoader destroyed";
}

bool LuaPluginLoader::can_load(const std::filesystem::path& file_path) const {
    if (!is_lua_available()) {
        return false;
    }

    return is_valid_lua_file(file_path);
}

qtplugin::expected<std::shared_ptr<IPlugin>, PluginError> LuaPluginLoader::load(
    const std::filesystem::path& file_path) {

    if (!is_lua_available()) {
        return make_error<std::shared_ptr<IPlugin>>(
            PluginErrorCode::NotSupported,
            "Lua bindings not available in this build");
    }

    if (!std::filesystem::exists(file_path)) {
        return make_error<std::shared_ptr<IPlugin>>(
            PluginErrorCode::FileNotFound,
            "Lua plugin file not found: " + file_path.string());
    }

    if (!is_valid_lua_file(file_path)) {
        return make_error<std::shared_ptr<IPlugin>>(
            PluginErrorCode::InvalidFormat,
            "Invalid Lua plugin file: " + file_path.string());
    }

    // Generate plugin ID
    std::string plugin_id = generate_lua_plugin_id(file_path);

    // Check if already loaded
    {
        std::shared_lock lock(m_plugins_mutex);
        if (m_loaded_plugins.find(plugin_id) != m_loaded_plugins.end()) {
            return make_error<std::shared_ptr<IPlugin>>(
                PluginErrorCode::LoadFailed,
                "Lua plugin already loaded: " + plugin_id);
        }
    }

    // Create Lua plugin bridge
    auto bridge = std::make_shared<LuaPluginBridge>();

    // Initialize the bridge
    auto init_result = bridge->initialize();
    if (!init_result) {
        return make_error<std::shared_ptr<IPlugin>>(
            init_result.error().code,
            "Failed to initialize Lua bridge: " + init_result.error().message);
    }

    // Load the Lua plugin script
    auto load_result = bridge->load_lua_plugin(QString::fromStdString(file_path.string()));
    if (!load_result) {
        return make_error<std::shared_ptr<IPlugin>>(
            load_result.error().code,
            "Failed to load Lua plugin: " + load_result.error().message);
    }

    // Create plugin info
    auto info = std::make_unique<LuaPluginInfo>();
    info->id = plugin_id;
    info->file_path = file_path;
    info->bridge = bridge;
    info->load_time = std::chrono::system_clock::now();

    // Store plugin info
    {
        std::unique_lock lock(m_plugins_mutex);
        m_loaded_plugins[plugin_id] = std::move(info);
    }

    qCDebug(luaLoaderLog) << "Loaded Lua plugin:" << QString::fromStdString(plugin_id)
                          << "from:" << QString::fromStdString(file_path.string());

    return std::static_pointer_cast<IPlugin>(bridge);
}

qtplugin::expected<void, PluginError> LuaPluginLoader::unload(std::string_view plugin_id) {
    std::unique_lock lock(m_plugins_mutex);

    auto it = m_loaded_plugins.find(std::string(plugin_id));
    if (it == m_loaded_plugins.end()) {
        return make_error<void>(PluginErrorCode::NotFound,
                               "Lua plugin not found: " + std::string(plugin_id));
    }

    // Shutdown the bridge
    if (it->second->bridge) {
        it->second->bridge->shutdown();
    }

    // Remove from loaded plugins
    m_loaded_plugins.erase(it);

    qCDebug(luaLoaderLog) << "Unloaded Lua plugin:" << QString::fromStdString(std::string(plugin_id));

    return make_success();
}

std::vector<std::string> LuaPluginLoader::supported_extensions() const {
    return {".lua"};
}

std::string_view LuaPluginLoader::name() const noexcept {
    return "LuaPluginLoader";
}

std::string_view LuaPluginLoader::description() const noexcept {
    return "Plugin loader for Lua script plugins";
}

Version LuaPluginLoader::version() const noexcept {
    return Version{3, 2, 0};
}

bool LuaPluginLoader::is_lua_available() {
#ifdef QTFORGE_LUA_BINDINGS
    return true;
#else
    return false;
#endif
}

size_t LuaPluginLoader::loaded_plugin_count() const {
    std::shared_lock lock(m_plugins_mutex);
    return m_loaded_plugins.size();
}

std::vector<std::string> LuaPluginLoader::loaded_plugin_ids() const {
    std::shared_lock lock(m_plugins_mutex);
    std::vector<std::string> ids;
    ids.reserve(m_loaded_plugins.size());

    for (const auto& [id, info] : m_loaded_plugins) {
        ids.push_back(id);
    }

    return ids;
}

std::shared_ptr<LuaPluginBridge> LuaPluginLoader::get_lua_bridge(std::string_view plugin_id) const {
    std::shared_lock lock(m_plugins_mutex);

    auto it = m_loaded_plugins.find(std::string(plugin_id));
    if (it != m_loaded_plugins.end()) {
        return it->second->bridge;
    }

    return nullptr;
}

bool LuaPluginLoader::is_valid_lua_file(const std::filesystem::path& file_path) const {
    QFileInfo file_info(QString::fromStdString(file_path.string()));

    if (!file_info.exists() || !file_info.isFile()) {
        return false;
    }

    // Check file extension
    QString suffix = file_info.suffix().toLower();
    return suffix == "lua";
}

qtplugin::expected<PluginMetadata, PluginError> LuaPluginLoader::extract_lua_metadata(
    const std::filesystem::path& file_path) const {

    QFile file(QString::fromStdString(file_path.string()));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return make_error<PluginMetadata>(PluginErrorCode::FileSystemError,
                                         "Cannot read Lua plugin file");
    }

    QTextStream stream(&file);
    QString content = stream.readAll();

    // Parse metadata from Lua comments
    // Look for metadata in the format:
    // --[[
    // @plugin_name: Example Plugin
    // @plugin_description: An example Lua plugin
    // @plugin_version: 1.0.0
    // @plugin_author: Plugin Author
    // ]]

    PluginMetadata metadata;
    metadata.name = file_info(file_path).stem().string(); // Default to filename
    metadata.description = "Lua Plugin";
    metadata.version = Version{1, 0, 0};
    metadata.author = "Unknown";
    metadata.capabilities = static_cast<PluginCapabilities>(PluginCapability::Scripting);

    // Simple regex-based metadata extraction
    std::string content_str = content.toStdString();

    std::regex name_regex(R"(@plugin_name:\s*(.+))");
    std::regex desc_regex(R"(@plugin_description:\s*(.+))");
    std::regex version_regex(R"(@plugin_version:\s*(\d+)\.(\d+)\.(\d+))");
    std::regex author_regex(R"(@plugin_author:\s*(.+))");

    std::smatch match;

    if (std::regex_search(content_str, match, name_regex)) {
        metadata.name = match[1].str();
    }

    if (std::regex_search(content_str, match, desc_regex)) {
        metadata.description = match[1].str();
    }

    if (std::regex_search(content_str, match, version_regex)) {
        int major = std::stoi(match[1].str());
        int minor = std::stoi(match[2].str());
        int patch = std::stoi(match[3].str());
        metadata.version = Version{major, minor, patch};
    }

    if (std::regex_search(content_str, match, author_regex)) {
        metadata.author = match[1].str();
    }

    return metadata;
}

std::string LuaPluginLoader::generate_lua_plugin_id(const std::filesystem::path& file_path) const {
    // Generate ID based on file path and timestamp
    QFileInfo file_info(QString::fromStdString(file_path.string()));
    QString base_name = file_info.baseName();
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);

    return QString("lua.%1.%2").arg(base_name, uuid.left(8)).toStdString();
}

// === LuaPluginLoaderFactory Implementation ===

std::unique_ptr<LuaPluginLoader> LuaPluginLoaderFactory::create() {
    if (!is_available()) {
        return nullptr;
    }

    return std::make_unique<LuaPluginLoader>();
}

bool LuaPluginLoaderFactory::is_available() {
    return LuaPluginLoader::is_lua_available();
}

void LuaPluginLoaderFactory::register_with_factory() {
    if (is_available()) {
        PluginLoaderFactory::register_loader("lua", create_lua_loader);
        qCDebug(luaLoaderLog) << "Lua plugin loader registered with factory";
    } else {
        qCWarning(luaLoaderLog) << "Lua plugin loader not available - skipping registration";
    }
}

std::unique_ptr<IPluginLoader> LuaPluginLoaderFactory::create_lua_loader() {
    return std::unique_ptr<IPluginLoader>(create().release());
}

// === CompositePluginLoader Implementation ===

CompositePluginLoader::CompositePluginLoader()
    : m_qt_loader(std::make_shared<QtPluginLoader>()) {

    // Create Lua loader if available
    if (LuaPluginLoaderFactory::is_available()) {
        m_lua_loader = LuaPluginLoaderFactory::create();
        qCDebug(luaLoaderLog) << "CompositePluginLoader created with Lua support";
    } else {
        qCDebug(luaLoaderLog) << "CompositePluginLoader created without Lua support";
    }
}

CompositePluginLoader::~CompositePluginLoader() {
    qCDebug(luaLoaderLog) << "CompositePluginLoader destroyed";
}

bool CompositePluginLoader::can_load(const std::filesystem::path& file_path) const {
    // Check Qt loader first
    if (m_qt_loader && m_qt_loader->can_load(file_path)) {
        return true;
    }

    // Check Lua loader if available
    if (m_lua_loader && m_lua_loader->can_load(file_path)) {
        return true;
    }

    return false;
}

qtplugin::expected<std::shared_ptr<IPlugin>, PluginError> CompositePluginLoader::load(
    const std::filesystem::path& file_path) {

    IPluginLoader* loader = select_loader(file_path);
    if (!loader) {
        return make_error<std::shared_ptr<IPlugin>>(
            PluginErrorCode::InvalidFormat,
            "No suitable loader found for file: " + file_path.string());
    }

    return loader->load(file_path);
}

qtplugin::expected<void, PluginError> CompositePluginLoader::unload(std::string_view plugin_id) {
    // Try Qt loader first
    if (m_qt_loader) {
        auto qt_result = m_qt_loader->unload(plugin_id);
        if (qt_result) {
            return qt_result;
        }
    }

    // Try Lua loader if available
    if (m_lua_loader) {
        auto lua_result = m_lua_loader->unload(plugin_id);
        if (lua_result) {
            return lua_result;
        }
    }

    return make_error<void>(PluginErrorCode::NotFound,
                           "Plugin not found in any loader: " + std::string(plugin_id));
}

std::vector<std::string> CompositePluginLoader::supported_extensions() const {
    std::vector<std::string> extensions;

    // Add Qt loader extensions
    if (m_qt_loader) {
        auto qt_exts = m_qt_loader->supported_extensions();
        extensions.insert(extensions.end(), qt_exts.begin(), qt_exts.end());
    }

    // Add Lua loader extensions
    if (m_lua_loader) {
        auto lua_exts = m_lua_loader->supported_extensions();
        extensions.insert(extensions.end(), lua_exts.begin(), lua_exts.end());
    }

    return extensions;
}

std::string_view CompositePluginLoader::name() const noexcept {
    return "CompositePluginLoader";
}

std::string_view CompositePluginLoader::description() const noexcept {
    return "Composite loader supporting Qt and Lua plugins";
}

Version CompositePluginLoader::version() const noexcept {
    return Version{3, 2, 0};
}

IPluginLoader* CompositePluginLoader::select_loader(const std::filesystem::path& file_path) const {
    // Check file extension to determine appropriate loader
    QFileInfo file_info(QString::fromStdString(file_path.string()));
    QString suffix = file_info.suffix().toLower();

    if (suffix == "lua" && m_lua_loader) {
        return m_lua_loader.get();
    } else if (m_qt_loader && m_qt_loader->can_load(file_path)) {
        return m_qt_loader.get();
    }

    return nullptr;
}

} // namespace qtplugin
