/**
 * @file data_processor_plugin_interface.hpp
 * @brief Data processing plugin interface for specialized data manipulation
 * capabilities
 * @version 3.0.0
 */

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QMetaType>
#include <QObject>
#include <QVariant>
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>
#include "../core/plugin_interface.hpp"

namespace qtplugin {

/**
 * @brief Data processing operation types
 */
enum class DataProcessingOperation : uint32_t {
    None = 0x0000,
    Transform = 0x0001,  ///< Data transformation
    Filter = 0x0002,     ///< Data filtering
    Aggregate = 0x0004,  ///< Data aggregation
    Validate = 0x0008,   ///< Data validation
    Convert = 0x0010,    ///< Data format conversion
    Compress = 0x0020,   ///< Data compression
    Encrypt = 0x0040,    ///< Data encryption
    Parse = 0x0080,      ///< Data parsing
    Serialize = 0x0100,  ///< Data serialization
    Index = 0x0200,      ///< Data indexing
    Search = 0x0400,     ///< Data searching
    Sort = 0x0800,       ///< Data sorting
    Merge = 0x1000,      ///< Data merging
    Split = 0x2000,      ///< Data splitting
    Analyze = 0x4000     ///< Data analysis
};

using DataProcessingOperations =
    std::underlying_type_t<DataProcessingOperation>;

/**
 * @brief Data processing context for operations
 */
struct DataProcessingContext {
    QString operation_id;                      ///< Unique operation identifier
    QJsonObject parameters;                    ///< Operation parameters
    QJsonObject metadata;                      ///< Additional metadata
    std::chrono::milliseconds timeout{30000};  ///< Operation timeout
    int priority = 0;                          ///< Operation priority
    bool async_execution = false;  ///< Whether to execute asynchronously

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static DataProcessingContext from_json(const QJsonObject& json);
};

/**
 * @brief Data processing result
 */
struct DataProcessingResult {
    bool success = false;                         ///< Operation success status
    QVariant data;                                ///< Processed data
    QJsonObject metadata;                         ///< Result metadata
    QString error_message;                        ///< Error message if failed
    std::chrono::milliseconds execution_time{0};  ///< Execution time
    size_t processed_items = 0;                   ///< Number of processed items
    size_t total_items = 0;                       ///< Total number of items

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static DataProcessingResult from_json(const QJsonObject& json);
};

/**
 * @brief Data processing progress callback
 */
using DataProcessingProgressCallback =
    std::function<void(int progress, const QString& status)>;

/**
 * @brief Data processor plugin interface
 *
 * This interface extends the base plugin interface with specialized
 * data processing capabilities for various data manipulation operations.
 */
class IDataProcessorPlugin : public virtual IPlugin {
public:
    ~IDataProcessorPlugin() override = default;

    // === Data Processing Operations ===

    /**
     * @brief Get supported data processing operations
     * @return Bitfield of supported operations
     */
    virtual DataProcessingOperations supported_operations() const noexcept = 0;

    /**
     * @brief Check if operation is supported
     * @param operation Operation to check
     * @return true if operation is supported
     */
    bool supports_operation(DataProcessingOperation operation) const noexcept {
        return (supported_operations() &
                static_cast<DataProcessingOperations>(operation)) != 0;
    }

    /**
     * @brief Process data synchronously
     * @param operation Operation to perform
     * @param input_data Input data to process
     * @param context Processing context
     * @return Processing result or error
     */
    virtual qtplugin::expected<DataProcessingResult, PluginError> process_data(
        DataProcessingOperation operation, const QVariant& input_data,
        const DataProcessingContext& context = {}) = 0;

    /**
     * @brief Process data asynchronously
     * @param operation Operation to perform
     * @param input_data Input data to process
     * @param context Processing context
     * @param progress_callback Optional progress callback
     * @return Future with processing result
     */
    virtual std::future<qtplugin::expected<DataProcessingResult, PluginError>>
    process_data_async(
        DataProcessingOperation operation, const QVariant& input_data,
        const DataProcessingContext& context = {},
        DataProcessingProgressCallback progress_callback = nullptr) = 0;

