/**
 * @file composition.cpp
 * @brief Implementation of plugin composition functionality for unified
 * workflow module
 * @version 3.1.0
 */

#include "qtplugin/workflow/composition.hpp"
#include <QJsonArray>
#include <QJsonDocument>
#include <QLoggingCategory>
#include <algorithm>
#include <exception>
#include <memory>
#include <set>
#include "qtplugin/core/plugin_manager.hpp"

namespace {
Q_LOGGING_CATEGORY(workflow_composition_log, "qtplugin.workflow.composition")
}  // namespace

namespace qtplugin::workflow::composition {

// === PluginComposition Implementation ===

qtplugin::expected<void, PluginError> PluginComposition::validate() const {
    if (m_id.isEmpty()) {
        return make_error<void>(PluginErrorCode::InvalidConfiguration,
                                "Composition ID cannot be empty");
    }

    if (m_plugins.empty()) {
        return make_error<void>(PluginErrorCode::InvalidConfiguration,
                                "Composition must have at least one plugin");
    }

    // Validate primary plugin
    if (!m_primary_plugin_id.isEmpty()) {
        auto it = m_plugins.find(m_primary_plugin_id);
        if (it == m_plugins.end() || it->second != PluginRole::Primary) {
            return make_error<void>(
                PluginErrorCode::InvalidConfiguration,
                "Primary plugin not found or not marked as primary");
        }
    }

    // Validate bindings
    for (const auto& binding : m_bindings) {
        if (!m_plugins.contains(binding.source_plugin_id)) {
            return make_error<void>(PluginErrorCode::DependencyMissing,
                                    "Binding source plugin not found: " +
                                        binding.source_plugin_id.toStdString());
        }

        if (!m_plugins.contains(binding.target_plugin_id)) {
            return make_error<void>(PluginErrorCode::DependencyMissing,
                                    "Binding target plugin not found: " +
                                        binding.target_plugin_id.toStdString());
        }
    }

    return make_success();
}

QJsonObject PluginComposition::to_json() const {
    QJsonObject json;
    json["id"] = m_id;
    json["name"] = m_name;
    json["description"] = m_description;
    json["strategy"] = static_cast<int>(m_strategy);
    json["primary_plugin_id"] = m_primary_plugin_id;
    json["configuration"] = m_configuration;

    // Serialize plugins
    QJsonObject plugins_json;
    for (const auto& [plugin_id, role] : m_plugins) {
        plugins_json[plugin_id] = static_cast<int>(role);
    }
    json["plugins"] = plugins_json;

    // Serialize bindings
    QJsonArray bindings_json;
    for (const auto& binding : m_bindings) {
        QJsonObject binding_json;
        binding_json["source_plugin_id"] = binding.source_plugin_id;
        binding_json["source_method"] = binding.source_method;
        binding_json["target_plugin_id"] = binding.target_plugin_id;
        binding_json["target_method"] = binding.target_method;
        binding_json["parameter_mapping"] = binding.parameter_mapping;
        binding_json["bidirectional"] = binding.bidirectional;
        binding_json["priority"] = binding.priority;
        bindings_json.append(binding_json);
    }
    json["bindings"] = bindings_json;

    return json;
}

qtplugin::expected<PluginComposition, PluginError> PluginComposition::from_json(
    const QJsonObject& json) {
    if (!json.contains("id") || !json["id"].isString()) {
        return make_error<PluginComposition>(
            PluginErrorCode::InvalidConfiguration, "Missing composition ID");
    }

    const QString id = json["id"].toString();
    const QString name = json.value("name").toString(id);

    PluginComposition composition(id, name);
    composition.set_description(json.value("description").toString());
    composition.set_strategy(
        static_cast<CompositionStrategy>(json.value("strategy").toInt()));
    composition.set_configuration(json.value("configuration").toObject());

    const QString primary_plugin_id =
        json.value("primary_plugin_id").toString();
    if (!primary_plugin_id.isEmpty()) {
        composition.set_primary_plugin(primary_plugin_id);
    }

    // Parse plugins
    if (json.contains("plugins") && json["plugins"].isObject()) {
        QJsonObject plugins_json = json["plugins"].toObject();
        for (auto it = plugins_json.begin(); it != plugins_json.end(); ++it) {
            const QString& plugin_id = it.key();
            const PluginRole role = static_cast<PluginRole>(it.value().toInt());
            composition.add_plugin(plugin_id, role);
        }
    }

    // Parse bindings
    if (json.contains("bindings") && json["bindings"].isArray()) {
        const QJsonArray bindings_json = json["bindings"].toArray();
        for (const auto& binding_value : bindings_json) {
            QJsonObject binding_json = binding_value.toObject();

            CompositionBinding binding;
            binding.source_plugin_id =
                binding_json["source_plugin_id"].toString();
            binding.source_method = binding_json["source_method"].toString();
            binding.target_plugin_id =
                binding_json["target_plugin_id"].toString();
            binding.target_method = binding_json["target_method"].toString();
            binding.parameter_mapping =
                binding_json["parameter_mapping"].toObject();
            binding.bidirectional =
                binding_json.value("bidirectional").toBool(false);
            binding.priority = binding_json.value("priority").toInt(0);

            composition.add_binding(binding);
        }
    }

    // Validate the composition
    auto validation_result = composition.validate();
    if (!validation_result) {
        return qtplugin::unexpected<PluginError>(validation_result.error());
    }

    return composition;
}

// === CompositePlugin Implementation ===

CompositePlugin::CompositePlugin(const PluginComposition& composition,
                                 QObject* parent)
    : QObject(parent),
      m_composition(composition),
      m_configuration(composition.configuration()) {
    m_id = composition.id().toStdString();
    m_name = composition.name().toStdString();
    m_description = composition.description().toStdString();

    qCDebug(workflow_composition_log)
        << "Created composite plugin:" << QString::fromStdString(m_id);
}

CompositePlugin::~CompositePlugin() {
    if (m_state != PluginState::Unloaded) {
        shutdown();
    }
}

qtplugin::expected<void, PluginError> CompositePlugin::initialize() {
    if (m_state != PluginState::Unloaded) {
        return make_error<void>(PluginErrorCode::InvalidState,
                                "Plugin already initialized");
    }

    m_state = PluginState::Loading;

    // Load component plugins
    auto load_result = load_component_plugins();
    if (!load_result) {
        m_state = PluginState::Error;
        return load_result;
    }

    // Initialize component plugins
    for (auto& [plugin_id, plugin] : m_component_plugins) {
        auto init_result = plugin->initialize();
        if (!init_result) {
            qCWarning(workflow_composition_log)
                << "Failed to initialize component plugin:" << plugin_id;
            m_state = PluginState::Error;
            return init_result;
        }
    }

    // Setup bindings
    auto binding_result = setup_bindings();
    if (!binding_result) {
        m_state = PluginState::Error;
        return binding_result;
    }

    // Calculate combined capabilities
    m_capabilities = 0;
    for (const auto& [plugin_id, plugin] : m_component_plugins) {
        m_capabilities |= plugin->capabilities();
    }

    m_state = PluginState::Running;

    qCDebug(workflow_composition_log)
        << "Composite plugin initialized:" << QString::fromStdString(m_id);

    return make_success();
}

void CompositePlugin::shutdown() noexcept {
    if (m_state == PluginState::Unloaded) {
        return;
    }

    try {
        m_state = PluginState::Stopping;

        // Shutdown component plugins in reverse insertion order
        std::vector<QString> keys;
        keys.reserve(m_component_plugins.size());
        for (const auto& kv : m_component_plugins) {
            keys.push_back(kv.first);
        }
        for (auto it = keys.rbegin(); it != keys.rend(); ++it) {
            try {
                auto found = m_component_plugins.find(*it);
                if (found != m_component_plugins.end() && found->second) {
                    found->second->shutdown();
                }
            } catch (const std::exception& e) {
                qCWarning(workflow_composition_log)
                    << "Exception during component plugin shutdown:"
                    << e.what();
            }
        }

        m_component_plugins.clear();
        m_active_bindings.clear();

        m_state = PluginState::Unloaded;

        qCDebug(workflow_composition_log)
            << "Composite plugin shutdown:" << QString::fromStdString(m_id);
    } catch (const std::exception& e) {
        qCWarning(workflow_composition_log)
            << "Exception during composite plugin shutdown:" << e.what();
        m_state = PluginState::Error;
    } catch (...) {
        qCWarning(workflow_composition_log)
            << "Unknown exception during composite plugin shutdown";
        m_state = PluginState::Error;
    }
}

qtplugin::expected<void, PluginError> CompositePlugin::configure(
    const QJsonObject& config) {
    m_configuration = config;

    // Configure component plugins
    for (auto& [plugin_id, plugin] : m_component_plugins) {
        if (config.contains(plugin_id)) {
            auto plugin_config = config[plugin_id].toObject();
            auto config_result = plugin->configure(plugin_config);
            if (!config_result) {
                qCWarning(workflow_composition_log)
                    << "Failed to configure component plugin:" << plugin_id;
                return config_result;
            }
        }
    }

    return make_success();
}

QJsonObject CompositePlugin::get_configuration() const {
    return m_configuration;
}

PluginMetadata CompositePlugin::metadata() const {
    PluginMetadata meta;
    meta.name = m_name;
    meta.description = m_description;
    meta.version = m_version;
    meta.author = m_author;
    meta.capabilities = m_capabilities;

    // Note: PluginMetadata doesn't have a custom_data field
    // Component information can be retrieved via get_component_plugins() method

    return meta;
}

qtplugin::expected<QJsonObject, PluginError> CompositePlugin::execute_command(
    std::string_view command, const QJsonObject& params) {
    if (m_state != PluginState::Running) {
        return make_error<QJsonObject>(PluginErrorCode::InvalidState,
                                       "Plugin not running");
    }

    // Execute based on composition strategy
    switch (m_composition.strategy()) {
        case CompositionStrategy::Aggregation:
            return execute_aggregation_command(command, params);
        case CompositionStrategy::Pipeline:
            return execute_pipeline_command(command, params);
        case CompositionStrategy::Facade:
            return execute_facade_command(command, params);
        default:
            return execute_aggregation_command(command, params);
    }
}

std::vector<std::string> CompositePlugin::available_commands() const {
    const std::set<std::string> all_commands = [this]() {
        std::set<std::string> commands;
        for (const auto& [plugin_id, plugin] : m_component_plugins) {
            auto plugin_commands = plugin->available_commands();
            commands.insert(plugin_commands.begin(), plugin_commands.end());
        }
        return commands;
    }();

    return {all_commands.begin(), all_commands.end()};
}

qtplugin::expected<void, PluginError>
CompositePlugin::load_component_plugins() {
    // PluginManager dependency to be injected by the host application; not
    // available here
    auto* plugin_manager = static_cast<PluginManager*>(nullptr);
    if (!plugin_manager) {
        return make_error<void>(PluginErrorCode::SystemError,
                                "Plugin manager not available");
    }

    for (const auto& [plugin_id, role] : m_composition.plugins()) {
        auto plugin = plugin_manager->get_plugin(plugin_id.toStdString());
        if (!plugin) {
            return make_error<void>(
                PluginErrorCode::PluginNotFound,
                "Component plugin not found: " + plugin_id.toStdString());
        }

        m_component_plugins[plugin_id] = plugin;

        qCDebug(workflow_composition_log)
            << "Loaded component plugin:" << plugin_id
            << "role:" << static_cast<int>(role);
    }

    return make_success();
}

qtplugin::expected<void, PluginError> CompositePlugin::setup_bindings() {
    m_active_bindings = m_composition.bindings();

    // Sort bindings by priority
    std::ranges::sort(m_active_bindings, [](const CompositionBinding& a,
                                            const CompositionBinding& b) {
        return a.priority > b.priority;
    });

    qCDebug(workflow_composition_log) << "Setup" << m_active_bindings.size()
                                      << "bindings for composite plugin";

    return make_success();
}

qtplugin::expected<QJsonObject, PluginError>
CompositePlugin::execute_aggregation_command(std::string_view command,
                                             const QJsonObject& params) {
    QJsonObject aggregated_result;
    bool any_success = false;
    QString last_error;

    // Execute command on all component plugins
    for (const auto& [plugin_id, plugin] : m_component_plugins) {
        auto commands = plugin->available_commands();
        if (std::ranges::find(commands, std::string(command)) !=
            commands.end()) {
            auto result = plugin->execute_command(command, params);
            if (result) {
                aggregated_result[plugin_id] = result.value();
                any_success = true;
            } else {
                last_error = QString::fromStdString(result.error().message);
                qCWarning(workflow_composition_log)
                    << "Component plugin" << plugin_id
                    << "failed to execute command:"
                    << QString::fromStdString(std::string(command));
            }
        }
    }

    if (!any_success) {
        return make_error<QJsonObject>(
            PluginErrorCode::ExecutionFailed,
            "No component plugin could execute command: " +
                last_error.toStdString());
    }

    return aggregated_result;
}

qtplugin::expected<QJsonObject, PluginError>
CompositePlugin::execute_pipeline_command(std::string_view command,
                                          const QJsonObject& params) {
    QJsonObject current_data = params;

    // Get plugins in execution order (primary first, then others)
    std::vector<QString> execution_order;

    // Add primary plugin first
    if (!m_composition.primary_plugin_id().isEmpty()) {
        execution_order.push_back(m_composition.primary_plugin_id());
    }

    // Add other plugins
    for (const auto& [plugin_id, role] : m_composition.plugins()) {
        if (plugin_id != m_composition.primary_plugin_id()) {
            execution_order.push_back(plugin_id);
        }
    }

    // Execute through pipeline
    for (const QString& plugin_id : execution_order) {
        auto plugin_it = m_component_plugins.find(plugin_id);
        if (plugin_it == m_component_plugins.end()) {
            continue;
        }

        auto plugin = plugin_it->second;
        auto commands = plugin->available_commands();

        if (std::ranges::find(commands, std::string(command)) !=
            commands.end()) {
            auto result = plugin->execute_command(command, current_data);
            if (result) {
                current_data = result.value();
            } else {
                return qtplugin::unexpected<PluginError>(result.error());
            }
        }
    }

    return current_data;
}

qtplugin::expected<QJsonObject, PluginError>
CompositePlugin::execute_facade_command(std::string_view command,
                                        const QJsonObject& params) {
    // Use primary plugin if available, otherwise first available plugin
    std::shared_ptr<IPlugin> target_plugin = find_primary_plugin();

    if (!target_plugin) {
        // Find first plugin that supports the command
        for (const auto& [plugin_id, plugin] : m_component_plugins) {
            auto commands = plugin->available_commands();
            if (std::ranges::find(commands, std::string(command)) !=
                commands.end()) {
                target_plugin = plugin;
                break;
            }
        }
    }

    if (!target_plugin) {
        return make_error<QJsonObject>(
            PluginErrorCode::CommandNotFound,
            "No component plugin supports command: " + std::string(command));
    }

    return target_plugin->execute_command(command, params);
}

std::shared_ptr<IPlugin> CompositePlugin::find_primary_plugin() const {
    if (m_composition.primary_plugin_id().isEmpty()) {
        return nullptr;
    }

    auto it = m_component_plugins.find(m_composition.primary_plugin_id());
    return it != m_component_plugins.end() ? it->second : nullptr;
}

#if 0   // These methods are not declared in the header - commenting out to fix
        // build
std::vector<contracts::ServiceContract> CompositePlugin::get_service_contracts()
    const {
    std::vector<contracts::ServiceContract> all_contracts;

    for (const auto& [plugin_id, plugin] : m_component_plugins) {
        // Get service contracts from plugin
        auto contracts = plugin->get_service_contracts();
        all_contracts.insert(all_contracts.end(), contracts.begin(),
                             contracts.end());
    }

    return all_contracts;
}

QJsonObject CompositePlugin::get_health_status() const {
    QJsonObject health;
    health["status"] =
        (m_state == PluginState::Running) ? "healthy" : "unhealthy";
    health["state"] = static_cast<int>(m_state);
    health["type"] = "composite";
    health["strategy"] = static_cast<int>(m_composition.strategy());

    // Add component health status
    QJsonArray components_health;
    for (const auto& [plugin_id, plugin] : m_component_plugins) {
        QJsonObject component_health;
        component_health["plugin_id"] = plugin_id;
        component_health["state"] = static_cast<int>(plugin->state());

        component_health["health"] = plugin->get_health_status();

        components_health.append(component_health);
    }
    health["components"] = components_health;

    return health;
}

// === IPlugin service delegation to component plugins ===
qtplugin::expected<QJsonObject, PluginError> CompositePlugin::call_service(
    const QString& service_name, const QString& method_name,
    const QJsonObject& parameters, std::chrono::milliseconds timeout) {
    Q_UNUSED(method_name)
    Q_UNUSED(parameters)
    Q_UNUSED(timeout)

    // For now, composite doesn’t handle services directly; return not
    // implemented
    return make_error<QJsonObject>(
        PluginErrorCode::NotFound,
        "No component plugin provides service: " + service_name.toStdString());
}

std::future<qtplugin::expected<QJsonObject, PluginError>>
CompositePlugin::call_service_async(const QString& service_name,
                                    const QString& method_name,
                                    const QJsonObject& parameters,
                                    std::chrono::milliseconds timeout) {
    Q_UNUSED(service_name)
    Q_UNUSED(method_name)
    Q_UNUSED(parameters)
    Q_UNUSED(timeout)

    return std::async(std::launch::deferred, [] {
        return make_error<QJsonObject>(
            PluginErrorCode::NotImplemented,
            "CompositePlugin::call_service_async not implemented");
    });
}

qtplugin::expected<QJsonObject, PluginError>
CompositePlugin::handle_service_call(const QString& service_name,
                                     const QString& method_name,
                                     const QJsonObject& parameters) {
    Q_UNUSED(service_name)
    Q_UNUSED(method_name)
    Q_UNUSED(parameters)

    return make_error<QJsonObject>(
        PluginErrorCode::NotImplemented,
        "CompositePlugin::handle_service_call not implemented");
}
#endif  // End of commented out methods

qtplugin::expected<void, PluginError> CompositePlugin::handle_event(
    const QString& event_type, const QJsonObject& event_data) {
    Q_UNUSED(event_type)
    Q_UNUSED(event_data)
    return make_success();
}

std::vector<QString> CompositePlugin::get_supported_events() const {
    return {};
}

void CompositePlugin::on_component_plugin_state_changed(
    const QString& plugin_id, int new_state) {
    Q_UNUSED(plugin_id)
    Q_UNUSED(new_state)
}

// === CompositionManager Implementation ===

CompositionManager& CompositionManager::instance() {
    static CompositionManager manager;
    return manager;
}

qtplugin::expected<void, PluginError> CompositionManager::register_composition(
    const PluginComposition& composition) {
    const std::unique_lock lock(m_compositions_mutex);

    auto validation_result = composition.validate();
    if (!validation_result) {
        return validation_result;
    }

    m_compositions[composition.id()] = composition;

    qCDebug(workflow_composition_log)
        << "Registered composition:" << composition.id();

    emit composition_registered(composition.id());
    return {};
}

qtplugin::expected<void, PluginError>
CompositionManager::unregister_composition(const QString& composition_id) {
    const std::unique_lock lock(m_compositions_mutex);

    auto it = m_compositions.find(composition_id);
    if (it == m_compositions.end()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Composition not found: " + composition_id.toStdString());
    }

    // Remove associated composite plugin if it exists
    {
        const std::unique_lock plugin_lock(m_composite_plugins_mutex);
        auto plugin_it = m_composite_plugins.find(composition_id);
        if (plugin_it != m_composite_plugins.end()) {
            plugin_it->second->shutdown();
            m_composite_plugins.erase(plugin_it);
            emit composite_plugin_destroyed(composition_id);
        }
    }

    m_compositions.erase(it);

    qCDebug(workflow_composition_log)
        << "Unregistered composition:" << composition_id;

    emit composition_unregistered(composition_id);
    return {};
}

qtplugin::expected<PluginComposition, PluginError>
CompositionManager::get_composition(const QString& composition_id) const {
    const std::shared_lock lock(m_compositions_mutex);

    auto it = m_compositions.find(composition_id);
    if (it == m_compositions.end()) {
        return make_error<PluginComposition>(
            PluginErrorCode::NotFound,
            "Composition not found: " + composition_id.toStdString());
    }

    return it->second;
}

std::vector<QString> CompositionManager::list_compositions() const {
    const std::shared_lock lock(m_compositions_mutex);

    std::vector<QString> result;
    result.reserve(m_compositions.size());

    for (const auto& [id, composition] : m_compositions) {
        result.push_back(id);
    }

    return result;
}

qtplugin::expected<std::shared_ptr<CompositePlugin>, PluginError>
CompositionManager::create_composite_plugin(const QString& composition_id) {
    // Get the composition
    auto composition_result = get_composition(composition_id);
    if (!composition_result) {
        return make_error<std::shared_ptr<CompositePlugin>>(
            composition_result.error());
    }

    const std::unique_lock lock(m_composite_plugins_mutex);

    // Check if composite plugin already exists
    auto existing_it = m_composite_plugins.find(composition_id);
    if (existing_it != m_composite_plugins.end()) {
        return existing_it->second;
    }

    // Create new composite plugin
    auto composite_plugin =
        std::make_shared<CompositePlugin>(composition_result.value());

    // Initialize the composite plugin
    auto init_result = composite_plugin->initialize();
    if (!init_result) {
        return make_error<std::shared_ptr<CompositePlugin>>(
            init_result.error());
    }

    m_composite_plugins[composition_id] = composite_plugin;

    qCDebug(workflow_composition_log)
        << "Created composite plugin:" << composition_id;

    emit composite_plugin_created(composition_id);
    return composite_plugin;
}

qtplugin::expected<void, PluginError>
CompositionManager::destroy_composite_plugin(const QString& composition_id) {
    const std::unique_lock lock(m_composite_plugins_mutex);

    auto it = m_composite_plugins.find(composition_id);
    if (it == m_composite_plugins.end()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Composite plugin not found: " + composition_id.toStdString());
    }

    it->second->shutdown();
    m_composite_plugins.erase(it);

    qCDebug(workflow_composition_log)
        << "Destroyed composite plugin:" << composition_id;

    emit composite_plugin_destroyed(composition_id);
    return {};
}

std::vector<QString> CompositionManager::list_composite_plugins() const {
    const std::shared_lock lock(m_composite_plugins_mutex);

    std::vector<QString> result;
    result.reserve(m_composite_plugins.size());

    for (const auto& [id, plugin] : m_composite_plugins) {
        result.push_back(id);
    }

    return result;
}

std::shared_ptr<CompositePlugin> CompositionManager::get_composite_plugin(
    const QString& composition_id) const {
    const std::shared_lock lock(m_composite_plugins_mutex);

    auto it = m_composite_plugins.find(composition_id);
    return it != m_composite_plugins.end() ? it->second : nullptr;
}

}  // namespace qtplugin::workflow::composition
