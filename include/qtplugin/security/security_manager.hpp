/**
 * @file security_manager.hpp
 * @brief Stub security manager for backward compatibility
 * @version 3.2.0
 * @note This is a minimal stub implementation. Security components have been removed from QtForge.
 *       SHA256 verification is preserved in the core PluginManager.
 */

#pragma once

#include <memory>
#include <string>

namespace qtplugin {

/**
 * @brief Stub security manager interface for backward compatibility
 * @deprecated Security components have been removed from QtForge
 * @note This is a minimal stub to maintain compilation compatibility
 */
class SecurityManager {
public:
    SecurityManager() = default;
    virtual ~SecurityManager() = default;

    // Stub methods for compatibility
    virtual bool isEnabled() const { return false; }
    virtual void setEnabled(bool) { /* stub */ }
    
    // Copy/move operations
    SecurityManager(const SecurityManager&) = default;
    SecurityManager& operator=(const SecurityManager&) = default;
    SecurityManager(SecurityManager&&) = default;
    SecurityManager& operator=(SecurityManager&&) = default;
};

/**
 * @brief Interface security manager alias for compatibility
 * @deprecated Use SecurityManager directly
 */
using ISecurityManager = SecurityManager;

} // namespace qtplugin
