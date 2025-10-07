/**
 * @file plugin_dependency_resolver.cpp
 * @brief Implementation of plugin dependency resolver
 * @version 3.0.0
 */

#include "../../include/qtplugin/core/plugin_dependency_resolver.hpp"
#include <QLoggingCategory>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include "../../include/qtplugin/core/plugin_manager.hpp"
#include "../../include/qtplugin/core/plugin_registry.hpp"

Q_LOGGING_CATEGORY(dependencyResolverLog, "qtplugin.dependency")

namespace qtplugin {

/**
 * @brief Private implementation for PluginDependencyResolver
 */
class PluginDependencyResolver::Impl {
public:
    std::unordered_map<std::string, DependencyNode> dependency_graph;
    mutable std::vector<IPluginDependencyResolver::CircularDependency> circular_dependencies;
    mutable bool circular_deps_cached = false;

    // Helper methods
    std::vector<std::string> topological_sort() const;
    int calculate_dependency_level(
        const std::string& plugin_id,
        const std::vector<std::string>& dependencies) const;
    void detect_circular_dependencies(PluginDependencyResolver* resolver);
    bool has_circular_dependency(
        const std::string& plugin_id, std::unordered_set<std::string>& visited,
        std::unordered_set<std::string>& recursion_stack) const;

