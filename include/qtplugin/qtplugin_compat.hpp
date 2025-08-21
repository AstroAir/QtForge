/**
 * @file qtplugin_compat.hpp
 * @brief Backward compatibility layer for QtPlugin library
 * @version 3.0.0
 *
 * This header provides backward compatibility for applications using
 * the previous monolithic architecture. It maps the old API to the
 * new component-based architecture internally.
 *
 * @deprecated This compatibility layer is provided for migration purposes.
 * New applications should use the component-based architecture directly.
 */

#pragma once

#include "qtplugin_components.hpp"

/**
 * @namespace qtplugin::compat
 * @brief Backward compatibility namespace
 *
 * This namespace contains compatibility wrappers and aliases for the
 * previous API to ease migration to the new component architecture.
 */
namespace qtplugin::compat {

/**
 * @brief Legacy plugin manager interface
 *
 * This interface provides the same API as the previous PluginManager
 * but uses the new component architecture internally.
 *
 * @deprecated Use qtplugin::PluginManager with component architecture instead
 */
class LegacyPluginManager {
public:
    LegacyPluginManager();
    ~LegacyPluginManager();

    // Legacy API methods - these delegate to the new component architecture
    bool load_plugin(const std::string& file_path);
    bool unload_plugin(const std::string& plugin_id);
    bool is_plugin_loaded(const std::string& plugin_id) const;
    std::vector<std::string> get_loaded_plugins() const;

    // Legacy configuration methods
    bool set_config_value(const std::string& key, const std::string& value);
    std::string get_config_value(const std::string& key) const;

    // Legacy security methods
    void set_security_level(int level);
    int get_security_level() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

/**
 * @brief Legacy security manager interface
 *
 * @deprecated Use qtplugin::components security components instead
 */
class LegacySecurityManager {
public:
    LegacySecurityManager();
    ~LegacySecurityManager();

    bool validate_plugin(const std::string& file_path);
    void set_security_level(int level);
    int get_security_level() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

/**
 * @brief Legacy configuration manager interface
 *
 * @deprecated Use qtplugin::components configuration components instead
 */
class LegacyConfigurationManager {
public:
    LegacyConfigurationManager();
    ~LegacyConfigurationManager();

    bool set_value(const std::string& key, const std::string& value);
    std::string get_value(const std::string& key) const;
    bool load_from_file(const std::string& file_path);
    bool save_to_file(const std::string& file_path) const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

/**
 * @brief Legacy resource manager interface
 *
 * @deprecated Use qtplugin::components resource components instead
 */
class LegacyResourceManager {
public:
    LegacyResourceManager();
    ~LegacyResourceManager();

    bool allocate_resource(const std::string& resource_type,
                           const std::string& plugin_id);
    bool deallocate_resource(const std::string& resource_id);
    std::vector<std::string> get_allocated_resources() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

/**
 * @brief Migration helper functions
 */
namespace migration {

/**
 * @brief Convert legacy plugin manager to new architecture
 * @param legacy_manager Legacy plugin manager instance
 * @return New component-based plugin manager
 */
std::unique_ptr<PluginManager> convert_legacy_manager(
    const LegacyPluginManager& legacy_manager);

/**
 * @brief Create component-based system from legacy configuration
 * @param config_file Legacy configuration file
 * @return Configured plugin system builder
 */
PluginSystemBuilder create_from_legacy_config(const std::string& config_file);

/**
 * @brief Migration status information
 */
struct MigrationStatus {
    bool is_complete = false;
    std::vector<std::string> remaining_issues;
    std::vector<std::string> completed_steps;
    std::string next_recommended_action;
};

/**
 * @brief Check migration status
 * @param legacy_manager Legacy manager to check
 * @return Migration status information
 */
MigrationStatus check_migration_status(
    const LegacyPluginManager& legacy_manager);

/**
 * @brief Generate migration guide
 * @param legacy_manager Legacy manager to analyze
 * @return Step-by-step migration guide
 */
std::vector<std::string> generate_migration_guide(
    const LegacyPluginManager& legacy_manager);

}  // namespace migration

/**
 * @brief Compatibility type aliases
 */
using PluginManager [[deprecated("Use qtplugin::PluginManager instead")]] =
    LegacyPluginManager;
using SecurityManager
    [[deprecated("Use qtplugin::components security components instead")]] =
        LegacySecurityManager;
using ConfigurationManager [[deprecated(
    "Use qtplugin::components configuration components instead")]] =
    LegacyConfigurationManager;
using ResourceManager
    [[deprecated("Use qtplugin::components resource components instead")]] =
        LegacyResourceManager;

}  // namespace qtplugin::compat

/**
 * @brief Global compatibility aliases for easier migration
 */
namespace qtplugin {
namespace v2 = compat;  ///< Alias for v2 compatibility
}

/**
 * @brief Compatibility macros
 */
#define QTPLUGIN_LEGACY_API_WARNING()                              \
    static_assert(false,                                           \
                  "This API is deprecated. Please migrate to the " \
                  "component-based architecture.")

#define QTPLUGIN_MIGRATION_HELPER(old_call, new_call) \
    [[deprecated("Use " #new_call " instead")]] old_call

/**
 * @brief Compatibility version information
 */
#define QTPLUGIN_COMPAT_VERSION_MAJOR 2
#define QTPLUGIN_COMPAT_VERSION_MINOR 9
#define QTPLUGIN_COMPAT_VERSION_PATCH 99
#define QTPLUGIN_COMPAT_VERSION "2.9.99"

namespace qtplugin::compat {

/**
 * @brief Get the compatibility layer version
 * @return Version string for compatibility layer
 */
inline constexpr const char* version() noexcept {
    return QTPLUGIN_COMPAT_VERSION;
}

/**
 * @brief Check if compatibility layer is enabled
 * @return true if compatibility layer is available
 */
inline constexpr bool is_enabled() noexcept { return true; }

/**
 * @brief Get migration recommendations
 * @return List of recommended migration steps
 */
std::vector<std::string> get_migration_recommendations();

}  // namespace qtplugin::compat
