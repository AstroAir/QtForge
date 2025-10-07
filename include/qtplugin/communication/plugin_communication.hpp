/**
 * @file plugin_communication.hpp
 * @brief Backward compatibility header for plugin communication components
 * @version 3.0.0
 * @deprecated Use specific component headers instead
 */

#pragma once

#ifdef __GNUC__
#pragma GCC warning "plugin_communication.hpp is deprecated, use specific component headers instead"
#elif defined(_MSC_VER)
#pragma message("Warning: plugin_communication.hpp is deprecated, use specific component headers instead")
#endif

// Include all communication components for backward compatibility
#include "message_bus.hpp"
#include "message_types.hpp"
#include "request_response_system.hpp"
#include "plugin_service_contracts.hpp"
#include "typed_event_system.hpp"

#ifdef QTFORGE_HAS_NETWORK
#include "plugin_service_discovery.hpp"
#endif

namespace qtplugin {
    // All types and functions are already available through the included headers
    // This header serves as a convenience include for backward compatibility
}
