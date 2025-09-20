/**
 * @file dynamic_plugin_interface.hpp
 * @brief Backward compatibility header - forwards to new location
 * @version 3.2.0
 * @deprecated Use #include "qtplugin/interfaces/core/dynamic_plugin_interface.hpp" instead
 * 
 * This header provides backward compatibility for existing code.
 * The interface has been moved to qtplugin/interfaces/core/ for better organization.
 * Please update your includes to use the new location.
 */

#pragma once

// Backward compatibility warning
#ifdef _MSC_VER
    #pragma message("Warning: qtplugin/core/dynamic_plugin_interface.hpp is deprecated. Use qtplugin/interfaces/core/dynamic_plugin_interface.hpp instead.")
#elif defined(__GNUC__) || defined(__clang__)
    #warning "qtplugin/core/dynamic_plugin_interface.hpp is deprecated. Use qtplugin/interfaces/core/dynamic_plugin_interface.hpp instead."
#endif

// Forward to new location
#include "../interfaces/core/dynamic_plugin_interface.hpp"
