/**
 * @file remote_security_manager.hpp
 * @brief Stub remote security manager for backward compatibility
 * @version 3.2.0
 * @note This is a minimal stub implementation. Security components have been removed from QtForge.
 *       SHA256 verification is preserved in the core PluginManager.
 */

#pragma once

#include "../security/security_manager.hpp"

namespace qtplugin {

/**
 * @brief Stub remote security manager for backward compatibility
 * @deprecated Security components have been removed from QtForge
 * @note This is a minimal stub to maintain compilation compatibility
 */
class RemoteSecurityManager : public SecurityManager {
public:
    RemoteSecurityManager() = default;
    virtual ~RemoteSecurityManager() = default;

    // Stub methods for compatibility
    virtual bool validateRemoteSource(const std::string&) const { return true; }
    virtual bool isRemoteAccessAllowed() const { return false; }
    
    // Copy/move operations
    RemoteSecurityManager(const RemoteSecurityManager&) = default;
    RemoteSecurityManager& operator=(const RemoteSecurityManager&) = default;
    RemoteSecurityManager(RemoteSecurityManager&&) = default;
    RemoteSecurityManager& operator=(RemoteSecurityManager&&) = default;
};

} // namespace qtplugin
