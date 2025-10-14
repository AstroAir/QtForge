/**
 * @file plugin_interface.cpp
 * @brief Implementation of complete plugin interface with service contracts
 * support
 * @version 3.1.0
 * @author QtPlugin Development Team
 */

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLoggingCategory>
#include <QUuid>
#include <future>
#include <qtplugin/interfaces/core/plugin_interface.hpp>
#include "qtplugin/communication/request_response_system.hpp"

Q_LOGGING_CATEGORY(pluginLog, "qtplugin.core")

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
    }
    json["priority"] = priority_str;

    return json;
}

}  // namespace qtplugin
