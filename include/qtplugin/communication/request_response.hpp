/**
 * @file request_response.hpp
 * @brief Backward compatibility alias for request_response_system.hpp
 * @version 3.0.0
 * @deprecated Use request_response_system.hpp instead
 */

#pragma once

#ifdef __GNUC__
#pragma GCC warning "request_response.hpp is deprecated, use request_response_system.hpp instead"
#elif defined(_MSC_VER)
#pragma message("Warning: request_response.hpp is deprecated, use request_response_system.hpp instead")
#endif

// Include the actual implementation
#include "request_response_system.hpp"

// Provide backward compatibility aliases if needed
namespace qtplugin {
    // All types and functions are already available through the included header
    // No additional aliases needed as the header names are the only difference
}