    // Enhanced helper methods (v3.2.0)
    void find_all_cycles() const;
    bool find_cycle_from_node(const std::string& node,
                             std::unordered_set<std::string>& visited,
                             std::unordered_set<std::string>& rec_stack,
                             std::vector<std::string>& path) const;
    std::string find_weakest_link(const std::vector<std::string>& cycle) const;
    void remove_dependency(const std::string& from, const std::string& to);
    bool is_strongly_connected(const std::string& from, const std::string& to) const;
};

PluginDependencyResolver::PluginDependencyResolver(QObject* parent)
    : QObject(parent), d(std::make_unique<Impl>()) {
    qCDebug(dependencyResolverLog) << "Plugin dependency resolver initialized";
}

PluginDependencyResolver::~PluginDependencyResolver() {
    clear();
    qCDebug(dependencyResolverLog) << "Plugin dependency resolver destroyed";
}

// Copy constructor
PluginDependencyResolver::PluginDependencyResolver(const PluginDependencyResolver& other)
    : QObject(other.parent()), d(std::make_unique<Impl>()) {
    d->dependency_graph = other.d->dependency_graph;
    d->circular_dependencies = other.d->circular_dependencies;
    d->circular_deps_cached = other.d->circular_deps_cached;
}

// Copy assignment operator
PluginDependencyResolver& PluginDependencyResolver::operator=(const PluginDependencyResolver& other) {
    if (this != &other) {
        d->dependency_graph = other.d->dependency_graph;
        d->circular_dependencies = other.d->circular_dependencies;
        d->circular_deps_cached = other.d->circular_deps_cached;
    }
    return *this;
}

// Move constructor
PluginDependencyResolver::PluginDependencyResolver(PluginDependencyResolver&& other) noexcept
    : QObject(other.parent()), d(std::move(other.d)) {
    other.d = std::make_unique<Impl>();
}

// Move assignment operator
PluginDependencyResolver& PluginDependencyResolver::operator=(PluginDependencyResolver&& other) noexcept {
    if (this != &other) {
        d = std::move(other.d);
        other.d = std::make_unique<Impl>();
    }
    return *this;
}

qtplugin::expected<void, PluginError>
PluginDependencyResolver::update_dependency_graph(
    IPluginRegistry* plugin_registry) {
    if (!plugin_registry) {
        return make_error<void>(PluginErrorCode::InvalidParameters,
                                "Plugin registry cannot be null");
    }

    // Clear existing dependency graph
    d->dependency_graph.clear();

    // Get all plugin information from registry
    auto all_plugin_info = plugin_registry->get_all_plugin_info();

    // Build dependency graph from loaded plugins
    for (const auto& plugin_info : all_plugin_info) {
        DependencyNode node;
        node.plugin_id = plugin_info.id;

        // Convert vector to unordered_set
        for (const auto& dep : plugin_info.metadata.dependencies) {
            node.dependencies.insert(dep);
        }
        node.dependents.clear();

        // Set load order based on dependency count (will be refined later)
        node.load_order =
            static_cast<int>(plugin_info.metadata.dependencies.size());

        d->dependency_graph[plugin_info.id] = std::move(node);
    }

    // Build reverse dependencies (dependents)
    for (auto& [plugin_id, node] : d->dependency_graph) {
        for (const auto& dependency : node.dependencies) {
            auto dep_it = d->dependency_graph.find(dependency);
            if (dep_it != d->dependency_graph.end()) {
                dep_it->second.dependents.insert(plugin_id);
            }
        }
    }

    // Detect circular dependencies
    d->detect_circular_dependencies(this);

    qCDebug(dependencyResolverLog) << "Dependency graph updated with"
                                   << d->dependency_graph.size() << "plugins";
    // TODO: Re-enable when MOC issues are resolved
    // emit dependency_graph_updated();

    return make_success();
}

std::unordered_map<std::string, DependencyNode>
PluginDependencyResolver::get_dependency_graph() const {
    return d->dependency_graph;
}

std::vector<std::string> PluginDependencyResolver::get_load_order() const {
    return d->topological_sort();
}

bool PluginDependencyResolver::can_unload_safely(
    const std::string& plugin_id) const {
    auto it = d->dependency_graph.find(plugin_id);
    if (it == d->dependency_graph.end()) {
        return true;  // Plugin not in graph, safe to unload
    }

    // Plugin can be safely unloaded if no other plugins depend on it
    return it->second.dependents.empty();
}

qtplugin::expected<void, PluginError>
PluginDependencyResolver::check_plugin_dependencies(
    const PluginInfo& plugin_info) const {
    // Check if all dependencies are available in the graph
    for (const auto& dep : plugin_info.metadata.dependencies) {
        if (d->dependency_graph.find(dep) == d->dependency_graph.end()) {
            return make_error<void>(PluginErrorCode::DependencyMissing,
                                    "Missing dependency: " + dep);
        }
    }

    return make_success();
}

bool PluginDependencyResolver::has_circular_dependencies() const {
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> recursion_stack;

    for (const auto& [plugin_id, node] : d->dependency_graph) {
        if (visited.find(plugin_id) == visited.end()) {
            if (d->has_circular_dependency(plugin_id, visited, recursion_stack)) {
                return true;
            }
        }
    }

    return false;
}

std::vector<std::string> PluginDependencyResolver::get_dependents(
    const std::string& plugin_id) const {
    auto it = d->dependency_graph.find(plugin_id);
    if (it == d->dependency_graph.end()) {
        return {};
    }

    return std::vector<std::string>(it->second.dependents.begin(),
                                    it->second.dependents.end());
}

std::vector<std::string> PluginDependencyResolver::get_dependencies(
    const std::string& plugin_id) const {
    auto it = d->dependency_graph.find(plugin_id);
    if (it == d->dependency_graph.end()) {
        return {};
    }

    return std::vector<std::string>(it->second.dependencies.begin(),
                                    it->second.dependencies.end());
}

void PluginDependencyResolver::clear() {
    size_t count = d->dependency_graph.size();
    d->dependency_graph.clear();

    qCDebug(dependencyResolverLog)
        << "Dependency graph cleared," << count << "nodes removed";
}






// Enhanced features (v3.2.0) implementations
std::vector<IPluginDependencyResolver::CircularDependency> PluginDependencyResolver::get_circular_dependencies() const {
    std::vector<IPluginDependencyResolver::CircularDependency> circular_deps;
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> recursion_stack;

    for (const auto& [plugin_id, node] : d->dependency_graph) {
        if (visited.find(plugin_id) == visited.end()) {
            std::vector<std::string> cycle;
            if (d->find_cycle_from_node(plugin_id, visited, recursion_stack, cycle)) {
                IPluginDependencyResolver::CircularDependency circular_dep;
                circular_dep.cycle_plugins = cycle;
                circular_dep.suggested_strategy = IPluginDependencyResolver::CircularResolutionStrategy::RemoveWeakest;
                circular_dep.suggested_break_point = d->find_weakest_link(cycle);
                circular_deps.push_back(circular_dep);
            }
        }
    }

    return circular_deps;
}

qtplugin::expected<void, PluginError> PluginDependencyResolver::resolve_circular_dependencies(
    IPluginDependencyResolver::CircularResolutionStrategy strategy) {

    auto circular_deps = get_circular_dependencies();
    if (circular_deps.empty()) {
        return make_success();
    }

    switch (strategy) {
        case IPluginDependencyResolver::CircularResolutionStrategy::None:
            // Just report, don't resolve
            return make_error<void>(PluginErrorCode::DependencyMissing,
                                   "Circular dependencies detected but resolution disabled");

        case IPluginDependencyResolver::CircularResolutionStrategy::RemoveWeakest:
            // Remove the dependency with the lowest priority
            for (const auto& circular_dep : circular_deps) {
                if (!circular_dep.cycle_plugins.empty()) {
                    // Remove the weakest link
                    const auto& weak_link = circular_dep.suggested_break_point;
                    if (!weak_link.empty() && circular_dep.cycle_plugins.size() > 1) {
                        auto it = std::find(circular_dep.cycle_plugins.begin(), circular_dep.cycle_plugins.end(), weak_link);
                        if (it != circular_dep.cycle_plugins.end()) {
                            auto next_it = std::next(it);
                            if (next_it == circular_dep.cycle_plugins.end()) {
                                next_it = circular_dep.cycle_plugins.begin();
                            }
                            d->remove_dependency(*it, *next_it);
                        }
                    }
                }
            }
            break;

        case IPluginDependencyResolver::CircularResolutionStrategy::DisablePlugin:
            // Disable one plugin in each cycle
            for (const auto& circular_dep : circular_deps) {
                if (!circular_dep.cycle_plugins.empty()) {
                    // For now, just remove the first plugin's dependencies
                    const auto& plugin_to_disable = circular_dep.cycle_plugins.front();
                    auto it = d->dependency_graph.find(plugin_to_disable);
                    if (it != d->dependency_graph.end()) {
                        it->second.dependencies.clear();
                    }
                }
            }
            break;

        case IPluginDependencyResolver::CircularResolutionStrategy::LoadAsGroup:
            // Mark plugins to be loaded as a group (implementation would need group loading support)
            return make_error<void>(PluginErrorCode::DependencyMissing,
                                   "Group loading strategy not yet implemented");
    }

    return make_success();
}

qtplugin::expected<void, PluginError> PluginDependencyResolver::validate_dependencies() const {
    // Check for circular dependencies
    auto circular_deps = get_circular_dependencies();
    if (!circular_deps.empty()) {
        return make_error<void>(PluginErrorCode::DependencyMissing,
                               "Circular dependencies detected");
    }

    // Check for missing dependencies
    for (const auto& [plugin_id, node] : d->dependency_graph) {
        for (const auto& dep : node.dependencies) {
            if (d->dependency_graph.find(dep) == d->dependency_graph.end()) {
                return make_error<void>(PluginErrorCode::DependencyMissing,
                                       "Missing dependency: " + dep + " for plugin: " + plugin_id);
            }
        }
    }

    return make_success();
}

std::vector<std::string> PluginDependencyResolver::get_missing_dependencies(
    const std::string& plugin_id) const {

    std::vector<std::string> missing_deps;
    auto it = d->dependency_graph.find(plugin_id);
    if (it != d->dependency_graph.end()) {
        for (const auto& dep : it->second.dependencies) {
            if (d->dependency_graph.find(dep) == d->dependency_graph.end()) {
                missing_deps.push_back(dep);
            }
        }
    }

    return missing_deps;
}

std::vector<std::string> PluginDependencyResolver::suggest_load_order(
    const std::vector<std::string>& plugin_ids) const {

    std::vector<std::string> load_order;
    std::unordered_set<std::string> loaded;
    std::unordered_set<std::string> loading;

    std::function<bool(const std::string&)> add_to_order = [&](const std::string& plugin_id) -> bool {
        if (loaded.find(plugin_id) != loaded.end()) {
            return true; // Already loaded
        }

        if (loading.find(plugin_id) != loading.end()) {
            return false; // Circular dependency
        }

        loading.insert(plugin_id);

        // Load dependencies first
        auto deps_it = d->dependency_graph.find(plugin_id);
        if (deps_it != d->dependency_graph.end()) {
            for (const auto& dep : deps_it->second.dependencies) {
                if (std::find(plugin_ids.begin(), plugin_ids.end(), dep) != plugin_ids.end()) {
                    if (!add_to_order(dep)) {
                        return false; // Failed to load dependency
                    }
                }
            }
        }

        load_order.push_back(plugin_id);
        loaded.insert(plugin_id);
        loading.erase(plugin_id);
        return true;
    };

    for (const auto& plugin_id : plugin_ids) {
        add_to_order(plugin_id);
    }

    return load_order;
}



// PluginDependencyResolver::Impl method implementations

void PluginDependencyResolver::Impl::find_all_cycles() const {
    // Implementation for finding all cycles - placeholder for now
}

bool PluginDependencyResolver::Impl::find_cycle_from_node(const std::string& node,
                                                         std::unordered_set<std::string>& visited,
                                                         std::unordered_set<std::string>& rec_stack,
                                                         std::vector<std::string>& path) const {
    visited.insert(node);
    rec_stack.insert(node);
    path.push_back(node);

    auto it = dependency_graph.find(node);
    if (it != dependency_graph.end()) {
        for (const auto& dep : it->second.dependencies) {
            if (rec_stack.find(dep) != rec_stack.end()) {
                // Found cycle - add the dependency that closes the cycle
                path.push_back(dep);
                return true;
            }

            if (visited.find(dep) == visited.end()) {
                if (find_cycle_from_node(dep, visited, rec_stack, path)) {
                    return true;
                }
            }
        }
    }

    rec_stack.erase(node);
    path.pop_back();
    return false;
}

std::string PluginDependencyResolver::Impl::find_weakest_link(const std::vector<std::string>& cycle) const {
    if (cycle.empty()) {
        return "";
    }

    // For now, return the first plugin in the cycle as the weakest link
    // In a more sophisticated implementation, this could consider factors like:
    // - Plugin priority/importance
    // - Number of other dependencies
    // - Plugin type (core vs optional)
    return cycle.front();
}

void PluginDependencyResolver::Impl::remove_dependency(const std::string& from, const std::string& to) {
    auto it = dependency_graph.find(from);
    if (it != dependency_graph.end()) {
        it->second.dependencies.erase(to);
    }

    // Also remove from dependents
    auto dep_it = dependency_graph.find(to);
    if (dep_it != dependency_graph.end()) {
        dep_it->second.dependents.erase(from);
    }
}

bool PluginDependencyResolver::Impl::is_strongly_connected(const std::string& from, const std::string& to) const {
    // Implementation for checking strong connectivity - placeholder for now
    Q_UNUSED(from)
    Q_UNUSED(to)
    return false;
}

std::vector<std::string> PluginDependencyResolver::Impl::topological_sort() const {
    std::vector<std::string> result;
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> temp_visited;

    // Helper function for DFS-based topological sort
    std::function<bool(const std::string&)> visit =
        [&](const std::string& plugin_id) -> bool {
        if (temp_visited.find(plugin_id) != temp_visited.end()) {
            return false;  // Circular dependency detected
        }

        if (visited.find(plugin_id) != visited.end()) {
            return true;  // Already processed
        }

        temp_visited.insert(plugin_id);

        auto it = dependency_graph.find(plugin_id);
        if (it != dependency_graph.end()) {
            for (const auto& dep : it->second.dependencies) {
                if (!visit(dep)) {
                    return false;
                }
            }
        }

        temp_visited.erase(plugin_id);
        visited.insert(plugin_id);
        result.push_back(plugin_id);

        return true;
    };

    // Visit all nodes
    for (const auto& [plugin_id, node] : dependency_graph) {
        if (visited.find(plugin_id) == visited.end()) {
            if (!visit(plugin_id)) {
                qCWarning(dependencyResolverLog)
                    << "Circular dependency detected during topological sort";
                return {};  // Return empty vector on circular dependency
            }
        }
    }

    return result;
}

int PluginDependencyResolver::Impl::calculate_dependency_level(
    const std::string& plugin_id,
    const std::vector<std::string>& dependencies) const {
    Q_UNUSED(plugin_id)

    if (dependencies.empty()) {
        return 0;
    }

    int max_level = 0;
    for (const auto& dep : dependencies) {
        auto it = dependency_graph.find(dep);
        if (it != dependency_graph.end()) {
            std::vector<std::string> dep_dependencies(it->second.dependencies.begin(), it->second.dependencies.end());
            int dep_level = calculate_dependency_level(dep, dep_dependencies);
            max_level = std::max(max_level, dep_level + 1);
        }
    }

    return max_level;
}

void PluginDependencyResolver::Impl::detect_circular_dependencies(PluginDependencyResolver* /* resolver */) {
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> recursion_stack;
    QStringList circular_plugins;

    for (const auto& [plugin_id, node] : dependency_graph) {
        if (visited.find(plugin_id) == visited.end()) {
            if (has_circular_dependency(plugin_id, visited, recursion_stack)) {
                circular_plugins.append(QString::fromStdString(plugin_id));
                qCWarning(dependencyResolverLog)
                    << "Circular dependency detected involving plugin:"
                    << QString::fromStdString(plugin_id);
            }
        }
    }

    if (!circular_plugins.isEmpty()) {
        // TODO: Re-enable when MOC issues are resolved
        // emit resolver->circular_dependency_detected(circular_plugins);
    }
}

bool PluginDependencyResolver::Impl::has_circular_dependency(
    const std::string& plugin_id, std::unordered_set<std::string>& visited,
    std::unordered_set<std::string>& recursion_stack) const {
    visited.insert(plugin_id);
    recursion_stack.insert(plugin_id);

    auto it = dependency_graph.find(plugin_id);
    if (it != dependency_graph.end()) {
        for (const auto& dep : it->second.dependencies) {
            if (recursion_stack.find(dep) != recursion_stack.end()) {
                return true;  // Circular dependency found
            }

            if (visited.find(dep) == visited.end() &&
                has_circular_dependency(dep, visited, recursion_stack)) {
                return true;
            }
        }
    }

    recursion_stack.erase(plugin_id);
    return false;
}

}  // namespace qtplugin
