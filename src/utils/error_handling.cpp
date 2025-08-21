/**
 * @file error_handling.cpp
 * @brief Error handling implementation
 * @version 3.0.0
 */

#include <qtplugin/utils/error_handling.hpp>
#include <sstream>

namespace qtplugin {

const char* error_code_to_string(PluginErrorCode code) noexcept {
    switch (code) {
        case PluginErrorCode::Success:
            return "Success";
        case PluginErrorCode::FileNotFound:
            return "FileNotFound";
        case PluginErrorCode::InvalidFormat:
            return "InvalidFormat";
        case PluginErrorCode::LoadFailed:
            return "LoadFailed";
        case PluginErrorCode::UnloadFailed:
            return "UnloadFailed";
        case PluginErrorCode::SymbolNotFound:
            return "SymbolNotFound";
        case PluginErrorCode::AlreadyLoaded:
            return "AlreadyLoaded";
        case PluginErrorCode::NotLoaded:
            return "NotLoaded";
        case PluginErrorCode::PluginNotFound:
            return "PluginNotFound";
        case PluginErrorCode::InitializationFailed:
            return "InitializationFailed";
        case PluginErrorCode::ConfigurationError:
            return "ConfigurationError";
        case PluginErrorCode::DependencyMissing:
            return "DependencyMissing";
        case PluginErrorCode::VersionMismatch:
            return "VersionMismatch";
        case PluginErrorCode::ExecutionFailed:
            return "ExecutionFailed";
        case PluginErrorCode::CommandNotFound:
            return "CommandNotFound";
        case PluginErrorCode::InvalidParameters:
            return "InvalidParameters";
        case PluginErrorCode::StateError:
            return "StateError";
        case PluginErrorCode::InvalidArgument:
            return "InvalidArgument";
        case PluginErrorCode::NotFound:
            return "NotFound";
        case PluginErrorCode::ResourceUnavailable:
            return "ResourceUnavailable";
        case PluginErrorCode::AlreadyExists:
            return "AlreadyExists";
        case PluginErrorCode::NotImplemented:
            return "NotImplemented";
        case PluginErrorCode::SecurityViolation:
            return "SecurityViolation";
        case PluginErrorCode::PermissionDenied:
            return "PermissionDenied";
        case PluginErrorCode::SignatureInvalid:
            return "SignatureInvalid";
        case PluginErrorCode::UntrustedSource:
            return "UntrustedSource";
        case PluginErrorCode::OutOfMemory:
            return "OutOfMemory";
        case PluginErrorCode::ResourceExhausted:
            return "ResourceExhausted";
        case PluginErrorCode::NetworkError:
            return "NetworkError";
        case PluginErrorCode::FileSystemError:
            return "FileSystemError";
        case PluginErrorCode::ThreadingError:
            return "ThreadingError";
        case PluginErrorCode::TimeoutError:
            return "TimeoutError";
        case PluginErrorCode::UnknownError:
        default:
            return "UnknownError";
    }
}

} // namespace qtplugin
