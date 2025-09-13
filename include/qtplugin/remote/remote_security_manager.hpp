/**
 * @file remote_security_manager.hpp
 * @brief Security manager for remote plugin system
 * @version 3.0.0
 */

#pragma once

#include <QObject>
#include <QTimer>
#include <QNetworkReply>
#include <QFuture>
#include <QtConcurrent>
#include <QList>
#include <QString>
#include <QByteArray>
#include <QDateTime>
#include <chrono>
#include <memory>

Q_DECLARE_LOGGING_CATEGORY(remoteSecurityLog)

namespace QtPlugin {
namespace Remote {

/**
 * @brief Security levels for remote plugin system
 */
enum class SecurityLevel {
    Disabled,    ///< Security checks disabled
    Basic,       ///< Basic signature verification
    Standard,    ///< Standard security with trusted publishers
    Strict,      ///< Strict security with full certificate validation
    Enterprise   ///< Enterprise-grade security with additional checks
};

/**
 * @brief Information about a trusted plugin publisher
 */
struct TrustedPublisher {
    QString id;                    ///< Publisher ID
    QString name;                  ///< Publisher name
    QString publicKey;             ///< Publisher's public key
    QByteArray certificate;        ///< Publisher's certificate
    QDateTime validFrom;           ///< Certificate valid from date
    QDateTime validUntil;          ///< Certificate valid until date
    QStringList permissions;       ///< Allowed permissions
    bool isValid = true;           ///< Whether publisher is currently valid
};

/**
 * @brief Store for trusted publishers
 */
struct TrustedPublisherStore {
    QList<TrustedPublisher> publishers;  ///< List of trusted publishers
    QStringList crlUrls;                  ///< Certificate Revocation List URLs
    std::chrono::hours refreshInterval{24}; ///< How often to refresh CRLs
};

/**
 * @brief Sandbox configuration for plugins
 */
struct SandboxConfiguration {
    bool enableSandbox = true;           ///< Enable sandboxing
    QStringList allowedPaths;            ///< Paths the plugin can access
    QStringList allowedNetworkHosts;     ///< Network hosts the plugin can contact
    qint64 maxMemoryUsage = 0;           ///< Maximum memory usage (0 = unlimited)
    int maxFileDescriptors = 0;          ///< Maximum file descriptors (0 = unlimited)
    std::chrono::milliseconds maxExecutionTime{0}; ///< Maximum execution time (0 = unlimited)
    QStringList deniedSystemCalls;       ///< System calls to deny
};

/**
 * @brief Network security policy
 */
struct NetworkSecurityPolicy {
    bool allowHttps = true;              ///< Allow HTTPS connections
    bool allowHttp = false;              ///< Allow HTTP connections (less secure)
    QStringList trustedHosts;            ///< List of trusted hosts
    QStringList blockedHosts;            ///< List of blocked hosts
    int connectionTimeout = 30000;       ///< Connection timeout in milliseconds
    int maxRedirects = 5;                ///< Maximum number of redirects to follow
    bool validateCertificates = true;    ///< Validate SSL/TLS certificates
};

/**
 * @brief Security manager for remote plugin operations
 * 
 * This class provides comprehensive security management for remote plugin operations,
 * including digital signature verification, publisher trust management, and
 * sandboxing configuration.
 */
class RemoteSecurityManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent object
     */
    explicit RemoteSecurityManager(QObject* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~RemoteSecurityManager();

    // === Security Level Management ===
    
    /**
     * @brief Set the security level
     * @param level Security level to set
     */
    void setSecurityLevel(SecurityLevel level);
    
    /**
     * @brief Get current security level
     * @return Current security level
     */
    SecurityLevel securityLevel() const;

    // === Digital Signature Verification ===
    
    /**
     * @brief Verify plugin digital signature
     * @param pluginPath Path to the plugin file
     * @param signature Digital signature to verify
     * @return true if signature is valid
     */
    bool verifyPluginSignature(const QString& pluginPath, const QByteArray& signature);
    
    /**
     * @brief Verify plugin digital signature asynchronously
     * @param pluginPath Path to the plugin file
     * @param signature Digital signature to verify
     * @return Future with verification result
     */
    QFuture<bool> verifyPluginSignatureAsync(const QString& pluginPath, const QByteArray& signature);

    // === Trusted Publisher Management ===
    
    /**
     * @brief Add a trusted publisher
     * @param publisher Publisher information
     */
    void addTrustedPublisher(const TrustedPublisher& publisher);
    
    /**
     * @brief Remove a trusted publisher
     * @param publisherId Publisher ID to remove
     */
    void removeTrustedPublisher(const QString& publisherId);
    
    /**
     * @brief Get list of trusted publishers
     * @return List of trusted publishers
     */
    QList<TrustedPublisher> trustedPublishers() const;
    
    /**
     * @brief Check if a publisher is trusted
     * @param publisherId Publisher ID to check
     * @return true if publisher is trusted
     */
    bool isPublisherTrusted(const QString& publisherId) const;

    // === Sandbox Configuration ===
    
    /**
     * @brief Set sandbox configuration
     * @param config Sandbox configuration
     */
    void setSandboxConfiguration(const SandboxConfiguration& config);
    
    /**
     * @brief Get current sandbox configuration
     * @return Current sandbox configuration
     */
    SandboxConfiguration sandboxConfiguration() const;

    // === Network Security Policy ===
    
    /**
     * @brief Set network security policy
     * @param policy Network security policy
     */
    void setNetworkSecurityPolicy(const NetworkSecurityPolicy& policy);
    
    /**
     * @brief Get current network security policy
     * @return Current network security policy
     */
    NetworkSecurityPolicy networkSecurityPolicy() const;

public slots:
    /**
     * @brief Refresh trusted publishers from certificate revocation lists
     */
    void refreshTrustedPublishers();

signals:
    /**
     * @brief Emitted when security level changes
     * @param level New security level
     */
    void securityLevelChanged(SecurityLevel level);
    
    /**
     * @brief Emitted when a plugin signature is verified
     * @param pluginPath Path to the plugin
     * @param isValid Whether signature is valid
     */
    void signatureVerified(const QString& pluginPath, bool isValid);
    
    /**
     * @brief Emitted when trusted publishers list changes
     */
    void trustedPublishersChanged();
    
    /**
     * @brief Emitted when sandbox configuration changes
     * @param config New sandbox configuration
     */
    void sandboxConfigurationChanged(const SandboxConfiguration& config);
    
    /**
     * @brief Emitted when network security policy changes
     * @param policy New network security policy
     */
    void networkSecurityPolicyChanged(const NetworkSecurityPolicy& policy);
    
    /**
     * @brief Emitted when certificate revocation list is updated
     */
    void certificateRevocationListUpdated();

private slots:
    /**
     * @brief Process certificate revocation list response
     * @param reply Network reply containing CRL data
     */
    void processCrlResponse(QNetworkReply* reply);

private:
    class RemoteSecurityManagerPrivate;
    std::unique_ptr<RemoteSecurityManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(RemoteSecurityManager)
};

} // namespace Remote
} // namespace QtPlugin
