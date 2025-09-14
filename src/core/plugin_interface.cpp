/**
 * @file plugin_interface.cpp
 * @brief Implementation of core plugin interface
 * @version 3.0.0
 */

#include <QJsonArray>
#include <QJsonDocument>
#include <QUuid>
#include <qtplugin/core/plugin_interface.hpp>

namespace qtplugin {

QJsonObject PluginMetadata::to_json() const {
    QJsonObject json;
    json["name"] = QString::fromStdString(name);
    json["description"] = QString::fromStdString(description);
    json["version"] = QString::fromStdString(version.to_string());
    json["author"] = QString::fromStdString(author);
    json["license"] = QString::fromStdString(license);
    json["homepage"] = QString::fromStdString(homepage);
    json["category"] = QString::fromStdString(category);

    // Convert tags to JSON array
    QJsonArray tags_array;
    for (const auto& tag : tags) {
        tags_array.append(QString::fromStdString(tag));
    }
    json["tags"] = tags_array;

    // Convert dependencies to JSON array
    QJsonArray deps_array;
    for (const auto& dep : dependencies) {
        deps_array.append(QString::fromStdString(dep));
    }
    json["dependencies"] = deps_array;

    // Capabilities as array of strings
    QJsonArray caps_array;
    if (capabilities & static_cast<PluginCapabilities>(PluginCapability::UI)) {
        caps_array.append("UI");
    }
    if (capabilities &
        static_cast<PluginCapabilities>(PluginCapability::Service)) {
        caps_array.append("Service");
    }
    if (capabilities &
        static_cast<PluginCapabilities>(PluginCapability::Network)) {
        caps_array.append("Network");
    }
    if (capabilities &
        static_cast<PluginCapabilities>(PluginCapability::DataProcessing)) {
        caps_array.append("DataProcessing");
    }
    if (capabilities &
        static_cast<PluginCapabilities>(PluginCapability::Scripting)) {
        caps_array.append("Scripting");
    }
    if (capabilities &
        static_cast<PluginCapabilities>(PluginCapability::FileSystem)) {
        caps_array.append("FileSystem");
    }
    if (capabilities &
        static_cast<PluginCapabilities>(PluginCapability::Database)) {
        caps_array.append("Database");
    }
    if (capabilities &
        static_cast<PluginCapabilities>(PluginCapability::AsyncInit)) {
        caps_array.append("AsyncInit");
    }
    if (capabilities &
        static_cast<PluginCapabilities>(PluginCapability::HotReload)) {
        caps_array.append("HotReload");
    }
    if (capabilities &
        static_cast<PluginCapabilities>(PluginCapability::Configuration)) {
        caps_array.append("Configuration");
    }
    if (capabilities &
        static_cast<PluginCapabilities>(PluginCapability::Logging)) {
        caps_array.append("Logging");
    }
    if (capabilities &
        static_cast<PluginCapabilities>(PluginCapability::Security)) {
        caps_array.append("Security");
    }
    if (capabilities &
        static_cast<PluginCapabilities>(PluginCapability::Threading)) {
        caps_array.append("Threading");
    }
    if (capabilities &
        static_cast<PluginCapabilities>(PluginCapability::Monitoring)) {
        caps_array.append("Monitoring");
    }
    json["capabilities"] = caps_array;

    // Priority
    const char* priority_str = "Normal";
    switch (priority) {
        case PluginPriority::Lowest:
            priority_str = "Lowest";
            break;
        case PluginPriority::Low:
            priority_str = "Low";
            break;
        case PluginPriority::Normal:
            priority_str = "Normal";
            break;
        case PluginPriority::High:
            priority_str = "High";
            break;
        case PluginPriority::Highest:
            priority_str = "Highest";
            break;
        case PluginPriority::Critical:
            priority_str = "Critical";
            break;
    }
    json["priority"] = priority_str;

    // Version constraints
    if (min_host_version) {
        json["min_host_version"] =
            QString::fromStdString(min_host_version->to_string());
    }
    if (max_host_version) {
        json["max_host_version"] =
            QString::fromStdString(max_host_version->to_string());
    }

    // Custom data
    if (!custom_data.isEmpty()) {
        json["custom_data"] = custom_data;
    }

    return json;
}

qtplugin::expected<PluginMetadata, PluginError> PluginMetadata::from_json(
    const QJsonObject& json) {
    PluginMetadata metadata;

    // Required fields
    if (!json.contains("name") || !json["name"].isString()) {
        return make_error<PluginMetadata>(PluginErrorCode::ConfigurationError,
                                          "Missing or invalid 'name' field");
    }
    metadata.name = json["name"].toString().toStdString();

    if (!json.contains("version") || !json["version"].isString()) {
        return make_error<PluginMetadata>(PluginErrorCode::ConfigurationError,
                                          "Missing or invalid 'version' field");
    }
    auto version_opt = Version::parse(json["version"].toString().toStdString());
    if (!version_opt) {
        return make_error<PluginMetadata>(PluginErrorCode::ConfigurationError,
                                          "Invalid version format");
    }
    metadata.version = *version_opt;

    // Optional fields
    if (json.contains("description") && json["description"].isString()) {
        metadata.description = json["description"].toString().toStdString();
    }

    if (json.contains("author") && json["author"].isString()) {
        metadata.author = json["author"].toString().toStdString();
    }

    if (json.contains("license") && json["license"].isString()) {
        metadata.license = json["license"].toString().toStdString();
    }

    if (json.contains("homepage") && json["homepage"].isString()) {
        metadata.homepage = json["homepage"].toString().toStdString();
    }

    if (json.contains("category") && json["category"].isString()) {
        metadata.category = json["category"].toString().toStdString();
    }

    // Tags array
    if (json.contains("tags") && json["tags"].isArray()) {
        const auto tags_array = json["tags"].toArray();
        for (const auto& tag : tags_array) {
            if (tag.isString()) {
                metadata.tags.push_back(tag.toString().toStdString());
            }
        }
    }

    // Dependencies array
    if (json.contains("dependencies") && json["dependencies"].isArray()) {
        const auto deps_array = json["dependencies"].toArray();
        for (const auto& dep : deps_array) {
            if (dep.isString()) {
                metadata.dependencies.push_back(dep.toString().toStdString());
            }
        }
    }

    // Capabilities
    metadata.capabilities = 0;
    if (json.contains("capabilities") && json["capabilities"].isArray()) {
        const auto caps_array = json["capabilities"].toArray();
        for (const auto& cap : caps_array) {
            if (cap.isString()) {
                const QString cap_str = cap.toString();
                if (cap_str == "UI") {
                    metadata.capabilities |=
                        static_cast<PluginCapabilities>(PluginCapability::UI);
                } else if (cap_str == "Service") {
                    metadata.capabilities |= static_cast<PluginCapabilities>(
                        PluginCapability::Service);
                } else if (cap_str == "Network") {
                    metadata.capabilities |= static_cast<PluginCapabilities>(
                        PluginCapability::Network);
                } else if (cap_str == "DataProcessing") {
                    metadata.capabilities |= static_cast<PluginCapabilities>(
                        PluginCapability::DataProcessing);
                } else if (cap_str == "Scripting") {
                    metadata.capabilities |= static_cast<PluginCapabilities>(
                        PluginCapability::Scripting);
                } else if (cap_str == "FileSystem") {
                    metadata.capabilities |= static_cast<PluginCapabilities>(
                        PluginCapability::FileSystem);
                } else if (cap_str == "Database") {
                    metadata.capabilities |= static_cast<PluginCapabilities>(
                        PluginCapability::Database);
                } else if (cap_str == "AsyncInit") {
                    metadata.capabilities |= static_cast<PluginCapabilities>(
                        PluginCapability::AsyncInit);
                } else if (cap_str == "HotReload") {
                    metadata.capabilities |= static_cast<PluginCapabilities>(
                        PluginCapability::HotReload);
                } else if (cap_str == "Configuration") {
                    metadata.capabilities |= static_cast<PluginCapabilities>(
                        PluginCapability::Configuration);
                } else if (cap_str == "Logging") {
                    metadata.capabilities |= static_cast<PluginCapabilities>(
                        PluginCapability::Logging);
                } else if (cap_str == "Security") {
                    metadata.capabilities |= static_cast<PluginCapabilities>(
                        PluginCapability::Security);
                } else if (cap_str == "Threading") {
                    metadata.capabilities |= static_cast<PluginCapabilities>(
                        PluginCapability::Threading);
                } else if (cap_str == "Monitoring") {
                    metadata.capabilities |= static_cast<PluginCapabilities>(
                        PluginCapability::Monitoring);
                }
            }
        }
    }

    // Priority
    if (json.contains("priority") && json["priority"].isString()) {
        const QString priority_str = json["priority"].toString();
        if (priority_str == "Lowest") {
            metadata.priority = PluginPriority::Lowest;
        } else if (priority_str == "Low") {
            metadata.priority = PluginPriority::Low;
        } else if (priority_str == "Normal") {
            metadata.priority = PluginPriority::Normal;
        } else if (priority_str == "High") {
            metadata.priority = PluginPriority::High;
        } else if (priority_str == "Highest") {
            metadata.priority = PluginPriority::Highest;
        } else if (priority_str == "Critical") {
            metadata.priority = PluginPriority::Critical;
        }
    }

    // Version constraints
    if (json.contains("min_host_version") &&
        json["min_host_version"].isString()) {
        auto min_version =
            Version::parse(json["min_host_version"].toString().toStdString());
        if (min_version) {
            metadata.min_host_version = *min_version;
        }
    }

    if (json.contains("max_host_version") &&
        json["max_host_version"].isString()) {
        auto max_version =
            Version::parse(json["max_host_version"].toString().toStdString());
        if (max_version) {
            metadata.max_host_version = *max_version;
        }
    }

    // Custom data
    if (json.contains("custom_data") && json["custom_data"].isObject()) {
        metadata.custom_data = json["custom_data"].toObject();
    }

    return metadata;
}

}  // namespace qtplugin
