# Data Processing Interfaces

This directory contains interfaces for data processing, transformation, validation, and format conversion functionality.

## Available Interfaces

### IDataProcessorPlugin (`data_processor_plugin_interface.hpp`)
**Version**: 3.2.0  
**Interface ID**: `qtplugin.IDataProcessorPlugin/3.2`

Comprehensive data processing interface supporting:

- **Data Operations**: Transform, filter, aggregate, validate, convert, compress, encrypt, parse, serialize, index, search, sort, merge, split, and analyze
- **Format Support**: Multiple input/output data formats with MIME type detection
- **Batch Processing**: Efficient processing of data batches with progress tracking
- **Data Validation**: Schema-based validation with custom validation rules
- **Streaming Support**: Large data processing with streaming capabilities
- **Async Processing**: Asynchronous data processing with timeout management
- **Performance Metrics**: Execution time tracking and performance statistics

#### Key Features

**Data Processing Operations**:
- Transform: Data transformation and mapping
- Filter: Data filtering with custom criteria
- Aggregate: Data aggregation and summarization
- Validate: Schema-based data validation
- Convert: Format conversion between different data types
- Compress: Data compression and decompression
- Encrypt: Data encryption and decryption
- Parse: Data parsing from various formats
- Serialize: Data serialization to different formats
- Index: Data indexing for fast retrieval
- Search: Data searching with query support
- Sort: Data sorting with custom comparators
- Merge: Data merging from multiple sources
- Split: Data splitting into multiple parts
- Analyze: Data analysis and statistics

**Data Structures**:
- `DataProcessingContext`: Operation context with parameters, metadata, timeout, and priority
- `DataProcessingResult`: Processing result with success status, data, metadata, and performance metrics
- `DataProcessingOperation`: Enumeration of supported operations

**Format Support**:
- Input format detection and validation
- Output format specification and conversion
- MIME type support for web compatibility
- Custom format extensibility

## Usage Examples

### Basic Data Processing

```cpp
#include "qtplugin/interfaces/data/data_processor_plugin_interface.hpp"

class MyDataProcessor : public qtplugin::IDataProcessorPlugin {
public:
    qtplugin::expected<qtplugin::DataProcessingResult, qtplugin::PluginError>
    process_data(qtplugin::DataProcessingOperation operation,
                 const QVariant& input_data,
                 const qtplugin::DataProcessingContext& context = {}) override {
        
        qtplugin::DataProcessingResult result;
        
        switch (operation) {
            case qtplugin::DataProcessingOperation::Transform:
                result.data = transformData(input_data);
                break;
            case qtplugin::DataProcessingOperation::Validate:
                result.success = validateData(input_data, context.parameters);
                break;
            default:
                return qtplugin::make_error<qtplugin::DataProcessingResult>(
                    qtplugin::PluginErrorCode::NotSupported,
                    "Operation not supported");
        }
        
        result.success = true;
        return result;
    }
    
    std::vector<QString> supported_input_formats() const override {
        return {"application/json", "text/xml", "text/csv"};
    }
    
    std::vector<QString> supported_output_formats() const override {
        return {"application/json", "text/xml"};
    }
};
```

### Batch Processing

```cpp
auto batch_data = std::vector<QVariant>{data1, data2, data3};
qtplugin::DataProcessingContext context;
context.async_execution = true;
context.timeout = std::chrono::milliseconds{60000};

auto results = processor->process_batch(
    qtplugin::DataProcessingOperation::Transform,
    batch_data,
    context
);

if (results) {
    for (const auto& result : results.value()) {
        if (result.success) {
            // Process successful result
            processResult(result.data);
        }
    }
}
```

## Interface Hierarchy

```text
IPlugin (base interface) - v3.2.0
└── IDataProcessorPlugin (data processing) - v3.2.0
```

## Version History

### Version 3.2.0 (Current)
- **IDataProcessorPlugin**: Updated to v3.2.0 with enhanced operation support and performance metrics
- **Interface Organization**: Moved to `interfaces/data/` directory for better organization
- **Backward Compatibility**: Forwarding header maintained in original location

### Version 3.0.0
- **IDataProcessorPlugin**: Initial data processing interface with basic operations

## See Also

- [Plugin Development Guide](../../../../docs/user-guide/plugin-development.md)
- [Data Processing Best Practices](../../../../docs/developer-guide/data-processing.md)
- [Serialization Utilities](../../../../docs/api/utils/serialization.md)
- [Interface Migration Guide](../MIGRATION_PLAN.md)
- [Core Interfaces](../core/README.md)
