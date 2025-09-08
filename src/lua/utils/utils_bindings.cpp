/**
 * @file utils_bindings.cpp
 * @brief Minimal utility bindings for Lua
 * @version 3.2.0
 */

#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#endif

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <random>
#include <regex>
#include <functional>

namespace qtforge_lua {

#ifdef QTFORGE_LUA_BINDINGS

/**
 * @brief Register comprehensive utils bindings
 */
void register_utils_bindings(sol::state& lua) {
    // Get or create qtforge.utils namespace
    sol::table qtforge = lua["qtforge"];
    sol::table utils = lua.create_table();
    qtforge["utils"] = utils;

    // === String Utilities ===
    utils["trim"] = [](const std::string& str) -> std::string {
        size_t start = str.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) return "";
        size_t end = str.find_last_not_of(" \t\n\r");
        return str.substr(start, end - start + 1);
    };

    utils["split"] = [](const std::string& str, const std::string& delimiter) -> std::vector<std::string> {
        std::vector<std::string> result;
        size_t start = 0;
        size_t end = str.find(delimiter);

        while (end != std::string::npos) {
            result.push_back(str.substr(start, end - start));
            start = end + delimiter.length();
            end = str.find(delimiter, start);
        }
        result.push_back(str.substr(start));
        return result;
    };

    utils["join"] = [](const std::vector<std::string>& strings, const std::string& delimiter) -> std::string {
        if (strings.empty()) return "";
        std::string result = strings[0];
        for (size_t i = 1; i < strings.size(); ++i) {
            result += delimiter + strings[i];
        }
        return result;
    };

    utils["to_lower"] = [](const std::string& str) -> std::string {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    };

    utils["to_upper"] = [](const std::string& str) -> std::string {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    };

    utils["starts_with"] = [](const std::string& str, const std::string& prefix) -> bool {
        return str.size() >= prefix.size() && str.substr(0, prefix.size()) == prefix;
    };

    utils["ends_with"] = [](const std::string& str, const std::string& suffix) -> bool {
        return str.size() >= suffix.size() && str.substr(str.size() - suffix.size()) == suffix;
    };

    // === File Path Utilities ===
    utils["get_filename"] = [](const std::string& path) -> std::string {
        size_t pos = path.find_last_of("/\\");
        return (pos == std::string::npos) ? path : path.substr(pos + 1);
    };

    utils["get_directory"] = [](const std::string& path) -> std::string {
        size_t pos = path.find_last_of("/\\");
        return (pos == std::string::npos) ? "" : path.substr(0, pos);
    };

    utils["get_extension"] = [](const std::string& path) -> std::string {
        size_t pos = path.find_last_of('.');
        return (pos == std::string::npos) ? "" : path.substr(pos + 1);
    };

    utils["join_path"] = [](const std::string& dir, const std::string& file) -> std::string {
        if (dir.empty()) return file;
        if (file.empty()) return dir;
        char sep = (dir.find('\\') != std::string::npos) ? '\\' : '/';
        return dir + sep + file;
    };

    // === Time Utilities ===
    utils["current_timestamp"] = []() -> double {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
    };

    utils["format_timestamp"] = [](double timestamp) -> std::string {
        auto time_point = std::chrono::system_clock::from_time_t(static_cast<time_t>(timestamp / 1000));
        auto time_t = std::chrono::system_clock::to_time_t(time_point);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    };

    // === UUID Generation ===
    utils["generate_uuid"] = []() -> std::string {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);
        std::uniform_int_distribution<> dis2(8, 11);

        std::stringstream ss;
        ss << std::hex;
        for (int i = 0; i < 8; i++) ss << dis(gen);
        ss << "-";
        for (int i = 0; i < 4; i++) ss << dis(gen);
        ss << "-4";
        for (int i = 0; i < 3; i++) ss << dis(gen);
        ss << "-";
        ss << dis2(gen);
        for (int i = 0; i < 3; i++) ss << dis(gen);
        ss << "-";
        for (int i = 0; i < 12; i++) ss << dis(gen);
        return ss.str();
    };

    // === Hash Utilities ===
    utils["hash_string"] = [](const std::string& str) -> size_t {
        return std::hash<std::string>{}(str);
    };

    // === Version Utilities ===
    utils["parse_version"] = [](const std::string& version_string) -> sol::table {
        sol::state& lua_state = static_cast<sol::state&>(lua);
        sol::table result = lua_state.create_table();

        std::vector<std::string> parts;
        std::stringstream ss(version_string);
        std::string part;

        while (std::getline(ss, part, '.')) {
            parts.push_back(part);
        }

        result["major"] = parts.size() > 0 ? std::stoi(parts[0]) : 0;
        result["minor"] = parts.size() > 1 ? std::stoi(parts[1]) : 0;
        result["patch"] = parts.size() > 2 ? std::stoi(parts[2]) : 0;
        result["valid"] = !parts.empty();

        return result;
    };

    utils["compare_versions"] = [](const std::string& v1, const std::string& v2) -> int {
        auto parse = [](const std::string& v) {
            std::vector<int> parts;
            std::stringstream ss(v);
            std::string part;
            while (std::getline(ss, part, '.')) {
                parts.push_back(std::stoi(part));
            }
            while (parts.size() < 3) parts.push_back(0);
            return parts;
        };

        auto parts1 = parse(v1);
        auto parts2 = parse(v2);

        for (size_t i = 0; i < 3; ++i) {
            if (parts1[i] < parts2[i]) return -1;
            if (parts1[i] > parts2[i]) return 1;
        }
        return 0;
    };

    // === Validation Utilities ===
    utils["is_valid_email"] = [](const std::string& email) -> bool {
        std::regex email_regex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
        return std::regex_match(email, email_regex);
    };

    utils["is_valid_url"] = [](const std::string& url) -> bool {
        std::regex url_regex(R"(^https?://[^\s/$.?#].[^\s]*$)");
        return std::regex_match(url, url_regex);
    };

    // === Math Utilities ===
    utils["clamp"] = [](double value, double min_val, double max_val) -> double {
        return std::max(min_val, std::min(value, max_val));
    };

    utils["lerp"] = [](double a, double b, double t) -> double {
        return a + t * (b - a);
    };

    // Test function
    utils["test"] = []() -> std::string {
        return "QtForge Utils module working!";
    };
}

#else // QTFORGE_LUA_BINDINGS not defined

// Forward declare sol::state for when Lua is not available
namespace sol { class state; }

void register_utils_bindings(sol::state& lua) {
    (void)lua; // Suppress unused parameter warning
    // No-op when Lua bindings are not available
}

#endif // QTFORGE_LUA_BINDINGS

} // namespace qtforge_lua
