/**
 * @file version.hpp
 * @brief Version handling utilities for the plugin system
 * @version 3.0.0
 */

#pragma once

#include <compare>
#include <string>
#include <string_view>
#include <tuple>

#include <optional>
#include <regex>

namespace qtplugin {

/**
 * @brief Version representation using semantic versioning
 *
 * This class represents a version number following semantic versioning
 * principles (major.minor.patch) with optional pre-release and build metadata.
 */
class Version {
public:
    /**
     * @brief Default constructor - creates version 0.0.0
     */
    constexpr Version() noexcept : m_major(0), m_minor(0), m_patch(0) {}

    /**
     * @brief Constructor with major, minor, and patch versions
     */
    constexpr Version(int major, int minor, int patch) noexcept
        : m_major(major), m_minor(minor), m_patch(patch) {}

    /**
     * @brief Constructor with major, minor, patch, and pre-release
     */
    Version(int major, int minor, int patch, std::string_view prerelease)
        : m_major(major),
          m_minor(minor),
          m_patch(patch),
          m_prerelease(prerelease) {}

    /**
     * @brief Constructor with all components
     */
    Version(int major, int minor, int patch, std::string_view prerelease,
            std::string_view build)
        : m_major(major),
          m_minor(minor),
          m_patch(patch),
          m_prerelease(prerelease),
          m_build(build) {}

    // === Accessors ===

    /**
     * @brief Get major version number
     */
    constexpr int major() const noexcept { return m_major; }

    /**
     * @brief Get minor version number
     */
    constexpr int minor() const noexcept { return m_minor; }

    /**
     * @brief Get patch version number
     */
    constexpr int patch() const noexcept { return m_patch; }

    /**
     * @brief Get pre-release identifier
     */
    const std::string& prerelease() const noexcept { return m_prerelease; }

    /**
     * @brief Get build metadata
     */
    const std::string& build() const noexcept { return m_build; }

    /**
     * @brief Check if this is a pre-release version
     */
    bool is_prerelease() const noexcept { return !m_prerelease.empty(); }

    /**
     * @brief Check if this version has build metadata
     */
    bool has_build_metadata() const noexcept { return !m_build.empty(); }

    // === String Conversion ===

    /**
     * @brief Convert to string representation
     * @param include_build Whether to include build metadata
     * @return String representation of the version
     */
    std::string to_string(bool include_build = true) const {
        std::string result = std::to_string(m_major) + "." +
                             std::to_string(m_minor) + "." +
                             std::to_string(m_patch);

        if (!m_prerelease.empty()) {
            result += "-" + m_prerelease;
        }

        if (include_build && !m_build.empty()) {
            result += "+" + m_build;
        }

        return result;
    }

    /**
     * @brief Parse version from string
     * @param version_string String to parse
     * @return Parsed version or nullopt if invalid
     */
    static std::optional<Version> parse(std::string_view version_string) {
        // Regex for semantic versioning: major.minor.patch[-prerelease][+build]
        static const std::regex version_regex(
            R"(^(\d+)\.(\d+)\.(\d+)(?:-([a-zA-Z0-9\-\.]+))?(?:\+([a-zA-Z0-9\-\.]+))?$)");

        std::match_results<std::string_view::const_iterator> matches;
        if (!std::regex_match(version_string.begin(), version_string.end(),
                              matches, version_regex)) {
            return std::nullopt;
        }

        try {
            int major = std::stoi(matches[1].str());
            int minor = std::stoi(matches[2].str());
            int patch = std::stoi(matches[3].str());

            std::string prerelease = matches[4].matched ? matches[4].str() : "";
            std::string build = matches[5].matched ? matches[5].str() : "";

            return Version{major, minor, patch, prerelease, build};
        } catch (const std::exception&) {
            return std::nullopt;
        }
    }

    // === Comparison Operators (C++20 three-way comparison) ===

    /**
     * @brief Three-way comparison operator
     *
     * Compares versions according to semantic versioning rules:
     * 1. Compare major.minor.patch numerically
     * 2. Pre-release versions have lower precedence than normal versions
     * 3. Build metadata is ignored in comparisons
     */
    std::strong_ordering operator<=>(const Version& other) const noexcept {
        // Compare major.minor.patch
        if (auto cmp = std::tie(m_major, m_minor, m_patch) <=>
                       std::tie(other.m_major, other.m_minor, other.m_patch);
            cmp != 0) {
            return cmp;
        }

        // Handle pre-release comparison
        bool this_prerelease = !m_prerelease.empty();
        bool other_prerelease = !other.m_prerelease.empty();

        if (this_prerelease && !other_prerelease) {
            return std::strong_ordering::less;
        }
        if (!this_prerelease && other_prerelease) {
            return std::strong_ordering::greater;
        }
        if (this_prerelease && other_prerelease) {
            return m_prerelease <=> other.m_prerelease;
        }

        return std::strong_ordering::equal;
    }

    /**
     * @brief Equality comparison operator
     */
    bool operator==(const Version& other) const noexcept {
        return (*this <=> other) == std::strong_ordering::equal;
    }

    // === Utility Methods ===

