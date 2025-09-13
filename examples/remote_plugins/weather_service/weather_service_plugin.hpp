/**
 * @file weather_service_plugin.hpp
 * @brief Example remote weather service plugin demonstrating best practices
 * @version 3.2.0
 */

#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrl>
#include <QGeoCoordinate>
#include <memory>

#include "../../../include/qtplugin/core/plugin_interface.hpp"
#include "../../../include/qtplugin/utils/error_handling.hpp"

/**
 * @brief Weather service remote plugin example
 * 
 * This example demonstrates:
 * - Remote plugin architecture integration
 * - Secure API communication
 * - Configuration management
 * - Error handling
 * - Threading and async operations
 * - Resource management
 * - Security best practices
 */
class WeatherServicePlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtforge.examples.WeatherServicePlugin" FILE "weather_service_plugin.json")
    Q_INTERFACES(qtplugin::IPlugin)

public:
    explicit WeatherServicePlugin(QObject* parent = nullptr);
    ~WeatherServicePlugin() override;

    // === IPlugin Interface ===
    
    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    void shutdown() override;
    qtplugin::PluginState state() const override { return m_state; }
    bool is_initialized() const override { return m_initialized; }

    qtplugin::PluginMetadata metadata() const override;
    std::string id() const override { return "qtforge.examples.weather_service"; }
    std::string name() const override { return "Weather Service Plugin"; }
    std::string version() const override { return "1.2.0"; }
    std::string description() const override { 
        return "Remote weather service plugin with secure API integration"; 
    }

    qtplugin::expected<void, qtplugin::PluginError> configure(const QJsonObject& config) override;
    QJsonObject current_configuration() const override;

    qtplugin::expected<QJsonObject, qtplugin::PluginError> execute_command(
        std::string_view command,
        const QJsonObject& params = {}
    ) override;

    std::vector<std::string> supported_commands() const override;
    bool supports_command(std::string_view command) const override;
    qtplugin::PluginCapabilities capabilities() const override;

    // === Weather Service API ===

    /**
     * @brief Weather data structure
     */
    struct WeatherData {
        QString location;
        QGeoCoordinate coordinates;
        double temperature_celsius;
        double humidity_percent;
        double pressure_hpa;
        double wind_speed_kmh;
        double wind_direction_degrees;
        QString condition;
        QString icon_code;
        QDateTime timestamp;
        QDateTime sunrise;
        QDateTime sunset;

        QJsonObject to_json() const;
        static WeatherData from_json(const QJsonObject& json);
    };

    /**
     * @brief Weather forecast entry
     */
    struct ForecastEntry {
        QDate date;
        double temp_min;
        double temp_max;
        QString condition;
        QString icon_code;
        double precipitation_chance;

        QJsonObject to_json() const;
        static ForecastEntry from_json(const QJsonObject& json);
    };

    /**
     * @brief Get current weather for location
     * @param location Location name or coordinates
     * @return Future with weather data
     */
    QFuture<qtplugin::expected<WeatherData, qtplugin::PluginError>> get_current_weather(
        const QString& location
    );

    /**
     * @brief Get weather forecast
     * @param location Location name or coordinates
     * @param days Number of forecast days (1-7)
     * @return Future with forecast data
     */
    QFuture<qtplugin::expected<std::vector<ForecastEntry>, qtplugin::PluginError>> get_forecast(
        const QString& location,
        int days = 5
    );

    /**
     * @brief Search for locations
     * @param query Location search query
     * @return Future with location suggestions
     */
    QFuture<qtplugin::expected<QStringList, qtplugin::PluginError>> search_locations(
        const QString& query
    );

signals:
    /**
     * @brief Emitted when weather data is updated
     */
    void weather_updated(const QString& location, const WeatherData& data);

    /**
     * @brief Emitted when forecast is updated  
     */
    void forecast_updated(const QString& location, const std::vector<ForecastEntry>& forecast);

    /**
     * @brief Emitted when API rate limit is approached
     */
    void api_limit_warning(int requests_remaining, const QDateTime& reset_time);

    /**
     * @brief Emitted when API error occurs
     */
    void api_error(const QString& error_message, int error_code);

private slots:
    void handle_network_reply();
    void handle_network_error(QNetworkReply::NetworkError error);
    void handle_ssl_errors(const QList<QSslError>& errors);
    void update_cached_data();
    void cleanup_expired_cache();

