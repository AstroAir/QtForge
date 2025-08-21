/**
 * @file plugin_composition.hpp
 * @brief Plugin composition system for creating composite plugins
 * @version 3.1.0
 * @author QtPlugin Development Team
 *
 * This file defines the composition system that allows multiple plugins
 * to be composed into larger functional units with aggregation patterns,
 * composite plugin interfaces, and coordinated lifecycle management.
 */

#pragma once

#include <QJsonObject>
#include <QObject>
#include <QString>
#include <functional>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>
#include "../communication/plugin_service_contracts.hpp"
#include "../core/enhanced_plugin_interface.hpp"
#include "../core/plugin_interface.hpp"
#include "../utils/error_handling.hpp"

namespace qtplugin::composition {

/**
 * @brief Plugin composition strategy
 */
enum class CompositionStrategy {
    Aggregation,  // Simple aggregation of plugins
    Pipeline,     // Pipeline processing through plugins
    Facade,       // Facade pattern - single interface to multiple plugins
    Decorator,    // Decorator pattern - enhance plugin functionality
    Proxy,        // Proxy pattern - control access to plugins
    Adapter,      // Adapter pattern - adapt plugin interfaces
    Bridge        // Bridge pattern - separate abstraction from implementation
};

/**
 * @brief Plugin role in composition
 */
enum class PluginRole {
    Primary,    // Primary plugin that drives the composition
    Secondary,  // Secondary plugin that supports the primary
    Auxiliary,  // Auxiliary plugin that provides additional functionality
    Decorator,  // Plugin that decorates another plugin
    Adapter,    // Plugin that adapts interfaces
    Bridge      // Plugin that bridges different implementations
};

/**
 * @brief Composition binding definition
 */
struct CompositionBinding {
    QString source_plugin_id;       // Source plugin ID
    QString source_method;          // Source method name
    QString target_plugin_id;       // Target plugin ID
    QString target_method;          // Target method name
    QJsonObject parameter_mapping;  // Parameter mapping configuration
    std::function<QJsonObject(const QJsonObject&)>
        transformer;            // Data transformer
    bool bidirectional{false};  // Whether binding is bidirectional
    int priority{0};            // Binding priority for ordering

    CompositionBinding() = default;
    CompositionBinding(const QString& src_plugin, const QString& src_method,
                       const QString& tgt_plugin, const QString& tgt_method)
        : source_plugin_id(src_plugin),
          source_method(src_method),
          target_plugin_id(tgt_plugin),
          target_method(tgt_method) {}
};

/**
 * @brief Plugin composition definition
 */
class PluginComposition {
public:
    PluginComposition(const QString& composition_id, const QString& name = "")
        : m_id(composition_id),
          m_name(name.isEmpty() ? composition_id : name) {}

    // === Composition Configuration ===

    PluginComposition& set_description(const QString& desc) {
        m_description = desc;
        return *this;
    }

    PluginComposition& set_strategy(CompositionStrategy strategy) {
        m_strategy = strategy;
        return *this;
    }

    PluginComposition& add_plugin(const QString& plugin_id,
                                  PluginRole role = PluginRole::Secondary) {
        m_plugins[plugin_id] = role;
        return *this;
    }

    PluginComposition& set_primary_plugin(const QString& plugin_id) {
        m_plugins[plugin_id] = PluginRole::Primary;
        m_primary_plugin_id = plugin_id;
        return *this;
    }

    PluginComposition& add_binding(const CompositionBinding& binding) {
        m_bindings.push_back(binding);
        return *this;
    }

    PluginComposition& set_configuration(const QJsonObject& config) {
        m_configuration = config;
        return *this;
    }

    // === Composition Access ===

    const QString& id() const noexcept { return m_id; }
    const QString& name() const noexcept { return m_name; }
    const QString& description() const noexcept { return m_description; }
    CompositionStrategy strategy() const noexcept { return m_strategy; }
    const QString& primary_plugin_id() const noexcept {
        return m_primary_plugin_id;
    }

