/**
 * @file signature_verifier.hpp
 * @brief Digital signature verifier interface and implementation
 * @version 3.0.0
 */

#pragma once

#include <QObject>
#include <filesystem>
#include <string>
#include <vector>
#include "../../utils/error_handling.hpp"

namespace qtplugin {

// Forward declarations
struct SecurityValidationResult;
enum class SecurityLevel;

/**
 * @brief Interface for digital signature verification
 *
 * The signature verifier handles cryptographic operations including
 * digital signature verification, certificate validation, and hash checking.
 */
class ISignatureVerifier {
public:
    virtual ~ISignatureVerifier() = default;

    /**
     * @brief Validate digital signature of a file
     * @param file_path Path to file to validate
     * @return Validation result
     */
    virtual SecurityValidationResult validate_signature(
        const std::filesystem::path& file_path) const = 0;

    /**
     * @brief Calculate file hash
     * @param file_path Path to file
     * @param algorithm Hash algorithm to use
     * @return File hash as hex string
     */
    virtual std::string calculate_file_hash(
        const std::filesystem::path& file_path,
        const std::string& algorithm = "SHA256") const = 0;

    /**
     * @brief Verify file hash against expected value
     * @param file_path Path to file
     * @param expected_hash Expected hash value
     * @param algorithm Hash algorithm used
     * @return true if hash matches
     */
    virtual bool verify_file_hash(
        const std::filesystem::path& file_path,
        const std::string& expected_hash,
        const std::string& algorithm = "SHA256") const = 0;

    /**
     * @brief Check if signature verification is enabled
     * @return true if enabled
     */
    virtual bool is_signature_verification_enabled() const = 0;

    /**
     * @brief Enable or disable signature verification
     * @param enabled Whether to enable signature verification
     */
    virtual void set_signature_verification_enabled(bool enabled) = 0;

    /**
     * @brief Verify certificate chain
     * @param file_path Path to file with certificate
     * @return Validation result
     */
    virtual SecurityValidationResult verify_certificate_chain(
        const std::filesystem::path& file_path) const = 0;
};

/**
 * @brief Digital signature verifier implementation
 *
 * Provides cryptographic signature verification functionality including
 * platform-specific signature validation and certificate chain verification.
 */
class SignatureVerifier : public QObject, public ISignatureVerifier {
    Q_OBJECT

public:
    explicit SignatureVerifier(QObject* parent = nullptr);
    ~SignatureVerifier() override;

    // ISignatureVerifier interface
    SecurityValidationResult validate_signature(
        const std::filesystem::path& file_path) const override;
    std::string calculate_file_hash(
        const std::filesystem::path& file_path,
        const std::string& algorithm = "SHA256") const override;
    bool verify_file_hash(
        const std::filesystem::path& file_path,
        const std::string& expected_hash,
        const std::string& algorithm = "SHA256") const override;
    bool is_signature_verification_enabled() const override;
    void set_signature_verification_enabled(bool enabled) override;
    SecurityValidationResult verify_certificate_chain(
        const std::filesystem::path& file_path) const override;

signals:
    /**
     * @brief Emitted when signature verification is performed
     * @param file_path Path that was verified
     * @param is_valid Whether verification passed
     */
    void signature_verified(const QString& file_path, bool is_valid);

    /**
     * @brief Emitted when signature verification fails
     * @param file_path Path where verification failed
     * @param reason Reason for failure
     */
    void signature_verification_failed(const QString& file_path,
                                       const QString& reason);

private:
    bool m_signature_verification_enabled = false;

    // Platform-specific signature verification
    bool verify_platform_signature(
        const std::filesystem::path& file_path) const;
    bool verify_windows_authenticode(
        const std::filesystem::path& file_path) const;
    bool verify_macos_codesign(const std::filesystem::path& file_path) const;
    bool verify_linux_gpg_signature(
        const std::filesystem::path& file_path) const;

    // Helper methods
    SecurityValidationResult create_signature_result(
        bool is_valid, const std::vector<std::string>& errors = {},
        const std::vector<std::string>& warnings = {}) const;
};

}  // namespace qtplugin
