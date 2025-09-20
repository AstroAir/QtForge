/**
 * @file qtplugin_components.hpp
 * @brief Component-based architecture headers for QtPlugin library
 * @version 3.2.0
 *
 * This header provides access to the component-based architecture
 * of the QtPlugin library with factory and builder patterns for
 * creating and configuring plugin systems.
 *
 * For direct component access, include qtplugin/components.hpp instead.
 * For basic usage, include qtplugin.hpp instead.
 */

#pragma once

// Include all components
#include "components.hpp"
#include "managers/components/resource_pool.hpp"

/**
 * @namespace qtplugin::components
 * @brief Namespace for component-based architecture
 *
 * This namespace contains all the individual components that make up
 * the QtPlugin system. These components can be used independently
 * for specialized scenarios or custom plugin system implementations.
 */
namespace qtplugin::components {

/**
 * @brief Component factory for creating plugin system components
 *
 * This factory provides a convenient way to create and configure
 * individual components of the plugin system.
 */
class ComponentFactory {
public:
    // Core components
    static std::unique_ptr<IPluginRegistry> create_plugin_registry();
    static std::unique_ptr<IPluginDependencyResolver>
    create_dependency_resolver();

    // Monitoring components
    static std::unique_ptr<IPluginHotReloadManager> create_hot_reload_manager();
    static std::unique_ptr<IPluginMetricsCollector> create_metrics_collector();

    // Security components - Removed
    // SHA256 verification functionality is preserved in PluginManager

    // Configuration components
    static std::unique_ptr<IConfigurationStorage>
    create_configuration_storage();
    static std::unique_ptr<IConfigurationValidator>
    create_configuration_validator();
    static std::unique_ptr<IConfigurationMerger> create_configuration_merger();
    static std::unique_ptr<IConfigurationWatcher>
    create_configuration_watcher();

    // Resource components
    template <typename T>
    static std::unique_ptr<ITypedComponentResourcePool<T>> create_resource_pool(
        const std::string& name, ResourceType type);
    static std::unique_ptr<IResourceAllocator> create_resource_allocator();
    static std::unique_ptr<IResourceMonitor> create_resource_monitor();
};

/**
 * @brief Component builder for creating configured plugin systems
 *
 * This builder provides a fluent interface for creating and configuring
 * a complete plugin system using individual components.
 */
class PluginSystemBuilder {
public:
    PluginSystemBuilder();
    ~PluginSystemBuilder();

    // Core components configuration
    PluginSystemBuilder& with_plugin_registry(
        std::unique_ptr<IPluginRegistry> registry);
    PluginSystemBuilder& with_dependency_resolver(
        std::unique_ptr<IPluginDependencyResolver> resolver);

    // Monitoring components configuration
    PluginSystemBuilder& with_hot_reload_manager(
        std::unique_ptr<IPluginHotReloadManager> manager);
    PluginSystemBuilder& with_metrics_collector(
        std::unique_ptr<IPluginMetricsCollector> collector);

    // Security components configuration - Removed

    // Configuration components configuration
    PluginSystemBuilder& with_configuration_storage(
        std::unique_ptr<IConfigurationStorage> storage);
    PluginSystemBuilder& with_configuration_validator(
        std::unique_ptr<IConfigurationValidator> validator);
    PluginSystemBuilder& with_configuration_merger(
        std::unique_ptr<IConfigurationMerger> merger);
    PluginSystemBuilder& with_configuration_watcher(
        std::unique_ptr<IConfigurationWatcher> watcher);

    // Resource components configuration
    PluginSystemBuilder& with_resource_allocator(
        std::unique_ptr<IResourceAllocator> allocator);
    PluginSystemBuilder& with_resource_monitor(
        std::unique_ptr<IResourceMonitor> monitor);

    // Build the complete system
    std::unique_ptr<PluginManager> build();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

/**
 * @brief Component registry for managing component instances
 *
 * This registry allows components to find and interact with each other
 * in a decoupled manner.
 */
class ComponentRegistry {
public:
    static ComponentRegistry& instance();

    // Register components
    void register_component(const std::string& name,
                            std::shared_ptr<void> component);

    // Retrieve components
    template <typename T>
    std::shared_ptr<T> get_component(const std::string& name) const;

    // Check if component exists
    bool has_component(const std::string& name) const;

    // Remove component
    void unregister_component(const std::string& name);

    // Clear all components
    void clear();

private:
    ComponentRegistry() = default;
    std::unordered_map<std::string, std::shared_ptr<void>> m_components;
    mutable std::shared_mutex m_mutex;
};

}  // namespace qtplugin::components

/**
 * @brief Convenience aliases for component types
 */
namespace qtplugin {
using ComponentFactory = components::ComponentFactory;
using PluginSystemBuilder = components::PluginSystemBuilder;
using ComponentRegistry = components::ComponentRegistry;
}  // namespace qtplugin

/**
 * @brief Component version information
 */
#define QTPLUGIN_COMPONENTS_VERSION_MAJOR 3
#define QTPLUGIN_COMPONENTS_VERSION_MINOR 2
#define QTPLUGIN_COMPONENTS_VERSION_PATCH 0
#define QTPLUGIN_COMPONENTS_VERSION "3.2.0"

namespace qtplugin::components {

/**
 * @brief Get the component architecture version
 * @return Version string in format "major.minor.patch"
 */
inline constexpr const char* version() noexcept {
    return QTPLUGIN_COMPONENTS_VERSION;
}

/**
 * @brief Check if component architecture is available
 * @return true if component architecture is available
 */
inline constexpr bool is_available() noexcept { return true; }

/**
 * @brief Check if all components are enabled
 * @return true if all components are enabled by default
 */
inline constexpr bool all_components_enabled() noexcept { return true; }

/**
 * @brief Get component count
 * @return Number of available components
 */
inline size_t get_component_count() noexcept {
    return get_available_components().size();
}

}  // namespace qtplugin::components