    const std::unordered_map<QString, PluginRole>& plugins() const noexcept {
        return m_plugins;
    }
    const std::vector<CompositionBinding>& bindings() const noexcept {
        return m_bindings;
    }
    const QJsonObject& configuration() const noexcept {
        return m_configuration;
    }

    std::vector<QString> get_plugins_by_role(PluginRole role) const {
        std::vector<QString> result;
        for (const auto& [plugin_id, plugin_role] : m_plugins) {
            if (plugin_role == role) {
                result.push_back(plugin_id);
            }
        }
        return result;
    }

    // === Validation ===

    qtplugin::expected<void, PluginError> validate() const;

    // === Serialization ===

    QJsonObject to_json() const;
    static qtplugin::expected<PluginComposition, PluginError> from_json(
        const QJsonObject& json);

private:
    QString m_id;
    QString m_name;
    QString m_description;
    CompositionStrategy m_strategy{CompositionStrategy::Aggregation};
    QString m_primary_plugin_id;
    std::unordered_map<QString, PluginRole> m_plugins;
    std::vector<CompositionBinding> m_bindings;
    QJsonObject m_configuration;
};

/**
 * @brief Composite plugin interface
 *
 * A composite plugin that aggregates multiple plugins and presents
 * them as a single unified interface.
 */
class CompositePlugin : public QObject, public virtual IEnhancedPlugin {
    Q_OBJECT

public:
    explicit CompositePlugin(const PluginComposition& composition,
                             QObject* parent = nullptr);
    ~CompositePlugin() override;

    // === IPlugin Implementation ===
    std::string_view name() const noexcept override { return m_name; }
    std::string_view description() const noexcept override {
        return m_description;
    }
    qtplugin::Version version() const noexcept override { return m_version; }
    std::string_view author() const noexcept override { return m_author; }
    std::string id() const noexcept override { return m_id; }

    qtplugin::expected<void, PluginError> initialize() override;
    void shutdown() noexcept override;
    PluginState state() const noexcept override { return m_state; }
    PluginCapabilities capabilities() const noexcept override {
        return m_capabilities;
    }

    qtplugin::expected<void, PluginError> configure(
        const QJsonObject& config) override;
    PluginMetadata metadata() const override;

    qtplugin::expected<QJsonObject, PluginError> execute_command(
        std::string_view command, const QJsonObject& params = {}) override;

    std::vector<std::string> available_commands() const override;

    // === IEnhancedPlugin Implementation ===
    std::vector<contracts::ServiceContract> get_service_contracts()
        const override;

    qtplugin::expected<QJsonObject, PluginError> call_service(
        const QString& service_name, const QString& method_name,
        const QJsonObject& parameters = {},
        std::chrono::milliseconds timeout = std::chrono::milliseconds{
            30000}) override;

    std::future<qtplugin::expected<QJsonObject, PluginError>>
    call_service_async(const QString& service_name, const QString& method_name,
                       const QJsonObject& parameters = {},
                       std::chrono::milliseconds timeout =
                           std::chrono::milliseconds{30000}) override;

    qtplugin::expected<QJsonObject, PluginError> handle_service_call(
        const QString& service_name, const QString& method_name,
        const QJsonObject& parameters) override;

    QJsonObject get_health_status() const override;

    // === Composition Management ===

    const PluginComposition& composition() const noexcept {
        return m_composition;
    }

    qtplugin::expected<void, PluginError> add_plugin(const QString& plugin_id,
                                                     PluginRole role);
    qtplugin::expected<void, PluginError> remove_plugin(
        const QString& plugin_id);

    std::vector<QString> get_component_plugins() const;
    std::shared_ptr<IPlugin> get_component_plugin(
        const QString& plugin_id) const;

