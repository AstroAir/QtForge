/**
 * @file security_bindings.cpp
 * @brief Security manager bindings for Lua
 * @version 3.2.0
 */

#include <QDebug>
#include <QLoggingCategory>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#endif

#include <qtplugin/security/security_manager.hpp>
#include <qtplugin/security/components/security_validator.hpp>
#include <qtplugin/security/components/signature_verifier.hpp>
#include <qtplugin/security/components/permission_manager.hpp>
#include <qtplugin/security/components/security_policy_engine.hpp>
#include "../qt_conversions.cpp"

Q_LOGGING_CATEGORY(securityBindingsLog, "qtforge.lua.security");

namespace qtforge_lua {

#ifdef QTFORGE_LUA_BINDINGS

/**
 * @brief Register SecurityLevel enum and related types
 */
void register_security_types_bindings(sol::state& lua) {
    // SecurityLevel enum
    lua.new_enum<qtplugin::SecurityLevel>("SecurityLevel", {
        {"None", qtplugin::SecurityLevel::None},
        {"Low", qtplugin::SecurityLevel::Low},
        {"Medium", qtplugin::SecurityLevel::Medium},
        {"High", qtplugin::SecurityLevel::High},
        {"Maximum", qtplugin::SecurityLevel::Maximum}
    });

    // TrustLevel enum
    lua.new_enum<qtplugin::TrustLevel>("TrustLevel", {
        {"Untrusted", qtplugin::TrustLevel::Untrusted},
        {"Limited", qtplugin::TrustLevel::Limited},
        {"Trusted", qtplugin::TrustLevel::Trusted},
        {"FullyTrusted", qtplugin::TrustLevel::FullyTrusted}
    });

    // ValidationResult type
    auto validation_result_type = lua.new_usertype<qtplugin::ValidationResult>("ValidationResult");
    validation_result_type["is_valid"] = &qtplugin::ValidationResult::is_valid;
    validation_result_type["trust_level"] = &qtplugin::ValidationResult::trust_level;
    validation_result_type["signature_valid"] = &qtplugin::ValidationResult::signature_valid;
    validation_result_type["certificate_valid"] = &qtplugin::ValidationResult::certificate_valid;

    // Issues (vector<string>)
    validation_result_type["issues"] = sol::property(
        [&lua](const qtplugin::ValidationResult& result) -> sol::object {
            sol::table table = lua.create_table();
            for (size_t i = 0; i < result.issues.size(); ++i) {
                table[i + 1] = result.issues[i];
            }
            return table;
        }
    );

    // Warnings (vector<string>)
    validation_result_type["warnings"] = sol::property(
        [&lua](const qtplugin::ValidationResult& result) -> sol::object {
            sol::table table = lua.create_table();
            for (size_t i = 0; i < result.warnings.size(); ++i) {
                table[i + 1] = result.warnings[i];
            }
            return table;
        }
    );

    // Metadata (QJsonObject)
    validation_result_type["metadata"] = sol::property(
        [&lua](const qtplugin::ValidationResult& result) -> sol::object {
            return qtforge_lua::qjson_to_lua(result.metadata, lua);
        }
    );

    qCDebug(securityBindingsLog) << "Security types bindings registered";
}

/**
 * @brief Register PluginValidator bindings
 */
void register_plugin_validator_bindings(sol::state& lua) {
    auto validator_type = lua.new_usertype<qtplugin::PluginValidator>("PluginValidator");

    validator_type["validate_plugin"] = [&lua](qtplugin::PluginValidator& validator,
                                               const std::string& file_path) -> sol::object {
        std::filesystem::path path(file_path);
        auto result = validator.validate_plugin(path);
        if (result) {
            return sol::make_object(lua, result.value());
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    validator_type["validate_signature"] = [&lua](qtplugin::PluginValidator& validator,
                                                  const std::string& file_path) -> sol::object {
        std::filesystem::path path(file_path);
        auto result = validator.validate_signature(path);
        if (result) {
            return sol::make_object(lua, result.value());
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    validator_type["check_permissions"] = [&lua](qtplugin::PluginValidator& validator,
                                                 const std::string& plugin_id,
                                                 const sol::table& requested_permissions) -> sol::object {
        std::vector<std::string> permissions;
        for (const auto& pair : requested_permissions) {
            if (pair.second.get_type() == sol::type::string) {
                permissions.push_back(pair.second.as<std::string>());
            }
        }

        auto result = validator.check_permissions(plugin_id, permissions);
        if (result) {
            return sol::make_object(lua, result.value());
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    validator_type["get_security_level"] = &qtplugin::PluginValidator::get_security_level;
    validator_type["set_security_level"] = &qtplugin::PluginValidator::set_security_level;

    qCDebug(securityBindingsLog) << "PluginValidator bindings registered";
}

/**
 * @brief Register TrustManager bindings
 */
void register_trust_manager_bindings(sol::state& lua) {
    auto trust_manager_type = lua.new_usertype<qtplugin::TrustManager>("TrustManager");

    trust_manager_type["get_trust_level"] = [](qtplugin::TrustManager& manager,
                                               const std::string& plugin_id) -> qtplugin::TrustLevel {
        return manager.get_trust_level(plugin_id);
    };

    trust_manager_type["set_trust_level"] = [&lua](qtplugin::TrustManager& manager,
                                                   const std::string& plugin_id,
                                                   qtplugin::TrustLevel level) -> sol::object {
        auto result = manager.set_trust_level(plugin_id, level);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    trust_manager_type["is_trusted"] = [](qtplugin::TrustManager& manager,
                                          const std::string& plugin_id) -> bool {
        return manager.is_trusted(plugin_id);
    };

    trust_manager_type["add_trusted_publisher"] = [&lua](qtplugin::TrustManager& manager,
                                                         const std::string& publisher_id) -> sol::object {
        auto result = manager.add_trusted_publisher(publisher_id);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    trust_manager_type["remove_trusted_publisher"] = [&lua](qtplugin::TrustManager& manager,
                                                            const std::string& publisher_id) -> sol::object {
        auto result = manager.remove_trusted_publisher(publisher_id);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    trust_manager_type["get_trusted_publishers"] = [&lua](qtplugin::TrustManager& manager) -> sol::object {
        auto publishers = manager.get_trusted_publishers();
        sol::table table = lua.create_table();
        for (size_t i = 0; i < publishers.size(); ++i) {
            table[i + 1] = publishers[i];
        }
        return table;
    };

    trust_manager_type["blacklist_plugin"] = [&lua](qtplugin::TrustManager& manager,
                                                    const std::string& plugin_id,
                                                    const std::string& reason) -> sol::object {
        auto result = manager.blacklist_plugin(plugin_id, reason);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    trust_manager_type["is_blacklisted"] = [](qtplugin::TrustManager& manager,
                                              const std::string& plugin_id) -> bool {
        return manager.is_blacklisted(plugin_id);
    };

    qCDebug(securityBindingsLog) << "TrustManager bindings registered";
}

/**
 * @brief Register SecurityManager bindings
 */
void register_security_manager_bindings(sol::state& lua) {
    auto security_manager_type = lua.new_usertype<qtplugin::SecurityManager>("SecurityManager");

    security_manager_type["validate_and_authorize"] = [&lua](qtplugin::SecurityManager& manager,
                                                             const std::string& plugin_id,
                                                             const std::string& file_path) -> sol::object {
        std::filesystem::path path(file_path);
        auto result = manager.validate_and_authorize(plugin_id, path);
        if (result) {
            return sol::make_object(lua, result.value());
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    security_manager_type["check_permission"] = [](qtplugin::SecurityManager& manager,
                                                   const std::string& plugin_id,
                                                   const std::string& permission) -> bool {
        return manager.check_permission(plugin_id, permission);
    };

    security_manager_type["grant_permission"] = [&lua](qtplugin::SecurityManager& manager,
                                                       const std::string& plugin_id,
                                                       const std::string& permission) -> sol::object {
        auto result = manager.grant_permission(plugin_id, permission);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    security_manager_type["revoke_permission"] = [&lua](qtplugin::SecurityManager& manager,
                                                        const std::string& plugin_id,
                                                        const std::string& permission) -> sol::object {
        auto result = manager.revoke_permission(plugin_id, permission);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    security_manager_type["get_permissions"] = [&lua](qtplugin::SecurityManager& manager,
                                                      const std::string& plugin_id) -> sol::object {
        auto permissions = manager.get_permissions(plugin_id);
        sol::table table = lua.create_table();
        for (size_t i = 0; i < permissions.size(); ++i) {
            table[i + 1] = permissions[i];
        }
        return table;
    };

    security_manager_type["create_sandbox"] = [&lua](qtplugin::SecurityManager& manager,
                                                     const std::string& plugin_id,
                                                     qtplugin::SecurityLevel level) -> sol::object {
        auto result = manager.create_sandbox(plugin_id, level);
        if (result) {
            return sol::make_object(lua, result.value());
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    security_manager_type["destroy_sandbox"] = [&lua](qtplugin::SecurityManager& manager,
                                                      const std::string& sandbox_id) -> sol::object {
        auto result = manager.destroy_sandbox(sandbox_id);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    security_manager_type["get_security_level"] = &qtplugin::SecurityManager::get_security_level;
    security_manager_type["set_security_level"] = &qtplugin::SecurityManager::set_security_level;

    qCDebug(securityBindingsLog) << "SecurityManager bindings registered";
}

/**
 * @brief Register SecurityValidator bindings
 */
void register_security_validator_bindings(sol::state& lua) {
    auto validator_type = lua.new_usertype<qtplugin::SecurityValidator>("SecurityValidator",
        sol::constructors<qtplugin::SecurityValidator>()
    );

    validator_type["validate_file_integrity"] = [&lua](qtplugin::SecurityValidator& validator, const std::string& file_path) -> sol::object {
        auto result = validator.validate_file_integrity(file_path);
        if (result) {
            return sol::make_object(lua, result.value());
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    validator_type["validate_plugin_metadata"] = [&lua](qtplugin::SecurityValidator& validator, const sol::object& metadata) -> sol::object {
        QJsonObject json_metadata;
        if (metadata.get_type() == sol::type::table) {
            QJsonValue json_value = qtforge_lua::lua_to_qjson(metadata);
            if (json_value.isObject()) {
                json_metadata = json_value.toObject();
            }
        }

        auto result = validator.validate_plugin_metadata(json_metadata);
        if (result) {
            return sol::make_object(lua, result.value());
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    validator_type["validate_plugin_dependencies"] = &qtplugin::SecurityValidator::validate_plugin_dependencies;
    validator_type["validate_plugin_permissions"] = &qtplugin::SecurityValidator::validate_plugin_permissions;

    qCDebug(securityBindingsLog) << "SecurityValidator bindings registered";
}

/**
 * @brief Register SignatureVerifier bindings
 */
void register_signature_verifier_bindings(sol::state& lua) {
    auto verifier_type = lua.new_usertype<qtplugin::SignatureVerifier>("SignatureVerifier",
        sol::constructors<qtplugin::SignatureVerifier>()
    );

    verifier_type["verify_plugin_signature"] = [&lua](qtplugin::SignatureVerifier& verifier, const std::string& plugin_path) -> sol::object {
        auto result = verifier.verify_plugin_signature(plugin_path);
        if (result) {
            return sol::make_object(lua, result.value());
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    verifier_type["add_trusted_certificate"] = &qtplugin::SignatureVerifier::add_trusted_certificate;
    verifier_type["remove_trusted_certificate"] = &qtplugin::SignatureVerifier::remove_trusted_certificate;
    verifier_type["get_trusted_certificates"] = &qtplugin::SignatureVerifier::get_trusted_certificates;
    verifier_type["clear_trusted_certificates"] = &qtplugin::SignatureVerifier::clear_trusted_certificates;

    qCDebug(securityBindingsLog) << "SignatureVerifier bindings registered";
}

/**
 * @brief Register PermissionManager bindings
 */
void register_permission_manager_bindings(sol::state& lua) {
    auto permission_manager_type = lua.new_usertype<qtplugin::PermissionManager>("PermissionManager",
        sol::constructors<qtplugin::PermissionManager>()
    );

    permission_manager_type["grant_permission"] = [&lua](qtplugin::PermissionManager& manager,
                                                          const std::string& plugin_id,
                                                          const std::string& permission) -> sol::object {
        auto result = manager.grant_permission(plugin_id, permission);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    permission_manager_type["revoke_permission"] = [&lua](qtplugin::PermissionManager& manager,
                                                           const std::string& plugin_id,
                                                           const std::string& permission) -> sol::object {
        auto result = manager.revoke_permission(plugin_id, permission);
        if (result) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    permission_manager_type["has_permission"] = &qtplugin::PermissionManager::has_permission;
    permission_manager_type["get_plugin_permissions"] = &qtplugin::PermissionManager::get_plugin_permissions;
    permission_manager_type["clear_plugin_permissions"] = &qtplugin::PermissionManager::clear_plugin_permissions;

    qCDebug(securityBindingsLog) << "PermissionManager bindings registered";
}

/**
 * @brief Register SecurityPolicyEngine bindings
 */
void register_security_policy_engine_bindings(sol::state& lua) {
    auto policy_engine_type = lua.new_usertype<qtplugin::SecurityPolicyEngine>("SecurityPolicyEngine",
        sol::constructors<qtplugin::SecurityPolicyEngine>()
    );

    policy_engine_type["evaluate_policy"] = [&lua](qtplugin::SecurityPolicyEngine& engine,
                                                    const std::string& plugin_id,
                                                    const sol::object& context) -> sol::object {
        QJsonObject json_context;
        if (context.get_type() == sol::type::table) {
            QJsonValue json_value = qtforge_lua::lua_to_qjson(context);
            if (json_value.isObject()) {
                json_context = json_value.toObject();
            }
        }

        auto result = engine.evaluate_policy(plugin_id, json_context);
        if (result) {
            return sol::make_object(lua, result.value());
        } else {
            return sol::make_object(lua, result.error());
        }
    };

    policy_engine_type["add_policy_rule"] = &qtplugin::SecurityPolicyEngine::add_policy_rule;
    policy_engine_type["remove_policy_rule"] = &qtplugin::SecurityPolicyEngine::remove_policy_rule;
    policy_engine_type["get_policy_rules"] = &qtplugin::SecurityPolicyEngine::get_policy_rules;
    policy_engine_type["clear_policy_rules"] = &qtplugin::SecurityPolicyEngine::clear_policy_rules;

    qCDebug(securityBindingsLog) << "SecurityPolicyEngine bindings registered";
}

/**
 * @brief Register all security bindings
 */
void register_security_bindings(sol::state& lua) {
    qCDebug(securityBindingsLog) << "Registering security bindings...";

    // Create qtforge.security namespace
    sol::table qtforge = lua["qtforge"];
    sol::table security = qtforge.get_or_create<sol::table>("security");

    // Register all security types
    register_security_types_bindings(lua);
    register_security_validator_bindings(lua);
    register_signature_verifier_bindings(lua);
    register_permission_manager_bindings(lua);
    register_security_policy_engine_bindings(lua);
    register_security_manager_bindings(lua);

    // Add convenience functions to security namespace
    security["level_to_string"] = [](qtplugin::SecurityLevel level) -> std::string {
        switch (level) {
            case qtplugin::SecurityLevel::None: return "None";
            case qtplugin::SecurityLevel::Low: return "Low";
            case qtplugin::SecurityLevel::Medium: return "Medium";
            case qtplugin::SecurityLevel::High: return "High";
            case qtplugin::SecurityLevel::Maximum: return "Maximum";
            default: return "Unknown";
        }
    };

    security["trust_to_string"] = [](qtplugin::TrustLevel level) -> std::string {
        switch (level) {
            case qtplugin::TrustLevel::Untrusted: return "Untrusted";
            case qtplugin::TrustLevel::Limited: return "Limited";
            case qtplugin::TrustLevel::Trusted: return "Trusted";
            case qtplugin::TrustLevel::FullyTrusted: return "FullyTrusted";
            default: return "Unknown";
        }
    };

    security["create_validation_result"] = []() {
        return qtplugin::ValidationResult{};
    };

    // Factory functions for security components
    security["create_security_validator"] = []() {
        return std::make_shared<qtplugin::SecurityValidator>();
    };

    security["create_signature_verifier"] = []() {
        return std::make_shared<qtplugin::SignatureVerifier>();
    };

    security["create_permission_manager"] = []() {
        return std::make_shared<qtplugin::PermissionManager>();
    };

    security["create_security_policy_engine"] = []() {
        return std::make_shared<qtplugin::SecurityPolicyEngine>();
    };

    qCDebug(securityBindingsLog) << "Security bindings registered successfully";
}

#else // QTFORGE_LUA_BINDINGS not defined

// Forward declare sol::state for when Lua is not available
namespace sol { class state; }

void register_security_bindings(sol::state& lua) {
    (void)lua; // Suppress unused parameter warning
    qCWarning(securityBindingsLog) << "Security bindings not available - Lua support not compiled";
}

#endif // QTFORGE_LUA_BINDINGS

} // namespace qtforge_lua
