/**
 * @file data_processor_plugin_interface.hpp
 * @brief Backward compatibility header - forwards to new location
 * @version 3.2.0
 * @deprecated Use #include "qtplugin/interfaces/data/data_processor_plugin_interface.hpp" instead
 *
 * This header provides backward compatibility for existing code.
 * The interface has been moved to qtplugin/interfaces/data/ for better organization.
 * Please update your includes to use the new location.
 */

#pragma once

// Backward compatibility warning
#ifdef _MSC_VER
    #pragma message("Warning: qtplugin/interfaces/data_processor_plugin_interface.hpp is deprecated. Use qtplugin/interfaces/data/data_processor_plugin_interface.hpp instead.")
#elif defined(__GNUC__) || defined(__clang__)
    #warning "qtplugin/interfaces/data_processor_plugin_interface.hpp is deprecated. Use qtplugin/interfaces/data/data_processor_plugin_interface.hpp instead."
#endif

// Forward to new location
#include "data/data_processor_plugin_interface.hpp"
