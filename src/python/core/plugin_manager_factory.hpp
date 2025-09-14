/**
 * @file plugin_manager_factory.hpp
 * @brief Factory for creating PluginManager instances for Python bindings
 * @version 3.0.0
 * 
 * This header provides a factory function that creates PluginManager instances
 * with stub implementations of the required dependencies, avoiding the
 * incomplete type issues in the Python bindings.
 */

#pragma once

#include <memory>

namespace qtplugin {
    class PluginManager;
}

namespace qtforge_python {

/**
 * @brief Create a PluginManager instance suitable for Python bindings
 * 
 * This function creates a PluginManager with stub implementations of all
 * required dependencies, avoiding the incomplete type issues that occur
 * when trying to create a PluginManager directly in Python bindings.
 * 
 * @return std::unique_ptr<qtplugin::PluginManager> A working PluginManager instance
 */
std::unique_ptr<qtplugin::PluginManager> create_plugin_manager_for_python();

} // namespace qtforge_python
