/**
 * @file concepts.hpp
 * @brief C++20 concepts for plugin system type validation
 * @version 3.0.0
 */

#pragma once

#include <concepts>
#include <string_view>
#include <memory>
#include <vector>

// Forward declarations
namespace qtplugin {
    class IPlugin;
}

namespace qtplugin::concepts {

/**
 * @brief Concept for basic plugin interface compliance
 */
template<typename T>
concept Plugin = requires(T t) {
    { t.name() };
    { t.description() };
    { t.author() };
    { t.id() };
    { t.initialize() };
    { t.shutdown() } noexcept;
    { t.capabilities() };
    { t.available_commands() };
};

/**
 * @brief Helper concept to check if a type is a smart pointer to a plugin
 */
template<typename T>
concept PluginPointer = requires {
    typename T::element_type;
} && (std::same_as<T, std::shared_ptr<typename T::element_type>> ||
      std::same_as<T, std::unique_ptr<typename T::element_type>> ||
      std::same_as<T, std::weak_ptr<typename T::element_type>>);

} // namespace qtplugin::concepts