    qtplugin::expected<void, PluginError> bind_plugins(
        const CompositionBinding& binding);
    qtplugin::expected<void, PluginError> unbind_plugins(
        const QString& source_plugin, const QString& source_method);

    // === Event Handling ===

    qtplugin::expected<void, PluginError> handle_event(
        const QString& event_type, const QJsonObject& event_data) override;

    std::vector<QString> get_supported_events() const override;

signals:
    void component_plugin_added(const QString& plugin_id);
    void component_plugin_removed(const QString& plugin_id);
    void binding_created(const QString& source_plugin,
                         const QString& target_plugin);
    void binding_removed(const QString& source_plugin,
                         const QString& target_plugin);

private slots:
    void on_component_plugin_state_changed(const QString& plugin_id,
                                           int new_state);

private:
    PluginComposition m_composition;
    std::string m_id;
    std::string m_name;
    std::string m_description;
    qtplugin::Version m_version{1, 0, 0};
    std::string m_author{"QtPlugin Composition System"};
    PluginState m_state{PluginState::Unloaded};
    PluginCapabilities m_capabilities{0};
    QJsonObject m_configuration;

    std::unordered_map<QString, std::shared_ptr<IPlugin>> m_component_plugins;
    std::vector<CompositionBinding> m_active_bindings;

    // Helper methods
    qtplugin::expected<void, PluginError> load_component_plugins();
    qtplugin::expected<void, PluginError> setup_bindings();
    qtplugin::expected<void, PluginError> execute_binding(
        const CompositionBinding& binding, const QJsonObject& data);

    std::shared_ptr<IPlugin> find_primary_plugin() const;
    QJsonObject transform_parameters(const QJsonObject& params,
                                     const CompositionBinding& binding) const;

    qtplugin::expected<QJsonObject, PluginError> execute_aggregation_command(
        std::string_view command, const QJsonObject& params);

    qtplugin::expected<QJsonObject, PluginError> execute_pipeline_command(
        std::string_view command, const QJsonObject& params);

    qtplugin::expected<QJsonObject, PluginError> execute_facade_command(
        std::string_view command, const QJsonObject& params);
};

/**
 * @brief Plugin composition manager
 */
class CompositionManager : public QObject {
    Q_OBJECT

public:
    static CompositionManager& instance();

    // === Composition Management ===

    qtplugin::expected<void, PluginError> register_composition(
        const PluginComposition& composition);
    qtplugin::expected<void, PluginError> unregister_composition(
        const QString& composition_id);

    qtplugin::expected<PluginComposition, PluginError> get_composition(
        const QString& composition_id) const;
    std::vector<QString> list_compositions() const;

    // === Composite Plugin Creation ===

    qtplugin::expected<std::shared_ptr<CompositePlugin>, PluginError>
    create_composite_plugin(const QString& composition_id);

    qtplugin::expected<void, PluginError> destroy_composite_plugin(
        const QString& composition_id);

    std::vector<QString> list_composite_plugins() const;
    std::shared_ptr<CompositePlugin> get_composite_plugin(
        const QString& composition_id) const;

signals:
    void composition_registered(const QString& composition_id);
    void composition_unregistered(const QString& composition_id);
    void composite_plugin_created(const QString& composition_id);
    void composite_plugin_destroyed(const QString& composition_id);

private:
    CompositionManager() = default;

    mutable std::shared_mutex m_compositions_mutex;
    mutable std::shared_mutex m_composite_plugins_mutex;
    std::unordered_map<QString, PluginComposition> m_compositions;
    std::unordered_map<QString, std::shared_ptr<CompositePlugin>>
        m_composite_plugins;
};

}  // namespace qtplugin::composition

Q_DECLARE_METATYPE(qtplugin::composition::CompositionStrategy)
Q_DECLARE_METATYPE(qtplugin::composition::PluginRole)
Q_DECLARE_METATYPE(qtplugin::composition::CompositionBinding)
Q_DECLARE_METATYPE(qtplugin::composition::PluginComposition)
