/**
 * @file signature_verifier.cpp
 * @brief Implementation of digital signature verifier
 * @version 3.0.0
 */

#include "../../../include/qtplugin/security/components/signature_verifier.hpp"
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QLoggingCategory>
#include <QProcess>
#include <algorithm>
#include "../../../include/qtplugin/security/security_manager.hpp"

Q_LOGGING_CATEGORY(signatureVerifierLog, "qtplugin.security.signature")

namespace qtplugin {

SignatureVerifier::SignatureVerifier(QObject* parent) : QObject(parent) {
    qCDebug(signatureVerifierLog) << "Signature verifier initialized";
}

SignatureVerifier::~SignatureVerifier() {
    qCDebug(signatureVerifierLog) << "Signature verifier destroyed";
}

SecurityValidationResult SignatureVerifier::validate_signature(
    const std::filesystem::path& file_path) const {
    SecurityValidationResult result;
    result.is_valid = false;
    result.validated_level = SecurityLevel::None;

    if (!m_signature_verification_enabled) {
        result.warnings.push_back("Signature verification is disabled");
        result.is_valid = true;
        result.validated_level = SecurityLevel::Basic;
        return result;
    }

    try {
        // Perform platform-specific signature verification
        bool signature_valid = verify_platform_signature(file_path);

        if (!signature_valid) {
            result.errors.push_back("Digital signature verification failed");
            // Note: Cannot emit signals from const method
            // emit
            // signature_verification_failed(QString::fromStdString(file_path.string()),
            //                                   "Invalid or missing
            //                                   signature");
            return result;
        }

        // Verify certificate chain if signature is valid
        auto cert_result = verify_certificate_chain(file_path);
        if (!cert_result.is_valid) {
            result.warnings.insert(result.warnings.end(),
                                   cert_result.errors.begin(),
                                   cert_result.errors.end());
            result.warnings.push_back(
                "Certificate chain verification failed, but signature is "
                "valid");
        }

        result.is_valid = true;
        result.validated_level = SecurityLevel::Standard;

        // Note: Cannot emit signals from const method
        // emit signature_verified(QString::fromStdString(file_path.string()),
        // true);

    } catch (const std::exception& e) {
        result.errors.push_back("Exception during signature validation: " +
                                std::string(e.what()));
        // Note: Cannot emit signals from const method
        // emit
        // signature_verification_failed(QString::fromStdString(file_path.string()),
        //                                   QString::fromStdString(e.what()));
    } catch (...) {
        result.errors.push_back(
            "Unknown exception during signature validation");
        // Note: Cannot emit signals from const method
        // emit
        // signature_verification_failed(QString::fromStdString(file_path.string()),
        //                                   "Unknown exception");
    }

    return result;
}

std::string SignatureVerifier::calculate_file_hash(
    const std::filesystem::path& file_path,
    const std::string& algorithm) const {
    try {
        QFile file(QString::fromStdString(file_path.string()));
        if (!file.open(QIODevice::ReadOnly)) {
            return {};
        }

        QCryptographicHash::Algorithm hash_algo = QCryptographicHash::Sha256;
        if (algorithm == "SHA1") {
            hash_algo = QCryptographicHash::Sha1;
        } else if (algorithm == "MD5") {
            hash_algo = QCryptographicHash::Md5;
        } else if (algorithm == "SHA512") {
            hash_algo = QCryptographicHash::Sha512;
        }

        QCryptographicHash hash(hash_algo);
        if (hash.addData(&file)) {
            return hash.result().toHex().toStdString();
        }

    } catch (...) {
        // Return empty string on error
    }

    return {};
}

bool SignatureVerifier::verify_file_hash(const std::filesystem::path& file_path,
                                         const std::string& expected_hash,
                                         const std::string& algorithm) const {
    std::string calculated_hash = calculate_file_hash(file_path, algorithm);

    if (calculated_hash.empty()) {
        return false;
    }

    // Case-insensitive comparison
    std::string expected_lower = expected_hash;
    std::string calculated_lower = calculated_hash;

    std::transform(expected_lower.begin(), expected_lower.end(),
                   expected_lower.begin(), ::tolower);
    std::transform(calculated_lower.begin(), calculated_lower.end(),
                   calculated_lower.begin(), ::tolower);

    return expected_lower == calculated_lower;
}

bool SignatureVerifier::is_signature_verification_enabled() const {
    return m_signature_verification_enabled;
}

void SignatureVerifier::set_signature_verification_enabled(bool enabled) {
    m_signature_verification_enabled = enabled;
    qCDebug(signatureVerifierLog)
        << "Signature verification" << (enabled ? "enabled" : "disabled");
}

SecurityValidationResult SignatureVerifier::verify_certificate_chain(
    const std::filesystem::path& file_path) const {
    SecurityValidationResult result;
    result.is_valid = false;
    result.validated_level = SecurityLevel::None;

    try {
        // This is a simplified implementation
        // In a real system, you would verify the entire certificate chain

        // For now, just check if the file has a valid signature
        bool has_signature = verify_platform_signature(file_path);

        if (has_signature) {
            result.is_valid = true;
            result.validated_level = SecurityLevel::Standard;
            result.warnings.push_back(
                "Certificate chain verification not fully implemented");
        } else {
            result.errors.push_back("No valid certificate found");
        }

    } catch (const std::exception& e) {
        result.errors.push_back("Exception during certificate verification: " +
                                std::string(e.what()));
    } catch (...) {
        result.errors.push_back(
            "Unknown exception during certificate verification");
    }

    return result;
}

bool SignatureVerifier::verify_platform_signature(
    const std::filesystem::path& file_path) const {
#ifdef Q_OS_WIN
    return verify_windows_authenticode(file_path);
#elif defined(Q_OS_MAC)
    return verify_macos_codesign(file_path);
#else
    return verify_linux_gpg_signature(file_path);
#endif
}

bool SignatureVerifier::verify_windows_authenticode(
    const std::filesystem::path& file_path) const {
#ifdef Q_OS_WIN
    // In a real implementation, you would use Windows API functions like
    // WinVerifyTrust to verify Authenticode signatures

    // For now, this is a simplified check
    Q_UNUSED(file_path);

    // Simulate signature verification
    // In practice, you would call WinVerifyTrust or similar
    qCDebug(signatureVerifierLog)
        << "Verifying Windows Authenticode signature (simplified)";

    // Return true for demonstration - in real implementation this would
    // perform actual Authenticode verification
    return true;
#else
    Q_UNUSED(file_path);
    return false;
#endif
}

bool SignatureVerifier::verify_macos_codesign(
    const std::filesystem::path& file_path) const {
#ifdef Q_OS_MAC
    // Use codesign utility to verify signature
    QProcess codesign_process;
    QStringList arguments;
    arguments << "--verify" << "--verbose"
              << QString::fromStdString(file_path.string());

    codesign_process.start("codesign", arguments);
    codesign_process.waitForFinished(5000);  // 5 second timeout

    int exit_code = codesign_process.exitCode();

    qCDebug(signatureVerifierLog) << "codesign exit code:" << exit_code;

    return exit_code == 0;
#else
    Q_UNUSED(file_path);
    return false;
#endif
}

bool SignatureVerifier::verify_linux_gpg_signature(
    const std::filesystem::path& file_path) const {
#ifdef Q_OS_LINUX
    // Check for GPG signature file
    std::filesystem::path sig_file = file_path;
    sig_file += ".sig";

    if (!std::filesystem::exists(sig_file)) {
        qCDebug(signatureVerifierLog)
            << "No GPG signature file found:" << sig_file.string().c_str();
        return false;
    }

    // Use gpg to verify signature
    QProcess gpg_process;
    QStringList arguments;
    arguments << "--verify" << QString::fromStdString(sig_file.string())
              << QString::fromStdString(file_path.string());

    gpg_process.start("gpg", arguments);
    gpg_process.waitForFinished(5000);  // 5 second timeout

    int exit_code = gpg_process.exitCode();

    qCDebug(signatureVerifierLog) << "gpg exit code:" << exit_code;

    return exit_code == 0;
#else
    Q_UNUSED(file_path);
    return false;
#endif
}

SecurityValidationResult SignatureVerifier::create_signature_result(
    bool is_valid, const std::vector<std::string>& errors,
    const std::vector<std::string>& warnings) const {
    SecurityValidationResult result;
    result.is_valid = is_valid;
    result.errors = errors;
    result.warnings = warnings;
    result.validated_level =
        is_valid ? SecurityLevel::Standard : SecurityLevel::None;
    return result;
}

}  // namespace qtplugin