    /**
     * @brief Process batch data
     * @param operation Operation to perform
     * @param input_batch Batch of input data
     * @param context Processing context
     * @return Vector of processing results
     */
    virtual qtplugin::expected<std::vector<DataProcessingResult>, PluginError>
    process_batch(DataProcessingOperation operation,
                  const std::vector<QVariant>& input_batch,
                  const DataProcessingContext& context = {}) = 0;

    // === Data Format Support ===

    /**
     * @brief Get supported input data formats
     * @return Vector of supported MIME types or format identifiers
     */
    virtual std::vector<QString> supported_input_formats() const = 0;

    /**
     * @brief Get supported output data formats
     * @return Vector of supported MIME types or format identifiers
     */
    virtual std::vector<QString> supported_output_formats() const = 0;

    /**
     * @brief Check if input format is supported
     * @param format Format to check (MIME type or identifier)
     * @return true if format is supported
     */
    virtual bool supports_input_format(const QString& format) const {
        auto formats = supported_input_formats();
        return std::find(formats.begin(), formats.end(), format) !=
               formats.end();
    }

    /**
     * @brief Check if output format is supported
     * @param format Format to check (MIME type or identifier)
     * @return true if format is supported
     */
    virtual bool supports_output_format(const QString& format) const {
        auto formats = supported_output_formats();
        return std::find(formats.begin(), formats.end(), format) !=
               formats.end();
    }

    // === Data Validation ===

    /**
     * @brief Validate input data
     * @param data Data to validate
     * @param schema Optional validation schema
     * @return Validation result or error
     */
    virtual qtplugin::expected<bool, PluginError> validate_data(
        const QVariant& data, const QJsonObject& schema = {}) = 0;

    /**
     * @brief Get data schema for operation
     * @param operation Operation to get schema for
     * @return JSON schema or nullopt if not available
     */
    virtual std::optional<QJsonObject> get_data_schema(
        DataProcessingOperation operation) const {
        Q_UNUSED(operation)
        return std::nullopt;
    }

    // === Performance and Monitoring ===

    /**
     * @brief Get processing statistics
     * @return Processing statistics as JSON object
     */
    virtual QJsonObject get_processing_statistics() const {
        return QJsonObject{};
    }

    /**
     * @brief Reset processing statistics
     */
    virtual void reset_statistics() {}

    /**
     * @brief Get estimated processing time
     * @param operation Operation to estimate
     * @param data_size Size of data to process
     * @return Estimated processing time in milliseconds
     */
    virtual std::chrono::milliseconds estimate_processing_time(
        DataProcessingOperation operation, size_t data_size) const {
        Q_UNUSED(operation)
        Q_UNUSED(data_size)
        return std::chrono::milliseconds{1000};  // Default 1 second estimate
    }

    // === Configuration ===

    /**
     * @brief Get operation-specific configuration
     * @param operation Operation to get configuration for
     * @return Configuration object or nullopt
     */
    virtual std::optional<QJsonObject> get_operation_config(
        DataProcessingOperation operation) const {
        Q_UNUSED(operation)
        return std::nullopt;
    }

    /**
     * @brief Set operation-specific configuration
     * @param operation Operation to configure
     * @param config Configuration object
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> set_operation_config(
        DataProcessingOperation operation, const QJsonObject& config) {
        Q_UNUSED(operation)
        Q_UNUSED(config)
        return make_success();
    }
};

}  // namespace qtplugin

// Register meta types for Qt's meta-object system
Q_DECLARE_METATYPE(qtplugin::DataProcessingOperation)
Q_DECLARE_METATYPE(qtplugin::DataProcessingContext)
Q_DECLARE_METATYPE(qtplugin::DataProcessingResult)

Q_DECLARE_INTERFACE(qtplugin::IDataProcessorPlugin,
                    "qtplugin.IDataProcessorPlugin/3.0")