private:
    // === Plugin State ===
    qtplugin::PluginState m_state = qtplugin::PluginState::Unloaded;
    bool m_initialized = false;
    QJsonObject m_configuration;

    // === Network Components ===
    std::unique_ptr<QNetworkAccessManager> m_network_manager;
    QString m_api_key;
    QString m_api_base_url;
    int m_api_requests_per_hour = 1000;
    int m_current_requests = 0;
    QDateTime m_rate_limit_reset;

    // === Caching ===
    struct CacheEntry {
        WeatherData data;
        QDateTime expiry;
        bool is_valid() const { return expiry > QDateTime::currentDateTime(); }
    };
    
    struct ForecastCacheEntry {
        std::vector<ForecastEntry> forecast;
        QDateTime expiry;
        bool is_valid() const { return expiry > QDateTime::currentDateTime(); }
    };

    mutable QMutex m_cache_mutex;
    std::unordered_map<QString, CacheEntry> m_weather_cache;
    std::unordered_map<QString, ForecastCacheEntry> m_forecast_cache;
    std::chrono::minutes m_cache_duration{30};

    // === Timers ===
    std::unique_ptr<QTimer> m_cache_update_timer;
    std::unique_ptr<QTimer> m_cache_cleanup_timer;
    std::unique_ptr<QTimer> m_rate_limit_timer;

    // === Request Management ===
    struct PendingRequest {
        QString request_id;
        QString location;
        QString type; // "current", "forecast", "search"
        QDateTime timestamp;
        QFutureInterface<qtplugin::expected<QJsonObject, qtplugin::PluginError>> future;
    };

    mutable QMutex m_requests_mutex;
    std::unordered_map<QNetworkReply*, PendingRequest> m_pending_requests;

    // === Helper Methods ===

    /**
     * @brief Create secure network request
     * @param endpoint API endpoint
     * @param params Query parameters
     * @return Configured network request
     */
    QNetworkRequest create_api_request(const QString& endpoint, 
                                      const QUrlQuery& params = {}) const;

    /**
     * @brief Check and update rate limiting
     * @return true if request is allowed
     */
    bool check_rate_limit();

    /**
     * @brief Parse API response
     * @param data Response data
     * @param type Response type
     * @return Parsed data or error
     */
    qtplugin::expected<QJsonObject, qtplugin::PluginError> parse_api_response(
        const QByteArray& data,
        const QString& type
    ) const;

    /**
     * @brief Convert API weather data to WeatherData
     * @param api_data Raw API response
     * @return Structured weather data
     */
    WeatherData convert_api_weather_data(const QJsonObject& api_data) const;

    /**
     * @brief Convert API forecast data to ForecastEntry list
     * @param api_data Raw API response  
     * @return Structured forecast data
     */
    std::vector<ForecastEntry> convert_api_forecast_data(const QJsonObject& api_data) const;

    /**
     * @brief Validate configuration
     * @param config Configuration to validate
     * @return Validation result
     */
    qtplugin::expected<void, qtplugin::PluginError> validate_configuration(
        const QJsonObject& config
    ) const;

    /**
     * @brief Setup network security
     */
    void setup_network_security();

    /**
     * @brief Setup cache management
     */
    void setup_cache_management();

    /**
     * @brief Get cached weather data
     * @param location Location identifier
     * @return Cached data if valid
     */
    std::optional<WeatherData> get_cached_weather(const QString& location) const;

    /**
     * @brief Cache weather data
     * @param location Location identifier
     * @param data Weather data to cache
     */
    void cache_weather_data(const QString& location, const WeatherData& data);

    /**
     * @brief Get cached forecast data
     * @param location Location identifier
     * @return Cached forecast if valid
     */
    std::optional<std::vector<ForecastEntry>> get_cached_forecast(const QString& location) const;

    /**
     * @brief Cache forecast data
     * @param location Location identifier
     * @param forecast Forecast data to cache
     */
    void cache_forecast_data(const QString& location, const std::vector<ForecastEntry>& forecast);

    /**
     * @brief Generate request ID
     * @return Unique request identifier
     */
    QString generate_request_id() const;

    /**
     * @brief Normalize location string for caching
     * @param location Raw location string
     * @return Normalized location string
     */
    QString normalize_location(const QString& location) const;

    /**
     * @brief Log plugin activity
     * @param level Log level
     * @param message Log message
     * @param details Additional details
     */
    void log(const QString& level, const QString& message, 
            const QJsonObject& details = {}) const;

    // === Command Handlers ===
    QJsonObject handle_get_weather_command(const QJsonObject& params);
    QJsonObject handle_get_forecast_command(const QJsonObject& params);
    QJsonObject handle_search_locations_command(const QJsonObject& params);
    QJsonObject handle_get_cache_stats_command(const QJsonObject& params);
    QJsonObject handle_clear_cache_command(const QJsonObject& params);
    QJsonObject handle_get_api_stats_command(const QJsonObject& params);
};

/**
 * @brief Weather service plugin factory
 */
class WeatherServicePluginFactory {
public:
    /**
     * @brief Create weather service plugin instance
     * @return Plugin instance
     */
    static std::unique_ptr<WeatherServicePlugin> create_instance();

    /**
     * @brief Get plugin metadata
     * @return Static plugin metadata
     */
    static qtplugin::PluginMetadata get_metadata();
};

// Export plugin factory function
extern "C" {
    Q_DECL_EXPORT std::unique_ptr<qtplugin::IPlugin> create_plugin();
    Q_DECL_EXPORT qtplugin::PluginMetadata get_plugin_metadata();
}
