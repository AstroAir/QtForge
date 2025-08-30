/**
 * @file marketplace_bindings.cpp
 * @brief Marketplace system Python bindings
 * @version 3.0.0
 * @author QtForge Development Team
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>

#include <qtplugin/marketplace/plugin_marketplace.hpp>

#include "../qt_conversions.hpp"

namespace py = pybind11;
using namespace qtplugin;

namespace qtforge_python {

void bind_marketplace(py::module& m) {
    // Marketplace plugin struct
    py::class_<MarketplacePlugin>(m, "MarketplacePlugin")
        .def(py::init<>())
        .def_readwrite("plugin_id", &MarketplacePlugin::plugin_id)
        .def_readwrite("name", &MarketplacePlugin::name)
        .def_readwrite("description", &MarketplacePlugin::description)
        .def_readwrite("author", &MarketplacePlugin::author)
        .def_readwrite("version", &MarketplacePlugin::version)
        .def_readwrite("category", &MarketplacePlugin::category)
        .def_readwrite("tags", &MarketplacePlugin::tags)
        .def_readwrite("license", &MarketplacePlugin::license)
        .def_readwrite("homepage", &MarketplacePlugin::homepage)
        .def_readwrite("repository", &MarketplacePlugin::repository)
        .def_readwrite("download_url", &MarketplacePlugin::download_url)
        .def_readwrite("download_size", &MarketplacePlugin::download_size)
        .def_readwrite("checksum", &MarketplacePlugin::checksum)
        .def_readwrite("rating", &MarketplacePlugin::rating)
        .def_readwrite("review_count", &MarketplacePlugin::review_count)
        .def_readwrite("download_count", &MarketplacePlugin::download_count)
        .def_readwrite("created_date", &MarketplacePlugin::created_date)
        .def_readwrite("updated_date", &MarketplacePlugin::updated_date)
        .def_readwrite("verified", &MarketplacePlugin::verified)
        .def_readwrite("premium", &MarketplacePlugin::premium)
        .def_readwrite("price", &MarketplacePlugin::price)
        .def_readwrite("currency", &MarketplacePlugin::currency)
        .def_readwrite("metadata", &MarketplacePlugin::metadata)
        .def("to_json", &MarketplacePlugin::to_json)
        .def_static("from_json", &MarketplacePlugin::from_json)
        .def("__repr__", [](const MarketplacePlugin& plugin) {
            return "<MarketplacePlugin id='" + plugin.plugin_id.toStdString() + 
                   "' name='" + plugin.name.toStdString() + "'>";
        });

    // Plugin review struct
    py::class_<PluginReview>(m, "PluginReview")
        .def(py::init<>())
        .def_readwrite("review_id", &PluginReview::review_id)
        .def_readwrite("plugin_id", &PluginReview::plugin_id)
        .def_readwrite("user_id", &PluginReview::user_id)
        .def_readwrite("username", &PluginReview::username)
        .def_readwrite("rating", &PluginReview::rating)
        .def_readwrite("title", &PluginReview::title)
        .def_readwrite("content", &PluginReview::content)
        .def_readwrite("created_date", &PluginReview::created_date)
        .def_readwrite("verified_purchase", &PluginReview::verified_purchase)
        .def_readwrite("helpful_count", &PluginReview::helpful_count)
        .def("to_json", &PluginReview::to_json)
        .def_static("from_json", &PluginReview::from_json)
        .def("__repr__", [](const PluginReview& review) {
            return "<PluginReview id='" + review.review_id.toStdString() + 
                   "' rating=" + std::to_string(review.rating) + ">";
        });

    // Search filters struct
    py::class_<SearchFilters>(m, "SearchFilters")
        .def(py::init<>())
        .def_readwrite("query", &SearchFilters::query)
        .def_readwrite("categories", &SearchFilters::categories)
        .def_readwrite("tags", &SearchFilters::tags)
        .def_readwrite("author", &SearchFilters::author)
        .def_readwrite("license", &SearchFilters::license)
        .def_readwrite("min_rating", &SearchFilters::min_rating)
        .def_readwrite("verified_only", &SearchFilters::verified_only)
        .def_readwrite("free_only", &SearchFilters::free_only)
        .def_readwrite("sort_by", &SearchFilters::sort_by)
        .def_readwrite("ascending", &SearchFilters::ascending)
        .def_readwrite("limit", &SearchFilters::limit)
        .def_readwrite("offset", &SearchFilters::offset)
        .def("to_json", &SearchFilters::to_json)
        .def("__repr__", [](const SearchFilters& filters) {
            return "<SearchFilters query='" + filters.query.toStdString() + 
                   "' limit=" + std::to_string(filters.limit) + ">";
        });

    // Installation progress struct
    py::class_<InstallationProgress>(m, "InstallationProgress")
        .def(py::init<>())
        .def_readwrite("plugin_id", &InstallationProgress::plugin_id)
        .def_readwrite("operation", &InstallationProgress::operation)
        .def_readwrite("progress_percent", &InstallationProgress::progress_percent)
        .def_readwrite("bytes_downloaded", &InstallationProgress::bytes_downloaded)
        .def_readwrite("total_bytes", &InstallationProgress::total_bytes)
        .def_readwrite("status_message", &InstallationProgress::status_message)
        .def_readwrite("completed", &InstallationProgress::completed)
        .def_readwrite("failed", &InstallationProgress::failed)
        .def_readwrite("error_message", &InstallationProgress::error_message)
        .def("to_json", &InstallationProgress::to_json)
        .def("__repr__", [](const InstallationProgress& progress) {
            return "<InstallationProgress plugin='" + progress.plugin_id.toStdString() + 
                   "' progress=" + std::to_string(progress.progress_percent) + "%>";
        });

    // Plugin marketplace
    py::class_<PluginMarketplace, std::shared_ptr<PluginMarketplace>>(m, "PluginMarketplace")
        .def(py::init<const QString&, QObject*>(),
             py::arg("marketplace_url") = "https://plugins.qtforge.org", py::arg("parent") = nullptr)
        .def("initialize", &PluginMarketplace::initialize)
        .def("search_plugins", &PluginMarketplace::search_plugins)
        .def("get_plugin_details", &PluginMarketplace::get_plugin_details)
        .def("get_plugin_reviews", &PluginMarketplace::get_plugin_reviews)
        .def("install_plugin", &PluginMarketplace::install_plugin)
        .def("update_plugin", &PluginMarketplace::update_plugin)
        .def("uninstall_plugin", &PluginMarketplace::uninstall_plugin)
        .def("get_installed_plugins", &PluginMarketplace::get_installed_plugins)
        .def("check_for_updates", &PluginMarketplace::check_for_updates)
        .def("get_categories", &PluginMarketplace::get_categories)
        .def("get_featured_plugins", &PluginMarketplace::get_featured_plugins)
        .def("submit_review", &PluginMarketplace::submit_review)
        .def("report_plugin", &PluginMarketplace::report_plugin)
        .def("set_api_key", &PluginMarketplace::set_api_key)
        .def("is_authenticated", &PluginMarketplace::is_authenticated)
        .def("__repr__", [](const PluginMarketplace& marketplace) {
            return "<PluginMarketplace>";
        });

    // Utility functions
    m.def(
        "create_marketplace",
        [](const std::string& url) -> std::shared_ptr<PluginMarketplace> {
            return std::make_shared<PluginMarketplace>(QString::fromStdString(url));
        },
        py::arg("marketplace_url") = "https://plugins.qtforge.org",
        "Create a new PluginMarketplace instance");

    m.def(
        "create_search_filters",
        [](const std::string& query) -> SearchFilters {
            SearchFilters filters;
            filters.query = QString::fromStdString(query);
            return filters;
        },
        py::arg("query") = "",
        "Create a new SearchFilters instance");

    // Helper functions for common marketplace operations
    m.def(
        "search_free_plugins",
        [](std::shared_ptr<PluginMarketplace> marketplace, const std::string& query) {
            SearchFilters filters;
            filters.query = QString::fromStdString(query);
            filters.free_only = true;
            filters.verified_only = true;
            return marketplace->search_plugins(filters);
        },
        py::arg("marketplace"), py::arg("query"),
        "Search for free, verified plugins");

    m.def(
        "search_by_category",
        [](std::shared_ptr<PluginMarketplace> marketplace, const std::string& category) {
            SearchFilters filters;
            filters.categories = QStringList{QString::fromStdString(category)};
            filters.verified_only = true;
            return marketplace->search_plugins(filters);
        },
        py::arg("marketplace"), py::arg("category"),
        "Search plugins by category");

    m.def(
        "get_top_rated_plugins",
        [](std::shared_ptr<PluginMarketplace> marketplace, int limit = 20) {
            SearchFilters filters;
            filters.sort_by = "rating";
            filters.ascending = false;
            filters.limit = limit;
            filters.min_rating = 4.0;
            return marketplace->search_plugins(filters);
        },
        py::arg("marketplace"), py::arg("limit") = 20,
        "Get top-rated plugins");
}

}  // namespace qtforge_python
