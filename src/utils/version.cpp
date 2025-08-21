/**
 * @file version.cpp
 * @brief Implementation of version handling utilities
 * @version 3.0.0
 */

#include "qtplugin/utils/version.hpp"
#include <regex>
#include <stdexcept>

namespace qtplugin {

// Version::parse is implemented inline in the header file

std::optional<VersionRange> VersionRange::parse(std::string_view range_string) {
    // Remove leading/trailing whitespace
    while (!range_string.empty() && std::isspace(range_string.front())) {
        range_string.remove_prefix(1);
    }
    while (!range_string.empty() && std::isspace(range_string.back())) {
        range_string.remove_suffix(1);
    }
    
    if (range_string.empty()) {
        return std::nullopt;
    }
    
    // Check for range operators
    if (range_string.starts_with(">=")) {
        auto version_str = range_string.substr(2);
        while (!version_str.empty() && std::isspace(version_str.front())) {
            version_str.remove_prefix(1);
        }
        auto version = Version::parse(version_str);
        if (!version) return std::nullopt;
        return VersionRange{RangeType::GreaterEqual, *version};
    }
    
    if (range_string.starts_with("<=")) {
        auto version_str = range_string.substr(2);
        while (!version_str.empty() && std::isspace(version_str.front())) {
            version_str.remove_prefix(1);
        }
        auto version = Version::parse(version_str);
        if (!version) return std::nullopt;
        return VersionRange{RangeType::LessEqual, *version};
    }
    
    if (range_string.starts_with(">")) {
        auto version_str = range_string.substr(1);
        while (!version_str.empty() && std::isspace(version_str.front())) {
            version_str.remove_prefix(1);
        }
        auto version = Version::parse(version_str);
        if (!version) return std::nullopt;
        return VersionRange{RangeType::GreaterThan, *version};
    }
    
    if (range_string.starts_with("<")) {
        auto version_str = range_string.substr(1);
        while (!version_str.empty() && std::isspace(version_str.front())) {
            version_str.remove_prefix(1);
        }
        auto version = Version::parse(version_str);
        if (!version) return std::nullopt;
        return VersionRange{RangeType::LessThan, *version};
    }
    
    if (range_string.starts_with("~")) {
        auto version_str = range_string.substr(1);
        while (!version_str.empty() && std::isspace(version_str.front())) {
            version_str.remove_prefix(1);
        }
        auto version = Version::parse(version_str);
        if (!version) return std::nullopt;
        return VersionRange{RangeType::Compatible, *version};
    }
    
    // Check for range (version1 - version2)
    auto dash_pos = range_string.find(" - ");
    if (dash_pos != std::string_view::npos) {
        auto version1_str = range_string.substr(0, dash_pos);
        auto version2_str = range_string.substr(dash_pos + 3);
        
        // Trim whitespace
        while (!version1_str.empty() && std::isspace(version1_str.back())) {
            version1_str.remove_suffix(1);
        }
        while (!version2_str.empty() && std::isspace(version2_str.front())) {
            version2_str.remove_prefix(1);
        }
        
        auto version1 = Version::parse(version1_str);
        auto version2 = Version::parse(version2_str);
        
        if (!version1 || !version2) return std::nullopt;
        return VersionRange{*version1, *version2};
    }
    
    // Default to exact version match
    auto version = Version::parse(range_string);
    if (!version) return std::nullopt;
    return VersionRange{RangeType::Exact, *version};
}

} // namespace qtplugin
