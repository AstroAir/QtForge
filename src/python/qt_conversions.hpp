/**
 * @file qt_conversions.hpp
 * @brief Qt type conversion utilities for Python bindings
 * @version 3.0.0
 * @author QtForge Development Team
 *
 * This file provides conversion utilities between Qt types and Python types
 * for use in pybind11 bindings.
 */

#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <string>

namespace qtforge_python {

/**
 * @brief Register Qt type conversions with pybind11
 * @note Placeholder for future Qt type conversions
 */
void register_qt_conversions(pybind11::module& m);

}  // namespace qtforge_python
