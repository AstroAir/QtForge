/**
 * @file request_response.hpp
 * @brief DEPRECATED - Backward compatibility alias for
 * request_response_system.hpp
 * @version 3.0.0
 * @deprecated This header is deprecated and will be removed in QtForge v4.0.0
 *
 * ## Migration Guide
 *
 * **Old code:**
 * ```cpp
 * #include <qtplugin/communication/request_response.hpp>
 * ```
 *
 * **New code:**
 * ```cpp
 * #include <qtplugin/communication/request_response_system.hpp>
 * ```
 *
 * No other changes are required. All types and functions remain the same.
 * Only the header filename has changed.
 *
 * **Removal Timeline:**
 * - v3.0.0: Deprecated with warnings
 * - v3.1.0: Stronger deprecation warnings
 * - v4.0.0: **REMOVED** - This header will no longer exist
 *
 * Please update your code before upgrading to v4.0.0.
 */

#pragma once

// Emit strong deprecation warnings
#ifdef __GNUC__
#pragma GCC warning \
    "request_response.hpp is DEPRECATED and will be REMOVED in v4.0.0. Use request_response_system.hpp instead."
#elif defined(_MSC_VER)
#pragma message( \
    "WARNING: request_response.hpp is DEPRECATED and will be REMOVED in v4.0.0. Use request_response_system.hpp instead.")
#endif

// Include the actual implementation
#include "request_response_system.hpp"

// Provide backward compatibility aliases if needed
namespace qtplugin {
// All types and functions are already available through the included header
// No additional aliases needed as the header names are the only difference
}
