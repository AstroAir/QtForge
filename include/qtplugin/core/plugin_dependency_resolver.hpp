/**
 * @file plugin_dependency_resolver.hpp
 * @brief Plugin dependency resolver interface and implementation
 * @version 3.0.0
 */

#pragma once

#include <QObject>
#include <memory>
#include <qtplugin/utils/error_handling.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace qtplugin {

// Forward declarations
struct PluginInfo;
class IPluginRegistry;

/**
 * @brief Plugin dependency graph node
 */
struct DependencyNode {
    std::string plugin_id;
    std::unordered_set<std::string> dependencies;
    std::unordered_set<std::string> dependents;
    int load_order = 0;
};

/**
 * @brief Interface for plugin dependency resolution
 *
 * The dependency resolver manages plugin dependencies, detects circular
 * dependencies, and provides load order calculation.
 */
class IPluginDependencyResolver {
public:
    virtual ~IPluginDependencyResolver() = default;

    /**
     * @brief Update dependency graph from plugin registry
     * @param plugin_registry Plugin registry to read from
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> update_dependency_graph(
        IPluginRegistry* plugin_registry) = 0;

    /**
     * @brief Get dependency graph
     * @return Map of plugin IDs to their dependency information
     */
    virtual std::unordered_map<std::string, DependencyNode>
    get_dependency_graph() const = 0;

    /**
     * @brief Get load order for plugins based on dependencies
     * @return Vector of plugin IDs in load order
     */
    virtual std::vector<std::string> get_load_order() const = 0;

    /**
     * @brief Check if plugin can be safely unloaded
     * @param plugin_id Plugin identifier
     * @return true if plugin can be unloaded without breaking dependencies
     */
    virtual bool can_unload_safely(const std::string& plugin_id) const = 0;

    /**
     * @brief Check plugin dependencies
     * @param plugin_info Plugin information
     * @return Success or error with dependency issues
     */
    virtual qtplugin::expected<void, PluginError> check_plugin_dependencies(
        const PluginInfo& plugin_info) const = 0;

    /**
     * @brief Detect circular dependencies
     * @return true if circular dependencies exist
     */
    virtual bool has_circular_dependencies() const = 0;

    /**
     * @brief Get plugins that depend on the specified plugin
     * @param plugin_id Plugin identifier
     * @return Vector of dependent plugin IDs
     */
    virtual std::vector<std::string> get_dependents(
        const std::string& plugin_id) const = 0;

    /**
     * @brief Get dependencies of the specified plugin
     * @param plugin_id Plugin identifier
     * @return Vector of dependency plugin IDs
     */
    virtual std::vector<std::string> get_dependencies(
        const std::string& plugin_id) const = 0;

    /**
     * @brief Clear dependency graph
     */
    virtual void clear() = 0;
    
    // === Enhanced Features (v3.2.0) ===
    
    /**
     * @brief Circular dependency resolution strategy
     */
    enum class CircularResolutionStrategy {
        None,           ///< Do not resolve, just report
        RemoveWeakest,  ///< Remove the weakest dependency in the cycle
        DisablePlugin,  ///< Disable one of the plugins in the cycle
        LoadAsGroup     ///< Load circular dependent plugins as a group
    };
    
    /**
     * @brief Circular dependency information
     */
    struct CircularDependency {
        std::vector<std::string> cycle_plugins;
        std::string suggested_break_point;
        CircularResolutionStrategy suggested_strategy;
    };
    
    /**
     * @brief Get all circular dependencies
     * @return Vector of circular dependency information
     */
    virtual std::vector<CircularDependency> get_circular_dependencies() const = 0;
    
    /**
     * @brief Resolve circular dependencies automatically
     * @param strategy Resolution strategy to use
     * @return Success or error with details
     */
    virtual qtplugin::expected<void, PluginError> resolve_circular_dependencies(
        CircularResolutionStrategy strategy = CircularResolutionStrategy::RemoveWeakest) = 0;
    
    /**
     * @brief Validate dependency graph for consistency
     * @return Success or error with validation issues
     */
    virtual qtplugin::expected<void, PluginError> validate_dependencies() const = 0;
    
    /**
     * @brief Get missing dependencies for a plugin
     * @param plugin_id Plugin identifier
     * @return Vector of missing dependency IDs
     */
    virtual std::vector<std::string> get_missing_dependencies(
        const std::string& plugin_id) const = 0;
    
    /**
     * @brief Suggest load order for a set of plugins
     * @param plugin_ids Plugin identifiers to order
     * @return Ordered vector of plugin IDs
     */
    virtual std::vector<std::string> suggest_load_order(
        const std::vector<std::string>& plugin_ids) const = 0;
};

/**
 * @brief Plugin dependency resolver implementation
 *
 * Manages plugin dependencies using a directed graph structure.
 * Provides topological sorting for load order and circular dependency
 * detection.
 */
class PluginDependencyResolver : public QObject,
                                 public IPluginDependencyResolver {
    Q_OBJECT

public:
    explicit PluginDependencyResolver(QObject* parent = nullptr);
    ~PluginDependencyResolver() override;

    // Copy constructor and assignment operator
    PluginDependencyResolver(const PluginDependencyResolver& other);
    PluginDependencyResolver& operator=(const PluginDependencyResolver& other);

    // Move constructor and assignment operator
    PluginDependencyResolver(PluginDependencyResolver&& other) noexcept;
    PluginDependencyResolver& operator=(PluginDependencyResolver&& other) noexcept;

    // IPluginDependencyResolver interface
    qtplugin::expected<void, PluginError> update_dependency_graph(
        IPluginRegistry* plugin_registry) override;

    std::unordered_map<std::string, DependencyNode> get_dependency_graph()
        const override;
    std::vector<std::string> get_load_order() const override;
    bool can_unload_safely(const std::string& plugin_id) const override;

    qtplugin::expected<void, PluginError> check_plugin_dependencies(
        const PluginInfo& plugin_info) const override;

    bool has_circular_dependencies() const override;
    std::vector<std::string> get_dependents(
        const std::string& plugin_id) const override;
    std::vector<std::string> get_dependencies(
        const std::string& plugin_id) const override;
    void clear() override;
    
    // Enhanced features (v3.2.0)
    std::vector<CircularDependency> get_circular_dependencies() const override;
    qtplugin::expected<void, PluginError> resolve_circular_dependencies(
        CircularResolutionStrategy strategy) override;
    qtplugin::expected<void, PluginError> validate_dependencies() const override;
    std::vector<std::string> get_missing_dependencies(
        const std::string& plugin_id) const override;
    std::vector<std::string> suggest_load_order(
        const std::vector<std::string>& plugin_ids) const override;

signals:
    /**
     * @brief Emitted when dependency graph is updated
     */
    void dependency_graph_updated();

    /**
     * @brief Emitted when circular dependency is detected
     * @param plugin_ids Plugins involved in circular dependency
     */
    void circular_dependency_detected(const QStringList& plugin_ids);

private:
    class Impl;
    std::unique_ptr<Impl> d;
};

}  // namespace qtplugin