    /**
     * @brief Check if this version is compatible with another version
     * @param other Version to check compatibility with
     * @param compatibility_mode Compatibility checking mode
     * @return true if versions are compatible
     */
    enum class CompatibilityMode {
        Exact,  ///< Exact version match required
        Major,  ///< Same major version required
        Minor,  ///< Same major.minor version required
        Patch   ///< Same major.minor.patch version required
    };

    bool is_compatible_with(
        const Version& other,
        CompatibilityMode mode = CompatibilityMode::Major) const noexcept {
        switch (mode) {
            case CompatibilityMode::Exact:
                return *this == other;
            case CompatibilityMode::Major:
                return m_major == other.m_major;
            case CompatibilityMode::Minor:
                return m_major == other.m_major && m_minor == other.m_minor;
            case CompatibilityMode::Patch:
                return m_major == other.m_major && m_minor == other.m_minor &&
                       m_patch == other.m_patch;
        }
        return false;
    }

    /**
     * @brief Create next major version
     */
    Version next_major() const { return Version{m_major + 1, 0, 0}; }

    /**
     * @brief Create next minor version
     */
    Version next_minor() const { return Version{m_major, m_minor + 1, 0}; }

    /**
     * @brief Create next patch version
     */
    Version next_patch() const {
        return Version{m_major, m_minor, m_patch + 1};
    }

    /**
     * @brief Check if this is a stable version (no pre-release)
     */
    bool is_stable() const noexcept { return m_prerelease.empty(); }

    /**
     * @brief Get core version (without pre-release and build metadata)
     */
    Version core_version() const { return Version{m_major, m_minor, m_patch}; }

private:
    int m_major;
    int m_minor;
    int m_patch;
    std::string m_prerelease;
    std::string m_build;
};

/**
 * @brief Version range for dependency specification
 */
class VersionRange {
public:
    /**
     * @brief Range type enumeration
     */
    enum class RangeType {
        Exact,         ///< Exact version match
        GreaterThan,   ///< Greater than specified version
        GreaterEqual,  ///< Greater than or equal to specified version
        LessThan,      ///< Less than specified version
        LessEqual,     ///< Less than or equal to specified version
        Compatible,    ///< Compatible with specified version (same major)
        Range          ///< Between two versions (inclusive)
    };

    /**
     * @brief Constructor for single version ranges
     */
    VersionRange(RangeType type, const Version& version)
        : m_type(type), m_min_version(version), m_max_version(version) {}

    /**
     * @brief Constructor for version ranges
     */
    VersionRange(const Version& min_version, const Version& max_version)
        : m_type(RangeType::Range),
          m_min_version(min_version),
          m_max_version(max_version) {}

    /**
     * @brief Check if a version satisfies this range
     */
    bool satisfies(const Version& version) const {
        switch (m_type) {
            case RangeType::Exact:
                return version == m_min_version;
            case RangeType::GreaterThan:
                return version > m_min_version;
            case RangeType::GreaterEqual:
                return version >= m_min_version;
            case RangeType::LessThan:
                return version < m_min_version;
            case RangeType::LessEqual:
                return version <= m_min_version;
            case RangeType::Compatible:
                return version.is_compatible_with(
                    m_min_version, Version::CompatibilityMode::Major);
            case RangeType::Range:
                return version >= m_min_version && version <= m_max_version;
        }
        return false;
    }

    /**
     * @brief Convert to string representation
     */
    std::string to_string() const {
        switch (m_type) {
            case RangeType::Exact:
                return m_min_version.to_string();
            case RangeType::GreaterThan:
                return ">" + m_min_version.to_string();
            case RangeType::GreaterEqual:
                return ">=" + m_min_version.to_string();
            case RangeType::LessThan:
                return "<" + m_min_version.to_string();
            case RangeType::LessEqual:
                return "<=" + m_min_version.to_string();
            case RangeType::Compatible:
                return "~" + m_min_version.to_string();
            case RangeType::Range:
                return m_min_version.to_string() + " - " +
                       m_max_version.to_string();
        }
        return "";
    }

    /**
     * @brief Parse version range from string
     */
    static std::optional<VersionRange> parse(std::string_view range_string);

private:
    RangeType m_type;
    Version m_min_version;
    Version m_max_version;
};

// === Convenience Functions ===

/**
 * @brief Create version from tuple
 */
constexpr Version make_version(int major, int minor, int patch) {
    return Version{major, minor, patch};
}

/**
 * @brief Create version range for exact match
 */
inline VersionRange exact_version(const Version& version) {
    return VersionRange{VersionRange::RangeType::Exact, version};
}

/**
 * @brief Create version range for minimum version
 */
inline VersionRange minimum_version(const Version& version) {
    return VersionRange{VersionRange::RangeType::GreaterEqual, version};
}

/**
 * @brief Create version range for maximum version
 */
inline VersionRange maximum_version(const Version& version) {
    return VersionRange{VersionRange::RangeType::LessEqual, version};
}

/**
 * @brief Create version range for compatible versions
 */
inline VersionRange compatible_version(const Version& version) {
    return VersionRange{VersionRange::RangeType::Compatible, version};
}

}  // namespace qtplugin

// Format support removed for C++20 compatibility

// === Hash Support ===

template <>
struct std::hash<qtplugin::Version> {
    std::size_t operator()(const qtplugin::Version& version) const noexcept {
        return std::hash<std::string>{}(
            version.to_string(false));  // Exclude build metadata
    }
};
