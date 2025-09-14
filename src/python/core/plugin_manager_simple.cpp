/**
 * @file plugin_manager_simple.cpp
 * @brief Simplified PluginManager implementation for Python bindings
 * @version 3.0.0
 */

#include "plugin_manager_simple.hpp"
#include <algorithm>
#include <sstream>
#include <chrono>

namespace qtplugin {

SimplePluginManager::SimplePluginManager() {
    // Initialize with default search paths
    m_search_paths.push_back("./plugins");
    m_search_paths.push_back("../plugins");
    m_search_paths.push_back("./lib/plugins");
}

expected<std::string, PluginError> SimplePluginManager::load_plugin(
    const std::filesystem::path& file_path,
    const PluginLoadOptions& options) {

    // Basic validation
    if (!std::filesystem::exists(file_path)) {
        return unexpected<PluginError>{
            PluginError{PluginErrorCode::FileNotFound, "Plugin file not found", file_path.string()}
        };
    }

    // Check file extension
    auto ext = file_path.extension().string();
    if (ext != ".dll" && ext != ".so" && ext != ".dylib") {
        return unexpected<PluginError>{
            PluginError{PluginErrorCode::InvalidFormat, "Invalid plugin file format", file_path.string()}
        };
    }

    // Generate plugin ID from filename
    std::string plugin_id = file_path.stem().string();

    // Check if already loaded
    if (std::find(m_loaded_plugins.begin(), m_loaded_plugins.end(), plugin_id) != m_loaded_plugins.end()) {
        return unexpected<PluginError>{
            PluginError{PluginErrorCode::AlreadyLoaded, "Plugin already loaded", plugin_id}
        };
    }

    // Simulate loading process
    if (options.validate_signature) {
        // Simulate signature validation
    }

    if (options.check_dependencies) {
        // Simulate dependency checking
    }

    // Add to loaded plugins
    m_loaded_plugins.push_back(plugin_id);

    // Create plugin info
    PluginInfo info;
    info.id = plugin_id;
    info.file_path = file_path;
    info.state = options.initialize_immediately ? PluginState::Running : PluginState::Loaded;
    info.load_time = std::chrono::system_clock::now();
    info.last_activity = info.load_time;
    info.hot_reload_enabled = options.enable_hot_reload;

    // Create basic metadata
    info.metadata.name = plugin_id;
    info.metadata.version = Version(1, 0, 0);
    info.metadata.description = "Plugin loaded via SimplePluginManager";
    info.metadata.author = "QtForge";

    m_plugin_info.push_back(info);

    return plugin_id;
}

expected<void, PluginError> SimplePluginManager::unload_plugin(
    std::string_view plugin_id,
    bool force) {

    // Suppress unused parameter warning
    (void)force;

    auto it = std::find(m_loaded_plugins.begin(), m_loaded_plugins.end(), std::string(plugin_id));
    if (it == m_loaded_plugins.end()) {
        return unexpected<PluginError>{
            PluginError{PluginErrorCode::NotFound, "Plugin not found", std::string(plugin_id)}
        };
    }

    // Remove from loaded plugins
    m_loaded_plugins.erase(it);

    // Remove from plugin info
    auto info_it = std::find_if(m_plugin_info.begin(), m_plugin_info.end(),
        [plugin_id](const PluginInfo& info) { return info.id == plugin_id; });
    if (info_it != m_plugin_info.end()) {
        m_plugin_info.erase(info_it);
    }

    return {};
}

std::vector<std::string> SimplePluginManager::loaded_plugins() const {
    return m_loaded_plugins;
}

std::vector<std::filesystem::path> SimplePluginManager::search_paths() const {
    return m_search_paths;
}

void SimplePluginManager::add_search_path(const std::filesystem::path& path) {
    if (std::find(m_search_paths.begin(), m_search_paths.end(), path) == m_search_paths.end()) {
        m_search_paths.push_back(path);
    }
}

void SimplePluginManager::remove_search_path(const std::filesystem::path& path) {
    auto it = std::find(m_search_paths.begin(), m_search_paths.end(), path);
    if (it != m_search_paths.end()) {
        m_search_paths.erase(it);
    }
}

std::shared_ptr<IPlugin> SimplePluginManager::get_plugin(std::string_view plugin_id) const {
    // Suppress unused parameter warning
    (void)plugin_id;

    // For now, return nullptr as we don't have actual plugin loading
    // In a real implementation, this would return the loaded plugin instance
    return nullptr;
}

std::vector<PluginInfo> SimplePluginManager::all_plugin_info() const {
    return m_plugin_info;
}

std::vector<std::filesystem::path> SimplePluginManager::discover_plugins(
    const std::filesystem::path& directory,
    bool recursive) const {

    std::vector<std::filesystem::path> discovered;

    if (!std::filesystem::exists(directory)) {
        return discovered;
    }

    try {
        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    auto ext = entry.path().extension().string();
                    if (ext == ".dll" || ext == ".so" || ext == ".dylib") {
                        discovered.push_back(entry.path());
                    }
                }
            }
        } else {
            for (const auto& entry : std::filesystem::directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    auto ext = entry.path().extension().string();
                    if (ext == ".dll" || ext == ".so" || ext == ".dylib") {
                        discovered.push_back(entry.path());
                    }
                }
            }
        }
    } catch (const std::filesystem::filesystem_error&) {
        // Ignore filesystem errors
    }

    return discovered;
}

bool SimplePluginManager::is_ready() const {
    return true;
}

std::string SimplePluginManager::get_statistics() const {
    std::ostringstream oss;
    oss << "{";
    oss << "\"loaded_plugins\":" << m_loaded_plugins.size() << ",";
    oss << "\"search_paths\":" << m_search_paths.size() << ",";
    oss << "\"total_plugins_info\":" << m_plugin_info.size();
    oss << "}";
    return oss.str();
}

std::string SimplePluginManager::__repr__() const {
    return "<SimplePluginManager: " + std::to_string(m_loaded_plugins.size()) + " plugins loaded>";
}

std::unique_ptr<SimplePluginManager> create_simple_plugin_manager() {
    return std::make_unique<SimplePluginManager>();
}

} // namespace qtplugin
